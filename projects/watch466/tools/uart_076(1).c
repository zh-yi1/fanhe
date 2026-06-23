/**
 * 蓝牙饭盒协议解析工具（支持多帧）—— 适配 MCU通信协议 v1.0.7
 * 用法: ./parser <hex_bytes...>
 * 示例: ./parser 55 aa 00 00 01 00 00 05 04 04 00 01 01 0f
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>

#define MAX_FRAME_LEN 512

#define ERR_NONE        0x00
#define ERR_EXCEPTION   0x01

#define TYPE_BOOL       0x01
#define TYPE_VALUE      0x02
#define TYPE_ENUM       0x04

/* 属性 ID 定义 */
#define ATTR_MASTER_SWITCH     1
#define ATTR_MODE              2
#define ATTR_BATTERY           3
#define ATTR_CHARGE_STATE      4
#define ATTR_HEAT_DURATION     5
#define ATTR_REMAIN_TIME       6
#define ATTR_HEAT_TEMP         7
#define ATTR_LANGUAGE          8
#define ATTR_FAULT             9
#define ATTR_IS_HEATING        10
#define ATTR_SYNC_TIME         11

/* 预约列表返回结构（44 字节） */
#pragma pack(push, 1)
typedef struct {
    uint8_t total;
    uint8_t index;
    uint8_t set_mode;
    uint8_t id;
    char name[32];
    uint32_t time;
    uint8_t temperature;
    uint8_t duration;
    uint8_t enabled;
    uint8_t repeat;
} ReservedInfoResp;

/* 预约操作请求结构（42 字节） */
typedef struct {
    uint8_t op;
    uint8_t id;
    char name[32];
    uint32_t time;
    uint8_t temperature;
    uint8_t duration;
    uint8_t enabled;
    uint8_t repeat;
} ReservedInfoReq;
#pragma pack(pop)

/* 辅助函数：打印原始十六进制数据 */
void print_hex(const uint8_t *data, int len) {
    printf(" (");
    for (int i = 0; i < len; i++) {
        printf("%02X", data[i]);
        if (i != len-1) printf(" ");
    }
    printf(")");
}

/* 大端32位转主机 */
uint32_t be32_to_cpu(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  | ((uint32_t)p[3]);
}

/* 校验和计算 */
uint8_t calc_checksum(const uint8_t *frame, int len_without_checksum) {
    uint16_t sum = 0;
    for (int i = 0; i < len_without_checksum; i++) sum += frame[i];
    return (uint8_t)(sum % 256);
}

/* 打印帧头信息 */
void print_frame_header(const uint8_t *frame, int frame_len, int frame_index) {
    printf("\n========== 帧 #%d 结构解析 ==========\n", frame_index);
    printf("帧头:       0x%02X%02X\n", frame[0], frame[1]);
    printf("版本:       0x%02X\n", frame[2]);
    printf("消息标志:   0x%02X\n", frame[3]);
    printf("命令字:     0x%02X", frame[4]);
    const char *cmd_name = "";
    switch(frame[4]) {
        case 0x01: cmd_name = "动态属性(DataPoint)"; break;
        case 0x02: cmd_name = "查询预约列表"; break;
        case 0x03: cmd_name = "预约操作(增/改/删)"; break;
        case 0x04: cmd_name = "升级包传输"; break;
        default: cmd_name = "未知命令"; break;
    }
    printf(" (%s)\n", cmd_name);
    printf("错误标志:   0x%02X %s\n", frame[5], frame[5]==ERR_NONE?"(正常)":"(异常)");
    
    uint16_t data_len = (frame[6] << 8) | frame[7];
    int real_data_len = frame_len - 9;
    if (real_data_len < 0) real_data_len = 0;
    
    printf("数据长度:   %u (0x%04X)", data_len, data_len);
    if (real_data_len != data_len) {
        printf(" [实际数据部分长度: %d]", real_data_len);
        if (real_data_len < data_len) {
            printf(" ⚠️ 数据不完整，声明需要%d字节，实际仅%d字节", data_len, real_data_len);
        } else if (real_data_len > data_len) {
            printf(" ⚠️ 有多余 %d 字节，将忽略", real_data_len - data_len);
        }
    }
    printf("\n");
    
    if (frame_len >= 9 + data_len) {
        uint8_t expected_checksum = frame[8 + data_len];
        uint8_t calc = calc_checksum(frame, 8 + data_len);
        printf("校验和:     0x%02X", expected_checksum);
        if (calc == expected_checksum) printf(" (正确)\n");
        else printf(" (错误，应为0x%02X)\n", calc);
    } else {
        printf("校验和:     无法计算（帧长度不足）\n");
    }
    
    printf("数据部分(声明长度%u字节): ", data_len);
    int print_len = (real_data_len < data_len) ? real_data_len : data_len;
    for (int i = 0; i < print_len; i++) {
        printf("%02X ", frame[8+i]);
    }
    if (real_data_len < data_len) {
        printf("... (缺少%d字节)", data_len - real_data_len);
    }
    printf("\n");
    
    if (real_data_len > data_len) {
        printf("多余的数据字节（从声明长度后开始）: ");
        for (int i = data_len; i < real_data_len; i++) {
            printf("%02X ", frame[8+i]);
        }
        printf("\n");
    }
}

