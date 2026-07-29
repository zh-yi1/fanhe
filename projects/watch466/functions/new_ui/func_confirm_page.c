#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_ui.h"
#include "lang.h"
#include "ui.h"

#if ELUNCHBOX_PANEL_EN

/*
 * 开盖确认弹窗 — overlay 模式（叠加在当前 form 上，不切换页面）
 *
 *   lid_open 标志位 → func_process 检测 → show overlay
 *   按键：UP/DOWN 切换 NO/YES；CONFIRM 确认；BACK 取消
 */

#define CONFIRM_DIM_ALPHA      160
#define CONFIRM_PANEL_W        280
#define CONFIRM_PANEL_H        160
#define CONFIRM_PANEL_RADIUS   16

#define CONFIRM_MSG_W          240
#define CONFIRM_MSG_H          48
#define CONFIRM_MSG_Y          ((s16)(GUI_SCREEN_CENTER_Y - 38))

#define CONFIRM_SUB_W          240
#define CONFIRM_SUB_H          28
#define CONFIRM_SUB_Y          ((s16)(GUI_SCREEN_CENTER_Y + 10))

#define CONFIRM_BTN_DIST       130
#define CONFIRM_BTN_W          110
#define CONFIRM_BTN_H          44
#define CONFIRM_BTN_Y          ((s16)(GUI_SCREEN_CENTER_Y + CONFIRM_PANEL_H / 2 - CONFIRM_BTN_H / 2 - 14))
#define CONFIRM_BTN_NO_X       ((s16)(GUI_SCREEN_CENTER_X - CONFIRM_BTN_DIST / 2))
#define CONFIRM_BTN_YES_X      ((s16)(GUI_SCREEN_CENTER_X + CONFIRM_BTN_DIST / 2))
#define CONFIRM_BTN_LABEL_Y    ((s16)(CONFIRM_BTN_Y - 4))

#define CONFIRM_COLOR_MSG      0x0AD8
#define CONFIRM_COLOR_SUB      0x9CD3
#define CONFIRM_COLOR_ON       COLOR_WHITE
#define CONFIRM_COLOR_OFF      COLOR_BLACK

enum {
    CONFIRM_SEL_NO = 0,
    CONFIRM_SEL_YES,
};

/* ---- overlay 静态状态 ---- */
static compo_shape_t      *g_cfm_dim;
static compo_shape_t      *g_cfm_panel;
static compo_textbox_t    *g_cfm_msg;
static compo_textbox_t    *g_cfm_sub;
static compo_picturebox_t *g_cfm_btn_no;
static compo_picturebox_t *g_cfm_btn_yes;
static compo_textbox_t    *g_cfm_txt_no;
static compo_textbox_t    *g_cfm_txt_yes;
static u8                  g_cfm_sel;      /* CONFIRM_SEL_NO / CONFIRM_SEL_YES */
static bool                g_cfm_visible;
static bool                g_cfm_done;     /* 用户已做出选择 */
static bool                g_cfm_result;   /* true=YES, false=NO */

/* ---- 内部 helpers ---- */

static void cfm_btn_show(compo_textbox_t *txt, s16 x, const char *label, u16 color)
{
    if (txt == NULL) return;
    compo_textbox_set_location(txt, x, CONFIRM_BTN_LABEL_Y, CONFIRM_BTN_W, CONFIRM_BTN_H);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set(txt, label);
    compo_textbox_set_visible(txt, true);
}

static void cfm_text_show(compo_textbox_t *txt, const char *str,
                          s16 y, u16 w, u16 h, u16 color)
{
    widget_text_t *widget;
    rect_t rect;
    area_t text_area;

    if (txt == NULL || str == NULL) return;
    widget = txt->txt;
    compo_textbox_set_multiline(txt, true);
    compo_textbox_set_align_center(txt, true);
    compo_textbox_set_wholewrap(txt, true);
    compo_textbox_set_autosize(txt, false);
    compo_textbox_set_autoroll(txt, false);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, y, w, h);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_TEST_14_BIN);
    compo_textbox_set_forecolor(txt, color);
    compo_textbox_set(txt, str);
    if (widget != NULL) {
        widget_set_align_center(widget, true);
        widget_text_set_ellipsis(widget, false);
        rect = widget_get_location(widget);
        text_area = widget_text_get_area(widget);
        if (rect.hei > text_area.hei) {
            widget_text_set_client(widget, 0, (rect.hei - text_area.hei) >> 1);
        } else {
            widget_text_set_client(widget, 0, 0);
        }
    }
    compo_textbox_set_visible(txt, true);
}

