/**
 * 加热模块 OTA .ota 文件打包工具
 * 按照蓝牙通讯协议 v1.0.8 §5 (BLE OTA)
 *   - 目标设备: 加热模块 (target=0x02)
 *   - 命令字: 0x0C (升级启动) / 0x0D (升级包传输) / 0x0E (升级结束)
 *   - 每包 128 字节, 对齐到 16 字节边界
 *
 * 用法: node pack_heat_ota.js <input.bin> [output.ota]
 */

const fs = require('fs');
const path = require('path');

// ============================================================
// 协议常量 (蓝牙通讯协议 v1.0.8)
// ============================================================
const FRAME_HEAD = 0x55AA;
const PROTO_VERSION = 0x00;
const MSG_FLAG = 0x00;
const ERR_OK = 0x00;

const CMD_OTA_START = 0x0C;  // 升级启动 §5.1
const CMD_OTA_DATA  = 0x0D;  // 升级包传输 §5.2
const CMD_OTA_END   = 0x0E;  // 升级结束 §5.3

const TARGET_HEAT = 0x02;    // 加热模块

const CHUNK_SIZE = 128;      // 每包数据字节数
const ALIGNMENT  = 16;       // 对齐字节数

// ============================================================
// 工具函数
// ============================================================

/** 校验和: 所有字节求和 mod 256 */
function checksum(bytes) {
    let sum = 0;
    for (const b of bytes) sum += b;
    return sum & 0xFF;
}

/** 构建协议帧 */
function buildFrame(command, data) {
    const dlen = data.length;
    const frame = [
        0x55, 0xAA,            // 帧头
        PROTO_VERSION,          // 版本
        MSG_FLAG,               // 消息标志
        command,                // 命令字
        ERR_OK,                 // 错误标志
        (dlen >> 8) & 0xFF,    // 数据长度 高字节
        dlen & 0xFF,           // 数据长度 低字节
        ...data,                // 数据
    ];
    frame.push(checksum(frame)); // 校验和
    return Buffer.from(frame);
}

/** 分包: 每包 CHUNK_SIZE 字节, 对齐到 ALIGNMENT */
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

// ============================================================
// 主逻辑
// ============================================================

