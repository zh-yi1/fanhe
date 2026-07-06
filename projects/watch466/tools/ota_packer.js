/**
 * OTA 固件打包工具 — 蓝牙饭盒 (BLEDebug 专用)
 * ==============================================
 * 按照蓝牙通讯协议 v1.0.8 §5，将固件文件打包为 BLE OTA 升级帧文件 (.ota)，
 * 可直接通过 BLEDebug 工具逐帧发送。
 *
 * 支持两种目标设备:
 *   mcu  — 主单片机 (.fot 文件), target=0x01
 *   heat — 加热模块 (.bin 文件), target=0x02
 *
 * 两者均使用 BLE OTA 协议 (CMD 0x0C/0x0D/0x0E)，
 * 数据区包含 256 字节 BIN 包头 (magic=0x11223344, CRC32/MPEG-2)。
 *
 * 用法:
 *   node ota_packer.js mcu --input test_ota.fot
 *   node ota_packer.js heat --input otah_Project_v005.bin
 *   node ota_packer.js heat --input fw.bin --version 1.0.0
 *   node ota_packer.js parse --input output.ota
 *
 * 输出目录 (默认):
 *   MCU  → Output/bin/ota_dog/mcu_ota/
 *   Heat → Output/bin/ota_dog/heat_ota/
 */

const fs = require('fs');
const path = require('path');

// ============================================================
// 路径配置
// ============================================================
const SCRIPT_DIR = __dirname;
const PROJECT_DIR = path.dirname(SCRIPT_DIR);  // watch466/
const OTA_DOG_DIR = path.join(PROJECT_DIR, 'Output', 'bin', 'ota_dog');
const DEFAULT_OUTPUT_DIR = {
    mcu:  path.join(OTA_DOG_DIR, 'mcu_ota'),
    heat: path.join(OTA_DOG_DIR, 'heat_ota'),
};

// ============================================================
// 协议常量 (蓝牙通讯协议 v1.0.8)
// ============================================================
const FRAME_HEAD = Buffer.from([0x55, 0xAA]);
const PROTO_VERSION = 0x00;
const MSG_FLAG = 0x00;
const ERR_OK = 0x00;

// BLE OTA 命令字 (§5)
const CMD_OTA_START = 0x0C;   // 升级启动 §5.1
const CMD_OTA_DATA  = 0x0D;   // 升级包传输 §5.2
const CMD_OTA_END   = 0x0E;   // 升级结束 §5.3

// 目标设备标识
const TARGET_MAIN_MCU    = 0x01;  // 主单片机
const TARGET_HEAT_MODULE = 0x02;  // 加热模块

// OTA 分包参数
const CHUNK_SIZE = 128;       // 每包数据字节数
const ALIGNMENT  = 16;        // 每包数据长度必须能被 16 整除

// BIN 文件头 (MCU 通信协议 §5.1 备注2)
const BIN_HEADER_SIZE = 256;
const BIN_MAGIC       = 0x11223344;
const BIN_PADDING_BYTE = 0xFF;

// ============================================================
// CRC32/MPEG-2 (非反射) — 预计算查找表
// ============================================================

// CRC32/MPEG-2 查找表 (多项式 0x04C11DB7, 非反射)
const CRC32_MPEG2_TABLE = [
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
];

function crc32_mpeg2(data) {
    /**
     * CRC32/MPEG-2 (non-reflected).
     * 多项式: 0x04C11DB7, 初始值: 0xFFFFFFFF, 终值 XOR: 0x00000000
     * 与 MCU 通信协议 §5.1 要求一致。
     */
    let crc = 0xFFFFFFFF;
    for (let i = 0; i < data.length; i++) {
        crc = (crc << 8) ^ CRC32_MPEG2_TABLE[((crc >>> 24) ^ data[i]) & 0xFF];
        crc >>>= 0;  // 保持 32-bit 无符号
    }
    return crc;
}

// ============================================================
// 帧构建
// ============================================================

function calcChecksum(data) {
    /** 校验和: 所有字节求和 mod 256 */
    let sum = 0;
    for (const b of data) sum += b;
    return sum & 0xFF;
}