static void cfm_update_display(void)
{
    bool no_sel  = (g_cfm_sel == CONFIRM_SEL_NO);
    bool yes_sel = (g_cfm_sel == CONFIRM_SEL_YES);

    if (g_cfm_btn_no != NULL) {
        compo_picturebox_set(g_cfm_btn_no,
                             no_sel ? UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN
                                    : UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN);
    }
    if (g_cfm_btn_yes != NULL) {
        compo_picturebox_set(g_cfm_btn_yes,
                             yes_sel ? UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN
                                     : UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN);
    }
    cfm_text_show(g_cfm_msg, i18n[STR_CONFIRM_LID_OPEN],
                  CONFIRM_MSG_Y, CONFIRM_MSG_W, CONFIRM_MSG_H, CONFIRM_COLOR_MSG);
    cfm_text_show(g_cfm_sub, i18n[STR_CONFIRM_CONTINUE_HEAT],
                  CONFIRM_SUB_Y, CONFIRM_SUB_W, CONFIRM_SUB_H, CONFIRM_COLOR_SUB);
    cfm_btn_show(g_cfm_txt_no, CONFIRM_BTN_NO_X, i18n[STR_BTN_NO],
                 no_sel ? CONFIRM_COLOR_ON : CONFIRM_COLOR_OFF);
    cfm_btn_show(g_cfm_txt_yes, CONFIRM_BTN_YES_X, i18n[STR_BTN_YES],
                 yes_sel ? CONFIRM_COLOR_ON : CONFIRM_COLOR_OFF);
}

static void cfm_cleanup(void)
{
    g_cfm_dim     = NULL;
    g_cfm_panel   = NULL;
    g_cfm_msg     = NULL;
    g_cfm_sub     = NULL;
    g_cfm_btn_no  = NULL;
    g_cfm_btn_yes = NULL;
    g_cfm_txt_no  = NULL;
    g_cfm_txt_yes = NULL;
    g_cfm_visible = false;
}

static bool cfm_ensure_widgets(compo_form_t *frm)
{
    if (frm == NULL) return false;
    if (g_cfm_dim != NULL) return true; /* already created */

    /* 半透明遮罩 */
    g_cfm_dim = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    if (g_cfm_dim == NULL) return false;
    compo_shape_set_color(g_cfm_dim, COLOR_WHITE);
    compo_shape_set_location(g_cfm_dim, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_shape_set_alpha(g_cfm_dim, CONFIRM_DIM_ALPHA);
    compo_shape_set_visible(g_cfm_dim, false);

    /* 白色圆角面板 */
    g_cfm_panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    if (g_cfm_panel == NULL) return false;
    compo_shape_set_color(g_cfm_panel, COLOR_WHITE);
    compo_shape_set_location(g_cfm_panel, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             CONFIRM_PANEL_W, CONFIRM_PANEL_H);
    compo_shape_set_radius(g_cfm_panel, CONFIRM_PANEL_RADIUS);
    compo_shape_set_visible(g_cfm_panel, false);

    /* 主/副标题 */
    g_cfm_msg = compo_textbox_create(frm, 80);
    g_cfm_sub = compo_textbox_create(frm, 64);

    /* NO / YES 按钮 */
    g_cfm_btn_no = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_GRAY_BJ2_BIN);
    compo_picturebox_set_pos(g_cfm_btn_no, CONFIRM_BTN_NO_X, CONFIRM_BTN_Y);
    compo_picturebox_set_size(g_cfm_btn_no, CONFIRM_BTN_W, CONFIRM_BTN_H);
    compo_picturebox_set_visible(g_cfm_btn_no, false);

    g_cfm_btn_yes = compo_picturebox_create(frm, UI_BUF_NEW_UI_NEW_BLUE_BJ2_BIN);
    compo_picturebox_set_pos(g_cfm_btn_yes, CONFIRM_BTN_YES_X, CONFIRM_BTN_Y);
    compo_picturebox_set_size(g_cfm_btn_yes, CONFIRM_BTN_W, CONFIRM_BTN_H);
    compo_picturebox_set_visible(g_cfm_btn_yes, false);

    g_cfm_txt_no  = compo_textbox_create(frm, 8);
    compo_textbox_set_font(g_cfm_txt_no, UI_BUF_0FONT_FONT_TEST_14_BIN);
    g_cfm_txt_yes = compo_textbox_create(frm, 8);
    compo_textbox_set_font(g_cfm_txt_yes, UI_BUF_0FONT_FONT_TEST_14_BIN);

    return true;
}

