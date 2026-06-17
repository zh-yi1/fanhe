// patch_app_cbp.js — 删除明确不用的SDK,硬件驱动,include目录
const fs = require('fs');
const path = 'c:/Users/31017/Desktop/ZNFH/elunchbox/projects/watch466/app.cbp';

let raw = fs.readFileSync(path, 'utf-8');
const lines = raw.split('\n');

// 单行删除(include目录 + 库链接) — 不包含<Unit>行
const removeLine = [
    /alipay_secure_sdk/,              // 支付宝所有单行引用(lib/include/port等)
    /modules\/asr_using\//,           // ASR语音库
    /libs\/awk\/lib_map\.a/,          // 地图库
    /libs\/awk/,                      // 地图库include
    /modules\/awk_map/,               // 地图模块include
    /modules\/yj/,                    // 友杰ASR include
    /bsp_modem/,                      // 4G Modem include
    /bsp_video/,                      // 视频 include
    /bsp_image_sensor/,               // 摄像头 include
    /bsp_sensor\/hx3602/,             // 心率传感器 include
    /bsp_sensor\/internal_sensor/,    // 内部传感器 include
    /prebuild_se\.bat/,               // 支付宝SE预构建
];

// Unit块删除(源文件 .c + 头文件 .h)
const removeUnit = [
    /alipay_secure_sdk/,              // 支付宝源文件
    /modules\/ws\//,                  // 华镇ASR源
    /modules\/ws_air\//,              // 华镇空调ASR源
    /modules\/yj\//,                  // 友杰ASR源
    /modules\/awk_map\//,             // 地图模块源
    /bsp_image_sensor\//,             // 摄像头驱动源
    /bsp_video\//,                    // 视频源
    /bsp_modem\//,                    // 4G Modem源
    /bsp_call_mgr/,                   // 通话管理源
    /bsp_emit/,                       // BT发射源
    /demo\//,                         // Demo源
    /func_alipay/,                    // 支付宝功能源(已由SECURITY_PAY_EN=0守卫,但仍删除)
    // BT通话/音乐/音频 — 饭盒不需要
    /functions\/func_bt_call/,        // BT通话
    /functions\/func_bt_ring/,        // BT铃声
    /functions\/func_bt_dut/,         // BT DUT测试
    /functions\/func_call/,           // 电话
    /functions\/func_music\.c/,       // 音乐播放器(仅.c,保留.h)
    /functions\/func_music_source/,   // 音乐源
    /bsp_bt\/a2dp/,                   // A2DP音频流
    /bsp_bt\/bsp_bt_call/,            // BT通话驱动
    /bsp_bt\/bsp_bt_ring/,            // BT铃声驱动
    /bsp_bt\/bt_sco/,                 // SCO音频
    /bsp_bt\/dev_vol/,                // 设备音量
    /bsp_bt\/hfhs/,                   // 免提/HSP
    /bsp_bt\/hid/,                    // HID
    /bsp_bt\/pbap/,                   // 电话本
    /bsp_bt\/profile/,                // BT配置文件
    /platform\/libs\/strong_sco/,     // SCO强符号
    /func_menu_sub_kaleidoscope/,     // 万花筒表盘
    /compo_kaleidoscope/,             // 万花筒组件
    /functions\/func_map/,            // 地图(需要awk_pr.h)
    /functions\/func_modem/,          // Modem通话/铃声
    /functions\/func_clock/,          // 表盘(引用modem_cb)
    /functions\/func_video_recode/,   // 视频录制
    /functions\/func_video_play/,     // 视频播放
    /functions\/func_setting_sub_about_4g/, // 4G设置
];

const result = [];
let i = 0;
let removed = 0;
let unitBlocksRemoved = 0;

while (i < lines.length) {
    const line = lines[i];
    const stripped = line.trim();

    // 处理<Unit>行
    if (stripped.startsWith('<Unit')) {
        let shouldRemove = false;
        for (const pat of removeUnit) {
            if (pat.test(line)) { shouldRemove = true; break; }
        }

        if (shouldRemove) {
            if (stripped.endsWith('/>')) {
                // 自闭合<Unit />（.h头文件）— 跳过这一行
                removed++;
                i++;
            } else {
                // 非自闭合<Unit>（.c源文件）— 跳过整个<Unit>...</Unit>块
                removed++; i++; // 跳过<Unit>开标签
                while (i < lines.length && !lines[i].trim().startsWith('</Unit>')) {
                    removed++;
                    i++;
                }
                // 跳过</Unit>闭标签
                if (i < lines.length) {
                    removed++;
                    i++;
                }
                unitBlocksRemoved++;
            }
            continue;
        }
        // 不匹配删除模式 — 保留
        result.push(line);
        i++;
        continue;
    }

    // 单行删除（非<Unit>行：include目录 + 库链接）
    let skip = false;
    for (const pat of removeLine) {
        if (pat.test(line)) { skip = true; break; }
    }
    if (skip) { removed++; i++; continue; }

    result.push(line);
    i++;
}

fs.writeFileSync(path, result.join('\r\n'), 'utf-8');
console.log('Removed', removed, 'lines (' + unitBlocksRemoved + ' Unit blocks)');
console.log('Remaining:', result.length, 'lines');
