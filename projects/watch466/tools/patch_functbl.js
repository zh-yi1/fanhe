// patch_functbl.js — 用 WATCH_FUNC_EN 宏包裹 func_tbl.h 中的手表专用条目
const fs = require('fs');
const path = 'c:/Users/31017/Desktop/ZNFH/elunchbox/projects/watch466/functions/func_tbl.h';

// 手表专用 FUNC_ ID
const WATCH_ONLY = new Set([
    'FUNC_MENU', 'FUNC_MENUSTYLE',
    'FUNC_CLOCK', 'FUNC_CLOCK_PREVIEW', 'FUNC_SIDEBAR', 'FUNC_CARD',
    'FUNC_HEARTRATE',
    'FUNC_ALARM_CLOCK', 'FUNC_ALARM_CLOCK_SUB_SET', 'FUNC_ALARM_CLOCK_SUB_REPEAT', 'FUNC_ALARM_CLOCK_SUB_EDIT',
    'FUNC_BLOOD_OXYGEN', 'FUNC_BLOODSUGAR', 'FUNC_BLOOD_PRESSURE',
    'FUNC_BREATHE', 'FUNC_CALCULATOR', 'FUNC_CAMERA',
    'FUNC_TIMER', 'FUNC_SLEEP', 'FUNC_STOPWATCH', 'FUNC_STOPWATCH_SUB_RECORD',
    'FUNC_WEATHER', 'FUNC_SPORT', 'FUNC_SPORT_CONFIG', 'FUNC_SPORT_SUB_RUN', 'FUNC_SPORT_SWITCH',
    'FUNC_GAME', 'FUNC_STYLE', 'FUNC_FINDPHONE', 'FUNC_ALTITUDE', 'FUNC_MAP',
    'FUNC_MESSAGE', 'FUNC_SCAN', 'FUNC_VOICE',
    'FUNC_COMPASS', 'FUNC_ADDRESS_BOOK',
    'FUNC_CALL', 'FUNC_CALL_SUB_RECORD', 'FUNC_CALL_SUB_DIAL',
    'FUNC_SETTING', 'FUNC_CALENDAER', 'FUNC_VOLUME', 'FUNC_ACTIVITY',
    'FUNC_FLASHLIGHT', 'FUNC_BRIGHTNESS', 'FUNC_LIGHT',
    'FUNC_SET_SUB_DOUSING', 'FUNC_SET_SUB_WRIST', 'FUNC_SET_SUB_DISTURD',
    'FUNC_DISTURD_SUB_SET', 'FUNC_SET_SUB_LANGUAGE', 'FUNC_SET_SUB_TIME',
    'FUNC_SET_SUB_MENU_NAVIGATION', 'FUNC_TIME_SUB_CUSTOM',
    'FUNC_SET_SUB_PASSWORD', 'FUNC_PASSWORD_SUB_DISP', 'FUNC_PASSWORD_SUB_SELECT',
    'FUNC_SET_SUB_SAV', 'FUNC_SET_SUB_ABOUT', 'FUNC_SET_SUB_4G',
    'FUNC_SET_SUB_RESTART', 'FUNC_SET_SUB_RSTFY', 'FUNC_SET_SUB_OFF',
    'FUNC_SMARTSTACK', 'FUNC_BIRD', 'FUNC_GIF',
    'FUNC_MODEM_CALL', 'FUNC_MODEM_RING',
    'FUNC_BT_DUT', 'FUNC_BT_CALL', 'FUNC_BT_RING',
    'FUNC_MUSIC', 'FUNC_MUSIC_SRC', 'FUNC_EMIT_LIST',
    'FUNC_FMRX', 'FUNC_RECORDER', 'FUNC_USBDEV', 'FUNC_IDLE',
    'FUNC_MESSAGE_REPLY', 'FUNC_PHOTO_VIEW',
    'FUNC_GAME_TETRIS', 'FUNC_GAME_TETRIS_START', 'FUNC_GAME_TETRIS_OVER',
    'FUNC_VIDEO_PLAY', 'FUNC_VIDEO_SHOWLIST', 'FUNC_VIDEO_RECODE',
    'FUNC_KALEIDOSCOPE',
]);

const entryRe = /^(\s*)\{FUNC_(\w+),/;

// Read with CRLF handling
let raw = fs.readFileSync(path, 'utf-8');
// Normalize line endings to LF for processing
raw = raw.replace(/\r\n/g, '\n').replace(/\r/g, '\n');
const lines = raw.split('\n');

const result = [];
let inTable = false;
let tableDepth = 0;

for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const stripped = line.trim();

    // Detect table start
    if (!inTable) {
        if (stripped.includes('tbl_func_create[]') ||
            stripped.includes('tbl_func_entry[]') ||
            stripped.includes('tbl_func_enter[]') ||
            stripped.includes('tbl_func_exit[]')) {
            inTable = true;
            tableDepth = 0;
        }
        result.push(line);
        continue;
    }

    // Track braces
    for (const ch of stripped) {
        if (ch === '{') tableDepth++;
        if (ch === '}') tableDepth--;
    }

    // Table ended
    if (tableDepth <= 0 && stripped.startsWith('};')) {
        inTable = false;
        tableDepth = 0;
        result.push(line);
        continue;
    }

    // Check if this is a table entry
    const m = line.match(entryRe);
    if (m) {
        const funcId = 'FUNC_' + m[2];
        if (WATCH_ONLY.has(funcId)) {
            // Collect consecutive watch-only entries
            const block = [];
            let j = i;
            while (j < lines.length) {
                const bline = lines[j];
                const bstripped = bline.trim();
                const bm = bline.match(entryRe);

                if (bm) {
                    const fid = 'FUNC_' + bm[2];
                    if (WATCH_ONLY.has(fid)) {
                        block.push(bline);
                        j++;
                        continue;
                    } else {
                        break;
                    }
                } else if (bstripped.startsWith('#if') || bstripped.startsWith('#endif') || bstripped.startsWith('};')) {
                    break;
                } else {
                    break;
                }
            }

            if (block.length > 0) {
                const indent = block[0].match(/^(\s*)/)[1];
                result.push(indent + '#if WATCH_FUNC_EN');
                for (const bl of block) result.push(bl);
                result.push(indent + '#endif // WATCH_FUNC_EN');
                i = j - 1;
                continue;
            }
        }
    }

    result.push(line);
}

// Insert WATCH_FUNC_EN define after the header guard
const defineIdx = result.findIndex(l => l.trim().startsWith('typedef struct func_t_'));
if (defineIdx > 0) {
    result.splice(defineIdx, 0,
        '',
        '#define WATCH_FUNC_EN                      0   // 手表功能开关，饭盒项目设为0',
        '');
}

const output = result.join('\r\n');
fs.writeFileSync(path, output, 'utf-8');
console.log('Done! Modified func_tbl.h');
console.log('Total lines:', result.length);
