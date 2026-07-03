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

/** @brief 计算 UART END 帧所需的最终 CRC32 */
static u32 heat_ota_get_final_crc(void)
{
    // lb_crc32 初始值 0xFFFFFFFF, 最终 XOR 0xFFFFFFFF (匹配桥模式)
    return g_heat_ota.crc_accum ^ 0xFFFFFFFF;
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
    g_heat_ota.retry_count = 0;

    u32 end_crc = heat_ota_get_final_crc();
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
        g_heat_ota.send_total = g_heat_ota.recv_size - HEAT_OTA_HEADER_SIZE;
    } else {
        g_heat_ota.send_total = g_heat_ota.recv_size;
    }

    g_heat_ota.send_offset = 0;
    g_heat_ota.uart_msg_flag = 0;
    g_heat_ota.sent_packets = 0;
    g_heat_ota.total_restarts = 0;
    g_heat_ota.retry_count = 0;
    g_heat_ota.total_packets = (u16)((g_heat_ota.send_total + HEAT_OTA_PACKET_SIZE - 1)
                                     / HEAT_OTA_PACKET_SIZE);

    printf("[HEAT_OTA] send_total=%lu packets=%u\n",
           (unsigned long)g_heat_ota.send_total, g_heat_ota.total_packets);

    // 直接开始发数据包，不发送 boot 引导命令
    // (加热模块对 offset=0xFFFFFFFF 回复 err=0x01，会陷入死循环重试)
    g_heat_ota.uart_phase = HEAT_UART_PHASE_DATA;
    g_heat_ota.state = HEAT_OTA_SENDING;
    heat_ota_send_next_packet();
}

//-----------------------------------------------------------------------------
// UART 应答处理
//-----------------------------------------------------------------------------

static void heat_ota_handle_ack(lb_rx_frame_t *rx)
{
    if (rx->err_flag != 0x00) {
        printf("[HEAT_OTA] ack err=0x%02X\n", rx->err_flag);
        heat_ota_handle_timeout();
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
            heat_ota_send_end_packet();
            return;
        }
        g_heat_ota.state = HEAT_OTA_WAIT_ACK;
    } else {
        printf("[HEAT_OTA] max retries, sending END and restarting\n");

        u32 end_crc = heat_ota_get_final_crc();
        u8 crc_data[4];
        crc_data[0] = (u8)(end_crc >> 24);
        crc_data[1] = (u8)(end_crc >> 16);
        crc_data[2] = (u8)(end_crc >> 8);
        crc_data[3] = (u8)(end_crc);

        g_heat_ota.uart_phase = HEAT_UART_PHASE_RESTART_END;
        u8 msg = g_heat_ota.uart_msg_flag++;
        g_heat_ota.current_msg_flag = msg;
        g_heat_ota.current_packet_offset = 0xFFFFFFFF;
        g_heat_ota.current_packet_len = 4;
        heat_ota_uart_send(0xFFFFFFFF, crc_data, 4, msg);
        g_heat_ota.state = HEAT_OTA_WAIT_ACK;
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

    // 擦除 Flash 区域 (32KB, 地址 32K 对齐)
    printf("[HEAT_OTA] erasing flash at 0x%06lX...\n", (unsigned long)HEAT_OTA_FLASH_ADDR);
    os_spiflash_erase_32k(HEAT_OTA_FLASH_ADDR);

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

    // 确保 CRC 覆盖了全部数据
    if (!g_heat_ota.header_parsed && g_heat_ota.recv_size >= HEAT_OTA_HEADER_SIZE) {
        heat_ota_parse_header();
    }

    // 补齐最后未 CRC 的数据
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

    // CRC 校验
    u32 final_crc = heat_ota_get_final_crc();
    printf("[HEAT_OTA] END: recv=%lu final_crc=0x%08lX\n",
           (unsigned long)g_heat_ota.recv_size, (unsigned long)final_crc);

    if (g_heat_ota.has_header) {
        printf("[HEAT_OTA] expected_crc=0x%08lX\n", (unsigned long)g_heat_ota.expected_crc32);
        crc_ok = (final_crc == g_heat_ota.expected_crc32);
    } else {
        printf("[HEAT_OTA] no header, skip CRC verify\n");
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

    if (rx->msg_flag != g_heat_ota.current_msg_flag) {
        return;  // 忽略不匹配的应答
    }

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
