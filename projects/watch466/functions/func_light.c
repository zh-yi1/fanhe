#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//组件ID
enum {
    //按键
    COMPO_ID_BTN_REDUCE = 1,
    COMPO_ID_BTN_INCREASE,

    //图像
	COMPO_ID_PIC_LEVEL1,            //亮度等级 1~5
	COMPO_ID_PIC_LEVEL2,
	COMPO_ID_PIC_LEVEL3,
	COMPO_ID_PIC_LEVEL4,
	COMPO_ID_PIC_LEVEL5,
	COMPO_ID_PIC_REDUCE_CLICK,      //按钮触摸效果图
	COMPO_ID_PIC_INCREASE_CLICK,
};

typedef struct f_light_t_ {
    int last_y_move;
    u8 light_level;
} f_light_t;

typedef struct light_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} light_pic_item_t;

#define LIGHT_PIC_ITEM_CNT    ((int)(sizeof(tbl_light_pic_item) / sizeof(tbl_light_pic_item[0])))

//图片item，创建时遍历一下
static const light_pic_item_t tbl_light_pic_item[] = {
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL1,         157,    349,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL2,         195,    349,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL3,         233,    349,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL4,         271,    349,    false},
    {UI_BUF_COMMON_BG2_BIN,     COMPO_ID_PIC_LEVEL5,         309,    349,    false},
};

//创建亮度调节窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_light_form_create(void)
{

    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
	compo_form_add_image(frm, UI_BUF_SETTING_LIGHT_LIGHT_BIN, GUI_SCREEN_CENTER_X, 220);
    compo_form_add_image(frm, UI_BUF_COMMON_BG1_BIN, GUI_SCREEN_CENTER_X, 349);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_SETTING_LIGHT]);

	//新建按钮
	compo_button_t *btn;
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_REDUCE1_BIN);
    compo_setid(btn, COMPO_ID_BTN_REDUCE);
    compo_button_set_pos(btn, 86, 349);

	btn = compo_button_create_by_image(frm, UI_BUF_COMMON_INCREASE1_BIN);
    compo_setid(btn, COMPO_ID_BTN_INCREASE);
    compo_button_set_pos(btn, 370, 349);

    //新建图像
    compo_picturebox_t *pic_level[LIGHT_PIC_ITEM_CNT];
    for (u8 idx = 0; idx < LIGHT_PIC_ITEM_CNT; idx++) {
        pic_level[idx] = compo_picturebox_create(frm, tbl_light_pic_item[idx].res_addr);
        compo_setid(pic_level[idx], tbl_light_pic_item[idx].pic_id);
        compo_picturebox_set_pos(pic_level[idx], tbl_light_pic_item[idx].x, tbl_light_pic_item[idx].y);
        compo_picturebox_set_visible(pic_level[idx], tbl_light_pic_item[idx].visible_en);
    }

    printf("%s-->light_level:%d\n",__func__, sys_cb.light_level);
    for (u8 i=0; i<sys_cb.light_level; i++) {
        compo_picturebox_set_visible(pic_level[i],true);
    }

    return frm;
}


//释放按钮效果处理
static void func_light_button_release_handle(void)
{
    compo_button_t *btn_reduce = compo_getobj_byid(COMPO_ID_BTN_REDUCE);
    compo_button_t *btn_increase = compo_getobj_byid(COMPO_ID_BTN_INCREASE);
    compo_button_set_alpha(btn_reduce, UI_BTN_NORMAL_EFFECT_ALPHA);
    compo_button_set_alpha(btn_increase, UI_BTN_NORMAL_EFFECT_ALPHA);
}

//单击按钮
static void func_light_button_click(void)
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
        if (sys_cb.light_level > 0 && sys_cb.light_level <= 5) {
            sys_cb.light_level--;
        }
        break;

    case COMPO_ID_BTN_INCREASE:
        if (sys_cb.light_level >= 0 && sys_cb.light_level < 5) {
            sys_cb.light_level++;
        }
        break;

    default:
        break;
    }

    for (u8 i=0; i<5; i++) {
        if (i < sys_cb.light_level) {
            compo_picturebox_set_visible(pic_level[i],true);
        } else {
            compo_picturebox_set_visible(pic_level[i],false);
        }
    }
    func_light_button_release_handle();
}


//亮度调节功能事件处理
static void func_light_process(void)
{
    func_process();
}

//亮度调节功能消息处理
static void func_light_message(size_msg_t msg)
{

    switch (msg) {
	case MSG_CTP_CLICK:
        func_light_button_click();
        uint8_t level_tmp = sys_cb.light_level;
        if(level_tmp < 1)
        {
            level_tmp = 1;
        }
        else if(level_tmp > 5)
        {
            level_tmp = 5;
        }
        lcd_drv_set_brightness(level_tmp*(100/5));
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_LONG:
        func_light_button_release_handle();
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_light_button_release_handle();
        func_message(msg);
        break;

    case MSG_QDEC_FORWARD:
    case MSG_QDEC_BACKWARD:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入亮度调节功能
void func_light_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_light_t));
    func_cb.frm_main = func_light_form_create();
}

//退出亮度调节功能
void func_light_exit(void)
{
    func_cb.last = FUNC_LIGHT;
}

//亮度调节功能
void func_light(void)
{
    printf("%s\n", __func__);
    func_light_enter();
    while (func_cb.sta == FUNC_LIGHT) {
        func_light_process();
        func_light_message(msg_dequeue());
    }
    func_light_exit();
}
