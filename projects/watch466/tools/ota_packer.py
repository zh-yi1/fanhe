#!/usr/bin/env python3
"""
OTA 固件打包工具 — 蓝牙饭盒
===========================
根据蓝牙通讯协议 v1.0.8 和 MCU 通信协议 v1.0.7，将固件文件打包为 OTA 升级帧文件。

支持两种目标设备:
  mcu  — 主单片机 (.fot 文件), 使用 BLE 协议 OTA 通道 (CMD 0x0C/0x0D/0x0E)
  heat — 加热模块 (.bin 文件), 使用 MCU/UART 协议 OTA 通道 (CMD 0x04)

用法:
  # 打包主单片机固件 (BLE 通道)
  python ota_packer.py mcu --input firmware.fot --output firmware_mcu.ota

  # 打包加热模块固件 (UART 通道), 指定版本号
  python ota_packer.py heat --input firmware.bin --output firmware_heat.ota --version 0x00010000

  # 加热模块, 附带复位到 Boot 的指令帧
  python ota_packer.py heat --input firmware.bin --output firmware_heat.ota --version 1.0.0 --reset

  # 查看打包结果摘要 (不输出文件)
  python ota_packer.py heat --input firmware.bin --dry-run

版本号格式:
  --version 接受三种格式:
    - 十六进制: 0x00010000
    - 十进制整数: 65536
    - 点分格式: 1.0.0  (转换为 major<<24 | minor<<16 | patch)
"""

import struct
import argparse
import sys
import os
from typing import Tuple, List, Optional

# ============================================================
# 协议常量
# ============================================================

FRAME_HEAD = b'\x55\xAA'
PROTO_VERSION = 0x00
MSG_FLAG = 0x00
ERR_OK = 0x00

# BLE OTA 命令字 (蓝牙通讯协议 v1.0.8 §5)
CMD_OTA_START = 0x0C   # 升级启动
CMD_OTA_DATA  = 0x0D   # 升级包传输
CMD_OTA_END   = 0x0E   # 升级结束

# MCU/UART OTA 命令字 (MCU 通信协议 v1.0.7 §5.1)
CMD_MCU_OTA   = 0x04   # 升级包开始/传输/结束 (三合一)

# 目标设备标识 (BLE 协议)
TARGET_MAIN_MCU   = 0x01
TARGET_HEAT_MODULE = 0x02

# OTA 分包参数
CHUNK_SIZE = 128       # 默认每包数据字节数
ALIGNMENT  = 16        # 每包数据长度必须能被 16 整除

# MCU BIN 文件头 (MCU 通信协议 §5.1 备注2)
BIN_HEADER_SIZE = 256
BIN_MAGIC       = 0x11223344
BIN_PADDING_BYTE = 0xFF

# MCU 协议特殊偏移量
OFFSET_START = 0x00000000
OFFSET_END   = 0xFFFFFFFF

# CRC 类型
CRC_STANDARD = 'standard'  # 标准 CRC32 (reflected, Ethernet)
CRC_MPEG2    = 'mpeg2'     # CRC32/MPEG-2 (non-reflected)

# ============================================================
# CRC32 计算
# ============================================================

def crc32_mpeg2(data: bytes) -> int:
    """
    CRC32/MPEG-2 (非反射).
    多项式: 0x04C11DB7, 初始值: 0xFFFFFFFF, 终值 XOR: 0x00000000
    """
    polynomial = 0x04C11DB7
    crc = 0xFFFFFFFF
    for byte in data:
        crc ^= (byte << 24)
        for _ in range(8):
            if crc & 0x80000000:
                crc = (crc << 1) ^ polynomial
            else:
                crc = (crc << 1)
            crc &= 0xFFFFFFFF
    return crc


def crc32_standard(data: bytes) -> int:
    """标准 CRC32 (Ethernet/zip, 反射)"""
    import binascii
    return binascii.crc32(data) & 0xFFFFFFFF


# ============================================================
# 帧构建
# ============================================================

def calc_checksum(data: bytes) -> int:
    """校验和: 所有字节求和 mod 256"""
    return sum(data) & 0xFF


def build_frame(command: int, payload: bytes = b'', error: int = ERR_OK) -> bytes:
    """
    构建完整协议帧.

    帧结构 (蓝牙通讯协议 v1.0.8 §2.1 / MCU 通信协议 v1.0.7 §2.1):
      帧头(2B) + 版本(1B) + 消息标志(1B) + 命令字(1B) + 错误标志(1B)
      + 数据长度(2B, 大端) + 数据(可变) + 校验和(1B)
    """
    body = struct.pack('>BBBBH',
                       PROTO_VERSION,
                       MSG_FLAG,
                       command,
                       error,
                       len(payload))
    frame = FRAME_HEAD + body + payload
    checksum = calc_checksum(frame)
    return frame + bytes([checksum])


