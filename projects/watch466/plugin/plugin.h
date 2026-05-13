#ifndef _PLUGIN_H
#define _PLUGIN_H

#include "user_halt.h"
#include "multi_lang.h"
#include "bt_call.h"
#include "port_ctp.h"
#include "port_sd.h"

void plugin_init(void);
void plugin_var_init(void);
void plugin_tmr5ms_isr(void);
void plugin_tmr1ms_isr(void);
void maxvol_tone_play(void);
void minvol_tone_play(void);
void plugin_music_eq(void);
void plugin_playmode_warning(void);

void loudspeaker_mute_init(void);
void loudspeaker_mute(void);
void loudspeaker_unmute(void);
void loudspeaker_disable(void);
void *ab_zalloc(size_t size);
void ab_free(void *ptr);

void amp_sel_cfg_init(u8 io_num);
void amp_sel_cfg_d(void);
void amp_sel_cfg_ab(void);
void amp_sel_cfg_dis(void);

bool bt_hfp_ring_number_en(void);
void sleep_wakeup_config(void);
void sleep_wakeup_exit(void);
bool is_sleep_dac_off_enable(void);
void plugin_vbat_filter(u32 *vbat);

void plugin_lowbat_vol_reduce(void);
void plugin_lowbat_vol_recover(void);



#endif
