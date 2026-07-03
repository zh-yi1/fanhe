#!/usr/bin/env python3
"""
加热模块 OTA 测试文件生成器
生成 BLE 帧二进制文件，可用 BLEDebug 直接发送
用法: python gen_heat_ota.py [Project.bin路径] [输出路径]
"""
import sys
import os

# CRC32/MPEG-2 lookup table (from head_55aa_pack.c)
CRC32_TABLE = [
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
]

def calc_checksum(data):
    """计算帧校验和 (累加取低8位)"""
    return sum(data) & 0xFF

def calc_crc32(data):
    """计算 CRC32/MPEG-2"""
    crc = 0xFFFFFFFF
    for b in data:
        crc = (crc << 8) ^ CRC32_TABLE[((crc >> 24) ^ b) & 0xFF]
    return crc & 0xFFFFFFFF

def build_ota_start(msg_flag, target, fw_size):
    """构建 OTA_START 帧 (0x0c)"""
    data = bytearray()
    data.append(target)              # target
    data.append((fw_size >> 24) & 0xFF)  # fw_size BE
    data.append((fw_size >> 16) & 0xFF)
    data.append((fw_size >> 8) & 0xFF)
    data.append(fw_size & 0xFF)

    frame = build_frame(msg_flag, 0x0C, bytes(data))
    return frame

def build_ota_data(msg_flag, target, offset, chunk):
    """构建 OTA_DATA 帧 (0x0d)"""
    data = bytearray()
    data.append(target)              # target
    data.append((offset >> 24) & 0xFF)  # offset BE
    data.append((offset >> 16) & 0xFF)
    data.append((offset >> 8) & 0xFF)
    data.append(offset & 0xFF)
    data.extend(chunk)               # firmware data

    frame = build_frame(msg_flag, 0x0D, bytes(data))
    return frame

def build_ota_end(msg_flag, target):
    """构建 OTA_END 帧 (0x0e)"""
    data = bytearray()
    data.append(target)              # target

    frame = build_frame(msg_flag, 0x0E, bytes(data))
    return frame

def build_frame(msg_flag, cmd, data):
    """构建 BLE 协议帧"""
    frame = bytearray()
    frame.append(0x55)               # 帧头高
    frame.append(0xAA)               # 帧头低
    frame.append(0x00)               # version
    frame.append(msg_flag & 0xFF)    # msg_flag
    frame.append(cmd)                # 命令字
    frame.append(0x00)               # err_flag
    frame.append((len(data) >> 8) & 0xFF)  # data_len 高
    frame.append(len(data) & 0xFF)         # data_len 低
    frame.extend(data)               # 数据

    cs = calc_checksum(bytes(frame))
    frame.append(cs)                 # 校验和

    return bytes(frame)


def main():
    # 路径
    project_bin = r"C:\Users\31017\Desktop\otat_extracted\otat\Project.bin"
    output_bin  = r"C:\Users\31017\Desktop\ZNFH\elunchbox\projects\watch466\Output\bin\ota_dog\ota_cmds_heat.bin"

    if len(sys.argv) >= 2:
        project_bin = sys.argv[1]
    if len(sys.argv) >= 3:
        output_bin = sys.argv[2]

    # 读取固件
    with open(project_bin, "rb") as f:
        fw_data = f.read()

    fw_size = len(fw_data)
    print(f"固件: {project_bin}")
    print(f"大小: {fw_size} bytes (0x{fw_size:08X})")
    print(f"CRC32: 0x{calc_crc32(fw_data):08X}")
    print()

    TARGET  = 0x02   # 加热模块
    PACKET  = 128    # 每包 128 字节
    msg_flag = 0

    all_frames = bytearray()

    # ─── OTA_START ───
    frame = build_ota_start(msg_flag, TARGET, fw_size)
    print(f"OTA_START: msg={msg_flag} len={len(frame)}")
    all_frames.extend(frame)
    msg_flag += 1

    # ─── OTA_DATA × N ───
    offset = 0
    packet_num = 0
    while offset < fw_size:
        chunk = fw_data[offset:offset + PACKET]
        frame = build_ota_data(msg_flag, TARGET, offset, chunk)
        packet_num += 1
        if packet_num <= 3 or packet_num % 50 == 0:
            print(f"OTA_DATA #{packet_num}: msg={msg_flag} off=0x{offset:08X} len={len(chunk)}")
        all_frames.extend(frame)
        msg_flag += 1
        offset += PACKET

    # ─── OTA_END ───
    frame = build_ota_end(msg_flag, TARGET)
    print(f"OTA_END: msg={msg_flag} len={len(frame)}")
    all_frames.extend(frame)
    msg_flag += 1

    # 写入
    os.makedirs(os.path.dirname(output_bin), exist_ok=True)
    with open(output_bin, "wb") as f:
        f.write(bytes(all_frames))

    print(f"\n总帧数: {packet_num + 2} (1 START + {packet_num} DATA + 1 END)")
    print(f"总大小: {len(all_frames)} bytes")
    print(f"输出: {output_bin}")
    print("完成!")

if __name__ == "__main__":
    main()