def chunk_firmware(data: bytes, chunk_size: int = CHUNK_SIZE,
                   alignment: int = ALIGNMENT) -> List[bytes]:
    """
    将固件数据按指定大小分包，每包补齐到 alignment 的整数倍.
    最后一个包不足 alignment 时补 0x00.
    """
    chunks = []
    for i in range(0, len(data), chunk_size):
        chunk = data[i:i + chunk_size]
        rem = len(chunk) % alignment
        if rem != 0:
            chunk += b'\x00' * (alignment - rem)
        chunks.append(chunk)
    return chunks


# ============================================================
# BLE OTA 打包器 — 主单片机 (.fot → OTA 帧)
# ============================================================

def pack_mcu_ble(firmware: bytes) -> List[bytes]:
    """
    将主单片机固件 (.fot) 打包为 BLE OTA 升级帧.

    生成帧序列:
      1. 启动帧 (0x0C): target=0x01 + firmware_size(4B BE)
      2. 数据帧 (0x0D): target=0x01 + offset(4B BE) + chunk (128B, 16B 对齐) × N
      3. 结束帧 (0x0E): target=0x01

    参考: 蓝牙通讯协议 v1.0.8 §5.1-5.3

    Args:
        firmware: 原始固件二进制数据

    Returns:
        协议帧列表 (每个元素为一个完整帧的 bytes)
    """
    frames = []
    fw_size = len(firmware)

    # --- 1. 升级启动帧 (CMD 0x0C) ---
    # 数据: 目标设备标识(1B) + 固件字节数(4B, 大端)
    start_payload = struct.pack('>BI', TARGET_MAIN_MCU, fw_size)
    frames.append(build_frame(CMD_OTA_START, start_payload))

    # --- 2. 升级数据帧 (CMD 0x0D) ---
    chunks = chunk_firmware(firmware)
    for i, chunk in enumerate(chunks):
        offset = i * CHUNK_SIZE
        # 数据: 目标设备标识(1B) + offset(4B, 大端) + 固件数据块
        data_payload = struct.pack('>BI', TARGET_MAIN_MCU, offset) + chunk
        frames.append(build_frame(CMD_OTA_DATA, data_payload))

    # --- 3. 升级结束帧 (CMD 0x0E) ---
    # 数据: 目标设备标识(1B)
    end_payload = struct.pack('>B', TARGET_MAIN_MCU)
    frames.append(build_frame(CMD_OTA_END, end_payload))

    return frames


# ============================================================
# MCU/UART OTA 打包器 — 加热模块 (.bin → OTA 帧)
# ============================================================

def build_bin_header(firmware: bytes, version: int,
                     crc_type: str = CRC_MPEG2) -> bytes:
    """
    构建 MCU 协议 BIN 文件的 256 字节头部.

    头部结构 (MCU 通信协议 v1.0.7 §5.1 备注2):
      字节 0-3:   Magic 0x11223344
      字节 4-7:   固件版本号 (大端)
      字节 8-11:  固件内容总长度 (不含头部, 大端)
      字节 12-15: 固件 CRC32/MPEG-2 校验码 (大端)
      字节 16-255: 填充 0xFF

    Args:
        firmware: 原始固件二进制
        version: 32-bit 版本号
        crc_type: CRC 算法类型

    Returns:
        256 字节头部
    """
    fw_len = len(firmware)
    crc_func = crc32_mpeg2 if crc_type == CRC_MPEG2 else crc32_standard
    crc_val = crc_func(firmware)

    header = struct.pack('>IIII', BIN_MAGIC, version, fw_len, crc_val)
    header += bytes([BIN_PADDING_BYTE] * (BIN_HEADER_SIZE - len(header)))
    return header


