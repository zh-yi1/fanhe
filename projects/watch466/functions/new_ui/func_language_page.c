#include "include.h"
#include "func.h"
#include "func_key.h"
#include "func_key_lock.h"
#include "general_title_ui.h"
#include "lang.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/*
 * 语言页 — 效果图 LANGUAGE
 *   顶栏：标题 i18n[STR_Language] + 蓝牙/电量（general_title_bar）
 *   白底圆角列表：English / Deutsch / Italiano / Francais（i18n）
 *   选中行：浅蓝底 + 蓝色文字；右侧箭头
 *   UP/DOWN 切换；CONFIRM 保存并回设置；BACK 回设置
 */
#define LANG_PAGE_BG_COLOR              0xEF5D
#define LANG_PAGE_PANEL_BG              COLOR_WHITE
#define LANG_PAGE_COLOR_SEL             0x0AD8
#define LANG_PAGE_COLOR_NOR             COLOR_BLACK
#define LANG_PAGE_COLOR_SEL_BG          0xCFDF
#define LANG_PAGE_COLOR_DIVIDER         0xCE79

#define LANG_PAGE_PANEL_W               280
#define LANG_PAGE_PANEL_H               188
#define LANG_PAGE_PANEL_Y               132
#define LANG_PAGE_PANEL_RADIUS          16

#define LANG_PAGE_ROW_FIRST_Y           68
#define LANG_PAGE_ROW_GAP               42
#define LANG_PAGE_ROW_H                 41
#define LANG_PAGE_SEL_W                 260

#define LANG_PAGE_LABEL_X               48
#define LANG_PAGE_ARROW_X               270
#define LANG_PAGE_ARROW_W               9
#define LANG_PAGE_ARROW_H               13

#define LANG_PAGE_ITEM_CNT              4

/* 语言页私有状态 */
typedef struct
{
    u8 selection;
    general_title_bar_t tb;
    compo_shape_t *shape_panel;
    compo_shape_t *shape_sel;
    compo_shape_t *shape_divider[LANG_PAGE_ITEM_CNT - 1];
    compo_textbox_t *txt_label[LANG_PAGE_ITEM_CNT];
    compo_textbox_t *txt_arrow[LANG_PAGE_ITEM_CNT];
} f_language_t;

static const u16 g_lang_label_ids[LANG_PAGE_ITEM_CNT] = {
    STR_LANGUAGE_ENG,
    STR_LANGUAGE_DE,
    STR_LANGUAGE_IT,
    STR_LANGUAGE_FN,
};

static s16 language_row_y(u8 row)
{
    return (s16)(LANG_PAGE_ROW_FIRST_Y + (s16)row * LANG_PAGE_ROW_GAP);
}

static void language_update_display(void)
{
    f_language_t *inf = (f_language_t *)func_cb.f_cb;
    u8 i;

    if (inf == NULL) {
        return;
    }

    if (inf->shape_sel != NULL) {
        compo_shape_set_location(inf->shape_sel, GUI_SCREEN_CENTER_X,
                                 language_row_y(inf->selection),
                                 LANG_PAGE_SEL_W, LANG_PAGE_ROW_H);
        compo_shape_set_visible(inf->shape_sel, true);
    }

    for (i = 0; i < LANG_PAGE_ITEM_CNT; i++) {
        bool selected = (i == inf->selection);
        if (inf->txt_label[i] != NULL) {
            compo_textbox_set_forecolor(inf->txt_label[i],
                                        selected ? LANG_PAGE_COLOR_SEL : LANG_PAGE_COLOR_NOR);
        }
    }
}

static void language_apply_confirm(f_language_t *inf)
{
    if (inf == NULL || inf->selection >= LANG_PAGE_ITEM_CNT) {
        return;
    }

    sys_cb.lang_id = inf->selection;
    param_lang_id_write();
    /* lang_select 仅支持 LANG_EN(0) / LANG_ZH(1)，其余只保存索引 */
    if (inf->selection == LANG_EN || inf->selection == LANG_ZH) {
        lang_select(inf->selection);
    }
    func_cb.sta = FUNC_NEW_SETUP;
}