/* 温度档位转摄氏度 */
int temp_enum_to_celsius(uint8_t val) {
    static const int temp_map[] = {40,50,60,70,80,90};
    if (val <= 5) return temp_map[val];
    return 90 + (val-5)*5;   // 扩展保留
}

/* 解析动态属性 DataPoint 列表（统一用于 0x01） */
void parse_datapoint(const uint8_t *data, int data_len) {
    int offset = 0;
    printf("\n--- 动态属性列表 ---\n");
    while (offset + 4 <= data_len) {
        uint8_t dpid = data[offset];
        uint8_t type = data[offset+1];
        uint16_t len = (data[offset+2] << 8) | data[offset+3];
        int start = offset;
        offset += 4;
        if (offset + len > data_len) {
            printf("  数据点格式错误，剩余数据不足\n");
            break;
        }
        printf("  dpid=%d, type=%d, len=%d, value=", dpid, type, len);
        
        if (type == TYPE_BOOL && len == 1) {
            uint8_t v = data[offset];
            printf("%s", v ? "开启" : "关闭");
        } else if (type == TYPE_VALUE && len == 4) {
            uint32_t v = be32_to_cpu(data+offset);
            printf("%u", v);
        } else if (type == TYPE_ENUM && len == 1) {
            uint8_t v = data[offset];
            printf("%u", v);
        } else {
            for (int i = 0; i < len; i++) printf("%02X ", data[offset+i]);
        }
        
        /* 友好说明 */
        switch(dpid) {
            case ATTR_MASTER_SWITCH:
                printf(" (总开关: %s)", data[offset] ? "开" : "关");
                break;
            case ATTR_MODE: {
                const char *mode_str[] = {"关闭","自定义加热","鸡腿模式","意面模式","预约模式","保温模式"};
                uint8_t m = data[offset];
                printf(" (加热模式: %s)", m<6 ? mode_str[m] : "未知");
                break;
            }
            case ATTR_BATTERY: {
                const char *bat[] = {"低电量(<25%)","中电量(25%-50%)","高电量(50%-75%)","满电量(≥75%)"};
                uint8_t b = data[offset]-1;
                printf(" (电量: %s)", b<4 ? bat[b] : "未知");
                break;
            }
            case ATTR_CHARGE_STATE: {
                const char *chg[] = {"未充电","充电中","已充满"};
                uint8_t c = data[offset];
                printf(" (充电状态: %s)", c<3 ? chg[c] : "未知");
                break;
            }
            case ATTR_HEAT_DURATION:
                printf(" (加热时长: %d分钟)", data[offset]);
                break;
            case ATTR_REMAIN_TIME:
                printf(" (剩余加热时间: %d分钟)", data[offset]);
                break;
            case ATTR_HEAT_TEMP: {
                int temp = temp_enum_to_celsius(data[offset]);
                printf(" (加热温度: %d°C, 档位值=%d)", temp, data[offset]);
                break;
            }
            case ATTR_LANGUAGE: {
                const char *lang[] = {"中文","英文","德语","法语","西班牙语","意大利语","日语","俄语"};
                uint8_t l = data[offset];
                printf(" (语言: %s)", l<8 ? lang[l] : "未知");
                break;
            }
            case ATTR_FAULT:
                printf(" (故障提示: %s)", data[offset] ? "高温告警" : "正常");
                break;
            case ATTR_IS_HEATING:
                printf(" (是否加热: %s)", data[offset] ? "加热" : "停止");
                break;
            case ATTR_SYNC_TIME: {
                uint32_t ts = be32_to_cpu(data+offset);
                time_t t = ts;
                struct tm *tm_info = localtime(&t);
                char buf[64];
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
                printf(" (app同步时间戳: %s)", buf);
                break;
            }
            default:
                printf(" (未知属性)");
        }
        printf("  [原始数据点:");
        print_hex(data+start, 4+len);
        printf("]");
        printf("\n");
        offset += len;
    }
}