def pack_heat_uart(firmware: bytes, version: int,
                   add_reset: bool = False,
                   crc_type: str = CRC_MPEG2) -> List[bytes]:
    """
    将加热模块固件 (.bin) 打包为 MCU/UART OTA 升级帧.

    处理流程:
      1. 检测/构建 256 字节 BIN 头部
      2. (可选) 复位到 Boot 帧: offset=0xFFFFFFFF
      3. 数据帧: offset(4B BE) + chunk (128B, 16B 对齐) × N
      4. 结束校验帧: offset=0xFFFFFFFF + CRC32(4B)

    参考: MCU 通信协议 v1.0.7 §5.1

    Args:
        firmware: 原始固件二进制数据 (不含头部, 或已含头部)
        version: 32-bit 版本号
        add_reset: 是否在最前面添加复位到 Boot 的指令帧
        crc_type: CRC 算法类型

    Returns:
        协议帧列表
    """
    frames = []

    # --- 检查是否已有 BIN 头部 ---
    if len(firmware) >= 4 and struct.unpack('>I', firmware[:4])[0] == BIN_MAGIC:
        bin_data = firmware  # 已有头部, 直接使用
        print(f"[info] 检测到 BIN 头部已存在 (magic=0x{BIN_MAGIC:08X}), 跳过头部生成")
    else:
        header = build_bin_header(firmware, version, crc_type)
        crc_func = crc32_mpeg2 if crc_type == CRC_MPEG2 else crc32_standard
        fw_crc = crc_func(firmware)
        print(f"[info] BIN 头部:")
        print(f"        Magic:    0x{BIN_MAGIC:08X}")
        print(f"        Version:  0x{version:08X}")
        print(f"        Fw Length: {len(firmware)} bytes (0x{len(firmware):08X})")
        print(f"        Fw CRC32:  0x{fw_crc:08X}")
        bin_data = header + firmware

    # --- 可选: 复位到 Boot 帧 ---
    # 当 MCU 处于 App 模式时, offset=0xFFFFFFFF 触发复位进入 Boot 模式
    if add_reset:
        reset_payload = struct.pack('>I', OFFSET_END)
        frames.append(build_frame(CMD_MCU_OTA, reset_payload))
        print(f"[info] 已添加复位到 Boot 帧 (offset=0xFFFFFFFF, 无 CRC)")

    # --- 数据帧 (CMD 0x04) ---
    chunks = chunk_firmware(bin_data)
    for i, chunk in enumerate(chunks):
        offset = i * CHUNK_SIZE
        # 数据: offset(4B, 大端) + 固件数据块
        data_payload = struct.pack('>I', offset) + chunk
        frames.append(build_frame(CMD_MCU_OTA, data_payload))

    # --- 结束校验帧 (CMD 0x04, offset=0xFFFFFFFF + CRC32) ---
    crc_func = crc32_mpeg2 if crc_type == CRC_MPEG2 else crc32_standard
    bin_crc = crc_func(bin_data)
    end_payload = struct.pack('>II', OFFSET_END, bin_crc)
    frames.append(build_frame(CMD_MCU_OTA, end_payload))

    print(f"[info] 完整 BIN CRC32: 0x{bin_crc:08X} (覆盖头部+固件)")
    return frames


# ============================================================
# 版本号解析
# ============================================================

def parse_version(ver_str: str) -> int:
    """
    解析版本号字符串.

    支持格式:
      - 十六进制: "0x00010000" → 65536
      - 十进制:   "65536"      → 65536
      - 点分:     "1.0.0"      → 0x01000000 (major<<24 | minor<<16 | patch)
      - 点分短:   "1.0"         → 0x01000000
      - 单数字:   "1"           → 0x01000000
    """
    ver_str = ver_str.strip()

    # 十六进制
    if ver_str.lower().startswith('0x'):
        return int(ver_str, 16)

    # 点分格式
    if '.' in ver_str:
        parts = ver_str.split('.')
        if len(parts) > 3:
            raise ValueError(f"版本号最多 3 段: major.minor.patch, 实际: {ver_str}")
        nums = [int(p) for p in parts]
        # 补齐到 3 段
        while len(nums) < 3:
            nums.append(0)
        if any(n < 0 or n > 255 for n in nums):
            raise ValueError(f"版本号每段必须在 0-255 之间: {ver_str}")
        return (nums[0] << 24) | (nums[1] << 16) | nums[2]

    # 十进制整数
    return int(ver_str)


# ============================================================
# 帧信息输出
# ============================================================

