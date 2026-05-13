#ifndef _FUNC_BT_H
#define _FUNC_BT_H

#define BT_WARN_CON             BIT(0)
#define BT_WARN_HID_CON         BIT(1)
#define BT_WARN_DISCON          BIT(4)
#define BT_WARN_HID_DISCON      BIT(5)
#define BT_WARN_PAIRING         BIT(6)

#define INCALL_FLAG_SCO         BIT(0)
#define INCALL_FLAG_CALL        BIT(1)
#define INCALL_FLAG_FADE        (INCALL_FLAG_SCO|INCALL_FLAG_CALL)

enum {
    AEC_PARAM_MIC_ANL_GAIN,
    AEC_PARAM_MIC_DIG_GAIN,
    AEC_PARAM_MIC_POST_GAIN,
    AEC_PARAM_NOISE,
    AEC_PARAM_LEVEL,
    AEC_PARAM_OFFSET,
};

void sfunc_bt_ring(void);
void sfunc_bt_call(void);
void sfunc_bt_ota(void);

void func_bt_mp3_res_play(u32 addr, u32 len);
void func_bt_mp3_play_restore(void);
void func_bt_exit(void);
void func_bt_init(void);
void func_bt_chk_off(void);
void func_bt_sub_process(void);
void func_bt_process(void);
void func_bt_enter(void);

void bt_sco_magic_voice_switch(void);
const char *hfp_get_last_call_number(uint8_t index);


#endif // _FUNC_BT_H
