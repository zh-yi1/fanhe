/**
 * @file    func_lunchbox_uart_heat.c
 * @brief   加热模块 OTA 升级 (BLE接收→SPI Flash存储→CRC校验→UART发送)
 *
 * 存储: 外部 4MB SPI Flash 空闲区域 (0x3E0000, 32KB), 无 RAM 占用
 * CRC:  lb_crc32() 增量计算, 匹配桥模式协议和加热模块 bootloader
 *
 * 流程:
 *   1. OTA_START → 擦除 Flash 32KB 区域, 回复 APP [target][0x02=擦除完成]
 *   2. OTA_DATA  → 写入 Flash, 累加 CRC32, 回复 ACK
 *   3. OTA_END   → 校验 CRC32:
 *        - 失败 → 回复 [target][0x00], 回 IDLE
 *        - 成功 → 回复 [target][0x01], 启动 UART 传输
 *   4. UART 发送:
 *        a) offset=0xFFFFFFFF → 引导加热模块进 Boot
 *        b) 逐包读 Flash → UART (128B/包, mod16对齐)
 *        c) offset=0xFFFFFFFF + CRC32 → 结束
 *   5. 重试: 3s超时重发, ≥3次后发END+CRC32重新开始
 */
#include "include.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_uart_heat.h"
#include "func.h"      // elunchbox_guioff_sleep_delay_reset()

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// 配置
//-----------------------------------------------------------------------------

/** @brief SPI Flash 暂存起始地址 (4MB Flash 空闲区: 0x360000~0x3FB000) */
#define HEAT_OTA_FLASH_ADDR      0x3E0000

/** @brief 暂存区大小 (32KB, 加热模块固件上限 ~20KB) */
#define HEAT_OTA_FLASH_SIZE      0x8000

/** @brief 包头魔数 */
#define HEAT_OTA_HEADER_MAGIC    0x11223344

/** @brief bin 包头大小 */
#define HEAT_OTA_HEADER_SIZE     256

//-----------------------------------------------------------------------------
// 子状态
//-----------------------------------------------------------------------------
typedef enum {
    HEAT_UART_PHASE_BOOT = 0,
    HEAT_UART_PHASE_DATA,
    HEAT_UART_PHASE_END,
    HEAT_UART_PHASE_RESTART_END,
} heat_uart_phase_t;

//-----------------------------------------------------------------------------
// 上下文 (BSS, ~60 字节)
//-----------------------------------------------------------------------------
typedef struct {
    heat_ota_state_t state;

    // BLE 接收
    u32 recv_size;
    u32 expected_crc32;       // 包头中的预期 CRC32 (有包头时)
    u32 crc_accum;            // lb_crc32 累加值 (初始 0xFFFFFFFF, 最终 ^0xFFFFFFFF)
    u32 crc_offset;           // 已 CRC 覆盖的 Flash 偏移 (避免重复计算)
    bool has_header;
    bool header_parsed;
    u8  ble_msg_flag_start;
    u8  ble_msg_flag_end;

    // UART 发送
    u32 send_offset;
    u32 send_total;
    u8  uart_msg_flag;
    u8  retry_count;
    u32 current_packet_offset;
    u16 current_packet_len;
    u8  current_msg_flag;
    u32 uart_send_tick;

    // 统计
    u16 total_packets;
    u16 sent_packets;
    u16 total_restarts;
    heat_uart_phase_t uart_phase;
} heat_ota_ctx_t;

static heat_ota_ctx_t g_heat_ota;

//-----------------------------------------------------------------------------
// 内部函数声明
//-----------------------------------------------------------------------------
static void heat_ota_reset(void);
static void heat_ota_parse_header(void);
static void heat_ota_uart_send(u32 offset, const u8 *data, u16 data_len, u8 msg_flag);
static void heat_ota_send_next_packet(void);
static void heat_ota_send_boot_cmd(void);
static void heat_ota_send_end_packet(void);
static void heat_ota_start_uart_transfer(void);
static void heat_ota_handle_ack(lb_rx_frame_t *rx);
static void heat_ota_handle_timeout(void);
static u32  heat_ota_get_final_crc(void);

//-----------------------------------------------------------------------------
// 内部函数
//-----------------------------------------------------------------------------

static void heat_ota_reset(void)
{
    memset(&g_heat_ota, 0, sizeof(g_heat_ota));
    g_heat_ota.state = HEAT_OTA_IDLE;
}