compo_form_t *func_language_page_form_create(void)
{
    f_language_t *inf = (f_language_t *)func_cb.f_cb;
    compo_form_t *frm = compo_form_create(true);
    compo_shape_t *shape;
    u8 i;

    /* 浅灰背景 */
    widget_set_visible(frm->icon, false);
    shape = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(shape, LANG_PAGE_BG_COLOR);
    compo_shape_set_location(shape, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y,
                             GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    /* 顶部标题栏 */
    general_title_bar_create(frm, &inf->tb, i18n[STR_LANGUAGE]);

    /* 白色圆角列表面板 */
    inf->shape_panel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(inf->shape_panel, LANG_PAGE_PANEL_BG);
    compo_shape_set_location(inf->shape_panel, GUI_SCREEN_CENTER_X, LANG_PAGE_PANEL_Y,
                             LANG_PAGE_PANEL_W, LANG_PAGE_PANEL_H);
    compo_shape_set_radius(inf->shape_panel, LANG_PAGE_PANEL_RADIUS);

    /* 选中行浅蓝底 */
    inf->shape_sel = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(inf->shape_sel, LANG_PAGE_COLOR_SEL_BG);
    compo_shape_set_radius(inf->shape_sel, 12);
    compo_shape_set_visible(inf->shape_sel, false);

    /* 分隔线 + 语言文字 + 右侧箭头文字
     * 注意：不要用 UI_BUF_NEW_UI_NEW_LEFT_BIN 直接 picturebox_create。
     * 该 bin 头格式标志为 2（常规图标为 0x100），首帧渲染会 resource halt C245。
     * 旧语言页也是 create(0)+RAM bind；此处用 ">" 文本更简单稳妥。 */
    for (i = 0; i < LANG_PAGE_ITEM_CNT; i++) {
        s16 row_y = language_row_y(i);

        if (i < LANG_PAGE_ITEM_CNT - 1) {
            s16 div_y = (s16)((row_y + language_row_y((u8)(i + 1))) / 2);
            inf->shape_divider[i] = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
            compo_shape_set_color(inf->shape_divider[i], LANG_PAGE_COLOR_DIVIDER);
            compo_shape_set_location(inf->shape_divider[i], GUI_SCREEN_CENTER_X, div_y,
                                     LANG_PAGE_SEL_W, 1);
        }

        inf->txt_label[i] = compo_textbox_create(frm, 20);
        /* textbox Y 为左上角，相对行中心上移半字高 */
        compo_textbox_set_location(inf->txt_label[i], LANG_PAGE_LABEL_X,
                                   (s16)(row_y - 7), 0, 0);
        compo_textbox_set_autosize(inf->txt_label[i], true);
        compo_textbox_set_align_center(inf->txt_label[i], false);
        compo_textbox_set_font(inf->txt_label[i], UI_BUF_0FONT_FONT_TEST_14_BIN);
        compo_textbox_set(inf->txt_label[i], i18n[g_lang_label_ids[i]]);
        compo_textbox_set_forecolor(inf->txt_label[i], LANG_PAGE_COLOR_NOR);

        inf->txt_arrow[i] = compo_textbox_create(frm, 4);
        compo_textbox_set_location(inf->txt_arrow[i], LANG_PAGE_ARROW_X,
                                   (s16)(row_y - 7), 0, 0);
        compo_textbox_set_autosize(inf->txt_arrow[i], true);
        compo_textbox_set_align_center(inf->txt_arrow[i], true);
        compo_textbox_set_font(inf->txt_arrow[i], UI_BUF_0FONT_FONT_TEST_14_BIN);
        compo_textbox_set(inf->txt_arrow[i], ">");
        compo_textbox_set_forecolor(inf->txt_arrow[i], LANG_PAGE_COLOR_SEL);
    }

    if (sys_cb.lang_id < LANG_PAGE_ITEM_CNT) {
        inf->selection = sys_cb.lang_id;
    } else {
        inf->selection = 0;
    }
    language_update_display();

    tft_bglight_force_on();
    return frm;
}

static void func_language_page_handle_keys(void)
{
    f_language_t *inf = (f_language_t *)func_cb.f_cb;
    func_key_event_t evt;

    while (func_key_get_event(&evt)) {
        func_key_logical_t key = func_key_map_logical(evt.tch);

        switch (key) {
        case FUNC_KEY_UP:
            inf->selection = (inf->selection == 0)
                                 ? (LANG_PAGE_ITEM_CNT - 1)
                                 : (inf->selection - 1);
            language_update_display();
            break;

        case FUNC_KEY_DOWN:
            inf->selection = (u8)((inf->selection + 1) % LANG_PAGE_ITEM_CNT);
            language_update_display();
            break;

        case FUNC_KEY_CONFIRM:
            language_apply_confirm(inf);
            break;

        case FUNC_KEY_BACK:
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


        default:
            break;
        }
    }
}

static void func_language_page_process(void)
{
    func_key_poll();
    func_language_page_handle_keys();

    func_key_lock_poll();

    if (func_key_lock_gui_dirty()) {
        if (func_key_lock_overlay_visible()) {
            func_lock_page_show(func_key_lock_overlay_is_unlock());
        } else {
            func_lock_page_hide();
        }
    }

    func_process();
}

static void func_language_page_message(size_msg_t msg)
{
    func_message(msg);
}

void func_language_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_language_t));
    func_key_reset();
    func_cb.frm_main = func_language_page_form_create();
    general_title_bar_attach(&((f_language_t *)func_cb.f_cb)->tb);
}

void func_language_page_exit(void)
{
    func_key_flush();
    general_title_bar_detach();
}

void func_language_page(void)
{
    printf("%s\n", __func__);
    func_language_page_enter();
    while (func_cb.sta == FUNC_NEW_LANGUAGE) {
        func_language_page_process();
        func_language_page_message(msg_dequeue());
    }
    func_language_page_exit();
}