def describe_frame(frame: bytes) -> str:
    """单帧可读描述"""
    if len(frame) < 8:
        return f"[ERR] 帧过短: {len(frame)}B"

    cmd = frame[4]
    data_len = struct.unpack('>H', frame[6:8])[0]
    total_len = len(frame)
    checksum = frame[-1]
    expected = calc_checksum(frame[:-1])

    cmd_names = {
        CMD_OTA_START: '升级启动(0x0C)',
        CMD_OTA_DATA:  '升级数据(0x0D)',
        CMD_OTA_END:   '升级结束(0x0E)',
        CMD_MCU_OTA:   'MCU OTA(0x04)',
    }
    cmd_name = cmd_names.get(cmd, f'未知(0x{cmd:02X})')

    ck_status = "✓" if checksum == expected else f"✗(期望0x{expected:02X})"

    return (f"  cmd={cmd_name} | data_len={data_len}B | "
            f"frame_len={total_len}B | checksum=0x{checksum:02X} {ck_status}")


def print_summary(frames: List[bytes], output_path: Optional[str] = None):
    """打印打包摘要"""
    total_size = sum(len(f) for f in frames)
    print(f"\n{'='*60}")
    print(f"OTA 打包完成")
    print(f"{'='*60}")
    print(f"  总帧数:   {len(frames)}")
    print(f"  总字节:   {total_size} bytes ({total_size/1024:.1f} KB)")

    # 统计数据帧
    data_frames = [f for f in frames if f[4] in (CMD_OTA_DATA, CMD_MCU_OTA)]
    payload_sizes = [struct.unpack('>H', f[6:8])[0] for f in data_frames]
    # 排除最后一帧 (offset=0xFFFFFFFF 的结束校验帧)
    actual_data_frames = data_frames[:-1] if len(data_frames) > 1 else data_frames
    if payload_sizes:
        firmware_total = sum(struct.unpack('>H', f[6:8])[0] for f in actual_data_frames)
        print(f"  有效数据: {firmware_total} bytes ({firmware_total/1024:.1f} KB)")

    print(f"\n帧列表:")
    for i, f in enumerate(frames):
        print(f"  [{i:4d}] {describe_frame(f)}")

    if output_path:
        print(f"\n输出文件: {output_path}")


# ============================================================
# CLI 主入口
# ============================================================