/** @brief CRC32/MPEG-2 单字节更新 (多项式 0x04C11DB7, 非反射, 无最终XOR) */
static u32 lb_crc32_mpeg2_byte(u32 crc, u8 byte)
{
    crc ^= (u32)byte << 24;
    u8 j;
    for (j = 0; j < 8; j++) {
        if (crc & 0x80000000)
            crc = (crc << 1) ^ 0x04C11DB7;
        else
            crc = crc << 1;
    }
    return crc;
}

/** @brief 计算固件 CRC32/MPEG-2 (不含 mod16 零填充), 用于本地校验 header CRC */
static u32 heat_ota_get_final_crc(void)
{
    // MCU通信协议.md §5: 包头的第13-16字节表示该固件的CRC32/MPEG-2校验码
    // 此处计算的是固件原始数据的 CRC32/MPEG-2, 不含 mod16 对齐填充
    // (END 帧发给加热模块时需要追加零填充, 由 heat_ota_send_end_packet 处理)
    u32 crc = 0xFFFFFFFF;
    u32 fw_start;
    u32 fw_size;

    if (g_heat_ota.has_header) {
        fw_start = HEAT_OTA_HEADER_SIZE;  // 跳过 256 字节包头

        // 从 header bytes 8-11 读取固件实际长度 (大端)
        // recv_size 包含 BLE 分包对齐产生的零填充，不可直接用于 CRC
        u8 len_buf[4];
        os_spiflash_read(len_buf, HEAT_OTA_FLASH_ADDR + 8, 4);
        fw_size = ((u32)len_buf[0] << 24) | ((u32)len_buf[1] << 16)
                | ((u32)len_buf[2] << 8)  | len_buf[3];

        // 合法性检查: fw_size 必须 >0 且 ≤ 实际接收的固件数据量
        if (fw_size == 0 || fw_size > (g_heat_ota.recv_size - fw_start)) {
            printf("[HEAT_OTA] CRC: bad fw_len=%lu in header, fallback to recv_size\n",
                   (unsigned long)fw_size);
            fw_size = g_heat_ota.recv_size - fw_start;
        }
    } else {
        fw_start = 0;
        fw_size = g_heat_ota.recv_size;
    }

    // === DEBUG: 打印 CRC 计算参数 ===
    printf("[HEAT_OTA] CRC: recv_size=%lu has_header=%d fw_start=%lu fw_size=%lu\n",
           (unsigned long)g_heat_ota.recv_size, g_heat_ota.has_header,
           (unsigned long)fw_start, (unsigned long)fw_size);

    u8 buf[128];
    u32 off = fw_start;

    // CRC 真实固件数据 (从 Flash 读取, 仅 fw_size 字节)
    u32 remain = fw_size;

    // 打印首尾各 16 字节用于校验
    if (remain >= 16) {
        u8 head[16], tail[16];
        os_spiflash_read(head, HEAT_OTA_FLASH_ADDR + fw_start, 16);
        os_spiflash_read(tail, HEAT_OTA_FLASH_ADDR + fw_start + fw_size - 16, 16);
        printf("[HEAT_OTA] CRC: head[0..15]=");
        u32 di;
        for (di = 0; di < 16; di++) printf("%02X ", head[di]);
        printf("\n[HEAT_OTA] CRC: tail[last16]=");
        for (di = 0; di < 16; di++) printf("%02X ", tail[di]);
        printf("\n");
    }

    while (remain > 0) {
        u32 len = remain;
        if (len > sizeof(buf)) len = sizeof(buf);
        os_spiflash_read(buf, HEAT_OTA_FLASH_ADDR + off, (u16)len);
        u32 i;
        for (i = 0; i < len; i++) {
            crc = lb_crc32_mpeg2_byte(crc, buf[i]);
        }
        off += len;
        remain -= len;
    }

    return crc;  // MPEG-2 无最终 XOR, 不含 mod16 填充
}

