/**
 * 小包 OTA 测试生成器 (1/10 固件)
 *
 * 用法: node generate_small_ota.js [比例]
 *   node generate_small_ota.js       — 默认 1/10
 *   node generate_small_ota.js 4     — 1/4
 *   node generate_small_ota.js 20    — 1/20
 *
 * 流程:
 *   1. 读取 ota_test.fot (Downloader 转换的原版 FOTA 文件)
 *   2. 截取前 1/N (16 字节对齐)
 *   3. 包 256B header (magic=0x11223344, fw_len, CRC32)
 *   4. 生成 BLE OTA 指令帧 → hex + bin
 */

const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const BASE_DIR = __dirname;
const FOT_FILE = path.join(BASE_DIR, 'ota_test.fot');

// ============================================================
// 参数
// ============================================================
const RATIO = parseInt(process.argv[2]) || 10;  // 默认 1/10
const BLE_PKT_SIZE = 128;
const TARGET = 0x01;

// 输入/输出文件名
const suffix = RATIO === 10 ? '10pct' : `1of${RATIO}`;
const FOT_SMALL = path.join(BASE_DIR, `ota_${suffix}.fot`);
const WRAPPED_BIN = path.join(BASE_DIR, `test_${suffix}_wrapped.bin`);
const HEX_OUT = path.join(BASE_DIR, `ota_cmds_${suffix}_hex.txt`);
const BIN_OUT = path.join(BASE_DIR, `ota_cmds_${suffix}_bin.bin`);

// ============================================================
// 工具函数
// ============================================================
function checksum(bytes) {
    let sum = 0;
    for (const b of bytes) sum += b;
    return sum & 0xFF;
}

function buildFrame(cmd, msgFlag, err, data) {
    const dlen = data.length;
    const frame = [
        0x55, 0xAA,
        0x00,
        msgFlag,
        cmd,
        err,
        (dlen >> 8) & 0xFF,
        dlen & 0xFF,
        ...data,
    ];
    const chk = checksum(frame);
    frame.push(chk);
    return Buffer.from(frame);
}

function formatHex(buf) {
    return Array.from(buf).map(b => b.toString(16).toUpperCase().padStart(2, '0')).join(' ');
}

// ============================================================
// Step 1: 截取 .fot 文件
// ============================================================
console.log('═'.repeat(60));
console.log(`OTA Small Test Generator (1/${RATIO})`);
console.log('═'.repeat(60));

const fullFot = fs.readFileSync(FOT_FILE);
console.log(`\n[1] 原版 .fot: ${fullFot.length.toLocaleString()} bytes`);

// 截取 + 16 字节对齐
let smallSize = Math.floor(fullFot.length / RATIO);
smallSize = smallSize - (smallSize % 16);  // 16 字节对齐
const smallFot = fullFot.slice(0, smallSize);

fs.writeFileSync(FOT_SMALL, smallFot);
console.log(`    截取后:   ${smallFot.length.toLocaleString()} bytes → ${path.basename(FOT_SMALL)}`);
console.log(`    16B 对齐: ${smallFot.length % 16 === 0 ? 'OK ✅' : 'FAIL ❌'}`);

// ============================================================
// Step 2: 包 256B header
// ============================================================
console.log(`\n[2] 包 256B header...`);

// 计算 CRC32
const fwCrc = zlib.crc32(smallFot) >>> 0;

// 构建 header: [magic:4][ver:4][fw_len:4][fw_crc:4][padding:240]
const header = Buffer.alloc(256);
header.writeUInt32BE(0x11223344, 0);    // magic
header.writeUInt32BE(0x00000001, 4);    // fw_ver
header.writeUInt32BE(smallFot.length, 8); // fw_len (.fot 大小, 不含 header)
header.writeUInt32BE(fwCrc, 12);        // fw_crc
// 其余为 padding (0x00)

const wrapped = Buffer.concat([header, smallFot]);
fs.writeFileSync(WRAPPED_BIN, wrapped);

console.log(`    magic:   0x11223344`);
console.log(`    fw_ver:  0x00000001`);
console.log(`    fw_len:  ${smallFot.length} (0x${smallFot.length.toString(16).toUpperCase()})`);
console.log(`    fw_crc:  0x${fwCrc.toString(16).toUpperCase()}`);
console.log(`    wrapped: ${wrapped.length.toLocaleString()} bytes → ${path.basename(WRAPPED_BIN)}`);

// 验证
const verifyMagic = wrapped.readUInt32BE(0);
const verifyLen = wrapped.readUInt32BE(8);
const verifyCrc = wrapped.readUInt32BE(12);
const actualCrc = zlib.crc32(wrapped.slice(256)) >>> 0;
if (verifyMagic !== 0x11223344 || verifyCrc !== actualCrc || verifyLen !== smallFot.length) {
    console.error('    ERROR: Header verification failed!');
    process.exit(1);
}
console.log(`    CRC32 verification: OK ✅`);

// ============================================================
// Step 3: 生成 BLE OTA 指令帧
// ============================================================
console.log(`\n[3] 生成 BLE OTA 指令帧...\n`);

const totalSize = wrapped.length;
const numPackets = Math.ceil(totalSize / BLE_PKT_SIZE);
const lastPktSize = totalSize % BLE_PKT_SIZE || BLE_PKT_SIZE;

