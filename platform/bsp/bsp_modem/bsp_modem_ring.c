#include "include.h"

#if MODEM_CAT1_EN
AT(.text.func.btring)
void bsp_modem_ring_process(void)
{
    func_process();
}

void bsp_modem_ring_enter(void)
{
// #if WARNING_BT_INCALL
    // memset(&f_bt_ring.play, 0, sizeof(t_play_cb));
    f_bt_ring.play.gain_offset = sys_cb.gain_offset;
    // f_bt_ring.play.ring_tick = tick_get() - 5000;
    // f_bt_ring.play.ios_ring_tick = tick_get();
    // bt_cb.ring_stop = 0;
    dac_set_anl_offset(0);
    bt_audio_bypass();
    music_control(MUSIC_MSG_STOP);
    bsp_change_volume(WARNING_VOLUME);

    // //先报号码，再响铃
    // for(int i=0; i<30; i++) {
    //     bsp_modem_status();
    //     if(f_bt_ring.len != 0 || !bt_is_ring() || (bt_cb.disp_status != BT_STA_INCOMING)) {
    //         break;
    //     }
    //     delay_5ms(10);
    // }
    // f_bt_ring.ring_sync = 6;
// #endif // WARNING_BT_INCALL
}

void bsp_modem_ring_exit(void)
{
#if WARNING_BT_INCALL
// #if BT_HFP_INBAND_RING_EN
//     if (f_bt_ring.play.inband_ring_sta == 2) {
//         f_bt_ring.play.inband_ring_sta = 0;
//         bt_audio_bypass();
//     }
// #endif
    // f_bt_ring.len = 0;
    mp3_res_play_exit();
    music_control(MUSIC_MSG_STOP);
    sys_cb.gain_offset = f_bt_ring.play.gain_offset;
    bsp_change_volume(sys_cb.vol);
    bt_audio_enable();
#endif
}

#endif
