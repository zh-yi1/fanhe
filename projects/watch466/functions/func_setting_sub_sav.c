#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define VOL_CHANGE          4  //音量等级每次增加或者减少4
#define MOTOR_MAX_LEVEL     4  //马达最大等级

typedef struct f_sav_t_ {
    u8 vol_swi;
    u8 shk_swi;
} f_sav_t;

enum {
    //按键
    COMPO_ID_BTN_REDUCE = 1,
    COMPO_ID_BTN_INCREASE,
    COMPO_ID_BTN_SWI_SHK,
    COMPO_ID_BTN_SWI_VOL,

    //图像
	COMPO_ID_PIC_LEVEL1,            //等级 1~5
	COMPO_ID_PIC_LEVEL2,
	COMPO_ID_PIC_LEVEL3,
	COMPO_ID_PIC_LEVEL4,
	COMPO_ID_PIC_LEVEL5,
};

typedef struct sav_disp_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} sav_disp_pic_item_t;

#define SAV_DISP_PIC_ITEM_CNT                       ((int)(sizeof(tbl_sav_disp_pic_item) / sizeof(tbl_sav_disp_pic_item[0])))
//图片item，创建时遍历一下
static const sav_disp_pic_item_t tbl_sav_disp_pic_item[] = {
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL1,          157,    349,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL2,          195,    349,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL3,          233,    349,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL4,          271,    349,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL5,          309,    349,    false},
};

//声音与振动页面
compo_form_t *func_set_sub_sav_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
    compo_form_add_image(frm, UI_BUF_COMMON_BG1_BIN, GUI_SCREEN_CENTER_X, 349);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SETTING_SAV]);

    //创建文本
    compo_textbox_t *txt_voice = compo_textbox_create(frm, 10);
    compo_textbox_set_align_center(txt_voice, false);
    compo_textbox_set_pos(txt_voice, 75, 146);
    compo_textbox_set(txt_voice, i18n[STR_SHK]);

    compo_textbox_t *txt_call = compo_textbox_create(frm, 10);
    compo_textbox_set_align_center(txt_call, false);
    compo_textbox_set_pos(txt_call, 75, 248);
    compo_textbox_set(txt_call, i18n[STR_VOL]);

    //新建按钮
	compo_button_t *btn;
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_SWITCH_BUTTON_ON_BIN);
    compo_setid(btn, COMPO_ID_BTN_SWI_SHK);
    compo_button_set_pos(btn, 368, 166);

    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_SWITCH_BUTTON_ON_BIN);
    compo_setid(btn, COMPO_ID_BTN_SWI_VOL);
    compo_button_set_pos(btn, 368, 268);

    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_REDUCE1_BIN);
    compo_setid(btn, COMPO_ID_BTN_REDUCE);
    compo_button_set_pos(btn, 86, 349);

	btn = compo_button_create_by_image(frm, UI_BUF_COMMON_INCREASE1_BIN);
    compo_setid(btn, COMPO_ID_BTN_INCREASE);
    compo_button_set_pos(btn, 370, 349);

    //新建图像
    compo_picturebox_t *pic_level[5];
    for (u8 idx = 0; idx < SAV_DISP_PIC_ITEM_CNT; idx++) {
        pic_level[idx] = compo_picturebox_create(frm, tbl_sav_disp_pic_item[idx].res_addr);
        compo_setid(pic_level[idx], tbl_sav_disp_pic_item[idx].pic_id);
        compo_picturebox_set_pos(pic_level[idx], tbl_sav_disp_pic_item[idx].x, tbl_sav_disp_pic_item[idx].y);
        compo_picturebox_set_visible(pic_level[idx], tbl_sav_disp_pic_item[idx].visible_en);
    }

    compo_picturebox_set_visible(pic_level[0], true);
    for (u8 i=0; i<VOL_MAX; i+=4) {
        if (i < sys_cb.vol) {
            compo_picturebox_set_visible(pic_level[i/4 + 1], true);
        } else {
            compo_picturebox_set_visible(pic_level[i/4 + 1], false);
        }
    }

    return frm;
}

