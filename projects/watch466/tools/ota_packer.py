#!/usr/bin/env python3
"""
OTA 固件打包工具 — 蓝牙饭盒 (BLEDebug 专用)
==============================================
按照蓝牙通讯协议 v1.0.8 §5，将固件文件打包为 BLE OTA 升级帧文件 (.ota)，
可直接通过 BLEDebug 工具逐帧发送。

支持两种目标设备:
  mcu  — 主单片机 (.fot 文件), target=0x01
  heat — 加热模块 (.bin 文件), target=0x02

两者均使用 BLE OTA 协议 (CMD 0x0C/0x0D/0x0E)，
数据区包含 256 字节 BIN 包头 (magic=0x11223344, CRC32/MPEG-2)。

用法:
  # 打包主单片机固件
  python ota_packer.py mcu --input test_ota.fot

  # 打包加热模块固件
  python ota_packer.py heat --input otah_Project_v005.bin

  # 指定版本号
  python ota_packer.py heat --input fw.bin --version 1.0.0

  # 解析已有的 .ota 文件
  python ota_packer.py parse --input output.ota

输出目录 (默认):
  MCU  → Output/bin/ota_dog/mcu_ota/
  Heat → Output/bin/ota_dog/heat_ota/
"""

import struct
import argparse
import sys
import os
from typing import List, Optional

# ============================================================
# 路径配置
# ============================================================
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)  # watch466/
OTA_DOG_DIR = os.path.join(PROJECT_DIR, 'Output', 'bin', 'ota_dog')
DEFAULT_OUTPUT_DIR = {
    'mcu':  os.path.join(OTA_DOG_DIR, 'mcu_ota'),
    'heat': os.path.join(OTA_DOG_DIR, 'heat_ota'),
}

# ============================================================
# 协议常量 (蓝牙通讯协议 v1.0.8)
# ============================================================

FRAME_HEAD = b'\x55\xAA'
PROTO_VERSION = 0x00
MSG_FLAG = 0x00
ERR_OK = 0x00

# BLE OTA 命令字 (§5)
CMD_OTA_START = 0x0C   # 升级启动 §5.1
CMD_OTA_DATA  = 0x0D   # 升级包传输 §5.2
CMD_OTA_END   = 0x0E   # 升级结束 §5.3

# 目标设备标识
TARGET_MAIN_MCU    = 0x01  # 主单片机
TARGET_HEAT_MODULE = 0x02  # 加热模块

# OTA 分包参数
CHUNK_SIZE = 128       # 每包数据字节数
ALIGNMENT  = 16        # 每包数据长度必须能被 16 整除

# BIN 文件头 (MCU 通信协议 §5.1 备注2)
BIN_HEADER_SIZE = 256
BIN_MAGIC       = 0x11223344
BIN_PADDING_BYTE = 0xFF

# ============================================================
# CRC32/MPEG-2 (非反射)
# ============================================================

def crc32_mpeg2(data: bytes) -> int:
    """
    CRC32/MPEG-2 (non-reflected).
    多项式: 0x04C11DB7, 初始值: 0xFFFFFFFF, 终值 XOR: 0x00000000
    与 MCU 通信协议 §5.1 要求一致。
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
    """标准 CRC32 (Ethernet/zlib, 反射) — 仅供参考"""
    import binascii
    return binascii.crc32(data) & 0xFFFFFFFF


# ============================================================
# 帧构建
# ============================================================

def calc_checksum(data: bytes) -> int:
    """校验和: 所有字节求和 mod 256"""
    return sum(data) & 0xFF


def build_frame(command: int, payload: bytes = b'') -> bytes:
    """
    构建完整 BLE 协议帧.

    帧结构 (蓝牙通讯协议 v1.0.8 §2.1):
      帧头(2B) + 版本(1B) + 消息标志(1B) + 命令字(1B) + 错误标志(1B)
      + 数据长度(2B, 大端) + 数据(可变) + 校验和(1B)
    """
    body = struct.pack('>BBBBH',
                       PROTO_VERSION,
                       MSG_FLAG,
                       command,
                       ERR_OK,
                       len(payload))
    frame = FRAME_HEAD + body + payload
    checksum = calc_checksum(frame)
    return frame + bytes([checksum])


def chunk_firmware(data: bytes, chunk_size: int = CHUNK_SIZE,
                   alignment: int = ALIGNMENT) -> List[bytes]:
    """
    将数据按指定大小分包，每包补齐到 alignment 的整数倍.
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
# BIN 包头构建
# ============================================================

