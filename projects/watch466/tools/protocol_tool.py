#!/usr/bin/env python3
"""
饭盒协议工具 — 校验和计算 + 指令自动解析
用法:
    python protocol_tool.py 55AA0000040000170204000103050200040000003C07040001050A01000101
    python protocol_tool.py "55 AA 00 00 04 00 00 17 02 04 00 01 03 05 02 00 04 00 00 00 3C 07 04 00 01 05 0A 01 00 01 01"
    python protocol_tool.py --file frame.txt
    echo "55 AA 00 00 02 00 00 00" | python protocol_tool.py
"""

import sys
import re
import os

# ============================================================
# 协议常量
# ============================================================

FRAME_HEADER = 0x55AA

# 命令字名称 (优先 BLE，兜底 UART)
CMD_NAMES = {
    # BLE (v1.0.7)
    0x01: "查询产品信息(BLE) / 动态属性(UART)",
    0x02: "查询动态属性(BLE) / 查询预约列表(UART)",
    0x03: "状态上报(异步) / 预约操作(UART)",
    0x04: "控制指令(BLE) / OTA升级(UART)",
    0x05: "查询预约列表(BLE)",
    0x06: "新增预约(BLE)",
    0x07: "修改预约(BLE)",
    0x08: "删除预约(BLE)",
    0x09: "获取模式信息(BLE)",
    0x0A: "修改模式信息(BLE)",
    # 0x0B 升级查询已删除 (v1.0.7)
    0x0C: "升级启动(BLE)",
    0x0D: "升级包传输(BLE)",
    0x0E: "升级结束(BLE)",
}

# 预约操作类型
RESERVATION_ACTIONS = {
    0: "删除预约",
    1: "自定义加热",
    2: "鸡腿模式",
    3: "意面模式",
    4: "预约模式",
    5: "保温模式",
}

# 错误码
ERR_NAMES = {0x00: "正常", 0x01: "执行异常"}

# DataPoint 类型
DP_TYPES = {0x01: "bool", 0x02: "value", 0x04: "enum"}

# 属性定义: dpid → (名称, 类型, 值映射)
ATTRS = {
    1:  ("总开关",   "bool",  {0: "关", 1: "开"}),
    2:  ("加热模式", "enum",  {0: "关闭", 1: "自定义加热", 2: "鸡腿模式", 3: "意面模式", 4: "预约模式", 5: "保温模式"}),
    3:  ("电量",     "enum",  {0: "耗尽(≈0%)", 1: "低(<25%)", 2: "中(25-50%)", 3: "高(50-75%)", 4: "满(>75%)"}),
    4:  ("充电状态", "enum",  {0: "未充电", 1: "充电中", 2: "已充满"}),
    5:  ("加热时长", "value", None),   # 4B 大端, 单位:分钟
    6:  ("剩余时间", "value", None),   # 4B 大端, 单位:分钟
    7:  ("加热温度", "enum",  {0: "40°C", 1: "50°C", 2: "60°C", 3: "70°C", 4: "80°C", 5: "90°C"}),
    8:  ("语言",     "enum",  {0: "中文", 1: "英文", 2: "德语", 3: "法语", 4: "西班牙语", 5: "意大利语", 6: "日语", 7: "俄语"}),
    9:  ("故障提示", "enum",  {0: "正常", 1: "高温告警"}),
    10: ("是否加热", "bool",  {0: "停止", 1: "加热"}),
    11: ("时间戳",   "value", None),   # 4B unix时间 (仅UART)
    12: ("按键通知", "enum",  None),   # MCU→加热模块按键通知 (仅UART)
    13: ("MCU版本号","value", None),   # v1.0.7 新增, 4B 固件版本号
}

# ============================================================
# 工具函数
# ============================================================

def parse_hex(hex_str: str) -> bytes:
    """清洗并解析 hex 字符串 → bytes"""
    s = hex_str.strip()
    s = s.replace(" ", "").replace("\n", "").replace("\r", "").replace("\t", "")
    s = s.replace("0x", "").replace("0X", "").replace(",", "")
    if len(s) % 2 != 0:
        raise ValueError(f"hex 长度必须为偶数，当前 {len(s)} 字符")
    return bytes.fromhex(s)


def calc_checksum(data: bytes) -> int:
    """计算校验和: 逐字节累加 % 256"""
    return sum(data) % 256


def be_to_int(b: bytes) -> int:
    """大端字节序 → 整数"""
    return int.from_bytes(b, 'big')


def fmt_hex(data: bytes, sep=" ") -> str:
    return sep.join(f"{b:02X}" for b in data)