/** @brief 解析包头 (收到 ≥256B 时调用) */
static void heat_ota_parse_header(void)
{
    if (g_heat_ota.header_parsed) return;

    u8 hdr[4];
    os_spiflash_read(hdr, HEAT_OTA_FLASH_ADDR, 4);
    u32 magic = ((u32)hdr[0] << 24) | ((u32)hdr[1] << 16)
              | ((u32)hdr[2] << 8)  | hdr[3];

    if (magic == HEAT_OTA_HEADER_MAGIC) {
        g_heat_ota.has_header = true;

        u8 crc_buf[4];
        os_spiflash_read(crc_buf, HEAT_OTA_FLASH_ADDR + 12, 4);
        g_heat_ota.expected_crc32 = ((u32)crc_buf[0] << 24) | ((u32)crc_buf[1] << 16)
                                  | ((u32)crc_buf[2] << 8)  | crc_buf[3];

        printf("[HEAT_OTA] header found, expected_crc=0x%08lX\n",
               (unsigned long)g_heat_ota.expected_crc32);

        // CRC 从 256 字节后开始
        if (g_heat_ota.recv_size > HEAT_OTA_HEADER_SIZE) {
            u8 fw_buf[128];
            u32 off = HEAT_OTA_HEADER_SIZE;
            while (off < g_heat_ota.recv_size) {
                u32 len = g_heat_ota.recv_size - off;
                if (len > sizeof(fw_buf)) len = sizeof(fw_buf);
                os_spiflash_read(fw_buf, HEAT_OTA_FLASH_ADDR + off, (u16)len);
                g_heat_ota.crc_accum = lb_crc32(fw_buf, len, g_heat_ota.crc_accum);
                off += len;
            }
        }
        g_heat_ota.crc_offset = g_heat_ota.recv_size;
    } else {
        g_heat_ota.has_header = false;
        printf("[HEAT_OTA] no header (magic=0x%08lX), CRC over all data\n", (unsigned long)magic);

        // CRC 所有已接收数据
        u8 buf[128];
        u32 off = 0;
        while (off < g_heat_ota.recv_size) {
            u32 len = g_heat_ota.recv_size - off;
            if (len > sizeof(buf)) len = sizeof(buf);
            os_spiflash_read(buf, HEAT_OTA_FLASH_ADDR + off, (u16)len);
            g_heat_ota.crc_accum = lb_crc32(buf, len, g_heat_ota.crc_accum);
            off += len;
        }
        g_heat_ota.crc_offset = g_heat_ota.recv_size;
    }

    g_heat_ota.header_parsed = true;
}

/** @brief 构建并发送 UART OTA 帧到加热模块 */
static void heat_ota_uart_send(u32 offset, const u8 *data, u16 data_len, u8 msg_flag)
{
    u8 buf[LB_TXBUF_SIZE];
    u16 off = 0;

    buf[off++] = (u8)(LB_FRAME_HEADER >> 8);  // 0x55
    buf[off++] = (u8)LB_FRAME_HEADER;          // 0xAA
    buf[off++] = LB_FRAME_VERSION;
    buf[off++] = msg_flag;
    buf[off++] = LB_UART_CMD_OTA;              // 0x04
    buf[off++] = 0x00;                         // err_flag

    u16 total_len = 4 + data_len;  // offset(4) + data
    buf[off++] = (u8)(total_len >> 8);
    buf[off++] = (u8)(total_len & 0xFF);

    buf[off++] = (u8)(offset >> 24);           // offset BE
    buf[off++] = (u8)(offset >> 16);
    buf[off++] = (u8)(offset >> 8);
    buf[off++] = (u8)(offset);

    if (data && data_len) {
        memcpy(buf + off, data, data_len);
        off += data_len;
    }

    buf[off] = lb_checksum(buf, off);
    off++;

    printf("UART==>TX[%u]: ", off);
    print_r(buf, off);

    uart_bufs_tx(UART_TYPE_1, buf, off);

    g_heat_ota.uart_send_tick = tick_get();
}

static void heat_ota_send_boot_cmd(void)
{
    g_heat_ota.uart_phase = HEAT_UART_PHASE_BOOT;
    g_heat_ota.retry_count = 0;
    printf("[HEAT_OTA] UART: boot cmd (offset=0xFFFFFFFF)\n");

    u8 msg = g_heat_ota.uart_msg_flag++;
    g_heat_ota.current_msg_flag = msg;
    g_heat_ota.current_packet_offset = 0xFFFFFFFF;
    g_heat_ota.current_packet_len = 0;
    heat_ota_uart_send(0xFFFFFFFF, NULL, 0, msg);
    g_heat_ota.state = HEAT_OTA_WAIT_ACK;
}

