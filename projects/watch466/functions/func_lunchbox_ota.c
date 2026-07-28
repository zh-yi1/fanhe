/**
 * @file    func_lunchbox_ota.c
 * @brief   饭盒 OTA 固件升级实现 (蓝牙通讯协议1.0.7.md §5)
 *
 * 管理主单片机 (target=0x01) 的固件升级流程:
 *   0x0c LB_CMD_OTA_START  — 升级启动
 *   0x0d LB_CMD_OTA_DATA   — 升级包传输
 *   0x0e LB_CMD_OTA_END    — 升级结束
 *
 * 对接 ota_pack_* 底层 FOTA 引擎 (压缩升级包写入)。
 */
#include "include.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_ota.h"
#include "func_lunchbox_uart_heat.h"

#if FUNC_LUNCHBOX_UART_EN

//-----------------------------------------------------------------------------
// OTA 升级状态机
//-----------------------------------------------------------------------------
typedef enum {
    LB_OTA_IDLE = 0,        // 空闲
    LB_OTA_READY,           // 已收到启动命令(0x0c)，等待数据
    LB_OTA_RECEIVING,       // 正在接收升级包数据(0x0d)
} lb_ota_state_t;

typedef struct {
    lb_ota_state_t state;   // 当前状态
    u32 fw_size;            // 固件总大小（字节，从 0x0c 获取，含 256B 包头）
    u32 next_offset;        // 期望的下一个数据偏移量（用于连续性校验）
    u32 recv_size;          // 已接收的数据总大小（含包头）
    u8  buf[512];           // 512 字节写入缓冲（ota_pack_write 要求 512 对齐）
    u16 buf_pos;            // 缓冲区已使用字节数
    u8  need_reset;         // 升级完成标志，主循环检测后延时复位
    u32 block_count;        // 已写入的 512B 块计数
    // 256 字节 bin 包头解析 (MCU通信协议.md §5.1 备注2)
    u8   header_buf[256];   // 包头累积缓冲区
    u16  header_pos;        // 已收集包头字节数
    bool header_done;       // 包头已收齐并解析
} lb_ota_ctx_t;

static lb_ota_ctx_t lb_ota_ctx;
static u32 lb_ota_reset_tick = 0;           // 升级完成后延时复位的 tick

/**
 * @brief CRC32 校验码计算 (兼容 zlib/uzlib 算法, 与 MCU通信协议.md §5.1 一致)
 *
 * 使用 16 项查找表, 支持增量计算:
 *   crc = lb_crc32(buf1, len1, 0xffffffff);        // 第一段
 *   crc = lb_crc32(buf2, len2, crc);                // 续算
 *   final = crc ^ 0xffffffff;                       // 取反得最终值
 */
u32 lb_crc32(const void *data, u32 len, u32 crc)
{
    static const u32 crc32_table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };
    const u8 *buf = (const u8 *)data;
    u32 i;
    for (i = 0; i < len; ++i) {
        crc ^= buf[i];
        crc = crc32_table[crc & 0x0f] ^ (crc >> 4);
        crc = crc32_table[crc & 0x0f] ^ (crc >> 4);
    }
    return crc;
}

/**
 * @brief 获取 OTA 帧的目标设备标识 (桥模式: 区分主MCU/加热模块)
 *
 * 解析 OTA 命令帧 (0x0c/0x0d/0x0e) 数据段首字节的 target 字段。
 * 仅桥模式需要此路由，本地模式 target 恒为 0x01 (主单片机)。
 *
 * @return LB_OTA_TARGET_MAIN_MCU(0x01) / LB_OTA_TARGET_HEAT_MODULE(0x02) / 0x00(未知)
 */
u8 lb_ota_get_target(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len == 0) return 0x00;

    switch (rx->cmd) {
    case LB_CMD_OTA_START:   // data_len=5: [target][fw_size:4B]
        return (rx->data_len >= 5) ? rx->data[0] : 0x00;
    case LB_CMD_OTA_DATA:    // data_len≥5: [target][offset:4B][data]
        return (rx->data_len >= 5) ? rx->data[0] : 0x00;
    case LB_CMD_OTA_END:     // data_len=1: [target]
        return rx->data[0];
    default:
        return 0x00;
    }
}

