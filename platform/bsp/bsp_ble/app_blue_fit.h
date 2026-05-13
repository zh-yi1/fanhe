#ifndef __APP_BLUE_FIT_H
#define __APP_BLUE_FIT_H

#include "include.h"

void ble_app_watch_init(void);
void ble_app_watch_process(void);
bool ble_app_watch_need_wakeup(void);
void ble_app_watch_disconnect_callback(void);
void ble_app_watch_connect_callback(void);
void ble_app_watch_client_cfg_callback(u16 handle, u8 cfg);

#endif // __APP_BLUE_FIT_H
