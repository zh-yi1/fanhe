#ifndef __AB_LINK_H
#define __AB_LINK_H

#include "include.h"

void ble_app_ab_link_init(void);
void ble_app_ab_link_process(void);
bool ble_app_ab_link_need_wakeup(void);
bool ble_app_ab_link_send_packet(u8 *buf, u16 len);

//APP protocol
int app_protocol_tx(u8 *buf, u8 len);

#endif // __AB_LINK_H
