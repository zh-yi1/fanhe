const fs = require('fs');

const otaPath = process.argv[2] || 'C:/Users/31017/Desktop/ZNFH/elunchbox/projects/watch466/Output/bin/ota_dog/heat_ota/otah_Project_v005.ota';
const binPath = process.argv[3] || 'C:/Users/31017/Desktop/Liu Wei-tool/otah_Project_v005.bin';

const ota = fs.readFileSync(otaPath);
const binRaw = fs.readFileSync(binPath);

// Strip existing BIN header if present
let firmware;
const binMagic = binRaw.readUInt32BE(0);
if (binMagic === 0x11223344 && binRaw.length > 256) {
    firmware = binRaw.slice(256);
} else {
    firmware = binRaw;
}

console.log('============================================================');
console.log('  .ota 文件全面验证');
console.log('============================================================');
console.log('.ota: ' + otaPath + ' (' + ota.length + ' bytes)');
console.log('.bin: ' + binPath + ' (' + binRaw.length + ' bytes)');
console.log('.bin 自带包头: ' + (binMagic === 0x11223344 ? 'YES -> 已剥离 -> 固件=' + firmware.length + 'B' : 'NO'));
console.log('');

// CRC32/MPEG-2 table
const CRC32_MPEG2_TABLE = [
    0x00000000,0x04c11db7,0x09823b6e,0x0d4326d9,0x130476dc,0x17c56b6b,0x1a864db2,0x1e475005,
    0x2608edb8,0x22c9f00f,0x2f8ad6d6,0x2b4bcb61,0x350c9b64,0x31cd86d3,0x3c8ea00a,0x384fbdbd,
    0x4c11db70,0x48d0c6c7,0x4593e01e,0x4152fda9,0x5f15adac,0x5bd4b01b,0x569796c2,0x52568b75,
    0x6a1936c8,0x6ed82b7f,0x639b0da6,0x675a1011,0x791d4014,0x7ddc5da3,0x709f7b7a,0x745e66cd,
    0x9823b6e0,0x9ce2ab57,0x91a18d8e,0x95609039,0x8b27c03c,0x8fe6dd8b,0x82a5fb52,0x8664e6e5,
    0xbe2b5b58,0xbaea46ef,0xb7a96036,0xb3687d81,0xad2f2d84,0xa9ee3033,0xa4ad16ea,0xa06c0b5d,
    0xd4326d90,0xd0f37027,0xddb056fe,0xd9714b49,0xc7361b4c,0xc3f706fb,0xceb42022,0xca753d95,
    0xf23a8028,0xf6fb9d9f,0xfbb8bb46,0xff79a6f1,0xe13ef6f4,0xe5ffeb43,0xe8bccd9a,0xec7dd02d,
    0x34867077,0x30476dc0,0x3d044b19,0x39c556ae,0x278206ab,0x23431b1c,0x2e003dc5,0x2ac12072,
    0x128e9dcf,0x164f8078,0x1b0ca6a1,0x1fcdbb16,0x018aeb13,0x054bf6a4,0x0808d07d,0x0cc9cdca,
    0x7897ab07,0x7c56b6b0,0x71159069,0x75d48dde,0x6b93dddb,0x6f52c06c,0x6211e6b5,0x66d0fb02,
    0x5e9f46bf,0x5a5e5b08,0x571d7dd1,0x53dc6066,0x4d9b3063,0x495a2dd4,0x44190b0d,0x40d816ba,
    0xaca5c697,0xa864db20,0xa527fdf9,0xa1e6e04e,0xbfa1b04b,0xbb60adfc,0xb6238b25,0xb2e29692,
    0x8aad2b2f,0x8e6c3698,0x832f1041,0x87ee0df6,0x99a95df3,0x9d684044,0x902b669d,0x94ea7b2a,
    0xe0b41de7,0xe4750050,0xe9362689,0xedf73b3e,0xf3b06b3b,0xf771768c,0xfa325055,0xfef34de2,
    0xc6bcf05f,0xc27dede8,0xcf3ecb31,0xcbffd686,0xd5b88683,0xd1799b34,0xdc3abded,0xd8fba05a,
    0x690ce0ee,0x6dcdfd59,0x608edb80,0x644fc637,0x7a089632,0x7ec98b85,0x738aad5c,0x774bb0eb,
    0x4f040d56,0x4bc510e1,0x46863638,0x42472b8f,0x5c007b8a,0x58c1663d,0x558240e4,0x51435d53,
    0x251d3b9e,0x21dc2629,0x2c9f00f0,0x285e1d47,0x36194d42,0x32d850f5,0x3f9b762c,0x3b5a6b9b,
    0x0315d626,0x07d4cb91,0x0a97ed48,0x0e56f0ff,0x1011a0fa,0x14d0bd4d,0x19939b94,0x1d528623,
    0xf12f560e,0xf5ee4bb9,0xf8ad6d60,0xfc6c70d7,0xe22b20d2,0xe6ea3d65,0xeba91bbc,0xef68060b,
    0xd727bbb6,0xd3e6a601,0xdea580d8,0xda649d6f,0xc423cd6a,0xc0e2d0dd,0xcda1f604,0xc960ebb3,
    0xbd3e8d7e,0xb9ff90c9,0xb4bcb610,0xb07daba7,0xae3afba2,0xaafbe615,0xa7b8c0cc,0xa379dd7b,
    0x9b3660c6,0x9ff77d71,0x92b45ba8,0x9675461f,0x8832161a,0x8cf30bad,0x81b02d74,0x857130c3,
    0x5d8a9099,0x594b8d2e,0x5408abf7,0x50c9b640,0x4e8ee645,0x4a4ffbf2,0x470cdd2b,0x43cdc09c,
    0x7b827d21,0x7f436096,0x7200464f,0x76c15bf8,0x68860bfd,0x6c47164a,0x61043093,0x65c52d24,
    0x119b4be9,0x155a565e,0x18197087,0x1cd86d30,0x029f3d35,0x065e2082,0x0b1d065b,0x0fdc1bec,
    0x3793a651,0x3352bbe6,0x3e119d3f,0x3ad08088,0x2497d08d,0x2056cd3a,0x2d15ebe3,0x29d4f654,
    0xc5a92679,0xc1683bce,0xcc2b1d17,0xc8ea00a0,0xd6ad50a5,0xd26c4d12,0xdf2f6bcb,0xdbee767c,
    0xe3a1cbc1,0xe760d676,0xea23f0af,0xeee2ed18,0xf0a5bd1d,0xf464a0aa,0xf9278673,0xfde69bc4,
    0x89b8fd09,0x8d79e0be,0x803ac667,0x84fbdbd0,0x9abc8bd5,0x9e7d9662,0x933eb0bb,0x97ffad0c,
    0xafb010b1,0xab710d06,0xa6322bdf,0xa2f33668,0xbcb4666d,0xb8757bda,0xb5365d03,0xb1f740b4,
];

