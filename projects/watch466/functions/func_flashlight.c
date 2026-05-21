#include "include.h"
#include "func.h"
#include "compo_shape.h"
#include "compo_picturebox.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/* 左右边缘滑动切换背景（屏宽约 1/12） */
#define FLASHLIGHT_EDGE_W               (GUI_SCREEN_WIDTH / 12)
#define FLASHLIGHT_BG_CNT               7
#define FLASHLIGHT_FLASH_PERIOD_MS      120

enum {
    COMPO_ID_PIC_BG = 1,
    COMPO_ID_SHAPE_FLASH_OFF,
};

typedef struct f_flashlight_t_ {
    compo_picturebox_t *pic_bg;
    compo_shape_t *shape_off;
    u8 bg_idx;
    u8 flash_en;                        /* 1：背景闪烁中 */
    u8 flash_phase;                     /* 0：显示背景图，1：黑屏 */
    u32 flash_tick;
} f_flashlight_t;

/* 退出后再次进入仍使用上次选择的背景（进程内保持） */
static u8 s_flashlight_bg_idx;

static void func_flashlight_bg_idx_save(u8 idx)
{
    s_flashlight_bg_idx = idx % FLASHLIGHT_BG_CNT;
}

/* ui.bin 内由 PNG 转成的 bin，地址见 ui.h（UI_BUF_FLASHLIGHT_BG_*） */
static const u32 tbl_flashlight_bg_res[FLASHLIGHT_BG_CNT] = {
    UI_BUF_FLASHLIGHT_BLUE_BIN,
    UI_BUF_FLASHLIGHT_GREEN_BIN,
    UI_BUF_FLASHLIGHT_ORANGE_BIN,
    UI_BUF_FLASHLIGHT_PURPLE_BIN,
    UI_BUF_FLASHLIGHT_RED_BIN,
    UI_BUF_FLASHLIGHT_WHITE_BIN,
    UI_BUF_FLASHLIGHT_YELLOW_BIN,
};

static void func_flashlight_bg_show(f_flashlight_t *fst)
{
    if (fst == NULL || fst->pic_bg == NULL) {
        return;
    }
    compo_picturebox_set(fst->pic_bg, tbl_flashlight_bg_res[fst->bg_idx % FLASHLIGHT_BG_CNT]);
    /* 换图后 widget 会恢复为资源原始尺寸，需重新拉伸至全屏 */
    compo_picturebox_set_size(fst->pic_bg, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_picturebox_set_visible(fst->pic_bg, true);
    if (fst->shape_off != NULL) {
        compo_shape_set_visible(fst->shape_off, false);
    }
}

static void func_flashlight_bg_next(f_flashlight_t *fst)
{
    fst->bg_idx = (fst->bg_idx + 1) % FLASHLIGHT_BG_CNT;
    func_flashlight_bg_idx_save(fst->bg_idx);
    func_flashlight_bg_show(fst);
}

static void func_flashlight_bg_prev(f_flashlight_t *fst)
{
    if (fst->bg_idx == 0) {
        fst->bg_idx = FLASHLIGHT_BG_CNT - 1;
    } else {
        fst->bg_idx--;
    }
    func_flashlight_bg_idx_save(fst->bg_idx);
    func_flashlight_bg_show(fst);
}

static void func_flashlight_flash_apply_phase(f_flashlight_t *fst)
{
    bool show_bg = (fst->flash_phase == 0);

    if (fst->shape_off != NULL) {
        compo_shape_set_visible(fst->shape_off, !show_bg);
    }
    if (fst->pic_bg != NULL) {
        compo_picturebox_set_visible(fst->pic_bg, show_bg);
    }
}

/* 点击切换：未闪烁则开始，闪烁中则停止并恢复当前背景 */
static void func_flashlight_flash_toggle(f_flashlight_t *fst)
{
    if (fst == NULL) {
        return;
    }
    if (fst->flash_en) {
        fst->flash_en = 0;
        func_flashlight_bg_show(fst);
        return;
    }
    fst->flash_en = 1;
    fst->flash_phase = 0;
    fst->flash_tick = tick_get();
    func_flashlight_flash_apply_phase(fst);
}

static void func_flashlight_flash_process(f_flashlight_t *fst)
{
    if (fst == NULL || !fst->flash_en) {
        return;
    }
    if (!tick_check_expire(fst->flash_tick, FLASHLIGHT_FLASH_PERIOD_MS)) {
        return;
    }
    fst->flash_tick = tick_get();
    fst->flash_phase ^= 1;
    func_flashlight_flash_apply_phase(fst);
}

static bool func_flashlight_touch_from_left(void)
{
    return ctp_get_sxy().x < FLASHLIGHT_EDGE_W;
}

static bool func_flashlight_touch_from_right(void)
{
    return ctp_get_sxy().x >= (GUI_SCREEN_WIDTH - FLASHLIGHT_EDGE_W);
}

compo_form_t *func_flashlight_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    compo_picturebox_t *pic_bg = compo_picturebox_create(frm,
        tbl_flashlight_bg_res[s_flashlight_bg_idx % FLASHLIGHT_BG_CNT]);
    compo_setid(pic_bg, COMPO_ID_PIC_BG);
    compo_picturebox_set_pos(pic_bg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(pic_bg, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    compo_shape_t *shape_off = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_setid(shape_off, COMPO_ID_SHAPE_FLASH_OFF);
    compo_shape_set_color(shape_off, COLOR_BLACK);
    compo_shape_set_location(shape_off, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                            GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_visible(shape_off, false);

    return frm;
}

static void func_flashlight_process(void)
{
    f_flashlight_t *fst = (f_flashlight_t *)func_cb.f_cb;
    func_flashlight_flash_process(fst);
    func_process();
}

static void func_flashlight_message(size_msg_t msg)
{
    f_flashlight_t *fst = (f_flashlight_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_SHORT_RIGHT:
        if (func_flashlight_touch_from_left()) {
            func_flashlight_bg_next(fst);
        }
        break;

    case MSG_CTP_SHORT_LEFT:
        if (func_flashlight_touch_from_right()) {
            func_flashlight_bg_prev(fst);
        }
        break;

    case MSG_CTP_CLICK:
        if (!func_flashlight_touch_from_left() && !func_flashlight_touch_from_right()) {
            func_flashlight_flash_toggle(fst);
        }
        break;

    case KU_RIGHT:                                      /* KEY2 返回上一级 */
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_back_to();
        }
        break;

    case KU_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_back_to();
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_flashlight_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_flashlight_t));
    func_cb.frm_main = func_flashlight_form_create();

    f_flashlight_t *fst = (f_flashlight_t *)func_cb.f_cb;
    fst->pic_bg = compo_getobj_byid(COMPO_ID_PIC_BG);
    fst->shape_off = compo_getobj_byid(COMPO_ID_SHAPE_FLASH_OFF);
    fst->bg_idx = s_flashlight_bg_idx;
    func_flashlight_bg_show(fst);
    func_cb.enter_tick = tick_get();
}

void func_flashlight_exit(void)
{
    f_flashlight_t *fst = (f_flashlight_t *)func_cb.f_cb;

    if (fst != NULL) {
        func_flashlight_bg_idx_save(fst->bg_idx);
    }
    func_cb.last = FUNC_FLASHLIGHT;
}

void func_flashlight(void)
{
    printf("%s\n", __func__);
    func_flashlight_enter();
    while (func_cb.sta == FUNC_FLASHLIGHT) {
        func_flashlight_process();
        func_flashlight_message(msg_dequeue());
    }
    func_flashlight_exit();
}