def main():
    parser = argparse.ArgumentParser(
        description='OTA 固件打包工具 — 蓝牙饭盒',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 打包主单片机固件 (.fot), BLE OTA 通道
  python ota_packer.py mcu --input firmware.fot --output firmware_mcu.ota

  # 打包加热模块固件 (.bin), UART OTA 通道
  python ota_packer.py heat --input firmware.bin --output firmware_heat.ota --version 1.0.0

  # 加热模块, 附带复位帧
  python ota_packer.py heat --input firmware.bin --output fw.ota --version 0x00010001 --reset

  # 仅预览, 不输出文件
  python ota_packer.py mcu --input firmware.fot --dry-run
        """)

    sub = parser.add_subparsers(dest='target', help='目标设备类型')

    # --- mcu 子命令 ---
    mcu_parser = sub.add_parser('mcu', help='主单片机 (.fot) — BLE OTA 协议')
    mcu_parser.add_argument('--input', '-i', required=True,
                            help='输入固件文件路径 (.fot)')
    mcu_parser.add_argument('--output', '-o', default=None,
                            help='输出 OTA 文件路径 (默认: <input>.ota)')
    mcu_parser.add_argument('--dry-run', '-n', action='store_true',
                            help='仅预览, 不写入文件')

    # --- heat 子命令 ---
    heat_parser = sub.add_parser('heat', help='加热模块 (.bin) — MCU/UART OTA 协议')
    heat_parser.add_argument('--input', '-i', required=True,
                             help='输入固件文件路径 (.bin)')
    heat_parser.add_argument('--output', '-o', default=None,
                             help='输出 OTA 文件路径 (默认: <input>.ota)')
    heat_parser.add_argument('--version', '-v', default='0x00000001',
                             help='固件版本号 (hex: 0x..., 点分: 1.0.0, 十进制: 65536)')
    heat_parser.add_argument('--reset', '-r', action='store_true',
                             help='在升级前添加复位到 Boot 的指令帧')
    heat_parser.add_argument('--crc', choices=[CRC_STANDARD, CRC_MPEG2],
                             default=CRC_MPEG2,
                             help='CRC 算法 (默认: mpeg2)')
    heat_parser.add_argument('--dry-run', '-n', action='store_true',
                             help='仅预览, 不写入文件')

    args = parser.parse_args()

    if not args.target:
        parser.print_help()
        sys.exit(1)

    # --- 读取输入文件 ---
    input_path = args.input
    if not os.path.isfile(input_path):
        print(f"[错误] 文件不存在: {input_path}")
        sys.exit(1)

    with open(input_path, 'rb') as f:
        firmware = f.read()

    if len(firmware) == 0:
        print("[错误] 固件文件为空")
        sys.exit(1)

    print(f"[info] 读取固件: {input_path}")
    print(f"       文件大小: {len(firmware)} bytes ({len(firmware)/1024:.1f} KB)")

    # --- 输出路径 ---
    output_path = args.output
    if not output_path and not args.dry_run:
        base = os.path.splitext(input_path)[0]
        output_path = base + '.ota'

    # --- 打包 ---
    if args.target == 'mcu':
        print(f"[info] 目标: 主单片机 (BLE OTA 通道, 0x0C/0x0D/0x0E)")
        frames = pack_mcu_ble(firmware)
    else:  # heat
        version = parse_version(args.version)
        print(f"[info] 目标: 加热模块 (MCU/UART OTA 通道, CMD 0x04)")
        print(f"       版本号: 0x{version:08X}")
        print(f"       CRC:    {args.crc}")
        print(f"       复位帧: {'是' if args.reset else '否'}")
        frames = pack_heat_uart(firmware, version,
                                add_reset=args.reset,
                                crc_type=args.crc)

    # --- 输出 ---
    print_summary(frames, output_path if not args.dry_run else None)

    if not args.dry_run:
        ota_data = b''.join(frames)
        with open(output_path, 'wb') as f:
            f.write(ota_data)
        print(f"[完成] OTA 文件已写入: {output_path}")
        print(f"       可使用 parse 模式查看文件内容:")
        print(f"       python ota_packer.py parse --input {output_path}")


# ============================================================
# 辅助: OTA 文件解析器
# ============================================================

def parse_ota_file(filepath: str) -> List[bytes]:
    """
    解析已有的 .ota 文件, 按帧边界拆分.
    返回帧列表.
    """
    with open(filepath, 'rb') as f:
        data = f.read()

    frames = []
    pos = 0
    while pos < len(data):
        # 最小帧: 2(头) + 1(ver) + 1(flag) + 1(cmd) + 1(err) + 2(len) + 0(data) + 1(cs) = 9
        if pos + 9 > len(data):
            print(f"[警告] 位置 {pos}: 剩余 {len(data)-pos} 字节不足以构成完整帧, 停止解析")
            break

        # 检查帧头
        if data[pos:pos+2] != FRAME_HEAD:
            print(f"[错误] 位置 {pos}: 帧头不匹配 (期望 0x55AA, 实际 "
                  f"0x{data[pos]:02X}{data[pos+1]:02X}), 停止解析")
            break

        cmd = data[pos + 4]
        data_len = struct.unpack('>H', data[pos+6:pos+8])[0]
        frame_total = 9 + data_len  # 头(2) + ver(1) + flag(1) + cmd(1) + err(1) + dlen(2) + data + cs(1)

        if pos + frame_total > len(data):
            print(f"[错误] 位置 {pos}: 帧长度 {frame_total} 超出文件范围, 停止解析")
            break

        frame = data[pos:pos + frame_total]
        frames.append(frame)
        pos += frame_total

    return frames


# ============================================================
# 扩展 CLI: parse 子命令 + 主入口
# ============================================================

def _parse_main():
    """OTA 文件解析入口 (通过 parse 子命令调用)"""
    parser = argparse.ArgumentParser(
        description='解析 .ota 文件并打印每帧信息',
        add_help=False)
    parser.add_argument('--input', '-i', required=True,
                        help='输入的 .ota 文件路径')
    args, _ = parser.parse_known_args()

    frames = parse_ota_file(args.input)
    if not frames:
        print("[错误] 未能解析任何帧")
        return

    print(f"解析 {args.input}: 共 {len(frames)} 帧, "
          f"{sum(len(f) for f in frames)} bytes\n")
    for i, f in enumerate(frames):
        cmd = f[4]
        dlen = struct.unpack('>H', f[6:8])[0]
        cs = f[-1]
        expected = calc_checksum(f[:-1])
        ok = "✓" if cs == expected else f"✗(期望0x{expected:02X})"
        print(f"[{i:4d}] cmd=0x{cmd:02X}  dlen={dlen:3d}B  "
              f"frame={len(f):3d}B  checksum=0x{cs:02X} {ok}")


if __name__ == '__main__':
    # 支持 parse 子命令 (hack: 在 argparse 处理前检查)
    if len(sys.argv) > 1 and sys.argv[1] == 'parse':
        _parse_main()
    else:
        main()