function main() {
    const srcPath = process.argv[2] || 'C:/Users/31017/Desktop/otah_Project_v005.bin';
    const dstPath = process.argv[3] || srcPath.replace(/\.bin$/i, '') + '.ota';

    if (!fs.existsSync(srcPath)) {
        console.error('[错误] 文件不存在: ' + srcPath);
        process.exit(1);
    }

    // 读取固件
    const firmware = fs.readFileSync(srcPath);
    const fwSize = firmware.length;

    console.log('========================================');
    console.log('加热模块 OTA 打包工具 (BLE 协议)');
    console.log('========================================');
    console.log('固件文件: ' + srcPath);
    console.log('固件大小: ' + fwSize + ' bytes (' + (fwSize / 1024).toFixed(1) + ' KB)');
    console.log('目标设备: 加热模块 (target=0x' + TARGET_HEAT.toString(16).padStart(2, '0') + ')');
    console.log('命令字:   0x0C(启动) / 0x0D(数据) / 0x0E(结束)');
    console.log('分包大小: ' + CHUNK_SIZE + ' bytes, 对齐: ' + ALIGNMENT + ' bytes');
    console.log('');

    const allFrames = [];

    // --- 1. 升级启动帧 (CMD 0x0C) ---
    // 数据: 目标设备标识(1B) + 固件字节数(4B, 大端)
    const startData = [
        TARGET_HEAT,
        (fwSize >> 24) & 0xFF,
        (fwSize >> 16) & 0xFF,
        (fwSize >> 8) & 0xFF,
        fwSize & 0xFF,
    ];
    const startFrame = buildFrame(CMD_OTA_START, startData);
    allFrames.push(startFrame);
    console.log('[1] 升级启动 (0x0C)');
    console.log('    目标: ' + TARGET_HEAT + ' (加热模块)');
    console.log('    固件大小: ' + fwSize + ' bytes (0x' + fwSize.toString(16).toUpperCase() + ')');
    console.log('    帧长度: ' + startFrame.length + ' bytes');
    console.log('    帧数据: ' + startFrame.toString('hex').replace(/(..)/g, '$1 ').toUpperCase());
    console.log('');

    // --- 2. 升级数据帧 (CMD 0x0D) ---
    const chunks = chunkFirmware(firmware);
    let totalDataBytes = 0;
    for (let i = 0; i < chunks.length; i++) {
        const offset = i * CHUNK_SIZE;
        const chunk = chunks[i];
        // 数据: 目标设备标识(1B) + offset(4B, 大端) + 固件数据块
        const dataPkt = [
            TARGET_HEAT,
            (offset >> 24) & 0xFF,
            (offset >> 16) & 0xFF,
            (offset >> 8) & 0xFF,
            offset & 0xFF,
            ...chunk,
        ];
        const frame = buildFrame(CMD_OTA_DATA, dataPkt);
        allFrames.push(frame);
        totalDataBytes += chunk.length;

        // 打印前3包和每50包的进度
        if (i < 3 || i === chunks.length - 1 || (i + 1) % 50 === 0) {
            console.log('[2] 升级数据 #' + (i + 1) + '/' + chunks.length +
                        ' (0x0D) off=0x' + offset.toString(16).toUpperCase().padStart(8, '0') +
                        ' len=' + chunk.length + ' frame=' + frame.length + 'B');
        }
    }
    console.log('    数据包总数: ' + chunks.length);
    console.log('    有效数据: ' + fwSize + ' bytes, 含对齐: ' + totalDataBytes + ' bytes');
    console.log('');

    // --- 3. 升级结束帧 (CMD 0x0E) ---
    const endData = [TARGET_HEAT];
    const endFrame = buildFrame(CMD_OTA_END, endData);
    allFrames.push(endFrame);
    console.log('[3] 升级结束 (0x0E)');
    console.log('    目标: ' + TARGET_HEAT + ' (加热模块)');
    console.log('    帧长度: ' + endFrame.length + ' bytes');
    console.log('    帧数据: ' + endFrame.toString('hex').replace(/(..)/g, '$1 ').toUpperCase());
    console.log('');

    // --- 写入输出 ---
    const output = Buffer.concat(allFrames);
    const outDir = path.dirname(dstPath);
    if (!fs.existsSync(outDir)) {
        fs.mkdirSync(outDir, { recursive: true });
    }
    fs.writeFileSync(dstPath, output);

    console.log('========================================');
    console.log('打包完成!');
    console.log('========================================');
    console.log('总帧数: ' + allFrames.length + ' (1 START + ' + chunks.length + ' DATA + 1 END)');
    console.log('总大小: ' + output.length + ' bytes (' + (output.length / 1024).toFixed(1) + ' KB)');
    console.log('输出文件: ' + dstPath);
    console.log('');

    // 打印帧列表 (前5+后2)
    console.log('--- 帧详情 (前5 + 后2) ---');
    const showFrames = allFrames.length <= 7 ? allFrames : [...allFrames.slice(0, 5), null, ...allFrames.slice(-2)];
    showFrames.forEach((f, idx) => {
        if (f === null) {
            console.log('  ... (' + (allFrames.length - 7) + ' frames omitted) ...');
            return;
        }
        const realIdx = idx < 5 ? idx : allFrames.length - (showFrames.length - idx);
        const cmd = f[4];
        const dlen = (f[6] << 8) | f[7];
        const cs = f[f.length - 1];
        const expected = checksum(f.slice(0, -1));
        const ok = cs === expected ? 'OK' : 'FAIL(expected 0x' + expected.toString(16) + ')';
        const cmdName = {0x0C:'START', 0x0D:'DATA', 0x0E:'END'}[cmd] || 'UNKNOWN';
        console.log('  [' + String(realIdx).padStart(3) + '] cmd=' + cmdName +
                    ' dlen=' + String(dlen).padStart(4) + 'B frame=' + String(f.length).padStart(4) +
                    'B check=0x' + cs.toString(16).toUpperCase().padStart(2,'0') + ' ' + ok);
    });

    console.log('');
    console.log('Done!');
}

main();
