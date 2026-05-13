#include "include.h"

#if BT_SCO_MAV_EN
//魔音化
static s16  buf_01[1536]  AT(.mavsco_buf);
static s16  buf_02[128]   AT(.mavsco_buf);
static s16  buf_03[128]   AT(.mavsco_buf);
static s16  buf_04[512]   AT(.mavsco_buf);
static s16  buf_05[64]    AT(.mavsco_buf);
static s16  buf_06[64]    AT(.mavsco_buf);
static char buf_07[512]   AT(.mavsco_buf);

typedef struct {
    u8 res[2];
    s16 *buf_used;
    s16 *inbuf;
    s16 *outbuf;
    s16 *cache;
    s16 *mav_out;
    s16 *out1_buf;
    char *s8_fdata;
} mav_cfg_t;

mav_cfg_t mav_cfg_te = {
    .res[0]         = 0,
    .res[1]         = 0,
    .buf_used           = buf_01,
    .inbuf              = buf_02,
    .outbuf             = buf_03,
    .cache              = buf_04,
    .mav_out            = buf_05,
    .out1_buf           = buf_06,
    .s8_fdata           = buf_07,
};

#define MAV_CHCHE_LEN        512
typedef struct {
    u16 wptr;
    u16 rptr;
    u16 optr;
    u16 len;
    u16 olen;
    s16 buf[MAV_CHCHE_LEN];
} mav_cache_t;
mav_cache_t mav_cache AT(.mavsco_buf.cache);

void mav_kick_start(void);
void magic_voice_init(void *cfg);
void magic_voice_set_level(u8 type, u8 level);
void lock_code_wsola(void);
void unlock_code_wsola(void);

AT(.rodata.magic)
const u8 magic_table[4][2] = {
    {0, 5},
    {0, 2},
    {1, 1},
    {1, 4},
};

///切换魔音
void bt_sco_magic_voice_switch(void)
{
    //设置魔音级别
    u8 magic_type, magic_level;
    static int magic_sel_ce = 0;
    magic_sel_ce++;
    if (magic_sel_ce > 4) {
        magic_sel_ce = 0;
    }
    if (!magic_sel_ce) {
        magic_type = 0;
        magic_level = 0;
    } else {
        magic_type = magic_table[magic_sel_ce - 1][0];
        magic_level = magic_table[magic_sel_ce - 1][1];
    }
    magic_voice_set_level(magic_type, magic_level);
//    printf("MAGIC VOICE ---> (%d)\n", magic_sel_ce);
}

AT(.text.mav.proc)
void bt_sco_mav_process_do(u8 *ptr, u32 samples)
{
    memcpy(mav_cfg_te.inbuf, ptr, 256);
    memcpy(ptr, mav_cfg_te.outbuf, 256);
    mav_kick_start();
}

AT(.text.mav.proc)
void bt_sco_mav_process(u8 *buf, u32 samples)
{
    u16 len = samples * 2;
    u16 *ptr = (u16 *)buf;
    mav_cache_t *p = &mav_cache;

    if ((MAV_CHCHE_LEN - p->wptr) >= samples) {
        memcpy(&p->buf[p->wptr], ptr, len);
        p->wptr += samples;
        if (p->wptr >= MAV_CHCHE_LEN) {
            p->wptr = 0;
        }
    } else {
        u16 remain = MAV_CHCHE_LEN - p->wptr;
        memcpy(&p->buf[p->wptr], ptr, remain * 2);
        memcpy(p->buf, &ptr[remain], (samples - remain) * 2);
        p->wptr = samples - remain;
    }
    p->len += samples;
    while (p->len >= 128) {
        bt_sco_mav_process_do((u8 *)&p->buf[p->rptr], 128);
        p->rptr += 128;
        if (p->rptr >= MAV_CHCHE_LEN) {
            p->rptr = 0;
        }
        p->len -= 128;
        p->olen += 128;
    }
    if (p->olen >= samples) {
        if ((MAV_CHCHE_LEN - p->optr) >= samples) {
            memcpy(ptr, &p->buf[p->optr], len);
            p->optr += samples;
            if (p->optr >= MAV_CHCHE_LEN) {
                p->optr = 0;
            }
        } else {
            u16 remain = MAV_CHCHE_LEN - p->optr;
            memcpy(ptr, &p->buf[p->optr], remain * 2);
            memcpy(&ptr[remain], p->buf, (samples - remain) * 2);
            p->optr = samples - remain;
        }
        p->olen -= samples;
    } else {
        memset(ptr, 0, len);
    }
}

///魔音初始化
void bt_sco_magic_init(void)
{
    lock_code_wsola();
    magic_voice_init(&mav_cfg_te);
    memset(&mav_cache, 0, sizeof(mav_cache_t));
}

void bt_sco_magic_exit(void)
{
    unlock_code_wsola();
}

#endif // BT_SCO_MAV_EN
