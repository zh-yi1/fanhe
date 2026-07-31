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

/* 圆环进度图 13 张：ANNULUS_12(空)→ANNULUS_0(满)，按 24H 均分 */
static const u32 ANNULUS_PICS[13] = {
    UI_BUF_NEW_UI_ANNULUS_12_BIN,  /*  0: 空环 (0%) */
    UI_BUF_NEW_UI_ANNULUS_11_BIN,  /*  1:       (~8%) */
    UI_BUF_NEW_UI_ANNULUS_10_BIN,  /*  2:       (~17%) */
    UI_BUF_NEW_UI_ANNULUS_9_BIN,   /*  3:       (25%) */
    UI_BUF_NEW_UI_ANNULUS_8_BIN,   /*  4:       (~33%) */
    UI_BUF_NEW_UI_ANNULUS_7_BIN,   /*  5:       (~42%) */
    UI_BUF_NEW_UI_ANNULUS_6_BIN,   /*  6:       (50%) */
    UI_BUF_NEW_UI_ANNULUS_5_BIN,   /*  7:       (~58%) */
    UI_BUF_NEW_UI_ANNULUS_4_BIN,   /*  8:       (~67%) */
    UI_BUF_NEW_UI_ANNULUS_3_BIN,   /*  9:       (75%) */
    UI_BUF_NEW_UI_ANNULUS_2_BIN,   /* 10:       (~83%) */
    UI_BUF_NEW_UI_ANNULUS_1_BIN,   /* 11:       (~92%) */
    UI_BUF_NEW_UI_ANNULUS_0_BIN,   /* 12: 满环 (100%) */
};

/* 加热页私有状态 */
typedef struct
{
    u8 display_stage; // 0 = 正常运行, 1 = 首帧跳过 func_process
    u8  last_idx;       // 上次圆环索引
    /* 已保温时长: 累加模块的进度增量, 只增不减 (算法见 process) */
    u32 warm_min;       // 显示值
    u32 warm_prev_raw;  // 上一帧模块这一轮已跑的分钟数 (增量参考点)
    general_status_bar_t sb;
    compo_picturebox_t *state_pic;
    compo_picturebox_t *schedule_pic;
    compo_textbox_t *temp_text;
    compo_textbox_t *temp_text1;
    compo_textbox_t *residue_time_text;
    compo_textbox_t *residue_time_text1;
} f_warm_page_t;

/* 盖盖弹窗 YES 进保温页: 继承开盖前的已保温时长, 不清零显示
 * (func.c 的 lb_lid_confirm_yes 在切页前置位, enter 取走) */
static bool warm_inherit_pending;

void func_warm_page_inherit_time(void)
{
    warm_inherit_pending = true;
}

compo_form_t *func_warm_page_form_create(void)
{
    f_warm_page_t *inf = (f_warm_page_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);

    /* 白色背景 */
    widget_set_visible(frm->icon, false);
    compo_shape_t *bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(bg, COLOR_WHITE);
    compo_shape_set_location(bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部状态栏 */
    general_status_bar_create(frm, &inf->sb, i18n[STR_WARM1], &g_ui_sys);

    /* ---- 加热圆环进度条 ---- */
    inf->schedule_pic = compo_picturebox_create(frm, ANNULUS_PICS[0]);
    compo_picturebox_set_pos(inf->schedule_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 15);

    /* ---- 加热圆环进度条内的文本 ---- */
    inf->residue_time_text = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->residue_time_text, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 10, 0, 0);
    compo_textbox_set_autosize(inf->residue_time_text, true);
    compo_textbox_set_align_center(inf->residue_time_text, true);
    compo_textbox_set_font(inf->residue_time_text, UI_BUF_0FONT_FONT_TEST_BIN);
    compo_textbox_set_multiline(inf->residue_time_text, false);
    compo_textbox_set_forecolor(inf->residue_time_text, COLOR_BLUE);
    compo_textbox_set(inf->residue_time_text, "0Min");

    inf->residue_time_text1 = compo_textbox_create(frm, 21);
    compo_textbox_set_location(inf->residue_time_text1, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 20, 0, 0);
    compo_textbox_set_autosize(inf->residue_time_text1, true);
    compo_textbox_set_align_center(inf->residue_time_text1, true);
    compo_textbox_set_font(inf->residue_time_text1, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set_multiline(inf->residue_time_text1, false);
    compo_textbox_set_forecolor(inf->residue_time_text1, COLOR_GRAY);
    compo_textbox_set(inf->residue_time_text1, i18n[STR_WARM_LAB]);

    /* 加热温度和时间的参数显示 */
    //图片
    inf->state_pic = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_SHOW_BIN);
    compo_picturebox_set_pos(inf->state_pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 85);

    //温度文本
    inf->temp_text = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->temp_text, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 75, 0, 0);
    compo_textbox_set_autosize(inf->temp_text, true);
    compo_textbox_set_align_center(inf->temp_text, true);
    compo_textbox_set_font(inf->temp_text, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_multiline(inf->temp_text, false);
    compo_textbox_set_forecolor(inf->temp_text, COLOR_BLUE);
    compo_textbox_set(inf->temp_text, "194F");

    inf->temp_text1 = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->temp_text1, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 95, 0, 0);
    compo_textbox_set_autosize(inf->temp_text1, true);
    compo_textbox_set_align_center(inf->temp_text1, true);
    compo_textbox_set_font(inf->temp_text1, UI_BUF_0FONT_FONT_TEST_10_BIN);
    compo_textbox_set_multiline(inf->temp_text1, false);
    compo_textbox_set_forecolor(inf->temp_text1, COLOR_GRAY);
    compo_textbox_set(inf->temp_text1, i18n[STR_HEAT_LAB]);

    tft_bglight_force_on();
    return frm;
}

