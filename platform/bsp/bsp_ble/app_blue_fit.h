#ifndef __APP_BLUE_FIT_H
#define __APP_BLUE_FIT_H

#include "include.h"

void ble_app_watch_init(void);
void ble_app_watch_process(void);
bool ble_app_watch_need_wakeup(void);
void ble_app_watch_disconnect_callback(void);
void ble_app_watch_connect_callback(void);
void ble_app_watch_client_cfg_callback(u16 handle, u8 cfg);

/**
 * @brief 取出一包饭盒协议数据 (主循环调用, 队列空返回 0)
 *
 * 饭盒协议的接收与分析在 lb_ble_app.c (functions/comm)，不再经过
 * ble_app_watch_process()。本函数是平台侧唯一出口。
 */
u16 ble_app_lunchbox_rx_pop(u8 *out, u16 cap);

#endif // __APP_BLUE_FIT_H