//声音与振动事件处理
static void func_set_sub_sav_process(void)
{
    func_process();
}

//更新显示界面
static void func_set_sub_sav_disp(void)
{
    f_sav_t *sav = (f_sav_t *)func_cb.f_cb;
    compo_picturebox_t *pic_level[SAV_DISP_PIC_ITEM_CNT];
    compo_button_t *btn_shk = compo_getobj_byid(COMPO_ID_BTN_SWI_SHK);
    compo_button_t *btn_vol = compo_getobj_byid(COMPO_ID_BTN_SWI_VOL);

    for (int i=0;i<SAV_DISP_PIC_ITEM_CNT;i++) {
        pic_level[i] = compo_getobj_byid(COMPO_ID_PIC_LEVEL1 + i);
    }

    if (sys_cb.vol > VOL_MAX) {
        sys_cb.vol = VOL_MAX;
    } else if (sys_cb.vol < 0) {
        sys_cb.vol = 0;
    }

    for (u8 i=0; i<VOL_MAX; i+=VOL_CHANGE) {
        if (i < sys_cb.vol) {
            compo_picturebox_set_visible(pic_level[i/VOL_CHANGE + 1], true);
        } else {
            compo_picturebox_set_visible(pic_level[i/VOL_CHANGE + 1], false);
        }
    }

    if (sav->shk_swi) {
        compo_button_set_bgimg(btn_shk, UI_BUF_COMMON_SWITCH_BUTTON_ON_BIN);
    } else {
        compo_button_set_bgimg(btn_shk, UI_BUF_COMMON_SWITCH_BUTTON_OFF_BIN);
    }

    if (sav->vol_swi) {
        compo_button_set_bgimg(btn_vol, UI_BUF_COMMON_SWITCH_BUTTON_ON_BIN);
    } else {
        compo_button_set_bgimg(btn_vol, UI_BUF_COMMON_SWITCH_BUTTON_OFF_BIN);
    }
}

//单击按钮
static void func_sav_button_click(void)
{
    f_sav_t *sav = (f_sav_t *)func_cb.f_cb;
    int id = compo_get_button_id();
    switch(id) {
    case COMPO_ID_BTN_SWI_SHK:
        sav->shk_swi ^= 1;
        break;

    case COMPO_ID_BTN_SWI_VOL:
        sav->vol_swi ^= 1;
        break;

    case COMPO_ID_BTN_REDUCE:       //音频音量
        if (sys_cb.vol > 0 && sys_cb.vol <= VOL_MAX) {
            sys_cb.vol -= VOL_CHANGE;
        }
        break;

    case COMPO_ID_BTN_INCREASE:
        if (sys_cb.vol >= 0 && sys_cb.vol <= VOL_MAX) {
            sys_cb.vol += VOL_CHANGE;
        }
        break;

    default:
        break;
    }
    func_set_sub_sav_disp();
}

//声音与振动功能消息处理
static void func_set_sub_sav_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_sav_button_click();
        break;

    default:
        func_message(msg);
        break;
    }

}

//进入声音与振动功能
void func_set_sub_sav_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sav_t));
    func_cb.frm_main = func_set_sub_sav_form_create();
    f_sav_t *sav = (f_sav_t *)func_cb.f_cb;
    sav->shk_swi = 1;
    sav->vol_swi = 1;
}

//退出声音与振动功能
void func_set_sub_sav_exit(void)
{
    func_cb.last = FUNC_SET_SUB_SAV;
}

//声音与振动功能
void func_set_sub_sav(void)
{
    printf("%s\n", __func__);
    func_set_sub_sav_enter();
    while (func_cb.sta == FUNC_SET_SUB_SAV) {
        func_set_sub_sav_process();
        func_set_sub_sav_message(msg_dequeue());
    }
    func_set_sub_sav_exit();
}
