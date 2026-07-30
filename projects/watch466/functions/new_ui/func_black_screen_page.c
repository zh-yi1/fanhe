#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_ui.h"
#include "ui.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 充电图标尺寸 */
#define BLACK_SCREEN_RIGHT_MARGIN       10
#define BLACK_SCREEN_BAT_X              ((s16)(GUI_SCREEN_WIDTH - BLACK_SCREEN_RIGHT_MARGIN - NEW_HOME_BAT_W / 2))
#define BLACK_SCREEN_BAT_Y              20

/* 电量档位 0~4 映射到 DL1~DL4 图标 */
static const u32 g_bs_bat_icons[5] = {
    UI_BUF_NEW_UI_DL1_BIN,  /* 0: 空 */
    UI_BUF_NEW_UI_DL1_BIN,  /* 1: 25% */
    UI_BUF_NEW_UI_DL2_BIN,  /* 2: 50% */
    UI_BUF_NEW_UI_DL3_BIN,  /* 3: 75% */
    UI_BUF_NEW_UI_DL4_BIN,  /* 4: 100% */
};

/* 充电动画帧 */
static const u32 g_bs_charge_icons[4] = {
    UI_BUF_NEW_UI_CHARGING_1_BIN,
    UI_BUF_NEW_UI_CHARGING_2_BIN,
    UI_BUF_NEW_UI_CHARGING_3_BIN,
    UI_BUF_NEW_UI_CHARGING_4_BIN,
};

/* 黑屏页私有状态 */
typedef struct
{
    compo_picturebox_t *pic_bat;
    u32 charge_tick_ms;
    u8  charge_frame;
    bool last_charging;
    u8  last_bat_level;
} f_black_screen_t;

compo_form_t *func_black_screen_page_form_create(void)
{
    f_black_screen_t *inf = (f_black_screen_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 黑色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_BLACK);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 右上角电池图标（占位图，后续 tick 中根据 charging 状态切换） */
    inf->pic_bat = compo_picturebox_create(frm, UI_BUF_ICON_ACTIVITY_BIN);
    compo_picturebox_set_pos(inf->pic_bat, BLACK_SCREEN_BAT_X, BLACK_SCREEN_BAT_Y);
    compo_picturebox_set_size(inf->pic_bat, NEW_HOME_BAT_W, NEW_HOME_BAT_H);

    inf->charge_tick_ms = 0;
    inf->charge_frame   = 0;
    inf->last_charging  = false;
    inf->last_bat_level = 0xff; /* 强制首次刷新 */

    tft_bglight_force_on();
    return frm;
}

/*
 * 按键处理
 */
static void func_black_screen_page_handle_keys(void)
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

        default:
            break;
        }
    }
}

static void func_black_screen_page_process(void)
{
    f_black_screen_t *inf = (f_black_screen_t *)func_cb.f_cb;

    /* 1. 按键处理 */
    func_key_poll();
    func_black_screen_page_handle_keys();

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

    /* 4. 电池图标更新 */
    if (inf->pic_bat != NULL)
    {
        if (g_ui_sys.charging)
        {
            /* 充电中 → 跑马灯动画 */
            if (tick_check_expire(inf->charge_tick_ms, 500))
            {
                inf->charge_tick_ms = tick_get();
                compo_picturebox_set(inf->pic_bat, g_bs_charge_icons[inf->charge_frame]);
                inf->charge_frame = (inf->charge_frame + 1) & 0x03;
            }

            if (inf->last_charging != true)
            {
                inf->last_charging = true;
                inf->charge_tick_ms = tick_get();
                inf->charge_frame   = 0;
            }
        }
        else
        {
            /* 未充电 → 静态电量图标 */
            u8 level = g_ui_sys.bat_level;
            if (level > 4) level = 4;

            if (inf->last_charging != false || inf->last_bat_level != level)
            {
                inf->last_charging  = false;
                inf->last_bat_level = level;
                compo_picturebox_set(inf->pic_bat, g_bs_bat_icons[level]);
            }
        }
    }
}

static void func_black_screen_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_black_screen_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_black_screen_t));
    func_key_reset();
    func_cb.frm_main = func_black_screen_page_form_create();
}

void func_black_screen_page_exit(void)
{
    func_key_flush();
}

void func_black_screen_page(void)
{
    printf("%s\n", __func__);
    func_black_screen_page_enter();
    while (func_cb.sta == FUNC_BLACK_SCREEN)
    {
        func_black_screen_page_process();
        func_black_screen_page_message(msg_dequeue());
    }
    func_black_screen_page_exit();
}