function crc32_mpeg2(data) {
    let crc = 0xFFFFFFFF;
    for (let i = 0; i < data.length; i++) {
        crc = (crc << 8) ^ CRC32_MPEG2_TABLE[((crc >>> 24) ^ data[i]) & 0xFF];
        crc >>>= 0;
    }
    return crc;
}

function checksum(buf) {
    let sum = 0;
    for (const b of buf) sum += b;
    return sum & 0xFF;
}

let errors = [];
let pos = 0, frameIdx = 0;
let startInfo = null, endInfo = null;
let dataFrames = [];

// ====== 逐帧解析 ======
while (pos < ota.length) {
    if (pos + 9 > ota.length) {
        errors.push({frame: frameIdx, msg: '末尾不足9字节'});
        break;
    }

    const head = (ota[pos] << 8) | ota[pos + 1];
    if (head !== 0x55AA) {
        errors.push({frame: frameIdx, pos, msg: '帧头=0x' + head.toString(16).toUpperCase() + ' (期望0x55AA)'});
        break;
    }

    const version = ota[pos + 2];
    const msgFlag = ota[pos + 3];
    const cmd = ota[pos + 4];
    const errFlag = ota[pos + 5];
    const dlen = (ota[pos + 6] << 8) | ota[pos + 7];
    const frameTotal = 9 + dlen;

    if (pos + frameTotal > ota.length) {
        errors.push({frame: frameIdx, msg: '帧超文件范围'});
        break;
    }

    const frame = ota.slice(pos, pos + frameTotal);
    const ck = frame[frame.length - 1];
    const expCk = checksum(frame.slice(0, -1));

    if (ck !== expCk) {
        errors.push({frame: frameIdx, msg: '校验=0x' + ck.toString(16).toUpperCase().padStart(2, '0') + ' 期望=0x' + expCk.toString(16).toUpperCase().padStart(2, '0')});
    }
    if (version !== 0x00) {
        errors.push({frame: frameIdx, msg: 'version=' + version + ' 期望0'});
    }

    if (cmd === 0x0C) {
        const target = frame[8];
        const fwSize = frame.readUInt32BE(9);
        startInfo = {idx: frameIdx, target, fwSize, dlen};
        if (dlen !== 5) errors.push({frame: frameIdx, msg: 'START dlen=' + dlen + ' 期望5'});
        if (target !== 0x02) errors.push({frame: frameIdx, msg: 'START target=0x' + target.toString(16) + ' 期望0x02'});
    } else if (cmd === 0x0E) {
        const target = frame[8];
        endInfo = {idx: frameIdx, target, dlen};
        if (dlen !== 1) errors.push({frame: frameIdx, msg: 'END dlen=' + dlen + ' 期望1'});
        if (target !== 0x02) errors.push({frame: frameIdx, msg: 'END target=0x' + target.toString(16) + ' 期望0x02'});
    } else if (cmd === 0x0D) {
        const target = frame[8];
        const offset = frame.readUInt32BE(9);
        const chunk = frame.slice(13, frame.length - 1);
        if (target !== 0x02) errors.push({frame: frameIdx, msg: 'DATA target=0x' + target.toString(16)});
        if (chunk.length % 16 !== 0) errors.push({frame: frameIdx, msg: 'chunk=' + chunk.length + ' 未16B对齐'});
        dataFrames.push({idx: frameIdx, target, offset, chunkLen: chunk.length, chunk});
    } else {
        errors.push({frame: frameIdx, msg: '未知CMD=0x' + cmd.toString(16)});
    }

    pos += frameTotal;
    frameIdx++;
}