static void heat_ota_send_next_packet(void)
{
    if (g_heat_ota.send_offset >= g_heat_ota.send_total) {
        g_heat_ota.retry_count = 0;  // END 阶段重新计数
        heat_ota_send_end_packet();
        return;
    }

    g_heat_ota.uart_phase = HEAT_UART_PHASE_DATA;
    g_heat_ota.retry_count = 0;

    u32 remaining = g_heat_ota.send_total - g_heat_ota.send_offset;
    u16 pkt_len = (remaining >= HEAT_OTA_PACKET_SIZE) ? HEAT_OTA_PACKET_SIZE
                                                       : (u16)remaining;
    // mod16 对齐
    u16 aligned_len = pkt_len;
    u16 mod = pkt_len & 0x0F;
    if (mod != 0) aligned_len = (pkt_len + 16) & ~0x0F;

    // 从 Flash 读取固件数据
    u8 packet_buf[HEAT_OTA_PACKET_SIZE + 16];
    u32 flash_off;
    if (g_heat_ota.has_header) {
        flash_off = HEAT_OTA_FLASH_ADDR + HEAT_OTA_HEADER_SIZE + g_heat_ota.send_offset;
    } else {
        flash_off = HEAT_OTA_FLASH_ADDR + g_heat_ota.send_offset;
    }
    os_spiflash_read(packet_buf, flash_off, pkt_len);
    if (aligned_len > pkt_len) {
        memset(packet_buf + pkt_len, 0x00, aligned_len - pkt_len);
    }

    u8 msg = g_heat_ota.uart_msg_flag++;
    g_heat_ota.current_msg_flag = msg;
    g_heat_ota.current_packet_offset = g_heat_ota.send_offset;
    g_heat_ota.current_packet_len = pkt_len;

    printf("[HEAT_OTA] UART: pkt %u/%u off=0x%08lX len=%u\n",
           g_heat_ota.sent_packets + 1, g_heat_ota.total_packets,
           (unsigned long)g_heat_ota.send_offset, pkt_len);

    heat_ota_uart_send(g_heat_ota.send_offset, packet_buf, aligned_len, msg);
    g_heat_ota.state = HEAT_OTA_WAIT_ACK;
}

static void heat_ota_send_end_packet(void)
{
    g_heat_ota.uart_phase = HEAT_UART_PHASE_END;

    // === DEBUG: 打印发送统计 ===
    printf("[HEAT_OTA] UART: END phase, send_total=%lu send_offset=%lu sent=%u/%u\n",
           (unsigned long)g_heat_ota.send_total, (unsigned long)g_heat_ota.send_offset,
           g_heat_ota.sent_packets, g_heat_ota.total_packets);

    // 获取原始固件 CRC32/MPEG-2 (不含 mod16 填充, 匹配 header CRC)
    u32 end_crc = heat_ota_get_final_crc();

    // 追加 mod16 零填充: 加热模块通过 UART 收到的数据含对齐填充,
    // END 帧 CRC 必须覆盖填充后的完整数据, 否则模块拒绝
    u16 mod = g_heat_ota.send_total & 0x0F;
    if (mod) {
        u32 pad = 16 - mod;
        u32 j;
        for (j = 0; j < pad; j++) {
            end_crc = lb_crc32_mpeg2_byte(end_crc, 0x00);
        }
        printf("[HEAT_OTA] UART: END CRC extended with %lu zero-padding bytes\n",
               (unsigned long)pad);
    }

    u8 crc_data[4];
    crc_data[0] = (u8)(end_crc >> 24);
    crc_data[1] = (u8)(end_crc >> 16);
    crc_data[2] = (u8)(end_crc >> 8);
    crc_data[3] = (u8)(end_crc);

    printf("[HEAT_OTA] UART: END frame CRC32=0x%08lX\n", (unsigned long)end_crc);

    u8 msg = g_heat_ota.uart_msg_flag++;
    g_heat_ota.current_msg_flag = msg;
    g_heat_ota.current_packet_offset = 0xFFFFFFFF;
    g_heat_ota.current_packet_len = 4;
    heat_ota_uart_send(0xFFFFFFFF, crc_data, 4, msg);
    g_heat_ota.state = HEAT_OTA_WAIT_ACK;
}

