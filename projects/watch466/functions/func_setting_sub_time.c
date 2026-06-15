#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_time_t_ {
    page_tp_move_t *ptm;
    bool flag_drag;                 //开始拖动
    s32 x_pos;
    s32 y_pos;
    s32 x;
    s32 y;
} f_time_t;

enum{
    //按键
    COMPO_ID_BIN_CUSTOM = 1,
    //按键
    COMPO_ID_BIN_CALENDER,
    COMPO_ID_BIN_TIME,
    COMPO_ID_BIN_DATA,
    //文本
    COMPO_ID_TEXT_CALENDE,
    COMPO_ID_TEXT_TIME,
    COMPO_ID_TEXT_DATA,
};

#define DISURD_DISP_BTN_ITEM_CNT    ((int)(sizeof(tbl_disturd_disp_btn_item) / sizeof(tbl_disturd_disp_btn_item[0])))

typedef struct disturd_disp_btn_item_t_ {
    u16 btn_id;
    s16 x;
    s16 y;
    s16 h;
    s16 l;
} disturd_disp_btn_item_t;

//按钮item，创建时遍历一下
static const disturd_disp_btn_item_t tbl_disturd_disp_btn_item[] = {
    {COMPO_ID_BIN_CALENDER,        GUI_SCREEN_CENTER_X,     125},
    {COMPO_ID_BIN_TIME,            GUI_SCREEN_CENTER_X,     225},
    {COMPO_ID_BIN_DATA,            GUI_SCREEN_CENTER_X,     325},
};

//时间设置页面
compo_form_t *func_set_sub_time_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    // compo_form_set_title_center(frm, true);
    // compo_form_set_title(frm, i18n[STR_SETTING_TIME]);

    // //创建按钮
    // compo_button_t *btn;
    // compo_picturebox_t *pic[3];
    // for (u8 idx_btn = 0; idx_btn < DISURD_DISP_BTN_ITEM_CNT; idx_btn++) {
    //     btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BG3_BIN);
    //     compo_setid(btn, tbl_disturd_disp_btn_item[idx_btn].btn_id);
    //     compo_button_set_pos(btn, tbl_disturd_disp_btn_item[idx_btn].x, tbl_disturd_disp_btn_item[idx_btn].y);
    //     //pic[idx_btn] = compo_picturebox_create(frm, UI_BUF_SETTING_PASSWORD_OPEN_BIN);
    //     compo_picturebox_set_pos(pic[idx_btn], tbl_disturd_disp_btn_item[idx_btn].x+150, tbl_disturd_disp_btn_item[idx_btn].y);
    //     compo_picturebox_set_visible(pic[idx_btn], true);
    // }

    // //文本
    // s16 txt_x_pos = 90;
    // s16 txt_y_pos = 90;
    // char buf[84];
    // component_t *compo = (component_t *)compo_textbox_create(frm, 20);
    // compo_setid(compo, COMPO_ID_TEXT_CALENDE);
    // compo_textbox_set_align_center((compo_textbox_t *)compo, false);
    // compo_textbox_set_pos((compo_textbox_t *)compo, txt_x_pos, txt_y_pos);
    // compo_textbox_set((compo_textbox_t *)compo, i18n[STR_CUSTOM_CALENDAR]);

    // component_t *compo_tws = (component_t *)compo_textbox_create(frm,20);
    // compo_setid(compo_tws, COMPO_ID_TEXT_CALENDE);
    // compo_textbox_set_align_center((compo_textbox_t *)compo_tws, false);
    // compo_textbox_set_pos((compo_textbox_t *)compo_tws, txt_x_pos, txt_y_pos+100);
    // compo_textbox_set((compo_textbox_t *)compo_tws, i18n[STR_CUSTOM_DATA]);

    // component_t *compo_thr = (component_t *)compo_textbox_create(frm,20);
    // compo_setid(compo_thr, COMPO_ID_TEXT_CALENDE);
    // compo_textbox_set_align_center((compo_textbox_t *)compo_thr, false);
    // compo_textbox_set_pos((compo_textbox_t *)compo_thr, txt_x_pos, txt_y_pos+200);
    // compo_textbox_set((compo_textbox_t *)compo_thr, i18n[STR_CUSTOM_TIME]);

    // snprintf(buf, sizeof(buf), "%d%s%s", 2026, ".01", ".26");
    // component_t *compo_four = (component_t *)compo_textbox_create(frm,20);
    // compo_textbox_set_font((compo_textbox_t *)compo_four, UI_BUF_0FONT_FONT_ASC_BIN);
    // compo_setid(compo_four, COMPO_ID_TEXT_CALENDE);
    // compo_textbox_set_align_center((compo_textbox_t *)compo_four, false);
    // compo_textbox_set_pos((compo_textbox_t *)compo_four, txt_x_pos, txt_y_pos+37);
    // compo_textbox_set((compo_textbox_t *)compo_four, buf);

    // snprintf(buf, sizeof(buf), "%d", 24);
    // component_t *compo_five = (component_t *)compo_textbox_create(frm,20);
    // compo_textbox_set_font((compo_textbox_t *)compo_five, UI_BUF_0FONT_FONT_ASC_BIN);
    // compo_setid(compo_five, COMPO_ID_TEXT_CALENDE);
    // compo_textbox_set_align_center((compo_textbox_t *)compo_five, false);
    // compo_textbox_set_pos((compo_textbox_t *)compo_five, txt_x_pos, txt_y_pos+137);
    // compo_textbox_set((compo_textbox_t *)compo_five, buf);

    // snprintf(buf, sizeof(buf), "%s", "12.20");
    // component_t *compo_six = (component_t *)compo_textbox_create(frm,20);
    // compo_textbox_set_font((compo_textbox_t *)compo_six, UI_BUF_0FONT_FONT_ASC_BIN);
    // compo_setid(compo_six, COMPO_ID_TEXT_CALENDE);
    // compo_textbox_set_align_center((compo_textbox_t *)compo_six, false);
    // compo_textbox_set_pos((compo_textbox_t *)compo_six, txt_x_pos, txt_y_pos+237);
    // compo_textbox_set((compo_textbox_t *)compo_six, buf);

    // // snprintf(buf, sizeof(buf), "%s", i18n[STR_TIME_TIP]);
    // component_t *compo_sev = (component_t *)compo_textbox_create(frm, 130);
    // compo_textbox_set_multiline((compo_textbox_t *)compo_sev, true);
    // compo_textbox_set_autosize((compo_textbox_t *)compo_sev, false);
    // compo_textbox_set_location((compo_textbox_t *)compo_sev, GUI_SCREEN_CENTER_X, txt_y_pos+470, 300, GUI_SCREEN_HEIGHT);
    // compo_textbox_set((compo_textbox_t *)compo_sev, i18n[STR_TIME_TIP]);
    // // widget_page_set_client(frm->page_body, 0, -60);
    return frm;
}

