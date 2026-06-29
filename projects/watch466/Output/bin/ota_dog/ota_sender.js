/**
 * OTA 发送辅助工具
 *
 * 用法:
 *   node ota_sender.js split    — 将大文件切成小块 (如果 BLEDebug 文件发送有大小限制)
 *   node ota_sender.js info     — 打印 OTA 文件信息
 *   node ota_sender.js cmds     — 打印关键 BLE 指令 (手动发送用)
 */

const fs = require('fs');
const path = require('path');

const CMD_BIN_FILE = path.join(__dirname, 'ota_cmds_bin.bin');
const CMD_HEX_FILE = path.join(__dirname, 'ota_cmds_hex.txt');
const CHUNK_DIR     = path.join(__dirname, 'ota_chunks');

// ============================================================
function checksum(hexLine) {
    const bytes = hexLine.trim().split(/\s+/).map(h => parseInt(h, 16));
    let sum = 0;
    for (let i = 0; i < bytes.length - 1; i++) sum += bytes[i]; // last byte is chk
    const expected = sum & 0xFF;
    const actual = bytes[bytes.length - 1];
    if (expected !== actual) {
        console.log(`  WARN: chk expected=0x${expected.toString(16)} got=0x${actual.toString(16)}`);
    }
    return expected === actual;
}

// ============================================================
function cmdInfo() {
    console.log('═'.repeat(60));
    console.log('OTA 关键指令 (逐条手动发送用)');
    console.log('═'.repeat(60));

    const lines = fs.readFileSync(CMD_HEX_FILE, 'utf-8').trim().split('\n').filter(l => l.trim());
    const total = lines.length;

    // 0x0c start
    console.log('\n--- Step 1: OTA Start (0x0c) ---');
    console.log(lines[0]);
    checksum(lines[0]);
    console.log('  期望应答: 55 AA 00 00 0C 00 00 02 01 02 10');

    // 首包 0x0d
    console.log('\n--- Step 2: 首包 0x0d (offset=0) ---');
    console.log(lines[1]);
    checksum(lines[1]);

    // 统计
    console.log(`\n... 中间 ${total - 4} 包 0x0d 数据 (offset 128 ~ ${(total-4)*128}) ...`);

    // 尾包 0x0d
    console.log('\n--- Step 2: 尾包 0x0d ---');
    console.log(lines[total - 2]);
    checksum(lines[total - 2]);

    // 0x0e end
    console.log('\n--- Step 3: OTA End (0x0e) ---');
    console.log(lines[total - 1]);
    checksum(lines[total - 1]);
    console.log('  期望应答 (成功): 55 AA 00 00 0E 00 00 02 01 01 11');
    console.log('  期望应答 (失败): 55 AA 00 00 0E 00 00 02 01 00 10');

    console.log('\n' + '═'.repeat(60));
    console.log(`完整指令文件: ${CMD_HEX_FILE}`);
    console.log(`二进制帧文件: ${CMD_BIN_FILE}`);
    console.log(`总帧数: ${total} | 二进制大小: ${fs.statSync(CMD_BIN_FILE).size} bytes`);
    console.log('═'.repeat(60));
}

// ============================================================
function fileInfo() {
    const bin = fs.statSync(CMD_BIN_FILE);
    console.log('═'.repeat(60));
    console.log('OTA 文件信息');
    console.log('═'.repeat(60));
    console.log(`test.fot          : 566,272 bytes (原始 FOTA 固件)`);
    console.log(`test_wrapped.bin   : 566,528 bytes (256B header + .fot)`);
    console.log(`ota_cmds_hex.txt   : hex 指令文本 (4,428 行)`);
    console.log(`ota_cmds_bin.bin   : ${bin.size} bytes (二进制帧拼接)`);
    console.log('');
    console.log(`BLE 帧明细:`);
    console.log(`  0x0c OTA Start   : 1 帧 (14 bytes)`);
    console.log(`  0x0d OTA Data    : 4,426 帧 × 142 bytes = 628,492 bytes`);
    console.log(`  0x0e OTA End     : 1 帧 (10 bytes)`);
    console.log(`  Total            : 4,428 帧 = ${bin.size} bytes`);
    console.log('═'.repeat(60));
}

// ============================================================
function splitChunks(chunkFrames) {
    // 将 ota_cmds_bin.bin 按帧切分成小块
    if (!fs.existsSync(CHUNK_DIR)) fs.mkdirSync(CHUNK_DIR, { recursive: true });

    const allBytes = fs.readFileSync(CMD_BIN_FILE);
    const lines = fs.readFileSync(CMD_HEX_FILE, 'utf-8').trim().split('\n').filter(l => l.trim());

    // 解析每帧的起止位置
    const frames = [];
    let pos = 0;
    for (const line of lines) {
        const bytes = line.trim().split(/\s+/).map(h => parseInt(h, 16));
        frames.push({ offset: pos, size: bytes.length, hex: line });
        pos += bytes.length;
    }

    console.log(`Total frames: ${frames.length}, total bytes: ${pos}`);

    // 按 chunkFrames 帧一组切分
    const totalChunks = Math.ceil(frames.length / chunkFrames);
    console.log(`Splitting into ${totalChunks} chunks of ${chunkFrames} frames each...\n`);

    for (let c = 0; c < totalChunks; c++) {
        const startIdx = c * chunkFrames;
        const endIdx = Math.min(startIdx + chunkFrames, frames.length);
        const chunkFramesArr = frames.slice(startIdx, endIdx);
        const chunkBytes = chunkFramesArr.map(f => {
            const b = Buffer.from(f.hex.trim().split(/\s+/).map(h => parseInt(h, 16)));
            return b;
        });
        const chunkBin = Buffer.concat(chunkBytes);

        const chunkName = `chunk_${String(c + 1).padStart(3, '0')}.bin`;
        const chunkPath = path.join(CHUNK_DIR, chunkName);
        fs.writeFileSync(chunkPath, chunkBin);

        const firstCmd = chunkFramesArr[0];
        const lastCmd = chunkFramesArr[chunkFramesArr.length - 1];
        const firstBytes = firstCmd.hex.trim().split(/\s+/).map(h => parseInt(h, 16));
        const lastBytes = lastCmd.hex.trim().split(/\s+/).map(h => parseInt(h, 16));
        const fc = firstBytes[4].toString(16).padStart(2, '0');
        const lc = lastBytes[4].toString(16).padStart(2, '0');

        console.log(`  ${chunkName}: ${chunkFramesArr.length} frames, ${chunkBin.length} bytes  [0x${fc}..0x${lc}]`);
    }

    console.log(`\nChunks written to: ${CHUNK_DIR}`);
    console.log('Send each chunk file in order via BLEDebug file send.');
}

// ============================================================
function main() {
    const cmd = process.argv[2] || 'info';

    if (!fs.existsSync(CMD_BIN_FILE)) {
        console.error('ERROR: ota_cmds_bin.bin not found.');
        console.error('Run: node generate_ota_cmds.js first.');
        process.exit(1);
    }

    switch (cmd) {
    case 'info':
        fileInfo();
        break;
    case 'cmds':
        cmdInfo();
        break;
    case 'split':
        const n = parseInt(process.argv[3]) || 200; // 默认每 200 帧一个文件
        splitChunks(n);
        break;
    default:
        console.log('Usage: node ota_sender.js [info|cmds|split]');
        console.log('  info   — show file information');
        console.log('  cmds   — print key BLE commands');
        console.log('  split  — split binary into smaller chunks (for BLEDebug file send)');
        break;
    }
}

main();