static void heat_ota_start_uart_transfer(void)
{
    printf("[HEAT_OTA] starting UART transfer\n");

    if (g_heat_ota.has_header) {
        // 从 header bytes 8-11 读取固件实际长度 (大端)
        // recv_size 包含 BLE 分包对齐产生的零填充，不可直接使用
        u8 len_buf[4];
        os_spiflash_read(len_buf, HEAT_OTA_FLASH_ADDR + 8, 4);
        u32 hdr_fw_len = ((u32)len_buf[0] << 24) | ((u32)len_buf[1] << 16)
                       | ((u32)len_buf[2] << 8)  | len_buf[3];

        if (hdr_fw_len > 0 && hdr_fw_len <= (g_heat_ota.recv_size - HEAT_OTA_HEADER_SIZE)) {
            g_heat_ota.send_total = hdr_fw_len;
        } else {
            printf("[HEAT_OTA] bad fw_len=%lu in header, fallback\n", (unsigned long)hdr_fw_len);
            g_heat_ota.send_total = g_heat_ota.recv_size - HEAT_OTA_HEADER_SIZE;
        }
    } else {
        g_heat_ota.send_total = g_heat_ota.recv_size;
    }

    g_heat_ota.send_offset = 0;
    g_heat_ota.uart_msg_flag = 0;
    g_heat_ota.sent_packets = 0;
    g_heat_ota.retry_count = 0;
    g_heat_ota.total_packets = (u16)((g_heat_ota.send_total + HEAT_OTA_PACKET_SIZE - 1)
                                     / HEAT_OTA_PACKET_SIZE);

    printf("[HEAT_OTA] recv_size=%lu send_total=%lu packets=%u has_header=%d\n",
           (unsigned long)g_heat_ota.recv_size, (unsigned long)g_heat_ota.send_total,
           g_heat_ota.total_packets, g_heat_ota.has_header);

    // 先发 boot 引导命令 (offset=0xFFFFFFFF) 让模块进入 boot 模式
    // 模块对 boot 命令回复 err=0x01 属正常行为, heat_ota_handle_ack 已处理
    g_heat_ota.state = HEAT_OTA_SENDING;
    heat_ota_send_boot_cmd();
}

//-----------------------------------------------------------------------------
// UART 应答处理
//-----------------------------------------------------------------------------

static void heat_ota_handle_ack(lb_rx_frame_t *rx)
{
    // 从响应中解析 offset (4 字节大端), 用于过滤过期应答
    // 模块可能返回 data_len=0 (按文档) 或 data_len=4 (实际观察), 有 offset 时校验
    // BOOT/END/RESTART_END 阶段: 模块对 offset=0xFFFFFFFF 的应答为 0xFFFFFFFE,
    // 与发送的 offset 不同, 跳过严格匹配
    if (rx->data && rx->data_len >= 4) {
        u32 rx_offset = ((u32)rx->data[0] << 24) | ((u32)rx->data[1] << 16)
                      | ((u32)rx->data[2] << 8)  | rx->data[3];
        if (g_heat_ota.uart_phase == HEAT_UART_PHASE_DATA &&
            rx_offset != g_heat_ota.current_packet_offset) {
            printf("[HEAT_OTA] rx offset=0x%08lX != expected=0x%08lX, dropping\n",
                   (unsigned long)rx_offset,
                   (unsigned long)g_heat_ota.current_packet_offset);
            return;
        }
    }

    // BOOT 阶段: err=0x01 表示模块收到复位指令, 视为成功
    if (rx->err_flag != 0x00) {
        if (g_heat_ota.uart_phase == HEAT_UART_PHASE_BOOT && rx->err_flag == 0x01) {
            printf("[HEAT_OTA] boot ack err=0x01 (expected), start data\n");
            g_heat_ota.retry_count = 0;
            g_heat_ota.uart_phase = HEAT_UART_PHASE_DATA;
            g_heat_ota.send_offset = 0;
            g_heat_ota.sent_packets = 0;
            g_heat_ota.state = HEAT_OTA_SENDING;
            heat_ota_send_next_packet();
        } else {
            // 模块回复失败 → 不重试同一个包, 直接从头开始 UART 传输
            printf("[HEAT_OTA] phase=%d err=0x%02X, restarting UART transfer\n",
                   g_heat_ota.uart_phase, rx->err_flag);
            g_heat_ota.total_restarts++;
            heat_ota_start_uart_transfer();
        }
        return;
    }

    g_heat_ota.retry_count = 0;

    switch (g_heat_ota.uart_phase) {
    case HEAT_UART_PHASE_BOOT:
        printf("[HEAT_OTA] boot acked, start data\n");
        g_heat_ota.uart_phase = HEAT_UART_PHASE_DATA;
        g_heat_ota.send_offset = 0;
        g_heat_ota.sent_packets = 0;
        g_heat_ota.state = HEAT_OTA_SENDING;
        heat_ota_send_next_packet();
        break;

    case HEAT_UART_PHASE_DATA:
        g_heat_ota.sent_packets++;
        g_heat_ota.send_offset += g_heat_ota.current_packet_len;
        g_heat_ota.state = HEAT_OTA_SENDING;
        heat_ota_send_next_packet();
        break;

    case HEAT_UART_PHASE_END:
        printf("[HEAT_OTA] DONE! pkts=%u restarts=%u\n",
               g_heat_ota.total_packets, g_heat_ota.total_restarts);
        heat_ota_reset();
        break;

    case HEAT_UART_PHASE_RESTART_END:
        printf("[HEAT_OTA] restart END acked, restarting #%u\n",
               g_heat_ota.total_restarts + 1);
        g_heat_ota.total_restarts++;
        g_heat_ota.state = HEAT_OTA_SENDING;
        heat_ota_send_boot_cmd();
        break;
    }
}

