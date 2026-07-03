/**
 * OTA BLE 指令生成器
 * 读取 test_wrapped.bin (256B header + .fot)，生成蓝牙通讯协议1.0.7 §5 的 OTA 指令序列
 *
 * 用法: node generate_ota_cmds.js
 * 输出:
 *   ota_cmds_hex.txt   — 每行一条完整 hex 帧 (供手动测试工具逐条发送)
 *   ota_cmds_bin.bin   — 纯二进制帧拼接 (供脚本/工具直接发送)
 */

const fs = require('fs');
const path = require('path');

const BASE_DIR = __dirname;
const WRAPPED_BIN = path.join(BASE_DIR, 'test_wrapped.bin');
const HEX_OUT = path.join(BASE_DIR, 'ota_cmds_hex.txt');
const BIN_OUT = path.join(BASE_DIR, 'ota_cmds_bin.bin');

// 协议常量
const TARGET = 0x01;           // 主单片机
const BLE_PKT_SIZE = 128;      // 每包 128 字节 (蓝牙通讯协议1.0.7 §5.2 默认值)
const BLE_MTU = 197;           // MTU 200, 实际可用 197

// --- 工具函数 ---
function checksum(bytes) {
    let sum = 0;
    for (const b of bytes) sum += b;
    return sum & 0xFF;  // mod 256
}

function buildFrame(cmd, msgFlag, err, data) {
    // 帧格式: [header:2][ver:1][flag:1][cmd:1][err:1][dlen:2 BE][data][chk:1]
    const dlen = data.length;
    const frame = [
        0x55, 0xAA,         // 帧头
        0x00,               // 版本
        msgFlag,            // 消息标志
        cmd,                // 命令字
        err,                // 错误标志
        (dlen >> 8) & 0xFF, // 数据长度高字节 (大端)
        dlen & 0xFF,        // 数据长度低字节 (大端)
        ...data,            // 数据
    ];
    const chk = checksum(frame);
    frame.push(chk);
    return Buffer.from(frame);
}

function formatHex(buf) {
    return Array.from(buf).map(b => b.toString(16).toUpperCase().padStart(2, '0')).join(' ');
}

// --- 主流程 ---
console.log('=== OTA BLE Command Generator ===\n');

// 1. 读取包裹后的 .bin 文件
const wrapped = fs.readFileSync(WRAPPED_BIN);
const totalSize = wrapped.length;
console.log(`Wrapped .bin: ${totalSize} bytes (0x${totalSize.toString(16).toUpperCase()})`);
console.log(`  = 256-byte header + ${totalSize - 256}-byte .fot`);

// 验证 header magic
const magic = wrapped.readUInt32BE(0);
const hdrVer = wrapped.readUInt32BE(4);
const hdrLen = wrapped.readUInt32BE(8);
const hdrCrc = wrapped.readUInt32BE(12);
console.log(`  Header: magic=0x${magic.toString(16).toUpperCase()} ver=0x${hdrVer.toString(16).toUpperCase()} fw_len=${hdrLen} crc=0x${hdrCrc.toString(16).toUpperCase()}`);

if (magic !== 0x11223344) {
    console.error('ERROR: Bad magic in header!');
    process.exit(1);
}

// 验证 CRC32 (使用 Node.js zlib)
const zlib = require('zlib');
const fotData = wrapped.slice(256);
const actualCrc = zlib.crc32(fotData) >>> 0;  // unsigned
if (actualCrc !== hdrCrc) {
    console.error(`ERROR: CRC mismatch! header=0x${hdrCrc.toString(16).toUpperCase()} actual=0x${actualCrc.toString(16).toUpperCase()}`);
    process.exit(1);
}
console.log('  CRC32 verification: OK\n');

// 2. 计算分包
const numPackets = Math.ceil(totalSize / BLE_PKT_SIZE);
const lastPktSize = totalSize % BLE_PKT_SIZE || BLE_PKT_SIZE;
console.log(`Packets: ${numPackets} × ${BLE_PKT_SIZE}B (last: ${lastPktSize}B)`);

