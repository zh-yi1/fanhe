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
    bool last_full_charge;
    u8  last_bat_level;
    bool unplug_sent;       /* 拔线关机只发一次 (重新插上复位) */
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

    inf->charge_tick_ms  = 0;
    inf->charge_frame    = 0;
    inf->last_charging   = false;
    inf->last_full_charge= false;
    inf->last_bat_level  = 0xff; /* 强制首次刷新 */

    tft_bglight_force_on();
    return frm;
}

/*
 * 按键处理
 */
static void func_black_screen_page_handle_keys(void)
{
    func_key_event_t evt;

    /* 开机 = 开关键(TCH5)长按 3s, 在 func_key_handle_pwr_long 里统一处理
     * (func_key_poll 内部)。短按/其他键一律无效, 这里只排空事件队列。 */
    while (func_key_get_event(&evt))
    {
        (void)evt;
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

#if FUNC_LUNCHBOX_UART_EN
    /* 本页语义是"关机+充电", 模块已经关了, 与真关机唯一区别是还收串口状态。
     * 拔线即真关机, 本机不去抖 —— 模块上报的 DP4 已在模块侧消抖过。
     * 重走一遍关机时序 (对已关的模块重发 stop/power_off 无害,
     * 不应答则各 500ms 超时推进), 时序完成后 func.c 落深睡。 */
    if (lunchbox_charging_now()) {
        inf->unplug_sent = false;
    } else if (!inf->unplug_sent) {
        inf->unplug_sent = true;
        printf("black_screen: unplugged -> shutdown\n");
        lunchbox_shutdown_start(false, 0);  /* 重复调用无副作用 */
    }
#endif

    /* 4. 电池图标更新 */
    if (inf->pic_bat != NULL)
    {
        if (g_ui_sys.charging && !g_ui_sys.full_charge)
        {
            /* 充电中（未满） → 跑马灯动画 */
            if (tick_check_expire(inf->charge_tick_ms, 500))
            {
                inf->charge_tick_ms = tick_get();
                compo_picturebox_set(inf->pic_bat, g_bs_charge_icons[inf->charge_frame]);
                inf->charge_frame = (inf->charge_frame + 1) & 0x03;
            }
            inf->last_bat_level    = 0xff; /* 强制退出充电后刷新 */
            inf->last_full_charge  = false;
        }
        /* 充满电（充电中充满）→ 静态显示 CHARGING_4 */
        else if (g_ui_sys.charging && g_ui_sys.full_charge)
        {
            if (inf->last_full_charge != true || inf->last_charging != true)
            {
                inf->last_charging    = true;
                inf->last_full_charge = true;
                inf->last_bat_level   = 0xff;
                inf->charge_tick_ms   = 0;
                inf->charge_frame     = 0;
                compo_picturebox_set(inf->pic_bat, g_bs_charge_icons[3]);
            }
        }
        /* 充满/非充电态：显示静态电量档位 */
        else if (g_ui_sys.bat_level != inf->last_bat_level
                 || g_ui_sys.full_charge != inf->last_full_charge
                 || g_ui_sys.charging != inf->last_charging)
        {
            inf->last_bat_level    = g_ui_sys.bat_level;
            inf->last_full_charge  = g_ui_sys.full_charge;
            inf->last_charging     = g_ui_sys.charging;
            inf->charge_tick_ms    = 0;
            inf->charge_frame      = 0;

            if (g_ui_sys.bat_level < 5)
            {
                compo_picturebox_set(inf->pic_bat,
                                     g_bs_bat_icons[g_ui_sys.bat_level]);
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