def build_bin_header(firmware: bytes, version: int) -> bytes:
    """
    构建 256 字节 BIN 包头 (MCU 通信协议 §5.1 备注2).

    头部结构:
      字节 0-3:   Magic 0x11223344
      字节 4-7:   固件版本号 (大端)
      字节 8-11:  固件内容总长度 (不含头部, 大端)
      字节 12-15: 固件 CRC32/MPEG-2 校验码 (大端)
      字节 16-255: 填充 0xFF

    Args:
        firmware: 原始固件二进制 (不含头部)
        version: 32-bit 版本号

    Returns:
        256 字节头部
    """
    fw_len = len(firmware)
    crc_val = crc32_mpeg2(firmware)

    header = struct.pack('>IIII', BIN_MAGIC, version, fw_len, crc_val)
    header += bytes([BIN_PADDING_BYTE] * (BIN_HEADER_SIZE - len(header)))
    return header


# ============================================================
# BLE OTA 打包 (mcu 和 heat 共用)
# ============================================================

def pack_ble_ota(firmware: bytes, version: int, target: int) -> List[bytes]:
    """
    将固件打包为 BLE OTA 升级帧 (蓝牙通讯协议 v1.0.8 §5).

    流程:
      1. 构建 256 字节 BIN 包头 (magic + version + fw_len + CRC32/MPEG-2)
      2. 包头 + 固件 → 完整 BIN 数据
      3. BIN 数据按 128 字节分包 (16 字节对齐)
      4. 生成帧序列: START(0x0C) + DATA(0x0D)×N + END(0x0E)

    Args:
        firmware: 原始固件二进制数据 (不含 BIN 包头)
        version: 32-bit 版本号
        target: 目标设备标识 (0x01=主单片机, 0x02=加热模块)

    Returns:
        协议帧列表 (每个元素为一个完整帧的 bytes)
    """
    frames = []

    # --- 1. 构建完整 BIN 数据 (256B 包头 + 固件) ---
    header = build_bin_header(firmware, version)
    bin_data = header + firmware
    total_size = len(bin_data)

    # --- 2. 升级启动帧 (CMD 0x0C) ---
    # 数据: 目标设备标识(1B) + 固件总字节数(4B, 大端)
    start_payload = struct.pack('>BI', target, total_size)
    frames.append(build_frame(CMD_OTA_START, start_payload))

    # --- 3. 升级数据帧 (CMD 0x0D) ---
    # 数据: 目标设备标识(1B) + offset(4B, 大端) + 数据块
    chunks = chunk_firmware(bin_data)
    for i, chunk in enumerate(chunks):
        offset = i * CHUNK_SIZE
        data_payload = struct.pack('>BI', target, offset) + chunk
        frames.append(build_frame(CMD_OTA_DATA, data_payload))

    # --- 4. 升级结束帧 (CMD 0x0E) ---
    # 数据: 目标设备标识(1B)
    end_payload = struct.pack('>B', target)
    frames.append(build_frame(CMD_OTA_END, end_payload))

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
      - ASCII:    "v005"        → 0x76303035
    """
    ver_str = ver_str.strip()

    # 十六进制
    if ver_str.lower().startswith('0x'):
        return int(ver_str, 16)

    # ASCII 字符串 (如 "v005")
    if not all(c.isdigit() or c == '.' for c in ver_str):
        b = ver_str.encode('ascii', errors='replace')
        b = b.ljust(4, b'\x00')[:4]
        return struct.unpack('>I', b)[0]

    # 点分格式
    if '.' in ver_str:
        parts = ver_str.split('.')
        if len(parts) > 3:
            raise ValueError(f"版本号最多 3 段: major.minor.patch, 实际: {ver_str}")
        nums = [int(p) for p in parts]
        while len(nums) < 3:
            nums.append(0)
        if any(n < 0 or n > 255 for n in nums):
            raise ValueError(f"版本号每段必须在 0-255 之间: {ver_str}")
        return (nums[0] << 24) | (nums[1] << 16) | nums[2]

    # 十进制整数
    return int(ver_str)


# ============================================================
# 帧描述
# ============================================================

CMD_NAMES = {
    CMD_OTA_START: 'START(0x0C)',
    CMD_OTA_DATA:  'DATA (0x0D)',
    CMD_OTA_END:   'END  (0x0E)',
}


def describe_frame(frame: bytes, idx: int) -> str:
    """单帧描述"""
    if len(frame) < 9:
        return f"[{idx:4d}] [ERR] 帧过短: {len(frame)}B"

    cmd = frame[4]
    data_len = struct.unpack('>H', frame[6:8])[0]
    total_len = len(frame)
    checksum = frame[-1]
    expected = calc_checksum(frame[:-1])

    cmd_name = CMD_NAMES.get(cmd, f'UNKN(0x{cmd:02X})')
    ck = "OK" if checksum == expected else f"FAIL(expected 0x{expected:02X})"

    return (f"[{idx:4d}] cmd={cmd_name}  dlen={data_len:4d}B  "
            f"frame={total_len:4d}B  checksum=0x{checksum:02X} {ck}")


def print_summary(frames: List[bytes], output_path: str,
                  fw_size: int, version: int, target: int):
    """打印打包摘要"""
    total_size = sum(len(f) for f in frames)
    target_names = {TARGET_MAIN_MCU: '主单片机 (target=0x01)',
                    TARGET_HEAT_MODULE: '加热模块 (target=0x02)'}
    target_name = target_names.get(target, f'未知(0x{target:02X})')

    num_data = len(frames) - 2  # 减去 START 和 END
    bad_frames = sum(1 for f in frames if f[-1] != calc_checksum(f[:-1]))

    print(f"\n{'='*60}")
    print(f"OTA 打包完成")
    print(f"{'='*60}")
    print(f"  目标设备:   {target_name}")
    print(f"  固件版本:   0x{version:08X}")
    print(f"  原始固件:   {fw_size} bytes ({fw_size/1024:.1f} KB)")
    print(f"  BIN 包头:   {BIN_HEADER_SIZE} bytes")
    print(f"  打包大小:   {total_size} bytes ({total_size/1024:.1f} KB)")
    print(f"  总帧数:     {len(frames)} (1 START + {num_data} DATA + 1 END)")
    print(f"  帧校验:     {'全部通过' if bad_frames == 0 else f'{bad_frames} 帧失败!'}")

    # 帧列表: 前3帧 + 后2帧
    print(f"\n帧列表 (前3 + 后2):")
    for i in range(min(3, len(frames))):
        print(f"  {describe_frame(frames[i], i)}")
    if len(frames) > 5:
        print(f"  ... 省略 {len(frames) - 5} 帧 ...")
    for i in range(max(3, len(frames) - 2), len(frames)):
        print(f"  {describe_frame(frames[i], i)}")

    print(f"\n输出文件: {output_path}")


# ============================================================
# OTA 文件解析器
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
        if pos + 9 > len(data):
            print(f"[警告] 位置 {pos}: 剩余 {len(data)-pos} 字节不足以构成完整帧, 停止解析")
            break

        if data[pos:pos+2] != FRAME_HEAD:
            print(f"[错误] 位置 {pos}: 帧头不匹配 (期望 0x55AA, 实际 "
                  f"0x{data[pos]:02X}{data[pos+1]:02X}), 停止解析")
            break

        cmd = data[pos + 4]
        data_len = struct.unpack('>H', data[pos+6:pos+8])[0]
        frame_total = 9 + data_len

        if pos + frame_total > len(data):
            print(f"[错误] 位置 {pos}: 帧长度 {frame_total} 超出文件范围, 停止解析")
            break

        frame = data[pos:pos + frame_total]
        frames.append(frame)
        pos += frame_total

    return frames


def cmd_parse(args):
    """解析 .ota 文件"""
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
        ok = "OK" if cs == expected else f"FAIL(expected 0x{expected:02X})"
        print(f"[{i:4d}] cmd=0x{cmd:02X}  dlen={dlen:3d}B  "
              f"frame={len(f):3d}B  checksum=0x{cs:02X} {ok}")


# ============================================================
# CLI 主入口
# ============================================================

def main():
    parser = argparse.ArgumentParser(
        description='OTA 固件打包工具 — 蓝牙饭盒 (BLEDebug 专用)',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 打包主单片机固件 (.fot) → BLE OTA
  python ota_packer.py mcu --input test_ota.fot

  # 打包加热模块固件 (.bin) → BLE OTA
  python ota_packer.py heat --input otah_Project_v005.bin

  # 指定版本号
  python ota_packer.py mcu --input fw.fot --version 1.0.0
  python ota_packer.py heat --input fw.bin --version 0x76303035

  # 指定输出路径
  python ota_packer.py mcu --input fw.fot -o /path/to/output.ota

  # 解析已有的 .ota 文件
  python ota_packer.py parse --input output.ota
        """)

    sub = parser.add_subparsers(dest='target', help='目标设备类型')

    # --- mcu 子命令 ---
    mcu_parser = sub.add_parser('mcu', help='主单片机 (.fot) — target=0x01, BLE OTA')
    mcu_parser.add_argument('--input', '-i', required=True,
                            help='输入固件文件路径 (.fot)')
    mcu_parser.add_argument('--output', '-o', default=None,
                            help='输出 .ota 文件路径 (默认: mcu_ota/<basename>.ota)')
    mcu_parser.add_argument('--version', '-v', default='0x00000001',
                            help='固件版本号 (默认: 0x00000001)')

    # --- heat 子命令 ---
    heat_parser = sub.add_parser('heat', help='加热模块 (.bin) — target=0x02, BLE OTA')
    heat_parser.add_argument('--input', '-i', required=True,
                             help='输入固件文件路径 (.bin)')
    heat_parser.add_argument('--output', '-o', default=None,
                             help='输出 .ota 文件路径 (默认: heat_ota/<basename>.ota)')
    heat_parser.add_argument('--version', '-v', default='0x00000001',
                             help='固件版本号 (默认: 0x00000001)')

    # --- parse 子命令 ---
    parse_parser = sub.add_parser('parse', help='解析已有的 .ota 文件')
    parse_parser.add_argument('--input', '-i', required=True,
                              help='输入的 .ota 文件路径')

    args = parser.parse_args()

    if not args.target:
        parser.print_help()
        sys.exit(1)

    # --- parse 子命令 ---
    if args.target == 'parse':
        cmd_parse(args)
        return

    # --- 解析版本号 ---
    try:
        version = parse_version(args.version)
    except ValueError as e:
        print(f"[错误] 版本号解析失败: {e}")
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

    # --- 确定目标 ---
    if args.target == 'mcu':
        target = TARGET_MAIN_MCU
    else:
        target = TARGET_HEAT_MODULE

    print(f"[info] 读取固件: {input_path}")
    print(f"       文件大小: {len(firmware)} bytes ({len(firmware)/1024:.1f} KB)")
    print(f"       版本号:   0x{version:08X}")
    print(f"       目标设备: {'主单片机 (0x01)' if target == TARGET_MAIN_MCU else '加热模块 (0x02)'}")
    print(f"       协议:     BLE OTA (0x0C/0x0D/0x0E)")
    print(f"       CRC32:    0x{crc32_mpeg2(firmware):08X} (MPEG-2)")

    # --- 打包 ---
    frames = pack_ble_ota(firmware, version, target)

    # --- 输出路径 ---
    output_path = args.output
    if not output_path:
        out_dir = DEFAULT_OUTPUT_DIR[args.target]
        os.makedirs(out_dir, exist_ok=True)
        base = os.path.splitext(os.path.basename(input_path))[0]
        output_path = os.path.join(out_dir, base + '.ota')

    # --- 写入 .ota 文件 ---
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    ota_data = b''.join(frames)
    with open(output_path, 'wb') as f:
        f.write(ota_data)

    # --- 打印摘要 ---
    print_summary(frames, output_path, len(firmware), version, target)

    print(f"\n[完成] 可打开 BLEDebug → 加载 {output_path} → 逐帧发送。")


if __name__ == '__main__':
    main()
