/**
 * 主单片机 OTA .ota 文件打包工具
 *
 * 流程:
 *   1. 读取 .fot 文件
 *   2. 计算 CRC32/MPEG-2, 构建 256B BIN header (MCU通信协议 §5.1 备注2)
 *   3. wrapped = 256B header + .fot 数据
 *   4. 打包为 BLE OTA 帧 (蓝牙通讯协议 v1.0.8 §5):
 *      0x0C START: target=0x01 + wrapped_size(4B BE)
 *      0x0D DATA:  target=0x01 + offset(4B BE) + chunk(128B, 16B对齐)
 *      0x0E END:   target=0x01
 *
 * 主MCU收到后:
 *   - func_lunchbox_ota.c 剥离 256B header
 *   - 只将 .fot 数据写入 flash
 *   - 验证通过后 3 秒复位
 *
 * 用法: node pack_mcu_ota.js <input.fot> [output.ota]
 */

const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

// ============================================================
// 协议常量
// ============================================================
const FRAME_HEAD = 0x55AA;
const PROTO_VERSION = 0x00;
const MSG_FLAG = 0x00;
const ERR_OK = 0x00;

const CMD_OTA_START = 0x0C;  // 升级启动 §5.1
const CMD_OTA_DATA  = 0x0D;  // 升级包传输 §5.2
const CMD_OTA_END   = 0x0E;  // 升级结束 §5.3

const TARGET_MAIN_MCU = 0x01;  // 主单片机

const CHUNK_SIZE = 128;   // 每包数据字节数
const ALIGNMENT  = 16;    // 对齐字节数

// BIN header 常量 (MCU通信协议 §5.1 备注2)
const BIN_HEADER_SIZE = 256;
const BIN_MAGIC = 0x11223344;
const BIN_PADDING = 0xFF;

// ============================================================
// CRC32/MPEG-2 (与 MCU通信协议 §5.1 一致)
// ============================================================
const CRC32_TABLE = [
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
];

function calcCRC32(data) {
    let crc = 0xFFFFFFFF;
    for (const b of data) {
        crc = ((crc << 8) >>> 0) ^ CRC32_TABLE[((crc >>> 24) ^ b) & 0xFF];
    }
    return crc >>> 0;
}

function checksum(bytes) {
    let sum = 0;
    for (const b of bytes) sum += b;
    return sum & 0xFF;
}

function buildFrame(command, data) {
    const dlen = data.length;
    const frame = [
        0x55, 0xAA,
        PROTO_VERSION,
        MSG_FLAG,
        command,
        ERR_OK,
        (dlen >> 8) & 0xFF,
        dlen & 0xFF,
        ...data,
    ];
    frame.push(checksum(frame));
    return Buffer.from(frame);
}

function chunkFirmware(data) {
    const chunks = [];
    for (let i = 0; i < data.length; i += CHUNK_SIZE) {
        let chunk = data.slice(i, i + CHUNK_SIZE);
        const rem = chunk.length % ALIGNMENT;
        if (rem !== 0) {
            const pad = Buffer.alloc(ALIGNMENT - rem, 0x00);
            chunk = Buffer.concat([chunk, pad]);
        }
        chunks.push(chunk);
    }
    return chunks;
}

function buildBinHeader(fotData, version) {
    const fwLen = fotData.length;
    const fwCrc = calcCRC32(fotData);

    const header = Buffer.alloc(BIN_HEADER_SIZE, BIN_PADDING);
    header.writeUInt32BE(BIN_MAGIC, 0);     // bytes 0-3:   magic 0x11223344
    header.writeUInt32BE(version, 4);       // bytes 4-7:   fw version
    header.writeUInt32BE(fwLen, 8);         // bytes 8-11:  fw length (不含header)
    header.writeUInt32BE(fwCrc, 12);        // bytes 12-15: fw CRC32/MPEG-2
    // bytes 16-255: padding 0xFF

    return { header, fwLen, fwCrc };
}

// ============================================================
// 主逻辑
// ============================================================