/* 解析重复周期 */
void parse_repeat(uint8_t repeat, const uint8_t *raw) {
    const char *days[] = {"周日","周一","周二","周三","周四","周五","周六"};
    printf("重复周期: 0x%02X (", repeat);
    int first = 1;
    for (int i=0;i<7;i++) {
        if (repeat & (1<<i)) {
            printf("%s%s", first?"":",", days[i]);
            first=0;
        }
    }
    if (first) printf("从不");
    printf(")");
    print_hex(raw, 1);
}

/* 解析预约列表返回（44字节） */
void parse_reservation_response(const uint8_t *data, int len) {
    if (len != 44) {
        printf("预约信息长度异常，期望44字节，实际%d\n", len);
        return;
    }
    const ReservedInfoResp *r = (const ReservedInfoResp*)data;
    int offset = 0;
    printf("总条数: %d", r->total);
    print_hex(data+offset, 1); offset+=1;
    printf("\n当前序号: %d", r->index);
    print_hex(data+offset, 1); offset+=1;
    printf("\n设置模式: %d", r->set_mode);
    print_hex(data+offset, 1); offset+=1;
    printf("\n预约ID: %d", r->id);
    print_hex(data+offset, 1); offset+=1;
    printf("\n预约名称: %s", r->name);
    print_hex(data+offset, 32); offset+=32;
    printf("\n结束时间: ");
    uint32_t time_sec = be32_to_cpu((const uint8_t*)&r->time);
    time_t t = time_sec;
    struct tm *tm_info = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s (%u秒)", buf, time_sec);
    print_hex(data+offset, 4); offset+=4;
    printf("\n加热温度档位: %d (对应 %d°C)", r->temperature, temp_enum_to_celsius(r->temperature));
    print_hex(data+offset, 1); offset+=1;
    printf("\n加热时长: %d分钟", r->duration);
    print_hex(data+offset, 1); offset+=1;
    printf("\n启用状态: %s", r->enabled ? "开启" : "关闭");
    print_hex(data+offset, 1); offset+=1;
    printf("\n");
    parse_repeat(r->repeat, data+offset);
    printf("\n");
}

/* 解析预约操作请求（42字节） */
void parse_reservation_request(const uint8_t *data, int len) {
    if (len != 42) {
        printf("预约操作长度异常，期望42字节，实际%d\n", len);
        return;
    }
    const ReservedInfoReq *r = (const ReservedInfoReq*)data;
    int offset = 0;
    printf("操作类型: %d (", r->op);
    switch(r->op) {
        case 0: printf("删除预约"); break;
        case 1: printf("自定义加热"); break;
        case 2: printf("鸡腿模式"); break;
        default: printf("未知");
    }
    printf(")");
    print_hex(data+offset, 1); offset+=1;
    printf("\n预约ID: %d (0表示自动分配)", r->id);
    print_hex(data+offset, 1); offset+=1;
    printf("\n预约名称: %s", r->name);
    print_hex(data+offset, 32); offset+=32;
    printf("\n结束时间: ");
    uint32_t time_sec = be32_to_cpu((const uint8_t*)&r->time);
    time_t t = time_sec;
    struct tm *tm_info = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s (%u秒)", buf, time_sec);
    print_hex(data+offset, 4); offset+=4;
    printf("\n加热温度档位: %d (对应 %d°C)", r->temperature, temp_enum_to_celsius(r->temperature));
    print_hex(data+offset, 1); offset+=1;
    printf("\n加热时长: %d分钟", r->duration);
    print_hex(data+offset, 1); offset+=1;
    printf("\n启用状态: %s", r->enabled ? "开启" : "关闭");
    print_hex(data+offset, 1); offset+=1;
    printf("\n");
    parse_repeat(r->repeat, data+offset);
    printf("\n");
}

