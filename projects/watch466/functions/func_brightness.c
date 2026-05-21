#include "include.h"
#include "func.h"
#include "compo_button.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define BRIGHTNESS_PIC_CNT              6       /* 6 张图对应 6 档背光 */
#define BRIGHTNESS_ICON_WH              ((s16)((s32)GUI_SCREEN_WIDTH * 65 / 100))

enum {
    COMPO_ID_BTN_BRIGHTNESS = 1,
};

typedef struct f_brightness_t_ {
    compo_button_t *btn;
    u8 pic_idx;
} f_brightness_t;

static const u32 tbl_brightness_pic[BRIGHTNESS_PIC_CNT] = {
    UI_BUF_DROPDOWN_BIG_BRIGHTNESS_ADJUSTMENT_ONE_BIN,
    UI_BUF_DROPDOWN_BIG_BRIGHTNESS_ADJUSTMENT_TWO_BIN,
    UI_BUF_DROPDOWN_BIG_BRIGHTNESS_ADJUSTMENT_THREE_BIN,
    UI_BUF_DROPDOWN_BIG_BRIGHTNESS_ADJUSTMENT_FOUR_BIN,
    UI_BUF_DROPDOWN_BIG_BRIGHTNESS_ADJUSTMENT_FIVE_BIN,
    UI_BUF_DROPDOWN_BIG_BRIGHTNESS_ADJUSTMENT_SIX_BIN,
};

/* 图标下标 0~5 -> 背光档 1~6 */
static u8 brightness_pic_to_level(u8 pic_idx)
{
    return (pic_idx % BRIGHTNESS_PIC_CNT) + 1;
}

/* 背光档 1~6 -> 图标下标 0~5 */
static u8 brightness_level_to_pic(u8 level)
{
    if (level < 1) {
        return 0;
    }
    if (level > BRIGHTNESS_PIC_CNT) {
        return BRIGHTNESS_PIC_CNT - 1;
    }
    return level - 1;
}

static u8 brightness_level_to_duty(u8 level)
{
    if (level < 1) {
        level = 1;
    }
    if (level > BRIGHTNESS_PIC_CNT) {
        level = BRIGHTNESS_PIC_CNT;
    }
    return (u8)((u16)level * 100 / BRIGHTNESS_PIC_CNT);
}

static void brightness_icon_show(f_brightness_t *f, u8 idx)
{
    if (f == NULL || f->btn == NULL) {
        return;
    }
    f->pic_idx = idx % BRIGHTNESS_PIC_CNT;
    compo_button_set_bgimg(f->btn, tbl_brightness_pic[f->pic_idx]);
    compo_button_set_size(f->btn, BRIGHTNESS_ICON_WH, BRIGHTNESS_ICON_WH);
}

static void brightness_icon_next(f_brightness_t *f)
{
    u8 level;

    brightness_icon_show(f, f->pic_idx + 1);
    level = brightness_pic_to_level(f->pic_idx);
    sys_cb.light_level = level;
    lcd_drv_set_brightness(brightness_level_to_duty(level));
}

compo_form_t *func_brightness_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    compo_button_t *btn = compo_button_create_by_image(frm,
        tbl_brightness_pic[brightness_level_to_pic(sys_cb.light_level)]);
    compo_setid(btn, COMPO_ID_BTN_BRIGHTNESS);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_button_set_size(btn, BRIGHTNESS_ICON_WH, BRIGHTNESS_ICON_WH);

    return frm;
}

static void func_brightness_process(void)
{
    func_process();
}

static void func_brightness_message(size_msg_t msg)
{
    f_brightness_t *f = (f_brightness_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_CLICK:
        brightness_icon_next(f);
        break;

    case KU_RIGHT:                                      /* KEY2 返回上一级 */
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_back_to();
        }
        break;

    /* 禁止左/右/上/下滑退出（足球菜单右滑会进 func_back_to，不交给 func_message） */
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_LONG:
    case MSG_CTP_LONG_UP:
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_brightness_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_brightness_t));
    func_cb.frm_main = func_brightness_form_create();

    f_brightness_t *f = (f_brightness_t *)func_cb.f_cb;
    f->btn = compo_getobj_byid(COMPO_ID_BTN_BRIGHTNESS);

    if (sys_cb.light_level < 1) {
        sys_cb.light_level = 1;
    } else if (sys_cb.light_level > BRIGHTNESS_PIC_CNT) {
        sys_cb.light_level = BRIGHTNESS_PIC_CNT;
    }
    brightness_icon_show(f, brightness_level_to_pic(sys_cb.light_level));
    lcd_drv_set_brightness(brightness_level_to_duty(sys_cb.light_level));
    func_cb.enter_tick = tick_get();
}

void func_brightness_exit(void)
{
    func_cb.last = FUNC_BRIGHTNESS;
}

void func_brightness(void)
{
    func_brightness_enter();
    while (func_cb.sta == FUNC_BRIGHTNESS) {
        func_brightness_process();
        func_brightness_message(msg_dequeue());
    }
    func_brightness_exit();
}
