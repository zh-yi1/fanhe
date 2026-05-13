/**********************************************************************
*
*   strong_ble.c
*   定义库里面ble部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

void strong_le_include(void) {}
#if !LE_EN
void ble_txpkt_init(void) {}
AT(.com_text.stack.ble.mtu)
u16 ble_get_gatt_mtu(void) { return 0; }
void btstack_ble_send_req(void) {}
int ble_send_notify_packet(void *context, void *buf) {return -1;}
void att_event_server_send(void) {}
//void hci_run_le_connection(void) {}
bool ble_event_cmd_complete(uint8_t *packet, int size) { return false; }
void ble_event_meta(uint8_t *packet, int size) {}
void btstack_ble_init(void) {}
void btstack_gatt_init(void) {}
void btstack_ble_update_conn_param(void) {}
void btstack_ble_set_adv_interval(void) {}
void btstack_ble_set_addr(void) {}

typedef uint8_t (*bb_msg_func_t)(uint16_t index, void const *param);
typedef uint8_t (*bb_cmd_func_t)(uint16_t cmd, void const *param);
typedef uint8_t (*ll_cntl_func_t)(uint8_t opcode);

struct ll_cntl_pdu_info
{
    ll_cntl_func_t  func;
    const char*     desc;
    uint16_t        length;
    uint8_t         flag;
};

struct bb_msg_info
{
    uint16_t index;
    bb_msg_func_t func;
};

struct bb_cmd_info
{
    uint16_t index;
    bb_cmd_func_t func;
};

AT(.rodata.le.ll_proc)
const struct bb_cmd_info ll_hci_cmd_tbl[1] = {0};
AT(.rodata.le.ll_proc)
const struct bb_msg_info ll_msg_tbl[1] = {0};
AT(.rodata.le.ll_cntl)
const struct ll_cntl_pdu_info ll_cntl_pdu_tbl[1] = {0};
AT(.rodata.le.ll_proc)
const struct bb_msg_info mgr_hci_cmd_tbl[1] = {0};
void ll_init(uint8_t init_type) {}
uint8_t ll_start(uint8_t index, void *param) {return -1;}
void ll_stop(uint8_t index) {}
void ll_cntl_state_set(uint8_t index, uint8_t txrx, uint8_t state) {}
void ll_proc_timer_set(uint8_t index, uint8_t type, bool enable) {}
void ll_proc_timer_set_state(uint8_t index, uint8_t type, bool enable) {}
void ll_cntl_send(uint8_t index, void *pdu, ll_cntl_func_t tx_func) {}
void ll_cntl_tx_check(uint8_t index) {}
bool ble_adv_end_con_ind(void const *param) { return false; }
AT(.com_text.bb.ble.chs)
void ble_channel_assess(uint8_t channel, bool rx_ok, uint32_t ts) {}
AT(.com_text.bb.ble.end)
void ble_con_rx_end(uint8_t index, bool rx_ok, uint16_t rxchass) {}
void aes_init(uint8_t init_type) {};
void aes_result_handler(uint8_t status, uint8_t* result) {};
void ble_ecpy(uint8_t *key, uint8_t *enc_data) {}
#endif

#if !LE_PAIR_EN
#if (LE_ADDRESS_TYPE == GAP_RANDOM_ADDRESS_TYPE_OFF)
void le_sm_init(void) {}
void gap_random_address_set(void) {}
void gap_random_address_set_mode(void) {}
void le_device_db_init(void){}
#endif
void sm_just_works_confirm(void) {}
void sm_request_pairing(void) {}
void sm_send_security_request(void) {}
int sm_authorization_state(void) { return 0; }
int sm_authenticated(void) { return 0; }
int sm_encryption_key_size(void) { return 0; }
void sm_add_event_handler(void) {}
void sm_cmac_signed_write_start(void){ }
int sm_cmac_ready(void) { return 0; }
uint16_t sm_set_setup_context(uint8_t *ptr){ return 0;}
uint16_t sm_get_setup_context(uint8_t *ptr){ return 0;}
uint16_t tws_get_sm_db_info(uint8_t *ptr){ return 0;}
uint16_t tws_set_sm_db_info(uint8_t *ptr, uint16_t len){ return 0;}
#endif

#if !LE_ANCS_CLIENT_EN
void ancs_client_init(void) {}
#endif

#if !LE_AMS_CLIENT_EN
void ams_client_init(void) {}
void ble_ams_remote_ctrl(uint8_t param){}
#endif