# ============================================================
# 帧解析
# ============================================================

def parse_frame(data: bytes, verbose=True):
    """解析完整帧"""
    if len(data) < 9:
        print("❌ 数据太短，至少需要 9 字节 (帧头+版本+msg+cmd+err+dlen+校验和)")
        return

    # 帧头
    header = be_to_int(data[0:2])
    if header != FRAME_HEADER:
        print(f"❌ 帧头错误: 期望 0x55AA, 实际 0x{header:04X}")
        return

    version = data[2]
    msg_flag = data[3]
    cmd = data[4]
    err_flag = data[5]
    data_len = be_to_int(data[6:8])

    expected_total = 9 + data_len
    if len(data) < expected_total:
        print(f"❌ 数据不完整: 期望 {expected_total} 字节 (头9+数据{data_len}), 实际 {len(data)}")
        return

    payload = data[8:8 + data_len] if data_len > 0 else b""
    received_chk = data[8 + data_len] if len(data) > 8 + data_len else None
    calc_chk = calc_checksum(data[:8 + data_len])

    cmd_name = CMD_NAMES.get(cmd, f"未知(0x{cmd:02X})")
    err_name = ERR_NAMES.get(err_flag, f"未知(0x{err_flag:02X})")

    # ── 输出 ──
    print("╔════════════════════════════════════════╗")
    print("║        饭盒协议帧解析 v1.0.7          ║")
    print("╠════════════════════════════════════════╣")
    print(f"║ 帧长度 : {expected_total} 字节")
    print(f"║ 帧头    : 55 AA                        ✅")
    print(f"║ 版本    : {version:02X}")
    print(f"║ 消息标志: {msg_flag:02X}")
    print(f"║ 命令字  : {cmd:02X} → {cmd_name}")
    print(f"║ 错误标志: {err_flag:02X} → {err_name}")
    print(f"║ 数据长度: {data_len} (0x{data_len:04X}, BE)")
    print(f"╠════════════════════════════════════════╣")

    # 校验和
    if received_chk is not None:
        status = "✅ 通过" if calc_chk == received_chk else "❌ 不匹配"
        print(f"║ 校验和  : 计算=0x{calc_chk:02X}, 收到=0x{received_chk:02X} → {status}")
        if calc_chk != received_chk:
            print(f"║          (全帧累加和={sum(data[:8 + data_len])}, %256={calc_chk})")

    # 数据区
    if data_len == 0:
        print(f"║ 数据区  : 无")
    else:
        print(f"║ 数据区  : {data_len} 字节")
        if data_len <= 64:
            print(f"║ 原始hex : {fmt_hex(payload)}")

        # ── 按优先级尝试多种数据格式 ──

        # 1) 预约信息结构 — UART 0x03 (42B) 或 BLE 0x06/0x07 (41B)
        if cmd == 0x03 and data_len == 1:
            # UART 0x03 MCU 应答: 仅包含分配的预约 ID
            print(f"╠════════════════════════════════════════╣")
            print(f"║  预约操作应答 (UART 0x03):")
            print(f"║  分配预约ID: {payload[0]}" + (" (失败)" if payload[0] == 0 else ""))
        elif cmd in (0x03, 0x06, 0x07) and data_len in (41, 42):
            parse_reservation(payload, is_uart=(data_len == 42))

        # 2) DataPoint 格式 (BLE 0x02/0x03/0x04)
        elif data_len >= 5:
            dps = parse_datapoints(payload)
            if dps:
                print(f"╠════════════════════════════════════════╣")
                print(f"║  DataPoint 解析 ({len(dps)}个):")
                for i, dp in enumerate(dps):
                    name = ATTRS.get(dp['dpid'], (f"未知({dp['dpid']})", "", {}))[0]
                    val_str = format_dp_value(dp)
                    print(f"║  [{i+1}] dpid={dp['dpid']:02X}({name}) type={DP_TYPES.get(dp['type'], f'0x{dp[\"type\"]:02X}'):6s} len={dp['len']} value={val_str}")

            # 3) timestamp + sleep_flag + DataPoints (UART 0x01)
            elif is_timestamp_prefix(payload):
                ts = be_to_int(payload[0:4])
                sf = payload[4]
                sf_name = {0: "MCU可休眠", 1: "MCU使能开机"}.get(sf, f"未知(0x{sf:02X})")
                print(f"╠════════════════════════════════════════╣")
                print(f"║  识别为: timestamp+sleep_flag+DPs (UART 0x01)")
                print(f"║  时间戳  : 0x{ts:08X} ({ts})")
                print(f"║  sleep   : {sf} → {sf_name}")
                if data_len > 5:
                    sub_dps = parse_datapoints(payload[5:])
                    if sub_dps:
                        print(f"║  DataPoint ({len(sub_dps)}个):")
                        for i, dp in enumerate(sub_dps):
                            name = ATTRS.get(dp['dpid'], (f"未知({dp['dpid']})", "", {}))[0]
                            val_str = format_dp_value(dp)
                            print(f"║  [{i+1}] dpid={dp['dpid']:02X}({name}) type={DP_TYPES.get(dp['type'], f'0x{dp[\"type\"]:02X}'):6s} len={dp['len']} value={val_str}")

    print("╚════════════════════════════════════════╝")

    # 如果有多余字节
    if len(data) > expected_total:
        extra = data[expected_total:]
        print(f"⚠️  帧后有 {len(extra)} 额外字节: {fmt_hex(extra)}")

    # 输出完整 hex (含校验和)
    full = data[:8 + data_len]
    chk = calc_checksum(full)
    print()
    print(f"完整帧:  {fmt_hex(full)} {chk:02X}")
    print(f"C 数组:  {{{', '.join(f'0x{b:02X}' for b in full)}, 0x{chk:02X}}}")