function main() {
    const srcPath = process.argv[2] || path.join(__dirname, 'test_ota.fot');
    const dstPath = process.argv[3] || srcPath.replace(/\.fot$/i, '') + '.ota';

    if (!fs.existsSync(srcPath)) {
        console.error('[错误] 文件不存在: ' + srcPath);
        process.exit(1);
    }

    const fotData = fs.readFileSync(srcPath);
    const fotSize = fotData.length;

    console.log('════════════════════════════════════════════');
    console.log('主单片机 OTA 打包工具 (BLE 协议)');
    console.log('════════════════════════════════════════════');
    console.log('.fot 文件: ' + srcPath);
    console.log('.fot 大小: ' + fotSize.toLocaleString() + ' bytes (' + (fotSize / 1024).toFixed(1) + ' KB)');
    console.log('');

    // --- Step 1: 构建 256B BIN header ---
    console.log('[1] 构建 256B BIN header (MCU通信协议 §5.1 备注2)');
    const version = 0x00000001;
    const { header, fwLen, fwCrc } = buildBinHeader(fotData, version);

    // 也用 zlib CRC32 对比验证
    const zlibCrc = zlib.crc32(fotData) >>> 0;
    console.log('    magic:   0x' + BIN_MAGIC.toString(16).toUpperCase());
    console.log('    version: 0x' + version.toString(16).padStart(8, '0').toUpperCase());
    console.log('    fw_len:  ' + fwLen.toLocaleString() + ' (0x' + fwLen.toString(16).toUpperCase() + ')');
    console.log('    fw_crc:  0x' + fwCrc.toString(16).toUpperCase() + ' (CRC32/MPEG-2)');
    console.log('    zlib_crc:0x' + zlibCrc.toString(16).toUpperCase() + ' (标准CRC32, 参考)');

    // --- Step 2: 合成 wrapped.bin ---
    const wrapped = Buffer.concat([header, fotData]);
    const wrappedSize = wrapped.length;
    console.log('');
    console.log('[2] 合成 wrapped = header + .fot');
    console.log('    总大小: ' + wrappedSize.toLocaleString() + ' bytes (' + (wrappedSize / 1024).toFixed(1) + ' KB)');
    console.log('    = 256B header + ' + fotSize.toLocaleString() + 'B .fot');

    // --- Step 3: 打包 BLE OTA 帧 ---
    console.log('');
    console.log('[3] 打包 BLE OTA 帧 (蓝牙协议 §5, target=0x01)');

    const allFrames = [];

    // --- 0x0C START ---
    const startData = [
        TARGET_MAIN_MCU,
        (wrappedSize >> 24) & 0xFF,
        (wrappedSize >> 16) & 0xFF,
        (wrappedSize >> 8) & 0xFF,
        wrappedSize & 0xFF,
    ];
    const startFrame = buildFrame(CMD_OTA_START, startData);
    allFrames.push(startFrame);
    console.log('    [START] 0x0C: target=0x01 size=' + wrappedSize.toLocaleString() +
                ' (0x' + wrappedSize.toString(16).toUpperCase() + ') frame=' + startFrame.length + 'B');
    console.log('            ' + startFrame.toString('hex').replace(/(..)/g, '$1 ').toUpperCase());

    // --- 0x0D DATA ---
    const chunks = chunkFirmware(wrapped);
    let totalData = 0;
    for (let i = 0; i < chunks.length; i++) {
        const offset = i * CHUNK_SIZE;
        const chunk = chunks[i];
        const data = [
            TARGET_MAIN_MCU,
            (offset >> 24) & 0xFF,
            (offset >> 16) & 0xFF,
            (offset >> 8) & 0xFF,
            offset & 0xFF,
            ...chunk,
        ];
        const frame = buildFrame(CMD_OTA_DATA, data);
        allFrames.push(frame);
        totalData += chunk.length;

        if (i < 3 || i === chunks.length - 1 || (i + 1) % 500 === 0) {
            console.log('    [DATA #' + (i + 1) + '/' + chunks.length + '] 0x0D:' +
                        ' off=0x' + offset.toString(16).toUpperCase().padStart(8, '0') +
                        ' len=' + chunk.length + ' frame=' + frame.length + 'B');
        } else if (i === 3 && chunks.length > 5) {
            console.log('    ... (' + (chunks.length - 4) + ' frames omitted) ...');
        }
    }
    console.log('    数据包总数: ' + chunks.length + ', 有效数据: ' + totalData.toLocaleString() + ' bytes');

    // --- 0x0E END ---
    const endData = [TARGET_MAIN_MCU];
    const endFrame = buildFrame(CMD_OTA_END, endData);
    allFrames.push(endFrame);
    console.log('    [END]   0x0E: target=0x01 frame=' + endFrame.length + 'B');
    console.log('            ' + endFrame.toString('hex').replace(/(..)/g, '$1 ').toUpperCase());

    // --- Step 4: 写入 ---
    const output = Buffer.concat(allFrames);
    const outDir = path.dirname(dstPath);
    if (!fs.existsSync(outDir)) {
        fs.mkdirSync(outDir, { recursive: true });
    }
    fs.writeFileSync(dstPath, output);

    console.log('');
    console.log('════════════════════════════════════════════');
    console.log('打包完成!');
    console.log('════════════════════════════════════════════');
    console.log('总帧数:    ' + allFrames.length + ' (1 START + ' + chunks.length + ' DATA + 1 END)');
    console.log('总大小:    ' + output.length.toLocaleString() + ' bytes (' + (output.length / 1024).toFixed(1) + ' KB)');
    console.log('输出文件:  ' + dstPath);
    console.log('');
    console.log('APP 下发流程:');
    console.log('  1. 发送 START 帧 → MCU 返回 55 AA 00 00 0C 00 00 02 01 02 10 (ERASE_DONE)');
    console.log('  2. 逐帧发送 DATA 帧 → 每帧 MCU 返回 55 AA 00 00 0D 00 00 00 0C (ACK)');
    console.log('  3. 发送 END 帧 → MCU 返回 55 AA 00 00 0E 00 00 02 01 01 11 (SUCCESS)');
    console.log('  4. MCU 校验通过后 3 秒自动复位');
    console.log('');

    // 校验所有帧
    let allOk = true;
    for (let i = 0; i < allFrames.length; i++) {
        const f = allFrames[i];
        const cs = f[f.length - 1];
        const expected = checksum(f.slice(0, -1));
        if (cs !== expected) {
            console.log('  [FAIL] Frame ' + i + ': checksum=0x' + cs.toString(16) +
                        ' expected=0x' + expected.toString(16));
            allOk = false;
        }
    }
    if (allOk) {
        console.log('所有帧校验和: ✅ 通过');
    }
    console.log('');
    console.log('Done!');
}

main();