//时间设置事件处理
static void func_set_sub_time_process(void)
{
//    s32 h_x = 0, h_y = 0;
    f_time_t *slp = (f_time_t *)func_cb.f_cb;
//    widget_page_t *page_body = func_cb.frm_main->page_body;
//
//    if(slp->flag_drag) {
//        slp->flag_drag = ctp_get_dxy(&h_x, &h_y);
//         if (slp->flag_drag ) {
//             slp->y = slp->y_pos + h_y;
//             widget_page_set_client(page_body, 0, slp->y);
//         } else {
//              slp->y_pos = slp->y;
//         }
//    }

    compo_page_move_process(slp->ptm);
    func_process();
}

//单击按钮
static void func_time_button_click(void)
{
    int id = compo_get_button_id();

    switch (id) {
    case COMPO_ID_BIN_CUSTOM:
        func_cb.sta = FUNC_TIME_SUB_CUSTOM;
        break;

    default:
        break;
    }
}

//时间设置功能消息处理
static void func_set_sub_time_message(size_msg_t msg)
{
    f_time_t *slp = (f_time_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_TOUCH:
        compo_page_move_touch_handler(slp->ptm);
        break;
    case MSG_CTP_CLICK:
        func_time_button_click();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
        slp->flag_drag = true;

    case KU_DELAY_BACK:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入时间设置功能
void func_set_sub_time_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_time_t));
    func_cb.frm_main = func_set_sub_time_form_create();
    f_time_t *slp = (f_time_t *)func_cb.f_cb;
    slp->ptm = (page_tp_move_t *)func_zalloc(sizeof(page_tp_move_t));
    page_move_info_t info = {
        .title_used = true,
        .page_size = GUI_SCREEN_HEIGHT + (GUI_SCREEN_HEIGHT / 2),
        .page_count = 1,
        .jump_perc = 20,
        .quick_jump_perc = 100,
        .up_over_perc = 10,
        .down_over_perc = 10,
    };
    compo_page_move_init(slp->ptm, func_cb.frm_main->page_body, &info);
    slp->y_pos = -60;
}

//退出时间设置功能
void func_set_sub_time_exit(void)
{
    f_time_t *slp = (f_time_t *)func_cb.f_cb;
    if (slp->ptm) {
        func_free(slp->ptm);
    }
    func_cb.last = FUNC_SET_SUB_TIME;
}

//时间设置功能
void func_set_sub_time(void)
{
    printf("%s\n", __func__);
    func_set_sub_time_enter();
    while (func_cb.sta == FUNC_SET_SUB_TIME) {
        func_set_sub_time_process();
        func_set_sub_time_message(msg_dequeue());
    }
    func_set_sub_time_exit();
}