function buildFrame(command, payload) {
    /**
     * 构建完整 BLE 协议帧.
     *
     * 帧结构 (蓝牙通讯协议 v1.0.8 §2.1):
     *   帧头(2B) + 版本(1B) + 消息标志(1B) + 命令字(1B) + 错误标志(1B)
     *   + 数据长度(2B, 大端) + 数据(可变) + 校验和(1B)
     */
    const dlen = payload.length;
    const frame = Buffer.alloc(9 + dlen);
    frame[0] = 0x55;
    frame[1] = 0xAA;
    frame[2] = PROTO_VERSION;
    frame[3] = MSG_FLAG;
    frame[4] = command;
    frame[5] = ERR_OK;
    frame[6] = (dlen >> 8) & 0xFF;
    frame[7] = dlen & 0xFF;
    if (dlen > 0) {
        payload.copy(frame, 8);
    }
    frame[8 + dlen] = calcChecksum(frame.slice(0, 8 + dlen));
    return frame;
}

function chunkFirmware(data, chunkSize = CHUNK_SIZE, alignment = ALIGNMENT) {
    /**
     * 将数据按指定大小分包，每包补齐到 alignment 的整数倍.
     * 最后一个包不足 alignment 时补 0x00.
     */
    const chunks = [];
    for (let i = 0; i < data.length; i += chunkSize) {
        let chunk = data.slice(i, i + chunkSize);
        const rem = chunk.length % alignment;
        if (rem !== 0) {
            const pad = Buffer.alloc(alignment - rem, 0x00);
            chunk = Buffer.concat([chunk, pad]);
        }
        chunks.push(chunk);
    }
    return chunks;
}

// ============================================================
// BIN 包头构建
// ============================================================

function buildBinHeader(firmware, version) {
    /**
     * 构建 256 字节 BIN 包头 (MCU 通信协议 §5.1 备注2).
     *
     * 头部结构:
     *   字节 0-3:   Magic 0x11223344
     *   字节 4-7:   固件版本号 (大端)
     *   字节 8-11:  固件内容总长度 (不含头部, 大端)
     *   字节 12-15: 固件 CRC32/MPEG-2 校验码 (大端)
     *   字节 16-255: 填充 0xFF
     */
    const fwLen = firmware.length;
    const crcVal = crc32_mpeg2(firmware);

    const header = Buffer.alloc(BIN_HEADER_SIZE, BIN_PADDING_BYTE);
    header.writeUInt32BE(BIN_MAGIC, 0);
    header.writeUInt32BE(version, 4);
    header.writeUInt32BE(fwLen, 8);
    header.writeUInt32BE(crcVal, 12);
    return header;
}

// ============================================================
// BLE OTA 打包 (mcu 和 heat 共用)
// ============================================================

function packBleOta(firmware, version, target) {
    /**
     * 将固件打包为 BLE OTA 升级帧 (蓝牙通讯协议 v1.0.8 §5).
     *
     * 流程:
     *   1. 构建 256 字节 BIN 包头 (magic + version + fw_len + CRC32/MPEG-2)
     *   2. 包头 + 固件 → 完整 BIN 数据
     *   3. BIN 数据按 128 字节分包 (16 字节对齐)
     *   4. 生成帧序列: START(0x0C) + DATA(0x0D)×N + END(0x0E)
     *
     * Returns:
     *   协议帧列表 (每个元素为一个 Buffer)
     */
    const frames = [];

    // --- 1. 构建完整 BIN 数据 (256B 包头 + 固件) ---
    const header = buildBinHeader(firmware, version);
    const binData = Buffer.concat([header, firmware]);
    const totalSize = binData.length;

    // --- 2. 升级启动帧 (CMD 0x0C) ---
    // 数据: 目标设备标识(1B) + 固件总字节数(4B, 大端)
    const startPayload = Buffer.alloc(5);
    startPayload[0] = target;
    startPayload.writeUInt32BE(totalSize, 1);
    frames.push(buildFrame(CMD_OTA_START, startPayload));

    // --- 3. 升级数据帧 (CMD 0x0D) ---
    // 数据: 目标设备标识(1B) + offset(4B, 大端) + 数据块
    const chunks = chunkFirmware(binData);
    for (let i = 0; i < chunks.length; i++) {
        const offset = i * CHUNK_SIZE;
        const dataPayload = Buffer.alloc(5 + chunks[i].length);
        dataPayload[0] = target;
        dataPayload.writeUInt32BE(offset, 1);
        chunks[i].copy(dataPayload, 5);
        frames.push(buildFrame(CMD_OTA_DATA, dataPayload));
    }

    // --- 4. 升级结束帧 (CMD 0x0E) ---
    // 数据: 目标设备标识(1B)
    const endPayload = Buffer.from([target]);
    frames.push(buildFrame(CMD_OTA_END, endPayload));

    return frames;
}

