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

/** @brief OTA 恢复元数据 Flash 地址 (紧接 OTA 数据区, 独立 4KB 扇区) */
#define HEAT_OTA_RESUME_ADDR     (HEAT_OTA_FLASH_ADDR + HEAT_OTA_FLASH_SIZE)

/** @brief 恢复元数据魔数 "HOTA" */
#define HEAT_OTA_RESUME_MAGIC    0x484F5441

/** @brief 崩溃恢复元数据 (写入独立 Flash 扇区, CRC 通过后才保存) */
typedef struct {
    u32 magic;           // HEAT_OTA_RESUME_MAGIC
    u32 recv_size;       // BLE 接收的总字节数
    u32 expected_crc32;  // 包头中的预期 CRC32
    u8  ble_target;      // 0x02
    u8  ble_msg_flag_end;
    u8  has_header;
    u8  crash_count;     // 累计崩溃重启次数 (跨复位持久化, 防无限循环)
    u32 final_crc32;     // 已计算的 CRC32 结果 (恢复后跳过 CRC 重算)
    u8  reserved[4];     // 预留给未来扩展
} heat_ota_resume_t;

//-----------------------------------------------------------------------------
// 子状态
//-----------------------------------------------------------------------------
typedef enum {
    HEAT_UART_PHASE_BOOT = 0,
    HEAT_UART_PHASE_BOOT_DELAY,     // 非阻塞等待加热模块 Flash 写引擎就绪
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
    bool has_header;
    u8  ble_msg_flag_start;
    u8  ble_msg_flag_end;
    u8  ble_target;           // 目标设备标识 (用于延时返回 0x0e 应答)

    // 延迟写入: BLE handler 不直接操作 SPI Flash (会阻塞 GPU),
    // 改为 heat_ota_process 统一刷出
    u8  pending_buf[256];     // 待写入 Flash 的数据缓冲
    u16 pending_len;          // 待写入数据长度
    u32 pending_flash_off;    // 待写入的 Flash 偏移
    bool has_pending_write;   // 是否有待写入数据

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

    // 后台 CRC 计算 (非阻塞, heat_ota_process 驱动)
    u32 bg_crc_val;       // 运行中的 CRC32/MPEG-2 值 (初值 0xFFFFFFFF)
    u32 bg_crc_off;       // 当前已计算到的 Flash 偏移
    u32 bg_crc_fw_start;  // CRC 起始偏移 (0 或 256)
    u32 bg_crc_fw_size;   // 需计算的固件总字节数
    u32 final_crc_val;    // 计算完成的 CRC 结果 (send_end_packet 复用)
    u32 boot_delay_tick;  // boot ACK 后延迟发送第一包的起始 tick (非阻塞)
    bool crc_result_ok;   // CRC 校验结果, 由下一轮主循环消费 (拆分避免 GPU 饿死)
    u8  crash_count;      // 崩溃恢复计数 (仅在 resume 时递增, 与 UART 重试分离)
} heat_ota_ctx_t;

static heat_ota_ctx_t g_heat_ota;

//-----------------------------------------------------------------------------
// 内部函数声明
//-----------------------------------------------------------------------------
static void heat_ota_reset(void);
static void heat_ota_uart_send(u32 offset, const u8 *data, u16 data_len, u8 msg_flag);
static void heat_ota_send_next_packet(void);
static void heat_ota_send_boot_cmd(void);
static void heat_ota_send_end_packet(void);
static void heat_ota_start_uart_transfer(void);
static void heat_ota_handle_ack(lb_rx_frame_t *rx);
static void heat_ota_handle_timeout(void);
static bool heat_ota_crc_step(void);  // 后台 CRC 分块计算, 返回 true=完成

//-----------------------------------------------------------------------------
// 内部函数
//-----------------------------------------------------------------------------

static void heat_ota_reset(void)
{
    memset(&g_heat_ota, 0, sizeof(g_heat_ota));
    g_heat_ota.state = HEAT_OTA_IDLE;
}

