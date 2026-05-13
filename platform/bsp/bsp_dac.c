#include "include.h"
#include "bsp_dac.h"

uint8_t cfg_spk_mute_en = LOUDSPEAKER_MUTE_EN;
uint16_t cfg_pcm_out_format = DAC_24BITS_EN * PCM_OUT_24BITS;
uint8_t cfg_dac_out_spr = DAC_OUT_SPR;
const u8 *dac_dvol_table;

#if DAC_24BITS_EN
AT(.dac_obuf.src)
u32 dac_obuf[(DAC_OBUF_SIZE + DAC_OBUF_EXSIZE)*(1+DAC_24BITS_EN)];
#endif

AT(.rodata.dac)
const u16 dac_dvol_tbl_db[61] = {
    DIG_N0DB,  DIG_N1DB,  DIG_N2DB,  DIG_N3DB,  DIG_N4DB,  DIG_N5DB,  DIG_N6DB,  DIG_N7DB,
    DIG_N8DB,  DIG_N9DB,  DIG_N10DB, DIG_N11DB, DIG_N12DB, DIG_N13DB, DIG_N14DB, DIG_N15DB,
    DIG_N16DB, DIG_N17DB, DIG_N18DB, DIG_N19DB, DIG_N20DB, DIG_N21DB, DIG_N22DB, DIG_N23DB,
    DIG_N24DB, DIG_N25DB, DIG_N26DB, DIG_N27DB, DIG_N28DB, DIG_N29DB, DIG_N30DB, DIG_N31DB,
    DIG_N32DB, DIG_N33DB, DIG_N34DB, DIG_N35DB, DIG_N36DB, DIG_N37DB, DIG_N38DB, DIG_N39DB,
    DIG_N40DB, DIG_N41DB, DIG_N42DB, DIG_N43DB, DIG_N44DB, DIG_N45DB, DIG_N46DB, DIG_N47DB,
    DIG_N48DB, DIG_N49DB, DIG_N50DB, DIG_N51DB, DIG_N52DB, DIG_N53DB, DIG_N54DB, DIG_N55DB,
    DIG_N56DB, DIG_N57DB, DIG_N58DB, DIG_N59DB, DIG_N60DB,
};

AT(.rodata.dac)
const u8 dac_dvol_tbl_16[16 + 1] = {
    60,  43,  32,  26,  24,  22,  20,  18, 16,
    14,  12,  10,  8,   6,   4,   2,   0,
};

AT(.rodata.dac)
const u8 dac_dvol_tbl_32[32 + 1] = {
    60,  50,  43,  38,  35,  30,  28,  26,
    24,  23,  22,  21,  20,  19,  18,  17,
    16,  15,  14,  13,  12,  11,  10,  9,
    8,   7,   6,   5,   4,   3,   2,   1,   0,
};

AT(.text.bsp.dac)
void bsp_change_volume(u8 vol)
{
    u8 level = 0;

    if (vol <= VOL_MAX) {
        level = dac_dvol_table[vol] + sys_cb.gain_offset;
        if (level > 60) {
            level = 60;
        }
        dac_set_dvol(dac_dvol_tbl_db[level]);
    }
}

AT(.text.bsp.dac)
bool bsp_set_volume(u8 vol)
{
    bsp_change_volume(vol);
    if (vol == sys_cb.vol) {
//        gui_box_show_vol();
        return false;
    }

    if (vol <= VOL_MAX) {
        sys_cb.vol = vol;
//        gui_box_show_vol();
        param_sys_vol_write();
        sys_cb.cm_times = 0;
        sys_cb.cm_vol_change = 1;
    }
    return true;
}

AT(.com_text.dac)
void dac_set_mute_callback(u8 mute_flag)
{
    if (mute_flag) {
        bsp_loudspeaker_mute();
    } else {
        if (!sys_cb.mute) {
            bsp_loudspeaker_unmute();
            //DAC延时淡入，防止UNMUTE时间太短导致喇叭声音不全的问题
            dac_unmute_set_delay(LOUDSPEAKER_UNMUTE_DELAY);
        }
    }
}

AT(.text.bsp.dac)
u8 bsp_volume_inc(u8 vol)
{
    vol++;
    if(vol > VOL_MAX)
        vol = VOL_MAX;
    return vol;
}

AT(.text.bsp.dac)
u8 bsp_volume_dec(u8 vol)
{
    if(vol > 0)
        vol--;
    return vol;
}

AT(.text.dac)
void dac_set_anl_offset(u8 bt_call_flag)
{
    if (bt_call_flag) {
        sys_cb.gain_offset = BT_CALL_MAX_GAIN;
    } else {
        sys_cb.gain_offset = DAC_MAX_GAIN;
    }
}

AT(.text.bsp.dac)
static void dac_set_vol_table(u8 vol_max)
{
    if (vol_max == 16) {
        dac_dvol_table = dac_dvol_tbl_16;
    } else {
        dac_dvol_table = dac_dvol_tbl_32;
    }
}

AT(.text.bsp.dac)
void dac_init(void)
{
    dac_set_vol_table(xcfg_cb.vol_max);
    dac_set_anl_offset(0);
    printf("[%s] vol_max:%d, offset: %d\n", __func__, xcfg_cb.vol_max, sys_cb.gain_offset);

    dac_aubuf_init();
    dac_power_on();
    plugin_music_eq();

#if DAC_DNR_EN
    dac_dnr_init(2, 0x18, 90, 0x10);
#endif

//    //测试用
//    void dac_tester(void);
//    dac_tester();
}