// ============================================================
// 版本号解析
// ============================================================

function parseVersion(verStr) {
    /**
     * 解析版本号字符串.
     *
     * 支持格式:
     *   - 十六进制: "0x00010000" → 65536
     *   - 十进制:   "65536"      → 65536
     *   - 点分:     "1.0.0"      → 0x01000000 (major<<24 | minor<<16 | patch)
     *   - ASCII:    "v005"        → 0x76303035
     */
    verStr = verStr.trim();

    // 十六进制
    if (verStr.toLowerCase().startsWith('0x')) {
        return parseInt(verStr, 16);
    }

    // ASCII 字符串 (如 "v005")
    if (!/^[\d.]+$/.test(verStr)) {
        const b = Buffer.alloc(4, 0x00);
        const ascii = Buffer.from(verStr.slice(0, 4), 'ascii');
        ascii.copy(b);
        return b.readUInt32BE(0);
    }

    // 点分格式
    if (verStr.includes('.')) {
        const parts = verStr.split('.');
        if (parts.length > 3) {
            throw new Error(`版本号最多 3 段: major.minor.patch, 实际: ${verStr}`);
        }
        const nums = parts.map(p => parseInt(p));
        while (nums.length < 3) nums.push(0);
        if (nums.some(n => n < 0 || n > 255)) {
            throw new Error(`版本号每段必须在 0-255 之间: ${verStr}`);
        }
        return (nums[0] << 24) | (nums[1] << 16) | nums[2];
    }

    // 十进制整数
    return parseInt(verStr);
}

// ============================================================
// 输出
// ============================================================

const CMD_NAMES = {
    [CMD_OTA_START]: 'START(0x0C)',
    [CMD_OTA_DATA]:  'DATA (0x0D)',
    [CMD_OTA_END]:   'END  (0x0E)',
};

const TARGET_NAMES = {
    [TARGET_MAIN_MCU]:    '主单片机 (target=0x01)',
    [TARGET_HEAT_MODULE]: '加热模块 (target=0x02)',
};

function describeFrame(frame, idx) {
    /** 单帧描述 */
    if (frame.length < 9) {
        return `[${String(idx).padStart(4)}] [ERR] 帧过短: ${frame.length}B`;
    }
    const cmd = frame[4];
    const dlen = frame.readUInt16BE(6);
    const checksum = frame[frame.length - 1];
    const expected = calcChecksum(frame.slice(0, -1));
    const cmdName = CMD_NAMES[cmd] || `UNKN(0x${cmd.toString(16).toUpperCase().padStart(2,'0')})`;
    const ck = checksum === expected ? 'OK' : `FAIL(expected 0x${expected.toString(16).toUpperCase().padStart(2,'0')})`;
    return `[${String(idx).padStart(4)}] cmd=${cmdName}  dlen=${String(dlen).padStart(4)}B  ` +
           `frame=${String(frame.length).padStart(4)}B  checksum=0x${checksum.toString(16).toUpperCase().padStart(2,'0')} ${ck}`;
}