/** @brief 保存 OTA 恢复上下文到 Flash (CRC 通过后调用, 崩溃后可恢复) */
static void heat_ota_save_resume_ctx(void)
{
    heat_ota_resume_t meta;
    memset(&meta, 0, sizeof(meta));
    meta.magic           = HEAT_OTA_RESUME_MAGIC;
    meta.recv_size       = g_heat_ota.recv_size;
    meta.expected_crc32  = g_heat_ota.expected_crc32;
    meta.final_crc32     = g_heat_ota.final_crc_val;
    meta.ble_target      = g_heat_ota.ble_target;
    meta.ble_msg_flag_end = g_heat_ota.ble_msg_flag_end;
    meta.has_header      = g_heat_ota.has_header ? 1 : 0;
    meta.crash_count     = g_heat_ota.crash_count;

    os_spiflash_erase(HEAT_OTA_RESUME_ADDR);
    WDT_CLR();
    os_spiflash_program(&meta, HEAT_OTA_RESUME_ADDR, sizeof(meta));
    WDT_CLR();
    printf("[HEAT_OTA] resume ctx saved (crash=%u)\n", meta.crash_count);
}

/** @brief 清除恢复元数据 (OTA 成功完成时调用) */
static void heat_ota_clear_resume_ctx(void)
{
    os_spiflash_erase(HEAT_OTA_RESUME_ADDR);
    printf("[HEAT_OTA] resume ctx cleared\n");
}

/**
 * @brief 启动时检查是否有未完成的 OTA (CRC 已通过, 只差 UART 传输)
 *
 * 若检测到恢复元数据, 直接从 Flash 重建上下文并启动 UART 传输,
 * 跳过 BLE 接收和 CRC 计算阶段。
 *
 * 调用时机: heat_ota_process() 首次调用 (state==IDLE 时)
 */