def is_timestamp_prefix(data: bytes) -> bool:
    """判断数据是否以 timestamp(4B) + sleep_flag(1B) 开头"""
    if len(data) < 5:
        return False
    sf = data[4]
    return sf in (0x00, 0x01)


def parse_datapoints(data: bytes) -> list:
    """从字节流中提取 DataPoint 列表，格式不正确则返回空"""
    dps = []
    pos = 0
    while pos + 4 <= len(data):
        dpid = data[pos]
        dtype = data[pos + 1]
        dlen = be_to_int(data[pos + 2:pos + 4])
        if pos + 4 + dlen > len(data):
            # 数据不够
            break
        value = data[pos + 4:pos + 4 + dlen]
        # 验证 dpid 合法性 (1~13)
        if not (1 <= dpid <= 13):
            break
        # 验证 type 合法性
        if dtype not in (0x01, 0x02, 0x04):
            break
        # 验证 len 与 type 匹配
        if dtype == 0x01 and dlen != 1:
            break
        if dtype == 0x04 and dlen != 1:
            break
        if dtype == 0x02 and dlen != 4:
            break
        dps.append({'dpid': dpid, 'type': dtype, 'len': dlen, 'value': value})
        pos += 4 + dlen
    return dps if dps and pos == len(data) else []


def format_dp_value(dp: dict) -> str:
    """格式化 DataPoint 的值为可读字符串"""
    dpid = dp['dpid']
    dtype = dp['type']
    val = dp['value']

    if dtype == 0x01:  # bool
        v = val[0]
        if dpid in ATTRS and ATTRS[dpid][2]:
            return f"{ATTRS[dpid][2].get(v, str(v))} (0x{v:02X})"
        return f"{'开' if v else '关'} (0x{v:02X})"

    elif dtype == 0x04:  # enum
        v = val[0]
        if dpid in ATTRS and ATTRS[dpid][2]:
            return f"{ATTRS[dpid][2].get(v, f'未知')} (档位{v})"
        return f"档位{v}"

    elif dtype == 0x02:  # value
        n = be_to_int(val)
        if dpid == 5:
            return f"{n} 分钟"
        elif dpid == 6:
            return f"{n} 分钟"
        elif dpid == 11:
            return f"Unix={n} (0x{n:08X})"
        return str(n)

    return f"0x{val.hex().upper()}"


# ============================================================
# 预约信息结构解析 (UART 0x03 / BLE 0x06 0x07)
# ============================================================