function printSummary(frames, outputPath, fwSize, version, target) {
    /** 打印打包摘要 */
    const totalSize = frames.reduce((sum, f) => sum + f.length, 0);
    const targetName = TARGET_NAMES[target] || `未知(0x${target.toString(16).toUpperCase().padStart(2,'0')})`;
    const numData = frames.length - 2;  // 减去 START 和 END
    const badFrames = frames.filter(f => f[f.length - 1] !== calcChecksum(f.slice(0, -1))).length;

    console.log(`\n${'='.repeat(60)}`);
    console.log(`OTA 打包完成`);
    console.log(`${'='.repeat(60)}`);
    console.log(`  目标设备:   ${targetName}`);
    console.log(`  固件版本:   0x${version.toString(16).toUpperCase().padStart(8, '0')}`);
    console.log(`  原始固件:   ${fwSize} bytes (${(fwSize / 1024).toFixed(1)} KB)`);
    console.log(`  BIN 包头:   ${BIN_HEADER_SIZE} bytes`);
    console.log(`  打包大小:   ${totalSize} bytes (${(totalSize / 1024).toFixed(1)} KB)`);
    console.log(`  总帧数:     ${frames.length} (1 START + ${numData} DATA + 1 END)`);
    console.log(`  帧校验:     ${badFrames === 0 ? '全部通过' : badFrames + ' 帧失败!'}`);

    console.log(`\n帧列表 (前3 + 后2):`);
    for (let i = 0; i < Math.min(3, frames.length); i++) {
        console.log(`  ${describeFrame(frames[i], i)}`);
    }
    if (frames.length > 5) {
        console.log(`  ... 省略 ${frames.length - 5} 帧 ...`);
    }
    for (let i = Math.max(3, frames.length - 2); i < frames.length; i++) {
        console.log(`  ${describeFrame(frames[i], i)}`);
    }

    console.log(`\n输出文件: ${outputPath}`);
}

// ============================================================
// OTA 文件解析
// ============================================================

function parseOtaFile(filepath) {
    /** 解析已有的 .ota 文件, 按帧边界拆分 */
    const data = fs.readFileSync(filepath);
    const frames = [];
    let pos = 0;

    while (pos < data.length) {
        if (pos + 9 > data.length) {
            console.log(`[警告] 位置 ${pos}: 剩余 ${data.length - pos} 字节不足以构成完整帧, 停止解析`);
            break;
        }

        if (data[pos] !== 0x55 || data[pos + 1] !== 0xAA) {
            console.log(`[错误] 位置 ${pos}: 帧头不匹配 (期望 0x55AA, 实际 ` +
                        `0x${data[pos].toString(16).toUpperCase().padStart(2, '0')}` +
                        `${data[pos+1].toString(16).toUpperCase().padStart(2, '0')}), 停止解析`);
            break;
        }

        const dlen = data.readUInt16BE(pos + 6);
        const frameTotal = 9 + dlen;

        if (pos + frameTotal > data.length) {
            console.log(`[错误] 位置 ${pos}: 帧长度 ${frameTotal} 超出文件范围, 停止解析`);
            break;
        }

        frames.push(data.slice(pos, pos + frameTotal));
        pos += frameTotal;
    }

    return frames;
}

function cmdParse(args) {
    /** 解析 .ota 文件 */
    const frames = parseOtaFile(args.input);
    if (frames.length === 0) {
        console.log('[错误] 未能解析任何帧');
        return;
    }

    const totalSize = frames.reduce((sum, f) => sum + f.length, 0);
    console.log(`解析 ${args.input}: 共 ${frames.length} 帧, ${totalSize} bytes\n`);
    for (let i = 0; i < frames.length; i++) {
        const f = frames[i];
        const cmd = f[4];
        const dlen = f.readUInt16BE(6);
        const cs = f[f.length - 1];
        const expected = calcChecksum(f.slice(0, -1));
        const ok = cs === expected ? 'OK' : `FAIL(expected 0x${expected.toString(16).toUpperCase().padStart(2,'0')})`;
        console.log(`[${String(i).padStart(4)}] cmd=0x${cmd.toString(16).toUpperCase().padStart(2,'0')}  ` +
                    `dlen=${String(dlen).padStart(3)}B  ` +
                    `frame=${String(f.length).padStart(3)}B  ` +
                    `checksum=0x${cs.toString(16).toUpperCase().padStart(2,'0')} ${ok}`);
    }
}

// ============================================================
// CLI 主入口
// ============================================================