static void heat_ota_resume_check(void)
{
    heat_ota_resume_t meta;

    os_spiflash_read(&meta, HEAT_OTA_RESUME_ADDR, sizeof(meta));
    if (meta.magic != HEAT_OTA_RESUME_MAGIC) return;

    printf("[HEAT_OTA] === RESUME: found pending OTA (crash=%u) ===\n",
           meta.crash_count);

    // 检查崩溃重启次数, 防止无限重启循环
    if (meta.crash_count >= HEAT_OTA_MAX_RESTARTS + 1) {
        printf("[HEAT_OTA] max crash resumes reached, abandoning OTA\n");
        heat_ota_clear_resume_ctx();
        return;
    }

    // 验证 Flash 中固件头是否完好
    u8 hdr[16];
    os_spiflash_read(hdr, HEAT_OTA_FLASH_ADDR, 16);
    u32 magic = ((u32)hdr[0] << 24) | ((u32)hdr[1] << 16)
              | ((u32)hdr[2] << 8)  | hdr[3];

    if (meta.has_header && magic != HEAT_OTA_HEADER_MAGIC) {
        printf("[HEAT_OTA] resume: header corrupted (magic=0x%08lX), abandon\n",
               (unsigned long)magic);
        heat_ota_clear_resume_ctx();
        return;
    }

    // 清除恢复标记 (UART 启动后会重新保存)
    heat_ota_clear_resume_ctx();

    // 重建上下文
    heat_ota_reset();
    g_heat_ota.recv_size       = meta.recv_size;
    g_heat_ota.expected_crc32  = meta.expected_crc32;
    g_heat_ota.final_crc_val   = meta.final_crc32;   // 恢复已计算的 CRC, 避免重算
    g_heat_ota.has_header      = meta.has_header != 0;
    g_heat_ota.ble_target      = meta.ble_target;
    g_heat_ota.ble_msg_flag_end = meta.ble_msg_flag_end;
    g_heat_ota.crash_count     = meta.crash_count + 1;  // 累加崩溃计数
    g_heat_ota.total_restarts  = 0;   // UART 协议重试从零开始 (与崩溃分离)

    // 从 header 读取固件长度
    if (g_heat_ota.has_header) {
        u32 hdr_fw_len = ((u32)hdr[8] << 24) | ((u32)hdr[9] << 16)
                       | ((u32)hdr[10] << 8) | hdr[11];
        if (hdr_fw_len > 0 && hdr_fw_len <= (meta.recv_size - HEAT_OTA_HEADER_SIZE)) {
            g_heat_ota.send_total = hdr_fw_len;
        } else {
            g_heat_ota.send_total = meta.recv_size - HEAT_OTA_HEADER_SIZE;
        }
    } else {
        g_heat_ota.send_total = meta.recv_size;
    }

    printf("[HEAT_OTA] resume: send_total=%lu packets=%u\n",
           (unsigned long)g_heat_ota.send_total,
           (u16)((g_heat_ota.send_total + HEAT_OTA_PACKET_SIZE - 1)
                 / HEAT_OTA_PACKET_SIZE));

    // 直接启动 UART 传输 (CRC 在保存恢复上下文前已校验通过)
    WDT_EN_OTA();
    WDT_CLR();
    heat_ota_start_uart_transfer();
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

/**
 * @brief 后台 CRC 分块计算 (由 heat_ota_process 每主循环周期调用一次)
 *
 * 每次处理一小块 Flash 数据 (64B) 后立即返回, 让 GUI/Timer 线程有机会运行,
 * 避免长时间阻塞主循环导致 GPU 线程硬件复位。
 *
 * @return true=CRC 计算完成, final_crc_val 已写入上下文
 */
#define HEAT_OTA_CRC_STEP_SIZE  64
static bool heat_ota_crc_step(void)
{
    // 首次调用时打印 CRC 参数 (不做 SPI Flash 读, 避免抢 GPU 总线)
    if (g_heat_ota.bg_crc_off == g_heat_ota.bg_crc_fw_start) {
        printf("[HEAT_OTA] CRC: recv_size=%lu fw_start=%lu fw_size=%lu\n",
               (unsigned long)g_heat_ota.recv_size,
               (unsigned long)g_heat_ota.bg_crc_fw_start,
               (unsigned long)g_heat_ota.bg_crc_fw_size);
    }

    // 每次处理 64 字节, 让出 CPU
    u8 buf[HEAT_OTA_CRC_STEP_SIZE];
    u32 len = g_heat_ota.bg_crc_fw_size - (g_heat_ota.bg_crc_off - g_heat_ota.bg_crc_fw_start);
    if (len > sizeof(buf)) len = sizeof(buf);
    if (len == 0) {
        // 计算完成
        g_heat_ota.final_crc_val = g_heat_ota.bg_crc_val;
        printf("[HEAT_OTA] CRC: step done, final_crc=0x%08lX\n",
               (unsigned long)g_heat_ota.final_crc_val);
        return true;
    }

    os_spiflash_read(buf, HEAT_OTA_FLASH_ADDR + g_heat_ota.bg_crc_off, (u16)len);
    for (u32 i = 0; i < len; i++) {
        g_heat_ota.bg_crc_val = lb_crc32_mpeg2_byte(g_heat_ota.bg_crc_val, buf[i]);
    }
    g_heat_ota.bg_crc_off += len;
    return false;
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

    // 复用后台 CRC 计算的结果 (不含 mod16 填充, 匹配 header CRC)
    u32 end_crc = g_heat_ota.final_crc_val;

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

    // 保存恢复上下文 (崩溃后可跳过 BLE+CRC, 直接从 UART 重来)
    heat_ota_save_resume_ctx();

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
            printf("[HEAT_OTA] boot ack err=0x01 (expected), delay %lums (non-blocking)\n",
                   (unsigned long)HEAT_OTA_BOOT_DELAY_MS);
            g_heat_ota.retry_count = 0;
            g_heat_ota.uart_phase = HEAT_UART_PHASE_BOOT_DELAY;
            g_heat_ota.send_offset = 0;
            g_heat_ota.sent_packets = 0;
            g_heat_ota.boot_delay_tick = tick_get();
            // state 保持 HEAT_OTA_WAIT_ACK, 由 heat_ota_process 轮询到期后发第一包
        } else {
            // 模块回复失败 → 检查重启次数限制
            if (g_heat_ota.total_restarts < HEAT_OTA_MAX_RESTARTS) {
                printf("[HEAT_OTA] phase=%d err=0x%02X, restarting UART transfer (%u/%u)\n",
                       g_heat_ota.uart_phase, rx->err_flag,
                       g_heat_ota.total_restarts + 1, HEAT_OTA_MAX_RESTARTS);
                g_heat_ota.total_restarts++;
                heat_ota_start_uart_transfer();
            } else {
                printf("[HEAT_OTA] phase=%d err=0x%02X, max restarts reached, OTA FAIL\n",
                       g_heat_ota.uart_phase, rx->err_flag);
                {
                    u8 rsp[2];
                    rsp[0] = g_heat_ota.ble_target;
                    rsp[1] = 0x00;  // 升级失败
                    lunchbox_uart_send_response(LB_CMD_OTA_END, g_heat_ota.ble_msg_flag_end,
                                                LB_ERR_EXEC_FAIL, rsp, 2);
                    printf("[HEAT_OTA] 0x0e failure response sent to APP\n");
                }
                heat_ota_clear_resume_ctx();  // 升级失败, 清除恢复标记
                WDT_EN();  // 恢复正常 WDT 超时
                heat_ota_reset();
            }
        }
        return;
    }

    g_heat_ota.retry_count = 0;

    switch (g_heat_ota.uart_phase) {
    case HEAT_UART_PHASE_BOOT:
        printf("[HEAT_OTA] boot acked, delay %lums (non-blocking)\n",
               (unsigned long)HEAT_OTA_BOOT_DELAY_MS);
        g_heat_ota.uart_phase = HEAT_UART_PHASE_BOOT_DELAY;
        g_heat_ota.send_offset = 0;
        g_heat_ota.sent_packets = 0;
        g_heat_ota.boot_delay_tick = tick_get();
        // state 保持 HEAT_OTA_WAIT_ACK, 由 heat_ota_process 轮询到期后发第一包
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
        heat_ota_clear_resume_ctx();  // OTA 成功, 清除恢复标记
        WDT_EN();  // 恢复正常 WDT 超时
        // 加热模块烧录成功, 此时才回复APP 0x0e 升级成功
        {
            u8 rsp[2];
            rsp[0] = g_heat_ota.ble_target;
            rsp[1] = 0x01;  // 升级成功
            lunchbox_uart_send_response(LB_CMD_OTA_END, g_heat_ota.ble_msg_flag_end,
                                        LB_ERR_SUCCESS, rsp, 2);
            printf("[HEAT_OTA] 0x0e success response sent to APP\n");
        }
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
        // 3 次重试全部失败
        if (g_heat_ota.uart_phase == HEAT_UART_PHASE_BOOT) {
            // 复位指令3次失败 → 判定加热模块升级失败
            printf("[HEAT_OTA] boot cmd failed 3 times, OTA FAIL\n");
            {
                u8 rsp[2];
                rsp[0] = g_heat_ota.ble_target;
                rsp[1] = 0x00;  // 升级失败
                lunchbox_uart_send_response(LB_CMD_OTA_END, g_heat_ota.ble_msg_flag_end,
                                            LB_ERR_EXEC_FAIL, rsp, 2);
                printf("[HEAT_OTA] 0x0e failure response sent to APP\n");
            }
            heat_ota_clear_resume_ctx();  // 升级失败, 清除恢复标记
            WDT_EN();  // 恢复正常 WDT 超时
            heat_ota_reset();
        } else if (g_heat_ota.total_restarts < HEAT_OTA_MAX_RESTARTS) {
            // 数据/END 指令3次失败 → 重新发复位指令 (从头开始 UART 传输)
            printf("[HEAT_OTA] max retries in phase=%d, restarting UART transfer (%u/%u)\n",
                   g_heat_ota.uart_phase,
                   g_heat_ota.total_restarts + 1, HEAT_OTA_MAX_RESTARTS);
            g_heat_ota.total_restarts++;
            heat_ota_start_uart_transfer();
        } else {
            // 重启后仍然失败 → 判定加热模块升级失败
            printf("[HEAT_OTA] max restarts reached, OTA FAIL\n");
            {
                u8 rsp[2];
                rsp[0] = g_heat_ota.ble_target;
                rsp[1] = 0x00;  // 升级失败
                lunchbox_uart_send_response(LB_CMD_OTA_END, g_heat_ota.ble_msg_flag_end,
                                            LB_ERR_EXEC_FAIL, rsp, 2);
                printf("[HEAT_OTA] 0x0e failure response sent to APP\n");
            }
            heat_ota_clear_resume_ctx();  // 升级失败, 清除恢复标记
            WDT_EN();  // 恢复正常 WDT 超时
            heat_ota_reset();
        }
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

    // OTA 期间临时延长 WDT (Flash 擦写耗时较长), 升级结束后恢复
    WDT_EN_OTA();
    WDT_CLR();

    if (fw_size > HEAT_OTA_FLASH_SIZE) {
        printf("[HEAT_OTA] fw_size=%lu > flash=%u\n",
               (unsigned long)fw_size, HEAT_OTA_FLASH_SIZE);
        u8 rsp[2] = { rx->data[0], 0x00 };
        lunchbox_uart_send_response(LB_CMD_OTA_START, msg_flag, LB_ERR_EXEC_FAIL, rsp, 2);
        WDT_EN();  // 恢复正常 WDT 超时
        return LB_ERR_EXEC_FAIL;
    }

    if (g_heat_ota.state != HEAT_OTA_IDLE) {
        printf("[HEAT_OTA] forced reset from state=%d\n", g_heat_ota.state);
    }
    heat_ota_reset();
    g_heat_ota.state = HEAT_OTA_RECEIVING;
    g_heat_ota.ble_msg_flag_start = msg_flag;

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
            WDT_CLR();  // 每个扇区擦除后喂狗, 8×4KB 累计耗时可达数秒
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

    if (data_len > sizeof(g_heat_ota.pending_buf)) {
        printf("[HEAT_OTA] DATA: packet too large %u > %u\n",
               data_len, (unsigned)sizeof(g_heat_ota.pending_buf));
        return LB_ERR_EXEC_FAIL;
    }

    // 若上一包数据还未刷入 Flash (主循环还没来得及处理), 先强制刷出
    if (g_heat_ota.has_pending_write) {
        os_spiflash_program(g_heat_ota.pending_buf,
                            HEAT_OTA_FLASH_ADDR + g_heat_ota.pending_flash_off,
                            g_heat_ota.pending_len);
        WDT_CLR();
        g_heat_ota.has_pending_write = false;
    }

    // 不直接写 SPI Flash — 会阻塞主循环导致 GPU 线程复位 (Flash 擦写阻塞 GPU)
    // 改为缓冲到 RAM, 由 heat_ota_process 在每次主循环迭代中统一刷出
    memcpy(g_heat_ota.pending_buf, pdata, data_len);
    g_heat_ota.pending_len = data_len;
    g_heat_ota.pending_flash_off = offset;
    g_heat_ota.has_pending_write = true;

    u32 new_total = offset + data_len;
    if (new_total > g_heat_ota.recv_size) {
        g_heat_ota.recv_size = new_total;
    }

    // ACK 立即回复 (Flash 写入延后, 不影响协议时序)
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

    // 保存 target 和 msg_flag, 等加热模块UART烧录完成后再回复APP
    g_heat_ota.ble_target = target;
    g_heat_ota.ble_msg_flag_end = msg_flag;

    // === 1. 直接从 Flash 读取包头 (不依赖 header_parsed 标志) ===
    // 读取前 16 字节: magic(4) + version(4) + fw_len(4) + crc(4)
    u8 hdr_buf[16];
    os_spiflash_read(hdr_buf, HEAT_OTA_FLASH_ADDR, 16);
    u32 magic = ((u32)hdr_buf[0] << 24) | ((u32)hdr_buf[1] << 16)
              | ((u32)hdr_buf[2] << 8)  | hdr_buf[3];

    printf("[HEAT_OTA] END: flash magic=0x%08lX recv=%lu\n",
           (unsigned long)magic, (unsigned long)g_heat_ota.recv_size);

    if (magic == HEAT_OTA_HEADER_MAGIC) {
        g_heat_ota.has_header = true;
        g_heat_ota.expected_crc32 = ((u32)hdr_buf[12] << 24) | ((u32)hdr_buf[13] << 16)
                                 | ((u32)hdr_buf[14] << 8)  | hdr_buf[15];
        g_heat_ota.bg_crc_fw_start = HEAT_OTA_HEADER_SIZE;
        // 从 header bytes 8-11 读取固件实际长度 (大端)
        u32 hdr_fw_len = ((u32)hdr_buf[8] << 24) | ((u32)hdr_buf[9] << 16)
                       | ((u32)hdr_buf[10] << 8) | hdr_buf[11];
        if (hdr_fw_len > 0 && hdr_fw_len <= (g_heat_ota.recv_size - HEAT_OTA_HEADER_SIZE)) {
            g_heat_ota.bg_crc_fw_size = hdr_fw_len;
        } else {
            printf("[HEAT_OTA] bad fw_len=%lu in header, fallback\n", (unsigned long)hdr_fw_len);
            g_heat_ota.bg_crc_fw_size = g_heat_ota.recv_size - HEAT_OTA_HEADER_SIZE;
        }
        printf("[HEAT_OTA] header found, expected_crc=0x%08lX\n",
               (unsigned long)g_heat_ota.expected_crc32);
    } else {
        g_heat_ota.has_header = false;
        g_heat_ota.bg_crc_fw_start = 0;
        g_heat_ota.bg_crc_fw_size = g_heat_ota.recv_size;
        printf("[HEAT_OTA] no header (magic=0x%08lX != 0x%08lX)\n",
               (unsigned long)magic, (unsigned long)HEAT_OTA_HEADER_MAGIC);
    }

    // === 2. 启动后台分块 CRC 计算 (非阻塞, 由 heat_ota_process 驱动) ===
    g_heat_ota.bg_crc_val = 0xFFFFFFFF;
    g_heat_ota.bg_crc_off = g_heat_ota.bg_crc_fw_start;
    g_heat_ota.state = HEAT_OTA_CRC_CALC;

    return LB_ERR_SUCCESS;
}

