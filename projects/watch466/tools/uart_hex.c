/**
 * 蓝牙饭盒 MCU 协议指令生成工具（支持时区偏移）
 * 依据《MCU通信协议.md》v1.0.7 严格实现
 * 用法: ./uart <cmd> [args...]
 *
 * 示例:
 *   ./uart 01 20260531043928 1          # 查询动态属性（DataPoint格式）
 *   ./uart 02                            # 查询预约列表
 *   ./uart 03 1 0 "yuyue1" 20260531043928 3 30 1 0x7e   # 新增预约（自定义加热）
 *   ./uart 03 0 1 "yuyue1" 20260531043928 3 30 1 0x7e   # 删除预约（ID=1）
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <arpa/inet.h>
#include <ctype.h>

/* 时区偏移（秒），默认0（UTC）。若需北京时间（UTC+8），设为 -8*3600 */
#define TIMEZONE_OFFSET  -8*3600

/* 打印帧十六进制 */
static void print_frame(const uint8_t *frame, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x", frame[i]);
        if (i != len - 1) printf(" ");
    }
    printf("\n");
}

/* 校验和：从帧头到数据结束（不含校验和）求和取低8位 */
static uint8_t calc_checksum(const uint8_t *data, int len) {
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) sum += data[i];
    return (uint8_t)(sum & 0xFF);
}