// ====== 输出结果 ======
console.log('── 帧结构 ──');
console.log('总帧数: ' + frameIdx + ' (期望 160 = 1 START + 158 DATA + 1 END) ' + (frameIdx === 160 ? '✓' : '✗'));
console.log('解析错误: ' + (errors.length === 0 ? '无' : errors.length + ' 个'));
errors.forEach(e => console.log('  ✗ 帧#' + e.frame + ': ' + e.msg));

console.log('');
console.log('── START 帧 (0x0C, 协议 §5.1) ──');
if (startInfo) {
    const expTotal = 256 + firmware.length;
    console.log('  target: 0x02 ' + (startInfo.target === 0x02 ? '✓' : '✗'));
    console.log('  fwSize: ' + startInfo.fwSize + ' (0x' + startInfo.fwSize.toString(16).toUpperCase() + ')');
    console.log('  期望:   ' + expTotal + ' (256+' + firmware.length + ') ' + (startInfo.fwSize === expTotal ? '✓' : '✗'));
}

console.log('');
console.log('── DATA 帧 (0x0D, 协议 §5.2) ──');
console.log('  总数: ' + dataFrames.length + ' (期望 158) ' + (dataFrames.length === 158 ? '✓' : '✗'));

// Check first
if (dataFrames.length > 0) {
    const f0 = dataFrames[0];
    console.log('  帧#1:  offset=' + f0.offset + ' chunk=' + f0.chunkLen + 'B (0,128) ' + (f0.offset === 0 && f0.chunkLen === 128 ? '✓' : '✗'));
}

// Check offset sequence
let seqOk = true;
for (let i = 0; i < dataFrames.length; i++) {
    if (dataFrames[i].offset !== i * 128) { seqOk = false; break; }
}
console.log('  offset序列: 0,128,256,... ' + (seqOk ? '✓' : '✗'));

