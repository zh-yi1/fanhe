#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_title_ui.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 版本信息页私有状态 */
typedef struct
{
    general_title_bar_t tb;
    compo_textbox_t *txt_version;
} f_verinfo_t;

compo_form_t *func_verInfo_page_form_create(void)
{
    f_verinfo_t *inf = (f_verinfo_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部标题栏（左上角标题 + 右上角蓝牙/电量） */
    general_title_bar_create(frm, &inf->tb, i18n[STR_VER_INFO]);

    /* 版本号文字（居中显示） */
    inf->txt_version = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_version, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, 0, 0);
    compo_textbox_set_autosize(inf->txt_version, true);
    compo_textbox_set_align_center(inf->txt_version, true);
    compo_textbox_set_font(inf->txt_version, UI_BUF_0FONT_FONT_TEST_18_BIN);
    compo_textbox_set(inf->txt_version, i18n[STR_VER_INFO_TEXT]);
    compo_textbox_set_forecolor(inf->txt_version, 0x2BF4);

    tft_bglight_force_on();
    return frm;
}

/*
 * 按键处理
 */
static void func_verInfo_page_handle_keys(void)
{
    func_key_event_t evt;

    while (func_key_get_event(&evt))
    {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key)
        {
        case FUNC_KEY_BACK:
        case FUNC_KEY_CONFIRM:
            func_cb.sta = FUNC_HOME;
            break;

        /* 直接按键：加热键 → 加热设置页 */
        case FUNC_KEY_HEAT:
            func_cb.sta = FUNC_NEW_HEAT_SET;
            break;

        /* 直接按键：模式键 → 模式页 */
        case FUNC_KEY_MODE:
            func_cb.sta = FUNC_NEW_MODE;
            break;

        /* 直接按键：预约键 → 预约页 */
        case FUNC_KEY_RESERVATION:
            func_cb.sta = FUNC_APPOINTMENT_TIME;
            break;


        default:
            break;
        }
    }
}

static void func_verInfo_page_process(void)
{
    /* 1. 按键处理 */
    func_key_poll();
    func_verInfo_page_handle_keys();

    /* 2. 童锁 */
    func_key_lock_poll();

    /* 3. 锁标志位 → UI 渲染 */
    if (func_key_lock_gui_dirty())
    {
        if (func_key_lock_overlay_visible())
        {
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        }
        else
        {
            func_lock_page_hide();
        }
    }

    func_process();
}

static void func_verInfo_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_verInfo_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_verinfo_t));
    func_key_reset();
    func_cb.frm_main = func_verInfo_page_form_create();
    general_title_bar_attach(&((f_verinfo_t *)func_cb.f_cb)->tb);
}

void func_verInfo_page_exit(void)
{
    func_key_flush();
    general_title_bar_detach();
}

void func_verInfo_page(void)
{
    printf("%s\n", __func__);
    func_verInfo_page_enter();
    while (func_cb.sta == FUNC_NEW_VERINFO)
    {
        func_verInfo_page_process();
        func_verInfo_page_message(msg_dequeue());
    }
    func_verInfo_page_exit();
}
