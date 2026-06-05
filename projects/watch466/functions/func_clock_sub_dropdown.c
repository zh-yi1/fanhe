#include "include.h"
#include "func.h"
#include "func_clock.h"
#include "compo_picturebox.h"
#include "compo_button.h"

/* 下拉帘式动画使用 FUNC_SWITCH_MENU_DROPDOWN_*；进入前须 compo_pool_set_top(sub_frm)，
 * 且 func_switching_menu 在池顶为空时须 return false（勿 halt），否则会 WDT。 */

#define MENU_DROPDOWN_ALPHA             200
/* 下拉背景：整屏纯黑 RGB(0,0,0)，不透明；不再使用 RAM 模糊图与半透明遮罩 */
#define MENU_DROPDOWN_BG_ALPHA          255

/* 环形铺满表圈：半径约为屏宽 37%，圆心取几何中心 */
#define MENU_DROPDOWN_RING_R            ((s16)((s32)GUI_SCREEN_WIDTH * 37 / 100))
/* 环上五键图标边长（与 set_pos 中心对齐），约屏宽 22%，比资源默认显示更大 */
#define MENU_DROPDOWN_RING_BTN_WH       ((s16)((s32)GUI_SCREEN_WIDTH * 22 / 100))

/* 进入下拉后短时间内：上滑跟手关 / 下滑关 防抖，避免与打开下拉的抬指冲突 */
#define MENU_DROPDOWN_SHORT_UP_DEBOUNCE_MS  700

enum {
    COMPO_ID_NULL = 0,
    COMPO_ID_DROPDOWN_VOLUME,           //音量
    COMPO_ID_DROPDOWN_DELETE,           //设置（资源名为 delete）
    COMPO_ID_DROPDOWN_BLUETOOTH,        //蓝牙音乐
    COMPO_ID_DROPDOWN_BRIGHTNESS,       //亮度
    COMPO_ID_DROPDOWN_FLASHLIGHT,       //手电筒
};

static u32 dropdown_short_up_ignore_tick;

static compo_picturebox_t *dropdown_center_pic_bt;
static compo_picturebox_t *dropdown_center_pic_bat;
static compo_button_t *dropdown_btn_brightness;
static u16 dropdown_last_vbat;
static u8  dropdown_last_ble;
static u8  dropdown_brightness_pic_idx;

/* 亮度图标：由暗到亮循环（ui/dropdown 下 bin；增图后改 CNT 并补表项） */
#define DROPDOWN_BRIGHTNESS_PIC_CNT     6
#define DROPDOWN_LIGHT_LEVEL_MAX        6       /* 6 张亮度图对应 6 档背光 */

static const u32 tbl_dropdown_brightness_pic[DROPDOWN_BRIGHTNESS_PIC_CNT] = {
    // UI_BUF_DROPDOWN_BRIGHTNESS_ADJUSTMENT_ONE_BIN,
    // UI_BUF_DROPDOWN_BRIGHTNESS_ADJUSTMENT_TWO_BIN,
    // UI_BUF_DROPDOWN_BRIGHTNESS_ADJUSTMENT_THREE_BIN,
    // UI_BUF_DROPDOWN_BRIGHTNESS_ADJUSTMENT_FOUR_BIN,
    // UI_BUF_DROPDOWN_BRIGHTNESS_ADJUSTMENT_FIVE_BIN,
    // UI_BUF_DROPDOWN_BRIGHTNESS_ADJUSTMENT_SIX_BIN
};

static void dropdown_brightness_icon_show(u8 idx)
{
    if (dropdown_btn_brightness == NULL) {
        return;
    }
    dropdown_brightness_pic_idx = idx % DROPDOWN_BRIGHTNESS_PIC_CNT;
    compo_button_set_bgimg(dropdown_btn_brightness,
                           tbl_dropdown_brightness_pic[dropdown_brightness_pic_idx]);
    compo_button_set_size(dropdown_btn_brightness,
                          MENU_DROPDOWN_RING_BTN_WH, MENU_DROPDOWN_RING_BTN_WH);
}

/* 图标下标 0~5 -> 背光档 1~6 */
static u8 dropdown_brightness_pic_to_level(u8 pic_idx)
{
    return (pic_idx % DROPDOWN_BRIGHTNESS_PIC_CNT) + 1;
}

/* 背光档 1~6 -> 图标下标 0~5 */
static u8 dropdown_brightness_level_to_pic(u8 level)
{
    if (level < 1) {
        return 0;
    }
    if (level > DROPDOWN_LIGHT_LEVEL_MAX) {
        return DROPDOWN_BRIGHTNESS_PIC_CNT - 1;
    }
    return level - 1;
}

