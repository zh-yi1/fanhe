#!/usr/bin/env python3
"""
饭盒蓝牙协议 — 校验和计算工具
协议文档: 蓝牙通讯协议1.0.4

用法:
  python checksum.py 55aa00000e00000101     # 算出校验和 0f
  python checksum.py "55 aa 00 00 0e 00 00 01 01"  # 空格分隔也行
  python checksum.py                         # 交互模式
"""

import sys
import re


def calc_checksum(data: bytes) -> int:
    """从帧头逐字节累加，对 256 取余"""
    return sum(data) % 256


def parse_hex(s: str) -> bytes:
    """支持 55AA.. 或 55 aa .. 或 0x55 0xAA .. 格式"""
    s = s.strip()
    # 去掉 0x/0X 前缀
    s = re.sub(r'0[xX]', '', s)
    # 去除非十六进制字符
    s = re.sub(r'[^0-9a-fA-F]', '', s)
    if len(s) % 2 != 0:
        raise ValueError(f"十六进制字符数必须是偶数，当前 {len(s)} 个")
    return bytes.fromhex(s)


def format_hex(data: bytes, sep: str = " ") -> str:
    return sep.join(f"{b:02X}" for b in data)


def main():
    args = sys.argv[1:]

    if args:
        # 命令行模式
        user_input = " ".join(args)
    else:
        # 交互模式
        print("=" * 60)
        print("  饭盒蓝牙协议 — 校验和计算器")
        print("  输入十六进制帧（不含校验和），自动拼完整帧并输出校验和")
        print("  例: 55aa00000e00000101")
        print("  输入 q 退出")
        print("=" * 60)

    while True:
        if not args:
            try:
                user_input = input("\n> ").strip()
            except (EOFError, KeyboardInterrupt):
                print()
                break
            if user_input.lower() in ("q", "quit", "exit"):
                break
            if not user_input:
                continue

        try:
            data = parse_hex(user_input)
        except ValueError as e:
            print(f"  解析错误: {e}")
            if args:
                sys.exit(1)
            continue

        if len(data) < 8:
            print("  错误: 帧至少 8 字节（帧头2+版本1+标志1+命令1+错误1+长度2）")
            if args:
                sys.exit(1)
            continue

        # 校验和 = 所有字节累加 % 256
        chk = calc_checksum(data)

        # 拼完整帧
        full_frame = data + bytes([chk])

        # 解析字段
        header  = int.from_bytes(data[0:2], "big")
        version = data[2]
        msg_f   = data[3]
        cmd     = data[4]
        err     = data[5]
        dlen    = int.from_bytes(data[6:8], "big")
        payload = data[8:] if len(data) > 8 else b""

        # 打印结果
        print(f"  ┌─────────────────────────────────────")
        print(f"  │ 帧头:     0x{header:04X}")
        print(f"  │ 版本:     0x{version:02X}")
        print(f"  │ 消息标志: 0x{msg_f:02X}")
        print(f"  │ 命令字:   0x{cmd:02X}")
        print(f"  │ 错误标志: 0x{err:02X}")
        print(f"  │ 数据长度: {dlen} (0x{dlen:04X})")
        if len(payload) > 0:
            print(f"  │ 数据:     {format_hex(payload)}")
        print(f"  ├─────────────────────────────────────")
        print(f"  │ 输入: {format_hex(data)}")
        print(f"  │ 累加和: {sum(data)} (0x{sum(data):X})")
        print(f"  │ 校验和: 0x{chk:02X} ({chk})")
        print(f"  ├─────────────────────────────────────")
        print(f"  │ 完整帧: {format_hex(full_frame)}")
        print(f"  └─────────────────────────────────────")

        if args:
            break


if __name__ == "__main__":
    main()