/* 解析升级包（0x04） */
void parse_upgrade(const uint8_t *data, int len) {
    if (len < 4) {
        printf("升级包数据长度不足4字节（至少需要offset）\n");
        return;
    }
    uint32_t offset = be32_to_cpu(data);
    int payload_len = len - 4;
    printf("包偏移(offset): %u", offset);
    print_hex(data, 4);
    printf("\n");
    if (offset == 0xFFFFFFFF) {
        if (payload_len >= 4) {
            uint32_t crc = be32_to_cpu(data + 4);
            printf("结束包，CRC32校验码: 0x%08X", crc);
            print_hex(data+4, 4);
            printf("\n");
            if (payload_len > 4) {
                printf("警告：结束包包含额外数据，可能不符合协议\n");
            }
        } else {
            printf("结束包缺少CRC32（应为4字节）\n");
        }
    } else {
        printf("升级数据长度: %d 字节", payload_len);
        if (payload_len > 0) {
            printf("\n数据(前16字节): ");
            for (int i=0; i<payload_len && i<16; i++) printf("%02X ", data[4+i]);
            if (payload_len>16) printf("...");
            printf("\n完整数据块:");
            print_hex(data, len);
        }
        printf("\n");
    }
}

/* 主解析分发 */
void parse_payload(uint8_t cmd, const uint8_t *data, int data_len) {
    if (data_len == 0) {
        printf("无数据部分\n");
        return;
    }
    switch(cmd) {
        case 0x01:
            /* 强制按 DataPoint 解析 */
            parse_datapoint(data, data_len);
            break;
        case 0x02:
            parse_reservation_response(data, data_len);
            break;
        case 0x03:
            if (data_len == 1) {
                printf("分配的预约ID: %d", data[0]);
                print_hex(data, 1);
                printf("\n");
            } else if (data_len == 42) {
                parse_reservation_request(data, data_len);
            } else {
                printf("未知的0x03数据长度: %d\n", data_len);
            }
            break;
        case 0x04:
            parse_upgrade(data, data_len);
            break;
        default:
            printf("未知命令字，原始数据:");
            print_hex(data, data_len);
            printf("\n");
            break;
    }
}

/* 十六进制字符串转字节 */
int hex_char_to_byte(const char *s, uint8_t *out) {
    unsigned int val;
    if (s[0]=='0' && (s[1]=='x'||s[1]=='X')) s+=2;
    if (sscanf(s, "%2x", &val) == 1) { *out = (uint8_t)val; return 1; }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <hex_bytes...>\n", argv[0]);
        fprintf(stderr, "示例: %s 55 aa 00 00 01 00 00 05 04 04 00 01 01 0f\n", argv[0]);
        return 1;
    }
    
    uint8_t data_stream[MAX_FRAME_LEN * 10];
    int stream_len = 0;
    int error_flag = 0;

    for (int i = 1; i < argc && stream_len < (int)sizeof(data_stream); i++) {
        uint8_t byte;
        if (!hex_char_to_byte(argv[i], &byte)) {
            fprintf(stderr, "警告: 忽略无效的十六进制字节 '%s'\n", argv[i]);
            error_flag = 1;
            continue;
        }
        data_stream[stream_len++] = byte;
    }
    
    if (stream_len < 9) {
        fprintf(stderr, "有效数据流长度至少9字节，当前仅%d字节\n", stream_len);
        return 1;
    }
    
    int pos = 0;
    int frame_count = 0;
    
    while (pos + 8 < stream_len) {
        if (data_stream[pos] != 0x55 || data_stream[pos+1] != 0xAA) {
            pos++;
            continue;
        }
        
        uint16_t data_len = (data_stream[pos+6] << 8) | data_stream[pos+7];
        int total_frame_len = 9 + data_len;
        
        if (pos + total_frame_len > stream_len) {
            fprintf(stderr, "警告: 帧 #%d 数据不完整（需要%d字节，剩余%d），停止解析\n", 
                    frame_count+1, total_frame_len, stream_len - pos);
            break;
        }
        
        frame_count++;
        printf("\n========================================\n");
        printf("检测到帧 #%d，位置偏移 %d\n", frame_count, pos);
        
        const uint8_t *frame = data_stream + pos;
        print_frame_header(frame, total_frame_len, frame_count);
        printf("\n========== 业务数据解析 ==========\n");
        parse_payload(frame[4], frame + 8, data_len);
        
        pos += total_frame_len;
    }
    
    if (frame_count == 0) {
        printf("未找到任何有效的 55 AA 帧\n");
    } else {
        printf("\n========================================\n");
        printf("共解析 %d 帧\n", frame_count);
    }
    
    return error_flag;
}