static u8 dropdown_brightness_level_to_duty(u8 level)
{
    if (level < 1) {
        level = 1;
    }
    if (level > DROPDOWN_LIGHT_LEVEL_MAX) {
        level = DROPDOWN_LIGHT_LEVEL_MAX;
    }
    return (u8)((u16)level * 100 / DROPDOWN_LIGHT_LEVEL_MAX);
}

static void dropdown_brightness_icon_next(void)
{
    u8 level;

    dropdown_brightness_icon_show(dropdown_brightness_pic_idx + 1);
    level = dropdown_brightness_pic_to_level(dropdown_brightness_pic_idx);
    sys_cb.light_level = level;
    lcd_drv_set_brightness(dropdown_brightness_level_to_duty(level));
}

static u32 dropdown_battery_icon_addr(void)
{
    // switch (sys_cb.vbat_percent) {
    // case 1 ... 16:
    //     return UI_BUF_DROPDOWN_POWER1_BIN;
    // case 17 ... 32:
    //     return UI_BUF_DROPDOWN_POWER2_BIN;
    // case 33 ... 48:
    //     return UI_BUF_DROPDOWN_POWER3_BIN;
    // case 49 ... 64:
    //     return UI_BUF_DROPDOWN_POWER4_BIN;
    // case 65 ... 80:
    //     return UI_BUF_DROPDOWN_POWER5_BIN;
    // case 81 ... 100:
    //     return UI_BUF_DROPDOWN_POWER6_BIN;
    // default:
    //     return UI_BUF_DROPDOWN_POWER6_BIN;
    // }
}

static void dropdown_center_icons_create(compo_form_t *frm)
{
    const s16 cx = GUI_SCREEN_CENTER_X;
    const s16 cy = GUI_SCREEN_CENTER_Y;

    u32 addr_bat = dropdown_battery_icon_addr();

    dropdown_center_pic_bat = compo_picturebox_create(frm, addr_bat);
    compo_picturebox_set_pos(dropdown_center_pic_bat, cx, cy);
    compo_picturebox_set_alpha(dropdown_center_pic_bat, MENU_DROPDOWN_ALPHA);

    dropdown_last_vbat = sys_cb.vbat_percent;
    dropdown_last_ble = ble_is_connect() ? 1 : 0;
}

static void dropdown_center_icons_refresh(void)
{
    // u8 ble_now = ble_is_connect() ? 1 : 0;

    // if (dropdown_center_pic_bat == NULL) {
    //     return;
    // }
    // if (dropdown_center_pic_bt != NULL && ble_now != dropdown_last_ble) {
    //     dropdown_last_ble = ble_now;
    //     compo_picturebox_set(dropdown_center_pic_bt,
    //                          ble_now ? UI_BUF_DROPDOWN_CONNECT_ON_BIN : UI_BUF_DROPDOWN_CONNECT_OFF_BIN);
    // }
    // if (sys_cb.vbat_percent != dropdown_last_vbat) {
    //     dropdown_last_vbat = sys_cb.vbat_percent;
    //     compo_picturebox_set(dropdown_center_pic_bat, dropdown_battery_icon_addr());
    // }
}

static void dropdown_center_icons_clear(void)
{
    dropdown_center_pic_bt = NULL;
    dropdown_center_pic_bat = NULL;
    dropdown_btn_brightness = NULL;
}

