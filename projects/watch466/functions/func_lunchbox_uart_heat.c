/**
 * @file    func_lunchbox_uart_heat.c
 * @brief   加热模块 OTA 升级 — 当前为 stub 占位
 *
 * TODO: 改为 BLE→UART 流式转发，避免全量缓存占用 SRAM/DISP
 */
#include "include.h"
#include "func_lunchbox_uart.h"
#include "func_lunchbox_uart_internal.h"
#include "func_lunchbox_uart_heat.h"

#if FUNC_LUNCHBOX_UART_EN

u8 heat_ota_handler_start(lb_rx_frame_t *rx, u8 msg_flag)
{
    (void)rx; (void)msg_flag;
    return LB_ERR_EXEC_FAIL;
}

u8 heat_ota_handler_data(lb_rx_frame_t *rx, u8 msg_flag)
{
    (void)rx; (void)msg_flag;
    return LB_ERR_EXEC_FAIL;
}

u8 heat_ota_handler_end(lb_rx_frame_t *rx, u8 msg_flag)
{
    (void)rx; (void)msg_flag;
    return LB_ERR_EXEC_FAIL;
}

void heat_ota_process(void) { }

void heat_ota_uart_response(lb_rx_frame_t *rx)
{
    (void)rx;
}

bool heat_ota_is_active(void) { return false; }

heat_ota_state_t heat_ota_get_state(void) { return HEAT_OTA_IDLE; }

#endif // FUNC_LUNCHBOX_UART_EN
