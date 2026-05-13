#ifndef __BSP_CALL_MGR_H__
#define __BSP_CALL_MGR_H__

typedef enum {
    CALL_MGR_NONE,

    CALL_MGR_BT_INCOM = 1,
    CALL_MGR_BT_OUTGO,
    CALL_MGR_BT_INCALL,
    CALL_MGR_BT_ENDCALL,

    CALL_MGR_MODEM_INCOM = 1 << 5,
    CALL_MGR_MODEM_OUTGO,
    CALL_MGR_MODEM_INCALL,
    CALL_MGR_MODEM_ENDCALL,
} bsp_call_mgr_msg_t;

typedef enum {
    CALL_AUDIO_PATH_NONE,
    CALL_AUDIO_PATH_BTHF2DAC,
    CALL_AUDIO_PATH_MODEM2DAC,
    CALL_AUDIO_PATH_MODEM2BTAG,
} bsp_call_audio_path_t;

typedef enum {
    CALL_UPDATE_IDLE,
    CALL_UPDATE_NOW,
    CALL_UPDATE_DELAY,
} bsp_call_mgr_update_t;

typedef struct {
    volatile bsp_call_mgr_update_t update;
    u8 msg;

#if CALL_MGR_DEFAULT
    u8 modem_call_num;
    u8 bt_call_num;
    u8 sta;
#endif
    u32 ticks;
} bsp_call_mgr_t;

extern bsp_call_mgr_t call_mgr;

void bsp_call_mgr_init(void);
void bsp_call_mgr_send(bsp_call_mgr_msg_t msg);
bool bsp_call_mgr_need_update(void);

void bsp_call_mgr_set_audio_path(bsp_call_audio_path_t path);
#endif