//创建下拉菜单
static void func_clock_sub_dropdown_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_BLACK);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(masklayer, MENU_DROPDOWN_BG_ALPHA);

    /* 5 枚环形按钮：等分 360°，从正上方起顺时针；坐标为按钮中心 */
    {
        const s16 rr = MENU_DROPDOWN_RING_R;
        const s16 cx = GUI_SCREEN_CENTER_X;
        const s16 cy = GUI_SCREEN_CENTER_Y;

        // compo_button_t *btn_volume = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_VOLUME_BIN);
        // compo_setid(btn_volume, COMPO_ID_DROPDOWN_VOLUME);
        // compo_button_set_pos(btn_volume, cx + (s16)(((s32)rr * 0)   / 128), cy + (s16)(((s32)rr * -128) / 128));
        // compo_button_set_size(btn_volume, MENU_DROPDOWN_RING_BTN_WH, MENU_DROPDOWN_RING_BTN_WH);
        // compo_button_set_alpha(btn_volume, MENU_DROPDOWN_ALPHA);

        // compo_button_t *btn_delete = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_DELETE_BIN);
        // compo_setid(btn_delete, COMPO_ID_DROPDOWN_DELETE);
        // compo_button_set_pos(btn_delete, cx + (s16)(((s32)rr * 122) / 128), cy + (s16)(((s32)rr * -40)  / 128));
        // compo_button_set_size(btn_delete, MENU_DROPDOWN_RING_BTN_WH, MENU_DROPDOWN_RING_BTN_WH);
        // compo_button_set_alpha(btn_delete, MENU_DROPDOWN_ALPHA);

        // compo_button_t *btn_bluetooth = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_BLUETOOTH_BIN);
        // compo_setid(btn_bluetooth, COMPO_ID_DROPDOWN_BLUETOOTH);
        // compo_button_set_pos(btn_bluetooth, cx + (s16)(((s32)rr * 75) / 128), cy + (s16)(((s32)rr * 104) / 128));
        // compo_button_set_size(btn_bluetooth, MENU_DROPDOWN_RING_BTN_WH, MENU_DROPDOWN_RING_BTN_WH);
        // compo_button_set_alpha(btn_bluetooth, MENU_DROPDOWN_ALPHA);

        // dropdown_btn_brightness = compo_button_create_by_image(frm,
        //     tbl_dropdown_brightness_pic[0]);
        // compo_setid(dropdown_btn_brightness, COMPO_ID_DROPDOWN_BRIGHTNESS);
        // compo_button_set_pos(dropdown_btn_brightness,
        //     cx + (s16)(((s32)rr * -75) / 128), cy + (s16)(((s32)rr * 104) / 128));
        // compo_button_set_size(dropdown_btn_brightness,
        //     MENU_DROPDOWN_RING_BTN_WH, MENU_DROPDOWN_RING_BTN_WH);
        // compo_button_set_alpha(dropdown_btn_brightness, MENU_DROPDOWN_ALPHA);
        // dropdown_brightness_icon_show(dropdown_brightness_level_to_pic(sys_cb.light_level));

        // compo_button_t *btn_flashlight = compo_button_create_by_image(frm, UI_BUF_DROPDOWN_FLASHLIGHT_BIN);
        // compo_setid(btn_flashlight, COMPO_ID_DROPDOWN_FLASHLIGHT);
        // compo_button_set_pos(btn_flashlight, cx + (s16)(((s32)rr * -122) / 128), cy + (s16)(((s32)rr * -40) / 128));
        // compo_button_set_size(btn_flashlight, MENU_DROPDOWN_RING_BTN_WH, MENU_DROPDOWN_RING_BTN_WH);
        // compo_button_set_alpha(btn_flashlight, MENU_DROPDOWN_ALPHA);
    }

    /* 中心：蓝牙连接态 + 电量（叠在环内，最后创建保证在上层） */
    dropdown_center_icons_create(frm);

    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->sub_frm = frm;

    f_clk->masklayer = masklayer;
}

//时钟表盘主要事件流程处理
static void func_clock_sub_dropdown_process(void)
{
    dropdown_center_icons_refresh();
    func_clock_sub_process();
}

void func_clock_sub_dropdown_btn_handle(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    u8 func_jump = FUNC_NULL;
    int btn_id;

    if (f_clk == NULL || f_clk->sub_frm == NULL) {
        return;
    }
    compo_pool_set_top(f_clk->sub_frm);

    btn_id = compo_get_button_id();
    if (btn_id == ID_NULL) {
        return;
    }

    switch (btn_id) {
    case COMPO_ID_DROPDOWN_VOLUME:
        func_jump = FUNC_VOLUME;
        break;
    case COMPO_ID_DROPDOWN_DELETE:
        func_jump = FUNC_SETTING;
        break;
    case COMPO_ID_DROPDOWN_BLUETOOTH:
        func_jump = FUNC_BT;
        break;
    case COMPO_ID_DROPDOWN_BRIGHTNESS:
        dropdown_brightness_icon_next();
        return;
    case COMPO_ID_DROPDOWN_FLASHLIGHT:
        func_jump = FUNC_FLASHLIGHT;
        break;
    default:
        break;
    }

    if (func_jump != FUNC_NULL) {
        f_clk->sta = FUNC_CLOCK_MAIN;
        /* 下拉 sub_frm 与表盘主窗体占满双缓冲池；不先释放则 func_switch_to 内
         * func_create_form 无法再 compo_pool_create -> resource halt 0x5106。 */
        if (f_clk->sub_frm != NULL) {
            compo_form_destroy(f_clk->sub_frm);
            f_clk->sub_frm = NULL;
            f_clk->masklayer = NULL;
            dropdown_center_icons_clear();
        }
        if (func_cb.frm_main != NULL) {
            compo_pool_set_top(func_cb.frm_main);
        }
        func_switch_to(func_jump, func_get_switching_mode_byidx(sys_cb.nav_index, true) | FUNC_SWITCH_AUTO);
    }
}