void heat_ota_process(void)
{
    if (g_heat_ota.state == HEAT_OTA_IDLE) {
        // 启动时检查是否有崩溃中断的 OTA 可恢复 (仅检查一次)
        static bool resume_checked = false;
        if (!resume_checked) {
            resume_checked = true;
            heat_ota_resume_check();
        }
        // resume_check 可能已重建上下文, 若仍为 IDLE 则无待恢复 OTA
        if (g_heat_ota.state == HEAT_OTA_IDLE) return;
    }

    // OTA 进行中: 喂狗防止 WDT 复位 + 持续重置休眠倒计时防止 MCU 自动休眠
    WDT_CLR();
    elunchbox_guioff_sleep_delay_reset();

    // 延迟 Flash 写入: 将 BLE handler 缓冲的数据刷入 SPI Flash
    // (BLE handler 不直接操作 Flash, 避免阻塞主循环→GPU 线程复位)
    if (g_heat_ota.has_pending_write) {
        os_spiflash_program(g_heat_ota.pending_buf,
                            HEAT_OTA_FLASH_ADDR + g_heat_ota.pending_flash_off,
                            g_heat_ota.pending_len);
        WDT_CLR();
        g_heat_ota.has_pending_write = false;
    }

    // 后台 CRC 分块计算 (非阻塞, 每次主循环迭代处理一小块)
    if (g_heat_ota.state == HEAT_OTA_CRC_CALC) {
        if (!heat_ota_crc_step()) return;  // 未完成, 下个循环继续

        // CRC 计算完成, 校验结果 (仅打印+保存结果, 不启动 UART)
        // 原因: 本轮主循环已消耗 SPI Flash 读+CRC 算时间,
        // 若再叠加 UART TX → GPU 线程被饿死 → gui thread miss → 复位
        printf("[HEAT_OTA] END: final_crc=0x%08lX\n",
               (unsigned long)g_heat_ota.final_crc_val);
        if (g_heat_ota.has_header) {
            g_heat_ota.crc_result_ok =
                (g_heat_ota.final_crc_val == g_heat_ota.expected_crc32);
            printf("[HEAT_OTA] CRC %s (computed=0x%08lX expected=0x%08lX)\n",
                   g_heat_ota.crc_result_ok ? "OK" : "FAIL",
                   (unsigned long)g_heat_ota.final_crc_val,
                   (unsigned long)g_heat_ota.expected_crc32);
        } else {
            printf("[HEAT_OTA] no header, skip CRC check\n");
            g_heat_ota.crc_result_ok = true;
        }
        // 延迟到下一轮主循环启动 UART 传输 (让 GPU 线程有时间渲染)
        g_heat_ota.state = HEAT_OTA_SENDING;
        return;
    }

    // CRC 校验完成, 延迟到本轮启动 UART 传输 (与 CRC 分块计算不在同一轮)
    if (g_heat_ota.state == HEAT_OTA_SENDING) {
        if (g_heat_ota.crc_result_ok) {
            printf("[HEAT_OTA] CRC OK, starting UART (0x0e response deferred)\n");
            heat_ota_start_uart_transfer();  // 内部保存恢复上下文
        } else {
            u8 rsp[2];
            rsp[0] = g_heat_ota.ble_target;
            rsp[1] = 0x00;
            lunchbox_uart_send_response(LB_CMD_OTA_END, g_heat_ota.ble_msg_flag_end,
                                        LB_ERR_EXEC_FAIL, rsp, 2);
            printf("[HEAT_OTA] CRC FAIL\n");
            WDT_EN();  // 恢复正常 WDT 超时
            heat_ota_reset();
        }
        return;
    }

    if (g_heat_ota.state != HEAT_OTA_WAIT_ACK) return;

    // BOOT_DELAY 阶段: 非阻塞等待, 避免 delay_ms() 阻塞主循环导致 GPU 线程复位
    if (g_heat_ota.uart_phase == HEAT_UART_PHASE_BOOT_DELAY) {
        if (tick_check_expire(g_heat_ota.boot_delay_tick, HEAT_OTA_BOOT_DELAY_MS)) {
            printf("[HEAT_OTA] boot delay done, starting data\n");
            g_heat_ota.uart_phase = HEAT_UART_PHASE_DATA;
            g_heat_ota.state = HEAT_OTA_SENDING;
            heat_ota_send_next_packet();
        }
        return;
    }

    if (tick_check_expire(g_heat_ota.uart_send_tick, HEAT_OTA_UART_TIMEOUT_MS)) {
        heat_ota_handle_timeout();
    }
}

bool heat_ota_uart_response(lb_rx_frame_t *rx)
{
    if (g_heat_ota.state != HEAT_OTA_WAIT_ACK) return false;

    // 加热模块 OTA 应答 msg_flag 恒为 0, 不做匹配校验
    // OTA 时序由 state/phase 状态机保证, 不依赖 msg_flag
    heat_ota_handle_ack(rx);
    return true;
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
