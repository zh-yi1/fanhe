#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_ui.h"
#include "ui.h"
extern bool func_confirm_overlay_visible(void);
extern void func_confirm_overlay_show(void);
extern bool func_confirm_overlay_poll(void);

/* 系统状态实例 —— 全部字段由 lb_ui_sync_pull() 从串口状态镜像刷新,
 * 见 general_ui.c 末尾; 页面只读, 不要在这里写初值 */
ui_sys_t g_ui_sys;

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 首页私有状态 */
typedef struct
{
    u8 selection; // 0 = 加热, 1 = 模式, 2 = 设置
    general_status_bar_t sb;
    compo_picturebox_t *logo;
    compo_picturebox_t *pic_heat;
    compo_picturebox_t *pic_mode;
    compo_picturebox_t *pic_set;
    compo_textbox_t *txt_heat;
    compo_textbox_t *txt_mode;
    compo_textbox_t *txt_set;
} f_home_t;

static void home_update_display(void)
{
    f_home_t *inf = (f_home_t *)func_cb.f_cb;
    bool sel_heat = (inf->selection == 0);
    bool sel_mode = (inf->selection == 1);
    bool sel_set  = (inf->selection == 2);

    /* 加热：选中态 vs 普通态 */
    compo_picturebox_set(inf->pic_heat, sel_heat
                                            ? UI_BUF_NEW_UI_HEAT_1_BIN
                                            : UI_BUF_NEW_UI_HEAT_0_BIN);
    compo_textbox_set_forecolor(inf->txt_heat, sel_heat ? COLOR_WHITE : COLOR_BLUE);

    /* 模式：选中态 vs 普通态 */
    compo_picturebox_set(inf->pic_mode, sel_mode
                                           ? UI_BUF_NEW_UI_MODE_1_BIN
                                           : UI_BUF_NEW_UI_MODE_0_BIN);
    compo_textbox_set_forecolor(inf->txt_mode, sel_mode ? COLOR_WHITE : COLOR_BLUE);

    /* 设置：选中态 vs 普通态 */
    compo_picturebox_set(inf->pic_set, sel_set
                                           ? UI_BUF_NEW_UI_SETUP_1_BIN
                                           : UI_BUF_NEW_UI_SETUP_0_BIN);
    compo_textbox_set_forecolor(inf->txt_set, sel_set ? COLOR_WHITE : COLOR_BLUE);
}

compo_form_t *func_home_page_form_create(void)
{
    f_home_t *inf = (f_home_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部状态栏 */
    general_status_bar_create(frm, &inf->sb, NULL, &g_ui_sys);

    /* logo图标 */
    inf->logo = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_LOGO_BIN);
    compo_picturebox_set_pos(inf->logo, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 50);

    /* 加热图标 */
    inf->pic_heat = compo_picturebox_create(frm, UI_BUF_NEW_UI_HEAT_1_BIN);
    compo_picturebox_set_pos(inf->pic_heat, GUI_SCREEN_CENTER_X-100, GUI_SCREEN_CENTER_Y + 45);

    inf->txt_heat = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_heat, GUI_SCREEN_CENTER_X - 98, GUI_SCREEN_CENTER_Y + 60, 0, 0);
    compo_textbox_set_autosize(inf->txt_heat, true);
    compo_textbox_set_align_center(inf->txt_heat, true);
    compo_textbox_set_font(inf->txt_heat, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set(inf->txt_heat, i18n[STR_HEAT]);

    /* 模式图标 */
    inf->pic_mode = compo_picturebox_create(frm, UI_BUF_NEW_UI_MODE_0_BIN);
    compo_picturebox_set_pos(inf->pic_mode, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 45);

    inf->txt_mode = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_mode, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 60, 0, 0);
    compo_textbox_set_autosize(inf->txt_mode, true);
    compo_textbox_set_align_center(inf->txt_mode, true);
    compo_textbox_set_font(inf->txt_mode, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set(inf->txt_mode, i18n[STR_MODE]);

    /* 设置图标 */
    inf->pic_set = compo_picturebox_create(frm, UI_BUF_NEW_UI_SETUP_0_BIN);
    compo_picturebox_set_pos(inf->pic_set, GUI_SCREEN_CENTER_X + 98, GUI_SCREEN_CENTER_Y + 45);

    inf->txt_set = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_set, GUI_SCREEN_CENTER_X + 98, GUI_SCREEN_CENTER_Y + 60, 0, 0);
    compo_textbox_set_autosize(inf->txt_set, true);
    compo_textbox_set_align_center(inf->txt_set, true);
    compo_textbox_set_font(inf->txt_set, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set(inf->txt_set, i18n[STR_SETUP]);

    /* 初始选中加热 */
    compo_textbox_set_forecolor(inf->txt_heat, COLOR_WHITE);
    compo_textbox_set_forecolor(inf->txt_mode, COLOR_BLUE);
    compo_textbox_set_forecolor(inf->txt_set, COLOR_BLUE);

    tft_bglight_force_on();
    return frm;
}

