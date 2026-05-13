#include "include.h"

void sco_audio_set_param(u8 *ptr, u8 size);

#if BT_SCO_DBG_EN
void bsp_aec_ack_param(u8 type)
{
    u8 buf[16], size = 12;
    buf[0] = 'a';
    buf[1] = 'e';
    buf[2] = 'c';
    buf[3] = (u8)xcfg_cb.bt_anl_gain;
    buf[4] = (u8)xcfg_cb.bt_dig_gain;
    buf[5] = (u8)xcfg_cb.mic_post_gain;
    buf[6] = (u8)xcfg_cb.bt_sco_nr_level;
    buf[7] = 0;
    buf[8] = (u8)xcfg_cb.bt_echo_level;
    buf[9] = (u8)xcfg_cb.bt_far_offset;

#if (USE_APP_TYPE == APP_AB_LINK)
    if (type) {
        ble_app_ab_link_send_packet(buf, size);
    } else
#endif
    {
        bt_spp_tx(SPP_SERVICE_CH0, buf, size);
    }
}

void bsp_aec_ack(u8 type)
{
    u8 buf[6];
    buf[0] = 'a';
    buf[1] = 'e';
    buf[2] = 'c';
    buf[3] = ' ';
    buf[4] = 'o';
    buf[5] = 'k';
#if (USE_APP_TYPE == APP_AB_LINK)
    if (type) {
        ble_app_ab_link_send_packet(buf, 6);
    } else
#endif
    {
        bt_spp_tx(SPP_SERVICE_CH0, buf, 6);
    }
}
#endif

#if (EQ_DBG_IN_SPP && BT_SCO_APP_DBG_EN && BT_SPP_EN)

#define MAX_SPP_TX_NUM          4
#define MAX_SPP_TX_LEN          128
#define SPP_TX_POOL_SIZE       (MAX_SPP_TX_LEN + sizeof(struct txbuf_tag)) * MAX_SPP_TX_NUM

AT(.nr_buf.spp_tx)
uint8_t spp_tx_pool[SPP_TX_POOL_SIZE];

void spp_txpkt_init(void)
{
    spp_txpkt_init_with_pool(spp_send_kick, spp_tx_pool, MAX_SPP_TX_NUM, MAX_SPP_TX_LEN);
}
#endif

void bt_spp_cmd_process(u8 *ptr, u16 size, u8 type)
{
#if EQ_DBG_IN_SPP
    if (xcfg_cb.eq_dgb_spp_en) {
        eq_spp_cb_t *p = &eq_dbg_cb.eq_spp_cb;
        if ((ptr[0] == 'E' && ptr[1] == 'Q') || (ptr[0] == 'D' && ptr[1] == 'R')) {       //EQ消息
            if (ptr[2] == '?') {
                memcpy(eq_rx_buf, ptr, size);
                msg_enqueue(EVT_ONLINE_SET_EQ);
                return;
            }
            u32 rx_size = little_endian_read_16(ptr, 4) + 6;
            memcpy(eq_rx_buf, ptr, size);
            p->rx_size = rx_size;
            if (size < rx_size) {
                p->remain = 1;
                p->remian_ptr = size;
            } else {
                p->remain = 0;
                p->remian_ptr = 0;
                msg_enqueue(EVT_ONLINE_SET_EQ);
            }
            return;
        }
        if (p->remain) {
            memcpy(&eq_rx_buf[p->remian_ptr], ptr, size);
            p->remian_ptr += size;
            if (p->rx_size == p->remian_ptr) {
                msg_enqueue(EVT_ONLINE_SET_EQ);
                memset(p, 0, sizeof(eq_spp_cb_t));
            }
            return;
        }
    }
#endif

//    printf("SPP RX:");
//    print_r(ptr, size);

#if BT_SCO_DBG_EN
    if (ptr[0] == 'a' && ptr[1] == 'e' && ptr[2] == 'c') {
        if (ptr[0] == 'a' && ptr[1] == 'e' && ptr[2] == 'c') {
            if (ptr[3] == '?') {
                bsp_aec_ack_param(type);
                return;
            }
            if (size == 12) {
//                memcpy(eq_rx_buf, ptr, size);
//                eq_dbg_cb.eq_spp_cb.rx_size = size;
//                msg_enqueue(EVT_ONLINE_SET_AEC);

                sco_audio_set_param(ptr, size);
                bsp_aec_ack(type);
            }
            return;
        }
    }
#endif

    //BTControl APP测试命令
    switch (ptr[0]) {
        case 0x01:
            //system control
            if (size != 4) {
                return;     //2byte data + 2byte crc16
            }
            switch (ptr[1]) {
                case 0x01:
                    bt_music_play_pause();
                    break;
                case 0x02:
                    bt_music_prev();
                    break;
                case 0x03:
                    bt_music_next();
                    break;
                case 0x04:
                    bt_volume_down();
                    break;
                case 0x05:
                    bt_volume_up();
                    break;
            }
            break;
    }
}

void spp_rx_callback(uint8_t *bd_addr, uint8_t ch, uint8_t *packet, uint16_t size)
{
#if BT_SPP_EN
    bt_spp_cmd_process(packet, size, 0);

#if BT_SCO_APP_DBG_EN
    bt_sco_app_dbg_proc(packet, size);
#endif //BT_SCO_APP_DBG_EN

#endif // BT_SPP_EN
}

void spp_connect_callback(uint8_t *bd_addr, uint8_t ch)
{
    printf("-->spp_connect_callback\n");
#if BT_SCO_APP_DBG_EN
    bt_sco_dbg_connect_ack();
#endif
}

void spp_disconnect_callback(uint8_t *bd_addr, uint8_t ch)
{
    printf("-->spp_disconnect_callback\n");
#if BT_SPP_EN && BT_SCO_APP_DBG_EN
    spp_txpkt_init(); //clear spp tx buf
#endif //BT_SPP_EN
}