static void heat_ota_handle_timeout(void)
{
    g_heat_ota.retry_count++;

    if (g_heat_ota.retry_count < HEAT_OTA_MAX_RETRIES) {
        printf("[HEAT_OTA] timeout retry %u/%u\n",
               g_heat_ota.retry_count, HEAT_OTA_MAX_RETRIES);

        if (g_heat_ota.uart_phase == HEAT_UART_PHASE_BOOT) {
            u8 msg = g_heat_ota.uart_msg_flag++;
            g_heat_ota.current_msg_flag = msg;
            heat_ota_uart_send(0xFFFFFFFF, NULL, 0, msg);
        } else if (g_heat_ota.uart_phase == HEAT_UART_PHASE_DATA) {
            // 从 Flash 重建当前包
            u32 remaining = g_heat_ota.send_total - g_heat_ota.current_packet_offset;
            u16 pkt_len = (remaining >= HEAT_OTA_PACKET_SIZE) ? HEAT_OTA_PACKET_SIZE
                                                               : (u16)remaining;
            u16 aligned_len = pkt_len;
            if (pkt_len & 0x0F) aligned_len = (pkt_len + 16) & ~0x0F;

            u8 packet_buf[HEAT_OTA_PACKET_SIZE + 16];
            u32 flash_off;
            if (g_heat_ota.has_header) {
                flash_off = HEAT_OTA_FLASH_ADDR + HEAT_OTA_HEADER_SIZE
                            + g_heat_ota.current_packet_offset;
            } else {
                flash_off = HEAT_OTA_FLASH_ADDR + g_heat_ota.current_packet_offset;
            }
            os_spiflash_read(packet_buf, flash_off, pkt_len);
            if (aligned_len > pkt_len) {
                memset(packet_buf + pkt_len, 0x00, aligned_len - pkt_len);
            }

            u8 msg = g_heat_ota.uart_msg_flag++;
            g_heat_ota.current_msg_flag = msg;
            heat_ota_uart_send(g_heat_ota.current_packet_offset,
                               packet_buf, aligned_len, msg);
        } else {
            // END / RESTART_END 超时重发 (保留原 phase)
            heat_uart_phase_t prev_phase = g_heat_ota.uart_phase;
            heat_ota_send_end_packet();
            g_heat_ota.uart_phase = prev_phase;
            return;
        }
        g_heat_ota.state = HEAT_OTA_WAIT_ACK;
    } else {
        // 3 次重试全部失败 → 从头开始 UART 传输 (重新发 boot 命令)
        printf("[HEAT_OTA] max retries in phase=%d, restarting UART transfer\n",
               g_heat_ota.uart_phase);
        g_heat_ota.total_restarts++;
        heat_ota_start_uart_transfer();
    }
}

//-----------------------------------------------------------------------------
// 公共 API
//-----------------------------------------------------------------------------

