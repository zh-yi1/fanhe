#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_volume_t_ {

} f_volume_t;

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
	COMPO_ID_PIC_REDUCE_CLICK,      //按钮触摸效果图
	COMPO_ID_PIC_INCREASE_CLICK,
};

typedef struct volume_disp_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} volume_disp_pic_item_t;

#define VOLUME_DISP_PIC_ITEM_CNT    ((int)(sizeof(tbl_volume_disp_pic_item) / sizeof(tbl_volume_disp_pic_item[0])))

//图片item，创建时遍历一下
static const volume_disp_pic_item_t tbl_volume_disp_pic_item[] = {
    // {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL1,         157,    349,    false},
    // {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL2,         195,    349,    false},
    // {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL3,         233,    349,    false},
    // {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL4,         271,    349,    false},
    // {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL5,         309,    349,    false},
};

//创建调节音量窗体
compo_form_t *func_volume_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
    // compo_form_add_image(frm, UI_BUF_ICON_VOLUME_BIN, GUI_SCREEN_CENTER_X, 220);
    // compo_form_add_image(frm, UI_BUF_COMMON_BG1_BIN, GUI_SCREEN_CENTER_X, 349);

    // //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    // compo_form_set_title_center(frm, true);
    // compo_form_set_title(frm, i18n[STR_VOLUME]);

    // //新建按钮
	// compo_button_t *btn;
    // btn = compo_button_create_by_image(frm, UI_BUF_COMMON_REDUCE1_BIN);
    // compo_setid(btn, COMPO_ID_BTN_REDUCE);
    // compo_button_set_pos(btn, 86, 349);

	// btn = compo_button_create_by_image(frm, UI_BUF_COMMON_INCREASE1_BIN);
    // compo_setid(btn, COMPO_ID_BTN_INCREASE);
    // compo_button_set_pos(btn, 370, 349);

    // //新建图像
    // compo_picturebox_t *pic_level[5];
    // for (u8 idx = 0; idx < VOLUME_DISP_PIC_ITEM_CNT; idx++) {
    //     pic_level[idx] = compo_picturebox_create(frm, tbl_volume_disp_pic_item[idx].res_addr);
    //     compo_setid(pic_level[idx], tbl_volume_disp_pic_item[idx].pic_id);
    //     compo_picturebox_set_pos(pic_level[idx], tbl_volume_disp_pic_item[idx].x, tbl_volume_disp_pic_item[idx].y);
    //     compo_picturebox_set_visible(pic_level[idx], tbl_volume_disp_pic_item[idx].visible_en);
    // }

    // compo_picturebox_set_visible(pic_level[0], true);
    // for (u8 i=0; i<VOL_MAX; i+=4) {
    //     if (i < sys_cb.vol) {
    //         compo_picturebox_set_visible(pic_level[i/4 + 1], true);
    //     } else {
    //         compo_picturebox_set_visible(pic_level[i/4 + 1], false);
    //     }
    // }
    return frm;
}

//音量功能事件处理
static void func_volume_process(void)
{
    func_process();
}

//释放按钮效果处理
static void func_volume_button_release_handle(void)
{
    compo_button_t *btn_reduce = compo_getobj_byid(COMPO_ID_BTN_REDUCE);
    compo_button_t *btn_increase = compo_getobj_byid(COMPO_ID_BTN_INCREASE);
    compo_button_set_alpha(btn_reduce, UI_BTN_NORMAL_EFFECT_ALPHA);
    compo_button_set_alpha(btn_increase, UI_BTN_NORMAL_EFFECT_ALPHA);
}

//单击按钮
static void func_volume_button_click(void)
{
    int id = compo_get_button_id();
    compo_picturebox_t *pic_level[5];
    pic_level[0] = compo_getobj_byid(COMPO_ID_PIC_LEVEL1);
    pic_level[1] = compo_getobj_byid(COMPO_ID_PIC_LEVEL2);
    pic_level[2] = compo_getobj_byid(COMPO_ID_PIC_LEVEL3);
    pic_level[3] = compo_getobj_byid(COMPO_ID_PIC_LEVEL4);
    pic_level[4] = compo_getobj_byid(COMPO_ID_PIC_LEVEL5);

    switch (id) {
    case COMPO_ID_BTN_REDUCE:
        if (sys_cb.vol > 0 && sys_cb.vol <= VOL_MAX) {
            sys_cb.vol -= 4;
        }
        break;

    case COMPO_ID_BTN_INCREASE:
        if (sys_cb.vol >= 0 && sys_cb.vol < VOL_MAX) {
            sys_cb.vol += 4;
        }
        break;

    default:
        break;
    }

    for (u8 i=0; i<VOL_MAX; i++) {
        if (i < sys_cb.vol) {
            compo_picturebox_set_visible(pic_level[i/4 + 1], true);
        } else {
            compo_picturebox_set_visible(pic_level[i/4 + 1], false);
        }
    }
    func_volume_button_release_handle();
}

//音量功能消息处理
static void func_volume_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_volume_button_click();
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入音量控制功能
void func_volume_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_volume_t));
    func_cb.frm_main = func_volume_form_create();
}

//退出音量功能
void func_volume_exit(void)
{
    func_cb.last = FUNC_VOLUME;
}

//音量功能
void func_volume(void)
{
    printf("%s\n", __func__);
    func_volume_enter();
    while (func_cb.sta == FUNC_VOLUME) {
        func_volume_process();
        func_volume_message(msg_dequeue());
    }
    func_volume_exit();
}