// 验证 16 字节对齐
for (let i = 0; i < numPackets; i++) {
    const pktSize = (i === numPackets - 1) ? lastPktSize : BLE_PKT_SIZE;
    if (pktSize & 0x0F) {
        console.error(`ERROR: Packet ${i} size ${pktSize} not 16-byte aligned!`);
        process.exit(1);
    }
}
console.log('  16-byte alignment: OK\n');

// 3. 生成所有 BLE 帧
const allFrames = [];
let hexLines = [];
let msgFlag = 0;

// --- 3a. OTA Start (0x0c) ---
console.log('--- Step 1: OTA Start (0x0c) ---');
{
    const fwSize = totalSize;
    const data = [
        TARGET,                     // target=0x01 (主单片机)
        (fwSize >> 24) & 0xFF,      // fw_size 大端
        (fwSize >> 16) & 0xFF,
        (fwSize >> 8) & 0xFF,
        fwSize & 0xFF,
    ];
    const frame = buildFrame(0x0C, msgFlag, 0x00, data);
    allFrames.push(frame);
    hexLines.push(formatHex(frame));
    console.log(`  TX: ${formatHex(frame)}`);
    console.log(`  Expect RX: 55 AA 00 00 0C 00 00 02 01 02 10 (ERASE_DONE)`);
    msgFlag++;
}
console.log('');

// --- 3b. OTA Data (0x0d) ---
console.log('--- Step 2: OTA Data (0x0d) — total ' + numPackets + ' packets ---');
for (let i = 0; i < numPackets; i++) {
    const offset = i * BLE_PKT_SIZE;
    const pktSize = (i === numPackets - 1) ? lastPktSize : BLE_PKT_SIZE;
    const chunk = wrapped.slice(offset, offset + pktSize);

    // data = target(1B) + offset(4B BE) + upgrade_data(N bytes)
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

    // 打印进度
    if (i === 0) {
        console.log(`  Packet[0] offset=0x00000000 len=${pktSize}`);
        console.log(`  TX: ${formatHex(frame)}`);
    } else if (i === numPackets - 1) {
        console.log(`  ...`);
        console.log(`  Packet[${numPackets - 1}] offset=0x${offset.toString(16).toUpperCase().padStart(8, '0')} len=${pktSize}`);
        console.log(`  TX: ${formatHex(frame)}`);
    } else if (i % 500 === 0) {
        console.log(`  ... Packet[${i}] offset=0x${offset.toString(16).toUpperCase().padStart(8, '0')} ...`);
    }
    msgFlag++;
}
console.log('  Expect RX per packet: 55 AA 00 00 0D 00 00 00 0C (ACK)\n');

// --- 3c. OTA End (0x0e) ---
console.log('--- Step 3: OTA End (0x0e) ---');
{
    const data = [TARGET];  // target=0x01
    const frame = buildFrame(0x0E, msgFlag, 0x00, data);
    allFrames.push(frame);
    hexLines.push(formatHex(frame));
    console.log(`  TX: ${formatHex(frame)}`);
    console.log(`  Expect RX: 55 AA 00 00 0E 00 00 02 01 01 11 (SUCCESS)`);
    console.log('  Then: MCU resets after 3 seconds');
}
console.log('');

// 4. 写出文件
fs.writeFileSync(HEX_OUT, hexLines.join('\n') + '\n', 'utf-8');
console.log(`Hex commands written: ${HEX_OUT} (${hexLines.length} lines)`);

const binBuf = Buffer.concat(allFrames);
fs.writeFileSync(BIN_OUT, binBuf);
console.log(`Binary frames written: ${BIN_OUT} (${binBuf.length} bytes)`);

// 5. 摘要
console.log('\n=== Summary ===');
console.log(`Total BLE frames: ${allFrames.length}`);
console.log(`  - 0x0c (OTA Start):  1 frame`);
console.log(`  - 0x0d (OTA Data):   ${numPackets} frames (${(numPackets * BLE_PKT_SIZE / 1024).toFixed(1)} KB total)`);
console.log(`  - 0x0e (OTA End):    1 frame`);
console.log(`Total BLE TX bytes: ${binBuf.length}`);
console.log(`Estimated Time @ 1pkt/50ms: ~${(numPackets * 0.05).toFixed(0)}s`);