u8 heat_ota_handler_start(lb_rx_frame_t *rx, u8 msg_flag)
{
    if (!rx->data || rx->data_len < 5) {
        printf("[HEAT_OTA] START: data too short\n");
        return LB_ERR_EXEC_FAIL;
    }

    u32 fw_size = ((u32)rx->data[1] << 24) | ((u32)rx->data[2] << 16)
                | ((u32)rx->data[3] << 8)  | rx->data[4];

    printf("[HEAT_OTA] START: fw_size=%lu\n", (unsigned long)fw_size);

    if (fw_size > HEAT_OTA_FLASH_SIZE) {
        printf("[HEAT_OTA] fw_size=%lu > flash=%u\n",
               (unsigned long)fw_size, HEAT_OTA_FLASH_SIZE);
        u8 rsp[2] = { rx->data[0], 0x00 };
        lunchbox_uart_send_response(LB_CMD_OTA_START, msg_flag, LB_ERR_EXEC_FAIL, rsp, 2);
        return LB_ERR_EXEC_FAIL;
    }

    if (g_heat_ota.state != HEAT_OTA_IDLE) {
        printf("[HEAT_OTA] forced reset from state=%d\n", g_heat_ota.state);
    }
    heat_ota_reset();
    g_heat_ota.state = HEAT_OTA_RECEIVING;
    g_heat_ota.ble_msg_flag_start = msg_flag;
    g_heat_ota.crc_accum = 0xFFFFFFFF;

    // 擦除 Flash 32KB 区域
    // 不使用 os_spiflash_erase_32k() — 该芯片可能不支持 32KB 擦除指令
    // 改为 8×4KB 扇区擦除, 与 bt_fota.c / data_storage.c 一致
    printf("[HEAT_OTA] erasing flash at 0x%06lX (8×4KB)...\n", (unsigned long)HEAT_OTA_FLASH_ADDR);
    {
        u8 old_val;
        os_spiflash_read(&old_val, HEAT_OTA_FLASH_ADDR, 1);
        printf("[HEAT_OTA] before erase: byte0=0x%02X\n", old_val);

        for (u32 sec = 0; sec < 8; sec++) {
            os_spiflash_erase(HEAT_OTA_FLASH_ADDR + sec * 4096);
        }

        // 等待最后一个扇区擦除完成 (读第一个字节, 等它变成 0xFF)
        u32 timeout = 500;  // 最多等待 500ms
        u8 val;
        do {
            delay_ms(10);
            timeout -= 10;
            os_spiflash_read(&val, HEAT_OTA_FLASH_ADDR, 1);
        } while (val != 0xFF && timeout > 0);
        printf("[HEAT_OTA] erase done: val=0x%02X (old=0x%02X) remain=%lums %s\n",
               val, old_val, (unsigned long)timeout,
               (val == 0xFF) ? "OK" : "TIMEOUT!");
    }

    // 回复 APP
    u8 rsp[2];
    rsp[0] = rx->data[0];  // target = 0x02
    rsp[1] = 0x02;          // 擦除完成
    lunchbox_uart_send_response(LB_CMD_OTA_START, msg_flag, LB_ERR_SUCCESS, rsp, 2);
    printf("[HEAT_OTA] START: replied OK\n");

    return LB_ERR_SUCCESS;
}