def parse_reservation(data: bytes, is_uart: bool):
    """解析预约信息数据结构并打印"""
    if is_uart:
        # UART: action(1) + id(1) + name(32) + time(4) + temp(1) + duration(1) + enabled(1) + repeat(1) = 42B
        if len(data) < 42:
            return False
        action = data[0]
        rid = data[1]
        name_raw = data[2:34]
        time_val = be_to_int(data[34:38])
        temp = data[38]
        duration = data[39]
        enabled = data[40]
        repeat = data[41]
        action_name = RESERVATION_ACTIONS.get(action, f"未知(0x{action:02X})")
        print(f"╠════════════════════════════════════════╣")
        print(f"║  预约信息结构 (UART 0x03, 42B):")
        print(f"║  操作类型  : 0x{action:02X} → {action_name}")
        print(f"║  预约ID    : {rid}" + (" (MCU自动分配)" if rid == 0 else ""))
        name_str = name_raw.rstrip(b'\x00').decode('ascii', errors='replace')
        if name_str:
            print(f"║  名称      : \"{name_str}\"")
        print(f"║  触发时间  : 0x{time_val:08X} ({time_val})" + (f" → {_fmt_unix(time_val)}" if 1500000000 < time_val < 2000000000 else ""))
    else:
        # BLE: id(1) + name(32) + time(4) + temp(1) + duration(1) + enabled(1) + repeat(1) = 41B
        if len(data) < 41:
            return False
        rid = data[0]
        name_raw = data[1:33]
        time_val = be_to_int(data[33:37])
        temp = data[37]
        duration = data[38]
        enabled = data[39]
        repeat = data[40]
        print(f"╠════════════════════════════════════════╣")
        print(f"║  预约信息结构 (BLE 0x06/0x07, 41B):")
        print(f"║  预约ID    : {rid}" + (" (MCU自动分配)" if rid == 0 else ""))
        name_str = name_raw.rstrip(b'\x00').decode('ascii', errors='replace')
        if name_str:
            print(f"║  名称      : \"{name_str}\"")
        print(f"║  触发时间  : 0x{time_val:08X} ({time_val})" + (f" → {_fmt_unix(time_val)}" if 1500000000 < time_val < 2000000000 else ""))

    # 公共字段
    temp_name = ATTRS.get(7, ("加热温度", "", {}))[2].get(temp, f"档位{temp}")
    print(f"║  加热温度  : {temp} → {temp_name}")
    print(f"║  加热时长  : {duration} 分钟")
    print(f"║  启用状态  : {enabled} → {'✅ 开启' if enabled else '❌ 关闭'}")
    # 重复周期: 位掩码，每 bit 代表一天
    day_names = ['一', '二', '三', '四', '五', '六', '日']
    if repeat == 0:
        repeat_str = "不重复"
    elif repeat == 0xff:
        repeat_str = "每天"
    elif repeat == 0x00:
        repeat_str = "不重复"
    else:
        active_days = [day_names[i] for i in range(7) if repeat & (1 << i)]
        repeat_str = f"0x{repeat:02X} (周{'、'.join(active_days)})"
    print(f"║  重复周期  : {repeat_str}")
    return True


def _fmt_unix(ts: int) -> str:
    """尝试把 unix 时间戳格式化为可读时间"""
    import datetime
    try:
        dt = datetime.datetime.fromtimestamp(ts)
        return dt.strftime("%Y-%m-%d %H:%M:%S")
    except (ValueError, OSError):
        return ""


# ============================================================
# 校验和单独计算
# ============================================================

def do_checksum(data: bytes):
    """只计算校验和"""
    total = sum(data)
    chk = total % 256
    print(f"输入({len(data)}字节): {fmt_hex(data)}")
    print(f"累加和: {total} (0x{total:04X})")
    print(f"校验和: 0x{chk:02X} ({chk})")
    print(f"完整帧: {fmt_hex(data)} {chk:02X}")


# ============================================================
# 主入口
# ============================================================

def main():
    args = sys.argv[1:]

    # 从文件读取
    if len(args) >= 2 and args[0] == "--file":
        with open(args[1], 'r') as f:
            content = f.read()
    elif len(args) >= 1 and args[0] in ("-h", "--help", "/?"):
        print(__doc__)
        return
    elif len(args) >= 1:
        content = " ".join(args)
    elif not sys.stdin.isatty():
        content = sys.stdin.read()
    else:
        print(__doc__)
        print("\n交互模式: 粘贴 hex 数据，然后按 Enter + Ctrl+Z(Win) / Ctrl+D(Unix)")
        print("─" * 50)
        lines = []
        try:
            while True:
                line = input()
                lines.append(line)
        except (EOFError, KeyboardInterrupt):
            pass
        content = "\n".join(lines)

    if not content or not content.strip():
        print("❌ 未输入数据")
        return

    try:
        data = parse_hex(content)
    except ValueError as e:
        print(f"❌ hex 解析失败: {e}")
        return

    # 自动判断：是完整帧(以 55AA 开头)还是纯数据校验
    if len(data) >= 2 and data[0] == 0x55 and data[1] == 0xAA:
        # 检测特殊命令
        if len(sys.argv) >= 2 and sys.argv[1] in ("crc", "checksum", "-c"):
            do_checksum(data)
        else:
            parse_frame(data)
    else:
        # 纯数据，只算校验和
        do_checksum(data)


if __name__ == "__main__":
    main()
