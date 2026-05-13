#ifndef __APP_H
#define __APP_H

#include "app_ab_link.h"
#include "app_watch.h"
#include "bt_fota.h"
#include "bsp_app.h"
#include "bsp_ble/hid/ble_hid_service.h"

void ble_app_init(void);
void ble_app_process(void);
bool ble_app_need_wakeup(void);
void ble_app_connect_callback(void);
void ble_app_disconnect_callback(void);
void ble_app_client_cfg_callback(u16 handle, u8 cfg);
void ble_app_ancs_client_notifiy_callback(u8 id, const char *att_name, const char *att_content);
void app_once_connect(u8 ble_app_ios);
#endif
