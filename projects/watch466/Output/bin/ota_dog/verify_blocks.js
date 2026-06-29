/**
 * BLE 传输验证 — PC 端块校验计算器
 *
 * 用法: node verify_blocks.js [文件名]
 *   node verify_blocks.js                     — 默认 ota_10pct.fot
 *   node verify_blocks.js ota_10pct.fot       — 指定文件
 *
 * 输出: 每个 512B 块的 XOR16 和 SUM32, 与 MCU 日志对比
 */

const fs = require('fs');
const path = require('path');

const BASE_DIR = __dirname;
const FOT_FILE = process.argv[2]
    ? path.join(BASE_DIR, process.argv[2])
    : path.join(BASE_DIR, 'ota_10pct.fot');

if (!fs.existsSync(FOT_FILE)) {
    console.error(`ERROR: ${FOT_FILE} not found`);
    process.exit(1);
}

const data = fs.readFileSync(FOT_FILE);
const totalSize = data.length;
const numBlocks = Math.ceil(totalSize / 512);

console.log('═'.repeat(55));
console.log(`PC-Side Block Verification: ${path.basename(FOT_FILE)}`);
console.log('═'.repeat(55));
console.log(`  File size:  ${totalSize.toLocaleString()} bytes`);
console.log(`  Blocks:     ${numBlocks} × 512B (last: ${totalSize % 512 || 512}B)`);
console.log();

// 计算每块
const expected = [];
for (let b = 0; b < numBlocks; b++) {
    const start = b * 512;
    const end = Math.min(start + 512, totalSize);
    const block = data.slice(start, end);

    let xor16 = 0;
    let sum32 = 0;
    for (let i = 0; i < 512; i++) {
        const byte = i < block.length ? block[i] : 0;  // 零填充
        xor16 ^= byte;
        sum32 += byte;
    }
    // 32-bit wrap
    sum32 = sum32 >>> 0;

    expected.push({ block: b, offset: start, xor16, sum32, actualLen: block.length });

    // 打印前 3 块 + 最后 1 块
    if (b < 3 || b === numBlocks - 1) {
        const padNote = block.length < 512 ? ` (pad from ${block.length})` : '';
        console.log(`  BLK[${b}] off=0x${start.toString(16).toUpperCase().padStart(6, '0')} xor=0x${xor16.toString(16).toUpperCase().padStart(4, '0')} sum=0x${sum32.toString(16).toUpperCase().padStart(8, '0')}${padNote}`);
    } else if (b === 3 && numBlocks > 5) {
        console.log(`  ...`);
    }
}

console.log();
console.log('═'.repeat(55));
console.log('Compare with MCU log:');
console.log('  OTA BLK[N] off=... xor=... sum=...');
console.log('═'.repeat(55));
console.log();
console.log('Expected MCU output:');
console.log(`  OTA TRANSPORT VERIFICATION`);
console.log(`    fw_size (含包头): ${(totalSize + 256).toLocaleString()} bytes`);
console.log(`    .fot 数据:        ${totalSize.toLocaleString()} bytes`);
console.log(`    期望 512B 块数:   ${numBlocks}`);
console.log(`    接收字节 (含包头): ${(totalSize + 256).toLocaleString()} / ${(totalSize + 256).toLocaleString()}`);
console.log(`    VERDICT: BLE 传输链路 ✅ 完整`);
