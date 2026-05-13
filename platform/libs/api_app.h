#ifndef _API_AB_H_
#define _API_AB_H_

#define     INSTRUCT_HEADER_LEN             5           //指令头长度

//协议解析
void ble_uart_service_write(u8 *buff, u8 len);			//协议数据解析, buff:接收数据缓冲区; len:长度;
void ab_app_sync_info(void);                            //同步设备数据
void ab_app_tx_battery(u8 value, bool charge_sta);      //电量上报, value:电量值; charge_sta:充电状态;
bool ab_app_tx_search_phone(bool en);                   //查找手机
#endif // _API_AB_H_
