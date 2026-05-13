#include "include.h"
#include "func.h"
#include "func_clock.h"

#define MENU_DROPDOWN_ALPHA             200
#define MENU_DROPDOWN_MODE              0   //1:下拉模糊 0:截图背景模糊+缩放

compo_form_t *func_clock_form_create(void);
compo_form_t *func_clock_form_create_by_screenshoot(void);
void func_switch_screenshot(void *cur_scbuf, void *next_scbuf, u8 next_sta);

enum {
    COMPO_ID_NULL = 0,
    COMPO_ID_BTN_NAV,
    COMPO_ID_TXT_NAV,
};

//创建下拉菜单
static void func_clock_sub_dropdown_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

#if GUI_USE_BLUR && MENU_DROPDOWN_MODE
    //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, 0);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_blur_set_ram(pic, cur_scbuf, blur_obuf, 1);
#else
    //创建遮罩层
    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_BLACK);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(masklayer, 140);
#endif
    //创建状态
    compo_form_add_image(frm, UI_BUF_DROPDOWN_CONNECT_BIN, 180, 51);
    compo_form_add_image(frm, UI_BUF_DROPDOWN_POWER3_BIN, 281, 51);

    //创建天气
//    compo_form_add_image(frm, UI_BUF_DROPDOWN_WINDY_BIN, 233, 207);

   //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, 207, 150, 50);
    compo_textbox_set(txt, i18n[STR_NAV_ZOOM+sys_cb.nav_index]);
    compo_setid(txt, COMPO_ID_TXT_NAV);

    //创建按钮
    compo_button_t *nav_btn = compo_button_create(frm);
    compo_button_set_location(nav_btn, GUI_SCREEN_CENTER_X, 207, 200, 150);
    compo_setid(nav_btn, COMPO_ID_BTN_NAV);

    //创建按钮
    compo_button_t *btn_connect = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_LIGHT_BIN);
    compo_button_set_pos(btn_connect, 68, 207);
    compo_button_set_alpha(btn_connect, MENU_DROPDOWN_ALPHA);

    compo_button_t *btn_disurb = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_CALL_BIN);
    compo_button_set_pos(btn_disurb, 76, 300);
    compo_button_set_alpha(btn_disurb, MENU_DROPDOWN_ALPHA);

    compo_button_t *btn_findphone = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_CONNECT_PHONE_BIN);
    compo_button_set_pos(btn_findphone, 137, 373);
    compo_button_set_alpha(btn_findphone, MENU_DROPDOWN_ALPHA);

    compo_button_t *btn_mute = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_FIND_PHONE_BIN);
    compo_button_set_pos(btn_mute, 233, 396);
    compo_button_set_alpha(btn_mute, MENU_DROPDOWN_ALPHA);

    compo_button_t *btn_flashlight = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_SETTING_BIN);
    compo_button_set_pos(btn_flashlight, 329, 373);
    compo_button_set_alpha(btn_flashlight, MENU_DROPDOWN_ALPHA);

    compo_button_t *btn_light = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_DISTURB_BIN);
    compo_button_set_pos(btn_light, 390, 300);
    compo_button_set_alpha(btn_light, MENU_DROPDOWN_ALPHA);

    compo_button_t *btn_scan = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_MUTE_BIN);
    compo_button_set_pos(btn_scan, 398, 207);
    compo_button_set_alpha(btn_scan, MENU_DROPDOWN_ALPHA);

    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->sub_frm = frm;

#if GUI_USE_BLUR && MENU_DROPDOWN_MODE
    f_clk->blur_pic = pic;
#else
    f_clk->masklayer = masklayer;
#endif
}

//时钟表盘主要事件流程处理
static void func_clock_sub_dropdown_process(void)
{
    func_clock_sub_process();
}

void func_clock_sub_dropdown_btn_handle(void)
{
    int id = compo_get_button_id();
    switch (id) {
    case COMPO_ID_BTN_NAV:
        {
            compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_TXT_NAV);
            sys_cb.nav_index++;
            if (sys_cb.nav_index > 10) {
                sys_cb.nav_index = 0;
            }
            compo_textbox_set(txt, i18n[STR_NAV_ZOOM+sys_cb.nav_index]);
        }
        break;

    default:
        break;
    }
}

//时钟表盘下拉菜单功能消息处理
static void func_clock_sub_dropdown_message(size_msg_t msg)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_clock_sub_dropdown_btn_handle();

    break;

    case MSG_CTP_SHORT_UP:
        {
            u16 sw_mode = FUNC_SWITCH_MENU_DROPDOWN_UP;
            if (f_clk->blur_pic) {
                if (f_clk->masklayer) {
                    sw_mode |= FUNC_SWITCH_DOWN_BG_BLUR;
                } else {
                    sw_mode |= FUNC_SWITCH_DOWN_BLUR;
                }
            }

            if (func_switching(sw_mode, NULL)) {
                f_clk->sta = FUNC_CLOCK_MAIN;                   //上滑返回到时钟主界面
            }
        }
        break;

    case KU_BACK:
    case MSG_QDEC_FORWARD:
        func_switching(FUNC_SWITCH_MENU_DROPDOWN_UP | FUNC_SWITCH_AUTO, NULL);
        f_clk->sta = FUNC_CLOCK_MAIN;                       //单击BACK键返回到时钟主界面
        break;

    case MSG_QDEC_BACKWARD:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
        break;

    default:
        func_clock_sub_message(msg);
        break;
    }
}

//时钟表盘下拉菜单进入处理
static void func_clock_sub_dropdown_enter(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    func_clock_butterfly_set_light_visible(false);
#if GUI_USE_BLUR
    func_switch_screenshot(cur_scbuf, NULL, 0);
    compo_form_destroy(func_cb.frm_main);
    func_cb.frm_main = func_clock_form_create_by_screenshoot();
#endif

    func_clock_sub_dropdown_form_create();
    u16 sw_mode = FUNC_SWITCH_MENU_DROPDOWN_DOWN;
    if (f_clk->blur_pic) {
        if (f_clk->masklayer) {
            sw_mode |= FUNC_SWITCH_DOWN_BG_BLUR;
        } else {
            sw_mode |= FUNC_SWITCH_DOWN_BLUR;
        }
    }

    if (!func_switching(sw_mode, NULL)) {
        return;                                             //下拉到一半取消
    }

    f_clk->sta = FUNC_CLOCK_SUB_DROPDOWN;                   //进入到下拉菜单
}

//时钟表盘下拉菜单退出处理
/* static  */void func_clock_sub_dropdown_exit(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    compo_form_destroy(f_clk->sub_frm);
    func_clock_butterfly_set_light_visible(true);
    f_clk->sub_frm = NULL;

#if GUI_USE_BLUR
    compo_form_destroy(func_cb.frm_main);
    func_cb.frm_main = func_clock_form_create();
#endif
}

//时钟表盘下拉菜单
void func_clock_sub_dropdown(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
#endif // VIDEO_PLAY_EN
    func_clock_sub_dropdown_enter();
    while (func_cb.sta == FUNC_CLOCK && ((f_clock_t *)func_cb.f_cb)->sta == FUNC_CLOCK_SUB_DROPDOWN) {
        func_clock_sub_dropdown_process();
        func_clock_sub_dropdown_message(msg_dequeue());
    }
    func_clock_sub_dropdown_exit();
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}