console.log(`    total_size: ${totalSize} bytes (0x${totalSize.toString(16).toUpperCase()})`);
console.log(`    packets:    ${numPackets} × ${BLE_PKT_SIZE}B (last: ${lastPktSize}B)`);

// 16 字节对齐检查
for (let i = 0; i < numPackets; i++) {
    const sz = (i === numPackets - 1) ? lastPktSize : BLE_PKT_SIZE;
    if (sz & 0x0F) {
        console.error(`    ERROR: Packet[${i}] size ${sz} not 16-byte aligned!`);
        process.exit(1);
    }
}
console.log(`    16B alignment: OK ✅`);

const allFrames = [];
const hexLines = [];
let msgFlag = 0;

// --- 0x0C OTA Start ---
console.log(`\n--- OTA Start (0x0C) ---`);
{
    const fwSize = totalSize;
    const data = [
        TARGET,
        (fwSize >> 24) & 0xFF,
        (fwSize >> 16) & 0xFF,
        (fwSize >> 8) & 0xFF,
        fwSize & 0xFF,
    ];
    const frame = buildFrame(0x0C, msgFlag, 0x00, data);
    allFrames.push(frame);
    hexLines.push(formatHex(frame));
    console.log(`  TX [${frame.length}B]: ${formatHex(frame)}`);
    msgFlag++;
}

// --- 0x0D OTA Data ---
console.log(`\n--- OTA Data (0x0D) × ${numPackets} ---`);
for (let i = 0; i < numPackets; i++) {
    const offset = i * BLE_PKT_SIZE;
    const pktSize = (i === numPackets - 1) ? lastPktSize : BLE_PKT_SIZE;
    const chunk = wrapped.slice(offset, offset + pktSize);

    const data = [
        TARGET,
        (offset >> 24) & 0xFF,
        (offset >> 16) & 0xFF,
        (offset >> 8) & 0xFF,
        offset & 0xFF,
        ...chunk,
    ];

    const frame = buildFrame(0x0D, msgFlag, 0x00, data);
    allFrames.push(frame);
    hexLines.push(formatHex(frame));

    if (i === 0) {
        console.log(`  [0]     offset=0x00000000 len=${pktSize}`);
        console.log(`          ${formatHex(frame).slice(0, 80)}...`);
    } else if (i === numPackets - 1) {
        console.log(`  [${i}] offset=0x${offset.toString(16).toUpperCase().padStart(8, '0')} len=${pktSize}`);
        const fh = formatHex(frame);
        console.log(`          ${fh.slice(0, 80)}...`);
    } else if (i % 100 === 0) {
        console.log(`  [${i}]   offset=0x${offset.toString(16).toUpperCase().padStart(8, '0')}`);
    }
    msgFlag++;
}

// --- 0x0E OTA End ---
console.log(`\n--- OTA End (0x0E) ---`);
{
    const data = [TARGET];
    const frame = buildFrame(0x0E, msgFlag, 0x00, data);
    allFrames.push(frame);
    hexLines.push(formatHex(frame));
    console.log(`  TX [${frame.length}B]: ${formatHex(frame)}`);
}
console.log('');

// ============================================================
// Step 4: 写出文件
// ============================================================
fs.writeFileSync(HEX_OUT, hexLines.join('\n') + '\n', 'utf-8');
const binBuf = Buffer.concat(allFrames);
fs.writeFileSync(BIN_OUT, binBuf);

// ============================================================
// 摘要
// ============================================================
console.log('═'.repeat(60));
console.log('SUMMARY');
console.log('═'.repeat(60));
console.log(`  原始 .fot:       ${fullFot.length.toLocaleString()} bytes`);
console.log(`  截取 .fot:       ${smallFot.length.toLocaleString()} bytes (1/${RATIO})`);
console.log(`  包 header:       ${wrapped.length.toLocaleString()} bytes`);
console.log(`  BLE 总帧数:      ${allFrames.length}`);
console.log(`    - 0x0C Start:  1`);
console.log(`    - 0x0D Data:   ${numPackets}`);
console.log(`    - 0x0E End:    1`);
console.log(`  BLE 总字节:      ${binBuf.length.toLocaleString()}`);
console.log(``);
console.log(`  输出文件:`);
console.log(`    ${path.basename(FOT_SMALL)}`);
console.log(`    ${path.basename(WRAPPED_BIN)}`);
console.log(`    ${path.basename(HEX_OUT)}`);
console.log(`    ${path.basename(BIN_OUT)}  ← 用 BLEDebug 发送这个`);
console.log(``);
console.log(`  测试步骤:`);
console.log(`    1. 发送 OTA Start 指令`);
console.log(`       TX: ${hexLines[0]}`);
console.log(`       RX: 55 AA 00 00 0C 00 00 02 01 02 10 (ERASE_DONE)`);
console.log(`    2. 用 BLEDebug "文件发送" 发送 ${path.basename(BIN_OUT)}`);
console.log(`    3. 观察最后一条 0x0E 应答`);
console.log(`       RX: 55 AA 00 00 0E 00 00 02 01 01 11 (SUCCESS)`);
console.log('═'.repeat(60));