static void func_warm_page_handle_keys(void)
{
    func_key_event_t evt;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_BACK:
            func_cb.sta = FUNC_HOME_PAGE;
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

static void func_warm_page_process(void)
{
    f_warm_page_t *inf = (f_warm_page_t *)func_cb.f_cb;

#if ELUNCHBOX_PANEL_EN
    if (inf->display_stage != 0) {
        inf->display_stage = 0;
        return;
    }
#endif

    func_key_poll();
    func_warm_page_handle_keys();
    func_key_lock_poll();

    if (func_key_lock_gui_dirty()) {
        if (func_key_lock_overlay_visible())
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        else
            func_lock_page_hide();
    }

    /* 保温计时 —— 累加模块的进度增量。
     *
     * 模块每收到一条命令就开新一轮: DP6 重置成 DP5, 然后往下减。
     * g_ui_sys.keep_warm_min = DP5 − DP6 = 模块"这一轮"已经跑了多少分钟,
     * 换轮时它会突变 (归零, 或因测试压缩直接跳到 1368), 直接显示必然乱跳。
     *
     * 判据只有一条: 只接受"合理的前进"。一帧是毫秒级, 真实进度一帧最多
     * 跨 1 分钟; 往回跳、或一次跨好几分钟, 都是模块重开了账, 这一帧的差值
     * 没有意义, 丢掉只对齐参考点。累加器因此只增不减, 模块怎么清零/重置
     * 都不影响显示 —— 加热中插电那条路上, 原加热那份跑完、补发保温命令
     * 换轮时, 显示就能接着往下走。
     *
     * 注: 走的是模块的进度, 不是墙上时间。测试模式模块按 1/20 压缩跑,
     * 显示也跟着快 20 倍, 这样一轮几分钟就能验完。 */
    #define WARM_STEP_MAX   2u      /* 一帧可信的最大进度(分钟), 超出即换轮 */
    {
        u32 raw = g_ui_sys.keep_warm_min;
        u32 warm_min;
        u8  idx;

        if (raw > inf->warm_prev_raw && raw - inf->warm_prev_raw <= WARM_STEP_MAX) {
            inf->warm_min += raw - inf->warm_prev_raw;
        }
        inf->warm_prev_raw = raw;
        warm_min = inf->warm_min;
        lb_ui_warm_elapsed_set(warm_min);   // 路由补发保温时按这个值续 DP6

        if (warm_min == 0) {
            idx = 0; /* 空环 */
        } else {
            idx = (u8)(warm_min * 12 / 1440); /* 0~12, 24H 为满环 */
            if (idx > 12) idx = 12;
        }

        /* 圆环：索引变化时切换图片 */
        if (idx != inf->last_idx) {
            inf->last_idx = idx;
            compo_picturebox_set(inf->schedule_pic, ANNULUS_PICS[idx]);
        }

        /* 已保温时间文本：每帧刷新 */
        {
            char buf[16];
            if (warm_min == 0) {
                snprintf(buf, sizeof(buf), "0Min");
            } else if (warm_min < 60) {
                snprintf(buf, sizeof(buf), "%uMin", (unsigned)warm_min);
            } else if (warm_min % 60 == 0) {
                snprintf(buf, sizeof(buf), "%uH", (unsigned)(warm_min / 60));
            } else {
                snprintf(buf, sizeof(buf), "%uH%02uMin",
                         (unsigned)(warm_min / 60), (unsigned)(warm_min % 60));
            }
            compo_textbox_set(inf->residue_time_text, buf);
        }
    }

    func_process();
    general_status_bar_tick(&inf->sb);
}

static void func_warm_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_warm_page_enter(void)
{
    f_warm_page_t *inf;

    func_cb.f_cb = func_zalloc(sizeof(f_warm_page_t));
    func_key_reset();
    func_cb.frm_main = func_warm_page_form_create();
    inf = (f_warm_page_t *)func_cb.f_cb;
    inf->display_stage = 1;
    inf->last_idx   = 0xff; /* 强制首帧刷新 */

#if FUNC_LUNCHBOX_UART_EN
    /* 下发保温: 固定 194F(=90°C) / 24 小时。
     * 模块已在保温说明本页是被 lb_ui_route_poll 驱动跳过来的, 不重发,
     * 否则会把模块那边的 24 小时计时清零 */
    {
        lb_ui_state_t *st = lb_ui_state_get();
        if (!(st->valid && st->heat_enable && st->heat_mode == LB_MODE_WARM)) {
            lb_heat_cmd_start(LB_MODE_WARM,
                              lunchbox_temp_f_to_idx(LB_WARM_TEMP_F),
                              LB_WARM_DURATION_MIN);
            printf("warm_page: keep warm %uF %umin sent\n",
                   (unsigned)LB_WARM_TEMP_F, (unsigned)LB_WARM_DURATION_MIN);
        }
    }

    /* 累加器从 0 起 —— 进页时模块那一轮可能已经跑了一段 (加热中插电转保温
     * 会带着原加热的 DP5/DP6), 不能把它算进"已保温"。
     * 盖盖弹窗 YES 进来的例外: 继承开盖前的已保温时长。 */
    inf->warm_min = warm_inherit_pending ? g_ui_sys.keep_warm_min : 0;
    if (warm_inherit_pending) {
        warm_inherit_pending = false;
        printf("warm_page: inherit warm time %umin\n", (unsigned)inf->warm_min);
    }
    inf->warm_prev_raw = g_ui_sys.keep_warm_min;    // 增量参考点
    lb_ui_warm_elapsed_set(inf->warm_min);          // 首帧之前也要有值可用
#endif

    /* 初始圆环图：空环 (ANNULUS_12) */
    compo_picturebox_set(inf->schedule_pic, ANNULUS_PICS[0]);

    /* 初始残留时间文本 */
    compo_textbox_set(inf->residue_time_text, "0Min");

    home_gpu_wait_idle();
    WDT_CLR();

    general_status_bar_attach(&inf->sb);
    func_key_lock_on_heating_start(); /* 保温 30s 后自动童锁 (与加热页一致) */
}

void func_warm_page_exit(void)
{
#if FUNC_LUNCHBOX_UART_EN
    /* 退出保温界面即停止保温 (与加热页一致)。
     * 只在"模块确实还在保温"时发: 已停(heat_enable=0)不重发;
     * 模块转去加热(APP 改的, route 把屏幕带走)时 heat_mode 已不是 WARM,
     * 那份加热不该被这里掐掉 */
    {
        lb_ui_state_t *st = lb_ui_state_get();
        if (st->valid && st->heat_enable && st->heat_mode == LB_MODE_WARM
            && !lunchbox_shutdown_is_active() && !lunchbox_shutdown_is_done()) {
            lb_heat_cmd_stop();
            printf("warm_page: exit -> stop keep warm\n");
        }
    }
#endif
    func_key_lock_on_heating_stop();
    func_key_flush();
    general_status_bar_detach();
}

void func_warm_page(void)
{
    printf("%s\n", __func__);
    func_warm_page_enter();
    while (func_cb.sta == FUNC_NEW_WARM_PAGE)
    {
        func_warm_page_process();
        func_warm_page_message(msg_dequeue());
    }
    func_warm_page_exit();
}