/* ---- 公开 API ---- */

void func_confirm_overlay_show(void)
{
    printf("confirm overlay show\n");

    if (func_cb.frm_main == NULL) return;

    /* form 切换后重建 */
    if (g_cfm_visible && g_cfm_dim == NULL) {
        cfm_cleanup();
    }

    if (!cfm_ensure_widgets(func_cb.frm_main)) return;

    g_cfm_sel    = CONFIRM_SEL_YES; /* 默认选中 YES */
    g_cfm_done   = false;
    g_cfm_result = false;
    cfm_update_display();

    compo_shape_set_visible(g_cfm_dim, true);
    compo_shape_set_visible(g_cfm_panel, true);
    compo_picturebox_set_visible(g_cfm_btn_no, true);
    compo_picturebox_set_visible(g_cfm_btn_yes, true);

    /* 置顶 */
    if (g_cfm_dim != NULL && g_cfm_dim->rect != NULL)
        widget_set_top(g_cfm_dim->rect, true);
    if (g_cfm_panel != NULL && g_cfm_panel->rect != NULL)
        widget_set_top(g_cfm_panel->rect, true);

    g_cfm_visible = true;
}

void func_confirm_overlay_hide(void)
{
    printf("confirm overlay hide\n");
    g_cfm_visible = false;
    if (g_cfm_dim != NULL)     compo_shape_set_visible(g_cfm_dim, false);
    if (g_cfm_panel != NULL)   compo_shape_set_visible(g_cfm_panel, false);
    if (g_cfm_btn_no != NULL)  compo_picturebox_set_visible(g_cfm_btn_no, false);
    if (g_cfm_btn_yes != NULL) compo_picturebox_set_visible(g_cfm_btn_yes, false);
    if (g_cfm_txt_no != NULL)  compo_textbox_set_visible(g_cfm_txt_no, false);
    if (g_cfm_txt_yes != NULL) compo_textbox_set_visible(g_cfm_txt_yes, false);
    if (g_cfm_msg != NULL)     compo_textbox_set_visible(g_cfm_msg, false);
    if (g_cfm_sub != NULL)     compo_textbox_set_visible(g_cfm_sub, false);
}

bool func_confirm_overlay_visible(void)
{
    return g_cfm_visible;
}

bool func_confirm_overlay_get_result(void)
{
    return g_cfm_result;
}

bool func_confirm_overlay_is_done(void)
{
    return g_cfm_done;
}

/*
 * 每帧调用 — 当 overlay 可见时处理按键。
 * 返回 true 表示用户已做出选择（done）。
 */
bool func_confirm_overlay_poll(void)
{
    func_key_event_t evt;

    if (!g_cfm_visible) return false;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_UP:
        case FUNC_KEY_DOWN:
            g_cfm_sel = (u8)((g_cfm_sel == CONFIRM_SEL_NO)
                             ? CONFIRM_SEL_YES : CONFIRM_SEL_NO);
            cfm_update_display();
            break;

        case FUNC_KEY_CONFIRM:
            g_cfm_result = (g_cfm_sel == CONFIRM_SEL_YES);
            g_cfm_done   = true;
            func_confirm_overlay_hide();
            return true;

        case FUNC_KEY_BACK:
            g_cfm_result = false;
            g_cfm_done   = true;
            func_confirm_overlay_hide();
            return true;

        default:
            break;
        }
    }
    return false;
}

#endif /* ELUNCHBOX_PANEL_EN */