/*
 * 按键处理 — 逻辑键由 func_key_map_logical 映射：
 *   TCH6 → UP, TCH2 → DOWN, TCH4 → CONFIRM, TCH5 → BACK
 */
static void func_home_page_handle_keys(void)
{
    f_home_t *inf = (f_home_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_UP:
            inf->selection = (inf->selection == 0) ? 2 : inf->selection - 1;
            home_update_display();
            break;

        case FUNC_KEY_DOWN:
            inf->selection = (inf->selection + 1) % 3;
            home_update_display();
            break;

        case FUNC_KEY_CONFIRM:
            if (inf->selection == 0)
                func_cb.sta = FUNC_NEW_HEAT_SET;
            else if (inf->selection == 1)
                func_cb.sta = FUNC_NEW_MODE;
            else
                func_cb.sta = FUNC_NEW_SETUP;
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

        case FUNC_KEY_BACK:
            /* 返回上一级 */
            break;

        default:
            break;
        }
    }
}

static void func_home_page_process(void)
{
    f_home_t *inf = (f_home_t *)func_cb.f_cb;

    /* 1. 按键处理：扫描 → 长按检测 → 童锁过滤 → 事件入队 */
    func_key_poll();
    if (!func_confirm_overlay_visible())
            func_home_page_handle_keys();

    /* 2. 童锁计时器（hint过期/自动锁），不碰 GUI */
    func_key_lock_poll();

    /* 3. 锁标志位 → UI 渲染（页面负责显示，key 模块不管 UI） */
    if (func_key_lock_gui_dirty()) {
        if (func_key_lock_overlay_visible()) {
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        } else {
            func_lock_page_hide();
        }
    }

    /* 4. 开盖弹窗：仅首页检测 lid_open */
    if (g_ui_sys.lid_open && !func_confirm_overlay_visible() && !g_ui_sys.lowbat) {
        func_confirm_overlay_show();
    }
    if (func_confirm_overlay_visible()) {
        func_confirm_overlay_poll();
        if (!func_confirm_overlay_visible()) {
            g_ui_sys.lid_open = false; /* 用户已处理 */
        }
    }

    func_process();

    /* 4. 状态栏时间刷新：放在 func_process 之后，
     *    避免 compo_textbox_set 与 compo_update 同帧竞争 TE 窗口 */
    general_status_bar_tick(&inf->sb);
}

static void func_home_page_message(size_msg_t msg)
{
    /* PT8028 按键已由 func_key_poll 统一处理，消息队列仅处理系统事件 */
    func_message(msg);
}

void func_home_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_home_t));
    func_key_reset();
    func_cb.frm_main = func_home_page_form_create();

    home_gpu_wait_idle();
    WDT_CLR();

    general_status_bar_attach(&((f_home_t *)func_cb.f_cb)->sb);
}

void func_home_page_exit(void)
{
    f_home_t *inf = (f_home_t *)func_cb.f_cb;
    if (inf->selection == 0)
        func_cb.last = FUNC_NEW_HEAT_SET;
    else if (inf->selection == 1)
        func_cb.last = FUNC_NEW_MODE;
    else
        func_cb.last = FUNC_NEW_SETUP;

    func_key_flush();
    general_status_bar_detach();
}

void func_home_page(void)
{
    printf("%s\n", __func__);
    func_home_page_enter();
    while (func_cb.sta == FUNC_HOME || func_cb.sta == FUNC_HOME_PAGE)
    {
        func_home_page_process();
        func_home_page_message(msg_dequeue());
    }
    func_home_page_exit();
}
