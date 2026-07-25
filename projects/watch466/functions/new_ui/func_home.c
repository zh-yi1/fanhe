#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

// 首页私有状态
typedef struct
{
    u8 selection; // 0 = 加热, 1 = 模式, 2 = 设置
    compo_picturebox_t *logo;
    compo_picturebox_t *pic_backup;
    compo_picturebox_t *pic_mode;
    compo_picturebox_t *pic_set;
    compo_textbox_t *txt_heat;
    compo_textbox_t *txt_mode;
    compo_textbox_t *txt_set;
} f_home_t;

static void home_update_display(void)
{
    f_home_t *inf = (f_home_t *)func_cb.f_cb;
    bool is_backup = (inf->selection == 0);

    // 备份模式：选中态 vs 普通态
    compo_picturebox_set(inf->pic_backup, is_backup
                                            ? UI_BUF_IMAGE_BIN_BACKUP_MODE_1_BIN
                                            : UI_BUF_IMAGE_BIN_BACKUP_MODE_BIN);
    compo_textbox_set_forecolor(inf->txt_heat, is_backup ? COLOR_BLACK : COLOR_WHITE);

    // 设置：选中态 vs 普通态
    compo_picturebox_set(inf->pic_set, is_backup
                                         ? UI_BUF_IMAGE_BIN_SET_BIN
                                         : UI_BUF_IMAGE_BIN_SET_1_BIN);
    compo_textbox_set_forecolor(inf->txt_set, is_backup ? COLOR_WHITE : COLOR_BLACK);
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

    /* logo图标 */
    inf->logo = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACKUP_MODE_1_BIN);
    compo_picturebox_set_pos(inf->logo, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 24);

    /* 加热图标 */
    inf->pic_backup = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_BACKUP_MODE_1_BIN);
    compo_picturebox_set_pos(inf->pic_backup, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 24);

    inf->txt_heat = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_heat, GUI_SCREEN_CENTER_X - 60, GUI_SCREEN_CENTER_Y - 24, 0, 0);
    compo_textbox_set_autosize(inf->txt_heat, true);
    compo_textbox_set_align_center(inf->txt_heat, true);
    compo_textbox_set(inf->txt_heat, i18n[STR_HEAT]);

    /* 模式图标 */
    inf->pic_mode = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_MODE_BIN);
    compo_picturebox_set_pos(inf->pic_mode, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 24);

    inf->txt_mode = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_mode, GUI_SCREEN_CENTER_X - 60, GUI_SCREEN_CENTER_Y - 24, 0, 0);
    compo_textbox_set_autosize(inf->txt_mode, true);
    compo_textbox_set_align_center(inf->txt_mode, true);
    compo_textbox_set(inf->txt_mode, i18n[STR_MODE]);

    /* 设置图标 */
    inf->pic_set = compo_picturebox_create(frm, UI_BUF_IMAGE_BIN_SET_BIN);
    compo_picturebox_set_pos(inf->pic_set, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 24);

    inf->txt_set = compo_textbox_create(frm, 16);
    compo_textbox_set_location(inf->txt_set, GUI_SCREEN_CENTER_X - 78, GUI_SCREEN_CENTER_Y + 24, 0, 0);
    compo_textbox_set_autosize(inf->txt_set, true);
    compo_textbox_set_align_center(inf->txt_set, true);
    compo_textbox_set(inf->txt_set, i18n[STR_SETUP]);

    /* 初始选中加热 */
    compo_textbox_set_forecolor(inf->txt_heat, COLOR_BLACK);
    compo_textbox_set_forecolor(inf->txt_mode, COLOR_BLACK);
    compo_textbox_set_forecolor(inf->txt_set, COLOR_WHITE);

    return frm;
}

static void func_home_page_process(void)
{
    func_process();
}

static void func_home_page_message(size_msg_t msg)
{
    f_home_t *inf = (f_home_t *)func_cb.f_cb;

    switch (msg)
    {
    case MSG_QDEC_FORWARD:
    case MSG_CTP_SHORT_DOWN:
        if (inf->selection != 1)
        {
            inf->selection = 1;
            home_update_display();
        }
        break;

    case MSG_QDEC_BACKWARD:
    case MSG_CTP_SHORT_UP:
        if (inf->selection != 0)
        {
            inf->selection = 0;
            home_update_display();
        }
        break;

    case KU_BACK:
    case MSG_CTP_CLICK:
        func_cb.sta = (inf->selection == 0) ? FUNC_SELECT_BACKUP : FUNC_SETTING;
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_home_page_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_home_t));
    func_cb.frm_main = func_home_page_form_create();
}

void func_home_page_exit(void)
{
    f_home_t *inf = (f_home_t *)func_cb.f_cb;
    func_cb.last = (inf->selection == 0) ? FUNC_SELECT_BACKUP : FUNC_SETTING;
}

void func_home_page(void)
{
    printf("%s\n", __func__);
    func_home_page_enter();
    while (func_cb.sta == FUNC_HOME_PAGE)
    {
        func_home_page_process();
        func_home_page_message(msg_dequeue());
    }
    func_home_page_exit();
}