function printUsage() {
    console.log(`OTA 固件打包工具 — 蓝牙饭盒 (BLEDebug 专用)

用法:
  node ota_packer.js mcu --input <file.fot> [--version <ver>] [--output <file.ota>]
  node ota_packer.js heat --input <file.bin> [--version <ver>] [--output <file.ota>]
  node ota_packer.js parse --input <file.ota>

示例:
  node ota_packer.js mcu --input test_ota.fot
  node ota_packer.js heat --input otah_Project_v005.bin
  node ota_packer.py heat --input fw.bin --version 1.0.0
  node ota_packer.py parse --input output.ota

输出目录 (默认):
  MCU  → Output/bin/ota_dog/mcu_ota/
  Heat → Output/bin/ota_dog/heat_ota/`);
}

function parseArgs(argv) {
    /** 简易命令行参数解析 (兼容 node < 18) */
    const args = { _: [] };
    let i = 0;
    while (i < argv.length) {
        const arg = argv[i];
        if (arg === '--input' || arg === '-i') {
            args.input = argv[++i];
        } else if (arg === '--output' || arg === '-o') {
            args.output = argv[++i];
        } else if (arg === '--version' || arg === '-v') {
            args.version = argv[++i];
        } else if (arg === '--help' || arg === '-h') {
            args.help = true;
        } else {
            args._.push(arg);
        }
        i++;
    }
    return args;
}

function main() {
    const rawArgs = process.argv.slice(2);
    const args = parseArgs(rawArgs);

    if (args.help || args._.length === 0 || (args._[0] !== 'mcu' && args._[0] !== 'heat' && args._[0] !== 'parse')) {
        printUsage();
        process.exit(args.help ? 0 : 1);
    }

    const target = args._[0];

    // --- parse 子命令 ---
    if (target === 'parse') {
        if (!args.input) {
            console.log('[错误] parse 需要 --input <file.ota>');
            process.exit(1);
        }
        cmdParse(args);
        return;
    }

    // --- 检查输入文件 ---
    if (!args.input) {
        console.log(`[错误] ${target} 需要 --input <file>`);
        process.exit(1);
    }

    if (!fs.existsSync(args.input)) {
        console.log(`[错误] 文件不存在: ${args.input}`);
        process.exit(1);
    }

    // --- 解析版本号 ---
    const verStr = args.version || '0x00000001';
    let version;
    try {
        version = parseVersion(verStr);
    } catch (e) {
        console.log(`[错误] 版本号解析失败: ${e.message}`);
        process.exit(1);
    }

    // --- 读取固件 ---
    const firmware = fs.readFileSync(args.input);
    if (firmware.length === 0) {
        console.log('[错误] 固件文件为空');
        process.exit(1);
    }

    // --- 确定目标 ---
    const targetVal = target === 'mcu' ? TARGET_MAIN_MCU : TARGET_HEAT_MODULE;

    console.log(`[info] 读取固件: ${args.input}`);
    console.log(`       文件大小: ${firmware.length} bytes (${(firmware.length / 1024).toFixed(1)} KB)`);
    console.log(`       版本号:   0x${version.toString(16).toUpperCase().padStart(8, '0')}`);
    console.log(`       目标设备: ${target === 'mcu' ? '主单片机 (0x01)' : '加热模块 (0x02)'}`);
    console.log(`       协议:     BLE OTA (0x0C/0x0D/0x0E)`);
    console.log(`       CRC32:    0x${crc32_mpeg2(firmware).toString(16).toUpperCase().padStart(8, '0')} (MPEG-2)`);

    // --- 打包 ---
    const frames = packBleOta(firmware, version, targetVal);

    // --- 输出路径 ---
    let outputPath = args.output;
    if (!outputPath) {
        const outDir = DEFAULT_OUTPUT_DIR[target];
        fs.mkdirSync(outDir, { recursive: true });
        const base = path.basename(args.input, path.extname(args.input));
        outputPath = path.join(outDir, base + '.ota');
    }

    // --- 写入 ---
    const outDir = path.dirname(outputPath);
    fs.mkdirSync(outDir, { recursive: true });
    const otaData = Buffer.concat(frames);
    fs.writeFileSync(outputPath, otaData);

    // --- 摘要 ---
    printSummary(frames, outputPath, firmware.length, version, targetVal);

    console.log(`\n[完成] 可打开 BLEDebug → 加载 ${outputPath} → 逐帧发送。`);
}

main();