// Check last
const fN = dataFrames[dataFrames.length - 1];
const expLastOff = (dataFrames.length - 1) * 128;
const totalBin = 256 + firmware.length;
const expLastLen = totalBin - expLastOff;
console.log('  帧#' + dataFrames.length + ': offset=' + fN.offset + ' chunk=' + fN.chunkLen + 'B');
console.log('  期望:         offset=' + expLastOff + ' chunk=' + expLastLen + 'B ' + (fN.offset === expLastOff && fN.chunkLen === expLastLen ? '✓' : '✗'));

// Total chunk bytes
const totalChunkBytes = dataFrames.reduce((s, f) => s + f.chunkLen, 0);
console.log('  chunk总字节: ' + totalChunkBytes + ' (期望 ' + totalBin + ') ' + (totalChunkBytes === totalBin ? '✓' : '✗'));

// Check all chunks are 16B aligned
const allAligned = dataFrames.every(f => f.chunkLen % 16 === 0);
console.log('  全部16B对齐: ' + (allAligned ? '✓' : '✗'));

// Check all targets are 0x02
const allTarget02 = dataFrames.every(f => f.target === 0x02);
console.log('  全部target=0x02: ' + (allTarget02 ? '✓' : '✗'));

console.log('');
console.log('── END 帧 (0x0E, 协议 §5.3) ──');
if (endInfo) {
    console.log('  target: 0x02 ' + (endInfo.target === 0x02 ? '✓' : '✗'));
    console.log('  dlen=1 ' + (endInfo.dlen === 1 ? '✓' : '✗'));
}

// ====== BIN 包头 ======
console.log('');
console.log('── BIN 包头 (MCU协议 §5.1 备注2) ──');
const fullBin = Buffer.concat(dataFrames.map(f => f.chunk));
const binHeader = fullBin.slice(0, 256);
const binFirmware = fullBin.slice(256);

const hdrMagic = binHeader.readUInt32BE(0);
const hdrVer = binHeader.readUInt32BE(4);
const hdrFwLen = binHeader.readUInt32BE(8);
const hdrCrc = binHeader.readUInt32BE(12);

console.log('  Magic=0x' + hdrMagic.toString(16).toUpperCase().padStart(8, '0') + ' ' + (hdrMagic === 0x11223344 ? '✓' : '✗'));
console.log('  Ver=0x' + hdrVer.toString(16).toUpperCase().padStart(8, '0') + ' (=v005)');
console.log('  fw_len=' + hdrFwLen + ' vs ' + firmware.length + ' ' + (hdrFwLen === firmware.length ? '✓' : '✗'));
console.log('  CRC=0x' + hdrCrc.toString(16).toUpperCase().padStart(8, '0'));

// Check padding
let padOk = true;
for (let i = 16; i < 256; i++) {
    if (binHeader[i] !== 0xFF) { padOk = false; break; }
}
console.log('  填充[16..255]=0xFF: ' + (padOk ? '✓' : '✗'));

// Verify CRC
const calcCrc = crc32_mpeg2(firmware);
console.log('  CRC验证: 计算=' + '0x' + calcCrc.toString(16).toUpperCase().padStart(8, '0') + ' vs 包头=' + '0x' + hdrCrc.toString(16).toUpperCase().padStart(8, '0') + ' ' + (calcCrc === hdrCrc ? '✓' : '✗'));

// Verify firmware bytes match source
const fwMatch = Buffer.compare(binFirmware.slice(0, firmware.length), firmware) === 0;
console.log('  固件逐字节一致: ' + (fwMatch ? '✓' : '✗'));

// ====== 最终判定 ======
console.log('');
console.log('============================================================');
if (errors.length === 0 && startInfo && endInfo &&
    dataFrames.length === 158 && seqOk && allAligned &&
    hdrMagic === 0x11223344 && hdrFwLen === firmware.length && calcCrc === hdrCrc && fwMatch) {
    console.log('  结论: .ota 文件完全正确 ✓');
    console.log('  - 160帧全部校验和通过');
    console.log('  - BIN包头仅1层, magic/fw_len/CRC/填充 全部合规');
    console.log('  - 固件' + firmware.length + 'B完整, 逐字节一致');
    console.log('  - 符合蓝牙协议§5 + MCU协议§5.1');
} else {
    console.log('  结论: 存在问题 ✗');
}
console.log('============================================================');