/* 解析时间字符串 YYYYMMDDHHMMSS → UTC 时间戳（应用偏移） */
static time_t parse_time_string(const char *str) {
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    if (sscanf(str, "%4d%2d%2d%2d%2d%2d",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) {
        fprintf(stderr, "错误：时间格式应为 YYYYMMDDHHMMSS\n");
        exit(1);
    }
    tm.tm_year -= 1900;
    tm.tm_mon  -= 1;
    tm.tm_isdst = 0;          // 不考虑夏令时

    time_t utc = timegm(&tm); // UTC 时间戳
    return utc + TIMEZONE_OFFSET; // 应用偏移
}

/* 构建并打印完整帧（通用） */
static void build_and_print_frame(uint8_t cmd, const uint8_t *data, uint16_t data_len) {
    uint8_t frame[512];
    int idx = 0;

    frame[idx++] = 0x55;
    frame[idx++] = 0xAA;
    frame[idx++] = 0x00;          // 版本
    frame[idx++] = 0x00;          // 消息标志
    frame[idx++] = cmd;
    frame[idx++] = 0x00;          // 错误标志（请求时固定0）
    frame[idx++] = (data_len >> 8) & 0xFF;
    frame[idx++] = data_len & 0xFF;

    memcpy(frame + idx, data, data_len);
    idx += data_len;

    uint8_t checksum = calc_checksum(frame, idx);
    frame[idx++] = checksum;

    print_frame(frame, idx);
}

/* ---------- 命令实现（依据 MCU通信协议.md v1.0.7） ---------- */

/* 命令 0x01：查询设备动态属性（数据域为两个 DataPoint）
 *   DataPoint1: dpid=11 (时间戳), type=value, len=4, value=时间戳
 *   DataPoint2: dpid=1  (使能),  type=bool, len=1, value=enable
 *   固定数据长度 13 字节
 */
static void cmd_01(const char *time_str, int enable) {
    time_t t = parse_time_string(time_str);
    uint32_t ts_be = htonl((uint32_t)t);

    uint8_t data[13];
    int idx = 0;

    // DataPoint 1: 时间戳 (dpid=11)
    data[idx++] = 0x0B;          // dpid
    data[idx++] = 0x02;          // type: value
    data[idx++] = 0x00;
    data[idx++] = 0x04;          // len = 4
    memcpy(data + idx, &ts_be, 4);
    idx += 4;

    // DataPoint 2: 使能 (dpid=1)
    data[idx++] = 0x01;          // dpid
    data[idx++] = 0x01;          // type: bool
    data[idx++] = 0x00;
    data[idx++] = 0x01;          // len = 1
    data[idx++] = (uint8_t)(enable ? 1 : 0);

    build_and_print_frame(0x01, data, sizeof(data));
}

/* 命令 0x02：查询预约列表（无数据） */
static void cmd_02(void) {
    build_and_print_frame(0x02, NULL, 0);
}

/* 命令 0x03：新增/修改/删除预约（数据长度42字节）
 *   op: 0=删除, 1=自定义加热, 2=鸡腿模式
 *   id: 预约ID（删除时必填，新增时填0表示自动分配）
 *   name: 名称字符串（最多32字节）
 *   time_str: 触发时间
 *   temp: 温度档位（0-5对应属性ID=7）
 *   duration: 加热时长（分钟）
 *   enabled: 0关闭 1开启
 *   repeat: 重复周期位掩码（如0x7e）
 */
static void cmd_03(int op, int id, const char *name, const char *time_str,
                   int temp, int duration, int enabled, int repeat) {
    time_t t = parse_time_string(time_str);
    uint32_t ts_be = htonl((uint32_t)t);

    uint8_t data[42];
    memset(data, 0, sizeof(data));

    data[0] = (uint8_t)op;
    data[1] = (uint8_t)id;
    strncpy((char*)data + 2, name, 32);          // 名称占32字节，位置2~33
    memcpy(data + 2 + 32, &ts_be, 4);            // 时间戳位置34~37
    data[2 + 32 + 4] = (uint8_t)temp;            // 温度位置38
    data[2 + 32 + 4 + 1] = (uint8_t)duration;    // 时长位置39
    data[2 + 32 + 4 + 2] = (uint8_t)enabled;     // 启用位置40
    data[2 + 32 + 4 + 3] = (uint8_t)repeat;      // 重复周期位置41

    build_and_print_frame(0x03, data, sizeof(data));
}

/* 解析整数（支持十六进制，如0x7e） */
static int parse_int(const char *str) {
    char *endptr;
    long val = strtol(str, &endptr, 0);
    if (endptr == str || *endptr != '\0') {
        fprintf(stderr, "错误：无效数字 '%s'\n", str);
        exit(1);
    }
    return (int)val;
}

/* ---------- 主程序 ---------- */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <命令> [参数...]\n", argv[0]);
        fprintf(stderr, "  01 <时间串> <使能>         例如: %s 01 20260531043928 1\n", argv[0]);
        fprintf(stderr, "  02                          例如: %s 02\n", argv[0]);
        fprintf(stderr, "  03 <操作> <ID> <名称> <时间串> <温度档位> <时长> <启用> <重复>\n");
        fprintf(stderr, "     操作: 0=删除, 1=自定义加热, 2=鸡腿模式\n");
        fprintf(stderr, "     ID: 删除时必填，新增时填0表示自动分配\n");
        fprintf(stderr, "     温度档位: 0=40°C, 1=50°C, 2=60°C, 3=70°C, 4=80°C, 5=90°C\n");
        fprintf(stderr, "     重复周期: 位掩码（如 0x7e 表示周一至周六）\n");
        fprintf(stderr, "     例如: %s 03 1 0 \"yuyue1\" 20260531043928 3 30 1 0x7e\n", argv[0]);
        return 1;
    }

    int cmd = parse_int(argv[1]);

    switch (cmd) {
        case 0x01:
            if (argc != 4) { fprintf(stderr, "命令 01 需要 2 个参数：时间串 使能\n"); return 1; }
            cmd_01(argv[2], parse_int(argv[3]));
            break;

        case 0x02:
            if (argc != 2) { fprintf(stderr, "命令 02 不需要参数\n"); return 1; }
            cmd_02();
            break;

        case 0x03:
            if (argc != 10) { fprintf(stderr, "命令 03 需要 8 个参数\n"); return 1; }
            {
                int op       = parse_int(argv[2]);
                int id       = parse_int(argv[3]);
                const char *name = argv[4];
                const char *time_str = argv[5];
                int temp     = parse_int(argv[6]);
                int duration = parse_int(argv[7]);
                int enabled  = parse_int(argv[8]);
                int repeat   = parse_int(argv[9]);
                cmd_03(op, id, name, time_str, temp, duration, enabled, repeat);
            }
            break;

        default:
            fprintf(stderr, "错误：不支持的命令 0x%02X\n", cmd);
            return 1;
    }
    return 0;
}