//时钟表盘下拉菜单功能消息处理
static void func_clock_sub_dropdown_message(size_msg_t msg)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    if (f_clk == NULL) {
        return;
    }
    switch (msg) {
    case MSG_CTP_CLICK:
        func_clock_sub_dropdown_btn_handle();

    break;

    case MSG_CTP_SHORT_UP:
        {
            if (!tick_check_expire(dropdown_short_up_ignore_tick, MENU_DROPDOWN_SHORT_UP_DEBOUNCE_MS)) {
                break;
            }
            if (func_switching(FUNC_SWITCH_MENU_DROPDOWN_UP, NULL)) {
                f_clk->sta = FUNC_CLOCK_MAIN;                   //上滑跟手收起后回到主表盘
            }
        }
        break;

    case MSG_CTP_SHORT_DOWN:
        /* default 里曾丢弃所有 CTP 子类型，SHORT_DOWN 被吞掉，导致只有边缘/顶部下滑才像能「出界面」；
         * 屏内任意处下滑同样触发帘式收起。 */
        {
            if (!tick_check_expire(dropdown_short_up_ignore_tick, MENU_DROPDOWN_SHORT_UP_DEBOUNCE_MS)) {
                break;
            }
            (void)func_switching(FUNC_SWITCH_MENU_DROPDOWN_UP | FUNC_SWITCH_AUTO, NULL);
            f_clk->sta = FUNC_CLOCK_MAIN;
        }
        break;

    case KU_BACK:
    case MSG_QDEC_FORWARD:
        (void)func_switching(FUNC_SWITCH_MENU_DROPDOWN_UP | FUNC_SWITCH_AUTO, NULL);
        f_clk->sta = FUNC_CLOCK_MAIN;                       //BACK/旋钮：自动收起
        break;

    case MSG_QDEC_BACKWARD:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
        break;

    default:
        if (msg >= MSG_CTP_TOUCH && msg <= MSG_CTP_DOUBLE_CLICK) {
            break;
        }
        func_clock_sub_message(msg);
        break;
    }
}

//时钟表盘下拉菜单进入处理
static void func_clock_sub_dropdown_enter(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    // func_clock_butterfly_set_light_visible(false);

    func_clock_sub_dropdown_form_create();
    if (f_clk->sub_frm != NULL) {
        compo_pool_set_top(f_clk->sub_frm);   /* 保证 compo_pool_get_top 为下拉窗体，帘式动画才能跟手 */
    }
    if (!func_switching(FUNC_SWITCH_MENU_DROPDOWN_DOWN, NULL)) {
        return;                                             /* 中途取消下拉或池异常，不进入子状态 */
    }

    f_clk->sta = FUNC_CLOCK_SUB_DROPDOWN;                   //进入到下拉菜单
    dropdown_short_up_ignore_tick = tick_get();
}

//时钟表盘下拉菜单退出处理
/* static  */void func_clock_sub_dropdown_exit(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    if (f_clk != NULL && f_clk->sub_frm != NULL) {
        compo_form_destroy(f_clk->sub_frm);
        f_clk->sub_frm = NULL;
    }
    if (f_clk != NULL) {
        f_clk->masklayer = NULL;
    }
    func_clock_butterfly_set_light_visible(true);
    dropdown_center_icons_clear();
}

//时钟表盘下拉菜单
void func_clock_sub_dropdown(void)
{
#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    compo_video_exit_lock(video);
    /* exit_lock 内会 bsp_video_play_uninit；若 flag_video_start 仍为 true，
     * func_clock_process 不会再调 compo_video_play，解码/GPU 与 UI 脱节易触发 resource halt C241。 */
    {
        f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
        if (f_clk != NULL) {
            f_clk->tick_video_start = tick_get();
            f_clk->flag_video_start = false;
        }
    }
#endif // VIDEO_PLAY_EN
    func_clock_sub_dropdown_enter();
    while (func_cb.sta == FUNC_CLOCK && func_cb.f_cb != NULL
           && ((f_clock_t *)func_cb.f_cb)->sta == FUNC_CLOCK_SUB_DROPDOWN) {
        func_clock_sub_dropdown_process();
        func_clock_sub_dropdown_message(msg_dequeue());
    }
    func_clock_sub_dropdown_exit();
#if VIDEO_PLAY_EN
    compo_video_exit_unlock(video);
#endif // VIDEO_PLAY_EN
}