u8 heat_ota_handler_data(lb_rx_frame_t *rx, u8 msg_flag)
{
    if (g_heat_ota.state != HEAT_OTA_RECEIVING) {
        printf("[HEAT_OTA] DATA: wrong state=%d\n", g_heat_ota.state);
        return LB_ERR_EXEC_FAIL;
    }

    u32 offset = ((u32)rx->data[1] << 24) | ((u32)rx->data[2] << 16)
               | ((u32)rx->data[3] << 8)  | rx->data[4];
    u16 data_len = rx->data_len - 5;
    const u8 *pdata = rx->data + 5;

    if (offset + data_len > HEAT_OTA_FLASH_SIZE) {
        printf("[HEAT_OTA] DATA: overflow off=%lu len=%u\n",
               (unsigned long)offset, data_len);
        return LB_ERR_EXEC_FAIL;
    }

    // 写入 Flash
    os_spiflash_program((void *)pdata, HEAT_OTA_FLASH_ADDR + offset, data_len);

    u32 new_total = offset + data_len;
    if (new_total > g_heat_ota.recv_size) {
        g_heat_ota.recv_size = new_total;
    }

    // 包头解析
    if (!g_heat_ota.header_parsed && g_heat_ota.recv_size >= HEAT_OTA_HEADER_SIZE) {
        heat_ota_parse_header();
    }

    // 增量 CRC: 只对固件部分 (跳过可能的包头)
    u32 crc_start = g_heat_ota.crc_offset;
    u32 crc_end   = g_heat_ota.recv_size;
    if (g_heat_ota.has_header && crc_start < HEAT_OTA_HEADER_SIZE) {
        crc_start = HEAT_OTA_HEADER_SIZE;
    }

    if (crc_end > crc_start) {
        u8 crc_buf[128];
        u32 off = crc_start;
        while (off < crc_end) {
            u32 len = crc_end - off;
            if (len > sizeof(crc_buf)) len = sizeof(crc_buf);
            os_spiflash_read(crc_buf, HEAT_OTA_FLASH_ADDR + off, (u16)len);
            g_heat_ota.crc_accum = lb_crc32(crc_buf, len, g_heat_ota.crc_accum);
            off += len;
        }
        g_heat_ota.crc_offset = crc_end;
    }

    // ACK
    lunchbox_uart_send_response(LB_CMD_OTA_DATA, msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

u8 heat_ota_handler_end(lb_rx_frame_t *rx, u8 msg_flag)
{
    if (g_heat_ota.state != HEAT_OTA_RECEIVING) {
        printf("[HEAT_OTA] END: wrong state=%d\n", g_heat_ota.state);
        return LB_ERR_EXEC_FAIL;
    }

    g_heat_ota.state = HEAT_OTA_VERIFY;
    g_heat_ota.ble_msg_flag_end = msg_flag;

    u8 target = (rx->data && rx->data_len > 0) ? rx->data[0] : 0x02;
    bool crc_ok = false;

    // === 1. 直接从 Flash 读取包头 (不依赖 header_parsed 标志) ===
    // 读取前 16 字节: magic(4) + version(4) + fw_len(4) + crc(4)
    u8 hdr_buf[16];
    os_spiflash_read(hdr_buf, HEAT_OTA_FLASH_ADDR, 16);
    u32 magic = ((u32)hdr_buf[0] << 24) | ((u32)hdr_buf[1] << 16)
              | ((u32)hdr_buf[2] << 8)  | hdr_buf[3];

    printf("[HEAT_OTA] END: flash magic=0x%08lX recv=%lu\n",
           (unsigned long)magic, (unsigned long)g_heat_ota.recv_size);

    if (magic == HEAT_OTA_HEADER_MAGIC) {
        // 包头存在, 提取 CRC32/MPEG-2 校验码 (字节 12-15, 大端)
        g_heat_ota.has_header = true;
        g_heat_ota.expected_crc32 = ((u32)hdr_buf[12] << 24) | ((u32)hdr_buf[13] << 16)
                                 | ((u32)hdr_buf[14] << 8)  | hdr_buf[15];
        printf("[HEAT_OTA] header found, expected_crc=0x%08lX\n",
               (unsigned long)g_heat_ota.expected_crc32);
    } else {
        g_heat_ota.has_header = false;
        printf("[HEAT_OTA] no header (magic=0x%08lX != 0x%08lX)\n",
               (unsigned long)magic, (unsigned long)HEAT_OTA_HEADER_MAGIC);
    }

    // === 2. 计算 CRC (仅固件部分, 不含包头) ===
    u32 final_crc = heat_ota_get_final_crc();
    printf("[HEAT_OTA] END: final_crc=0x%08lX\n", (unsigned long)final_crc);

    if (g_heat_ota.has_header) {
        crc_ok = (final_crc == g_heat_ota.expected_crc32);
        printf("[HEAT_OTA] CRC %s (computed=0x%08lX expected=0x%08lX)\n",
               crc_ok ? "OK" : "FAIL",
               (unsigned long)final_crc, (unsigned long)g_heat_ota.expected_crc32);
    } else {
        // 没有合法包头, 跳过本地 CRC 校验
        // 加热模块 bootloader 收到 END 帧后会自行校验 CRC32
        printf("[HEAT_OTA] no header, skip local CRC check (fw_size=%lu)\n",
               (unsigned long)(g_heat_ota.recv_size));
        crc_ok = true;
    }

    u8 rsp[2];
    rsp[0] = target;
    if (crc_ok) {
        rsp[1] = 0x01;
        lunchbox_uart_send_response(LB_CMD_OTA_END, msg_flag, LB_ERR_SUCCESS, rsp, 2);
        printf("[HEAT_OTA] CRC OK, starting UART\n");
        heat_ota_start_uart_transfer();
    } else {
        rsp[1] = 0x00;
        lunchbox_uart_send_response(LB_CMD_OTA_END, msg_flag, LB_ERR_EXEC_FAIL, rsp, 2);
        printf("[HEAT_OTA] CRC FAIL\n");
        heat_ota_reset();
    }

    return LB_ERR_SUCCESS;
}

void heat_ota_process(void)
{
    if (g_heat_ota.state == HEAT_OTA_IDLE) return;

    // OTA 进行中: 持续重置休眠倒计时, 防止 MCU 自动休眠打断升级
    elunchbox_guioff_sleep_delay_reset();

    if (g_heat_ota.state != HEAT_OTA_WAIT_ACK) return;

    if (tick_check_expire(g_heat_ota.uart_send_tick, HEAT_OTA_UART_TIMEOUT_MS)) {
        heat_ota_handle_timeout();
    }
}

void heat_ota_uart_response(lb_rx_frame_t *rx)
{
    if (g_heat_ota.state != HEAT_OTA_WAIT_ACK) return;

    // 加热模块 OTA 应答 msg_flag 恒为 0, 不做匹配校验
    // OTA 时序由 state/phase 状态机保证, 不依赖 msg_flag
    heat_ota_handle_ack(rx);
}

bool heat_ota_is_active(void)
{
    return g_heat_ota.state != HEAT_OTA_IDLE;
}

heat_ota_state_t heat_ota_get_state(void)
{
    return g_heat_ota.state;
}

#endif // FUNC_LUNCHBOX_UART_EN