/**
 * @brief 0x0c — 升级启动 (v1.0.7)
 *
 * APP 发送: 5 字节 [target(1B)][firmware_size(4B, 大端)]
 * MCU 返回: 2 字节 [target(1B)][status(1B)]
 *   status: 0x00=收到升级指令, 0x01=擦除flash中, 0x02=擦除完成
 */
u8 lb_handler_ota_start(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 5) {
        lb_ble_send_response(LB_CMD_OTA_START, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8  target = rx->data[0];
    u32 fw_size = ((u32)rx->data[1] << 24) | ((u32)rx->data[2] << 16)
                | ((u32)rx->data[3] << 8)  | rx->data[4];

    printf("OTA start: target=0x%02X fw_size=%lu\n", target, fw_size);

    // OTA 期间临时延长 WDT (Flash 擦写耗时较长), 升级结束后恢复
    WDT_EN_OTA();
    WDT_CLR();
    ota_pack_init();
    WDT_CLR();
    load_code_fota();
    WDT_CLR();  // load_code_fota 涉及 Flash 锁定/解锁, 之后喂狗

    // 重置 OTA 上下文
    memset(&lb_ota_ctx, 0, sizeof(lb_ota_ctx));
    lb_ota_ctx.state = LB_OTA_READY;
    lb_ota_ctx.fw_size = fw_size;

    u8 rsp[2];
    rsp[0] = target;
    rsp[1] = LB_OTA_START_ERASE_DONE;  // 初始化完成，可以传输升级包
    lb_ble_send_response(LB_CMD_OTA_START, rx->msg_flag, LB_ERR_SUCCESS, rsp, 2);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0d — 升级包传输 (v1.0.7)
 *
 * APP 发送: 5+N 字节 [target(1B)][offset(4B, 大端)][upgrade_data(N bytes)]
 *   数据长度 = N + 5, 每包数据长度必须可被 16 整除，不足补 0
 * MCU 返回: 无数据 (ack 帧)
 */
u8 lb_handler_ota_data(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 5) {
        lb_ble_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    // 必须先收到启动命令
    if (lb_ota_ctx.state < LB_OTA_READY) {
        printf("OTA data err: not started\n");
        lb_ble_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8  target = rx->data[0];
    u32 offset = ((u32)rx->data[1] << 24) | ((u32)rx->data[2] << 16)
               | ((u32)rx->data[3] << 8)  | rx->data[4];
    u8 *data = rx->data + 5;
    u16 data_size = rx->data_len - 5;
    (void)target;  // target 字段已在路由层校验（本地模式恒为 0x01）

    // 校验 16 字节对齐 (蓝牙通讯协议1.0.7 §5.2: 每包数据长度必须可被 16 整除)
    if (data_size & 0x0F) {
        printf("OTA data err: size=%u not 16B aligned\n", data_size);
        lb_ble_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    // 校验 offset 连续性 (ota_pack_write 顺序写入，不支持随机偏移)
    if (offset != lb_ota_ctx.next_offset) {
        printf("OTA seq err: expected=%lu got=%lu\n", lb_ota_ctx.next_offset, offset);
        lb_ble_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    lb_ota_ctx.state = LB_OTA_RECEIVING;

    // 保存原始数据长度 (用于 offset/recv_size 追踪，含包头)
    u16 orig_data_size = data_size;

    // 剥离 .bin 文件 256 字节包头 (MCU通信协议.md §5.1 备注2)
    // 包头不写入 flash，但 offset/recv_size 按完整 .bin 文件追踪
    if (!lb_ota_ctx.header_done && offset < 256) {
        u16 hdr_bytes = (u16)(256 - offset);
        if (hdr_bytes > data_size) hdr_bytes = data_size;

        memcpy(lb_ota_ctx.header_buf + offset, data, hdr_bytes);
        lb_ota_ctx.header_pos += hdr_bytes;

        // 包头收齐 (256 字节)，解析字段
        if (lb_ota_ctx.header_pos >= 256) {
            u32 magic = ((u32)lb_ota_ctx.header_buf[0] << 24)
                      | ((u32)lb_ota_ctx.header_buf[1] << 16)
                      | ((u32)lb_ota_ctx.header_buf[2] << 8)
                      | lb_ota_ctx.header_buf[3];
            u32 fw_ver = ((u32)lb_ota_ctx.header_buf[4] << 24)
                       | ((u32)lb_ota_ctx.header_buf[5] << 16)
                       | ((u32)lb_ota_ctx.header_buf[6] << 8)
                       | lb_ota_ctx.header_buf[7];
            u32 fw_len = ((u32)lb_ota_ctx.header_buf[8] << 24)
                       | ((u32)lb_ota_ctx.header_buf[9] << 16)
                       | ((u32)lb_ota_ctx.header_buf[10] << 8)
                       | lb_ota_ctx.header_buf[11];
            u32 fw_crc = ((u32)lb_ota_ctx.header_buf[12] << 24)
                       | ((u32)lb_ota_ctx.header_buf[13] << 16)
                       | ((u32)lb_ota_ctx.header_buf[14] << 8)
                       | lb_ota_ctx.header_buf[15];

            if (magic == 0x11223344) {
                printf("OTA bin hdr: ver=0x%08lX len=%lu crc=0x%08lX\n",
                       fw_ver, fw_len, fw_crc);
                if (fw_len != lb_ota_ctx.fw_size) {
                    printf("OTA warn: hdr_len=%lu != fw_size=%lu\n",
                           fw_len, lb_ota_ctx.fw_size);
                }
            } else {
                printf("OTA warn: bad magic 0x%08lX, expect 0x11223344\n", magic);
            }
            lb_ota_ctx.header_done = true;
        }

        // 跳过包头字节，只把固件数据传入缓冲写入
        data += hdr_bytes;
        data_size -= hdr_bytes;

        if (data_size == 0) {
            // 整包都是包头，无固件数据
            lb_ota_ctx.next_offset = offset + orig_data_size;
            lb_ota_ctx.recv_size += orig_data_size;
            lb_ble_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
            return LB_ERR_SUCCESS;
        }
    }

    // 缓冲写入: 将固件数据填入 512 字节缓冲，满一块写一块
    u16 remaining = data_size;
    u8 *src = data;

    while (remaining > 0) {
        u16 space = 512 - lb_ota_ctx.buf_pos;
        u16 copy = (remaining < space) ? remaining : space;
        memcpy(lb_ota_ctx.buf + lb_ota_ctx.buf_pos, src, copy);
        lb_ota_ctx.buf_pos += copy;
        src += copy;
        remaining -= copy;

        if (lb_ota_ctx.buf_pos >= 512) {
            lb_ota_ctx.block_count++;
            printf("OTA BLK[%lu] off=0x%06lX\n",
                   lb_ota_ctx.block_count - 1,
                   (lb_ota_ctx.block_count - 1) * 512);
            lb_ota_ctx.buf_pos = 0;

            WDT_CLR();  // Flash 写前喂狗，防止擦写阻塞超时复位
            ota_pack_write(lb_ota_ctx.buf);
            WDT_CLR();  // Flash 写后喂狗，确保系统不卡死

            if (ota_pack_get_err() != FOT_ERR_OK) {
                printf("OTA write err: 0x%x\n", ota_pack_get_err());
                lb_ble_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
                return LB_ERR_EXEC_FAIL;
            }
        }
    }

    lb_ota_ctx.next_offset = offset + orig_data_size;
    lb_ota_ctx.recv_size += orig_data_size;

    lb_ble_send_response(LB_CMD_OTA_DATA, rx->msg_flag, LB_ERR_SUCCESS, NULL, 0);
    return LB_ERR_SUCCESS;
}

/**
 * @brief 0x0e — 升级结束 (v1.0.7)
 *
 * APP 发送: 1 字节 [target(1B)] 指定结束哪个设备的升级
 * MCU 返回: 2 字节 [target(1B)][result(1B)]
 *   result: 0x00=升级失败, 0x01=升级成功
 */
u8 lb_handler_ota_end(lb_rx_frame_t *rx)
{
    if (!rx->data || rx->data_len < 1) {
        lb_ble_send_response(LB_CMD_OTA_END, rx->msg_flag, LB_ERR_EXEC_FAIL, NULL, 0);
        return LB_ERR_EXEC_FAIL;
    }

    u8 target = rx->data[0];
    u8 result = LB_OTA_RESULT_FAIL;
    bool write_ok = true;  // 追踪写入是否成功，决定是否进入校验

    printf("OTA end: target=0x%02X recv_size=%lu fw_size=%lu\n",
           target, lb_ota_ctx.recv_size, lb_ota_ctx.fw_size);

    if (lb_ota_ctx.state >= LB_OTA_READY) {
        // 刷出最后一块 (不足 512 字节的部分补 0)
        if (lb_ota_ctx.buf_pos > 0) {
            u16 pad_start = lb_ota_ctx.buf_pos;
            memset(lb_ota_ctx.buf + lb_ota_ctx.buf_pos, 0, 512 - lb_ota_ctx.buf_pos);
            lb_ota_ctx.block_count++;
            printf("OTA BLK[%lu] off=0x%06lX (last, pad from %d)\n",
                   lb_ota_ctx.block_count - 1,
                   (lb_ota_ctx.block_count - 1) * 512,
                   pad_start);

            WDT_CLR();
            ota_pack_write(lb_ota_ctx.buf);
            WDT_CLR();
            if (ota_pack_get_err() != FOT_ERR_OK) {
                printf("OTA write err on final block: 0x%x\n", ota_pack_get_err());
                write_ok = false;
            }
            lb_ota_ctx.buf_pos = 0;
        }

        // FOTA 校验 & 完成
        if (write_ok) {
            printf("\n========================================\n");
            printf("OTA FOTA VERIFICATION\n");
            printf("========================================\n");
            printf("  接收字节: %lu / %lu\n", lb_ota_ctx.recv_size, lb_ota_ctx.fw_size);
            printf("  总块数:   %lu\n", lb_ota_ctx.block_count);

            if (ota_pack_is_write_done()) {
                WDT_CLR();
                if (ota_pack_verify()) {
                    WDT_CLR();
                    ota_pack_done();
                    printf("  VERIFY: 校验通过\n");
                    printf("  DONE: 升级完成, 3秒后复位\n");
                    result = LB_OTA_RESULT_SUCCESS;
                    lb_ota_ctx.need_reset = true;
                    lb_ota_reset_tick = tick_get();
                } else {
                    printf("  VERIFY: 校验失败 (err=0x%x)\n", ota_pack_get_err());
                }
            } else {
                printf("  WRITE: 数据未完整写入 (err=0x%x)\n", ota_pack_get_err());
            }
            printf("========================================\n\n");
        }
    }

    // 清理状态 (need_reset 保持，由 lb_ota_process 处理复位)
    lb_ota_ctx.state = LB_OTA_IDLE;

    if (result != LB_OTA_RESULT_SUCCESS) {
        unlock_code_fota();  // 升级失败，解锁代码区
    }
    WDT_EN();  // 恢复正常 WDT 超时 (~1s)

    u8 rsp[2];
    rsp[0] = target;
    rsp[1] = result;
    lb_ble_send_response(LB_CMD_OTA_END, rx->msg_flag, LB_ERR_SUCCESS, rsp, 2);
    return LB_ERR_SUCCESS;
}

//-----------------------------------------------------------------------------
// OTA 升级流程管理
//-----------------------------------------------------------------------------

/**
 * @brief OTA 升级流程处理 (需在主循环中轮询调用)
 */
void lb_ota_process(void)
{
    if (lb_ota_ctx.need_reset && lb_ota_reset_tick) {
        if (tick_check_expire(lb_ota_reset_tick, 3000)) {
            printf("OTA reset now...\n");
            WDT_RST();
        }
    }
}

bool lb_ota_is_active(void)
{
    return (lb_ota_ctx.state >= LB_OTA_READY) || heat_ota_is_active();
}

#endif // FUNC_LUNCHBOX_UART_EN
