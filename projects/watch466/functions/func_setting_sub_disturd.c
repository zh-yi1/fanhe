#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_disturd_t_ {

} f_disturd_t;

enum{
    //数字
    COMPO_ID_NUM_DISP_START = 1,
    COMPO_ID_NUM_DISP_END,
    //按钮
    COMPO_ID_BIN_TIMING_ON,
    COMPO_ID_BIN_ALLDAY_ON,
    COMPO_ID_BIN_START_TIME,
    COMPO_ID_BIN_END_TIME,
    //文字
    COMPO_ID_TXT_ALL,
    COMPO_ID_TXT_TIM,
    COMPO_ID_TXT_START,
    COMPO_ID_TXT_END,
};

typedef struct disturd_disp_pic_item_t_ {
    u32 res_addr;
    u16 pic_id;
    s16 x;
    s16 y;
    bool visible_en;
} disturd_disp_pic_item_t;

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
    {COMPO_ID_BIN_ALLDAY_ON,        388,     156                                },
    {COMPO_ID_BIN_TIMING_ON,        388,     248                                },
    {COMPO_ID_BIN_START_TIME,       110,     370,   GUI_SCREEN_WIDTH/2,    120  },
    {COMPO_ID_BIN_END_TIME,         320,     370,   GUI_SCREEN_WIDTH/2,    120  },
};

typedef struct disturd_num_item_t_ {
    u32 res_addr;
    int num_cnt;
    u16 num_id;
    int val;
    s16 x;
    s16 y;
    bool visible_en;
} disturd_num_item_t;

#define DISTURD_NUM_ITEM_CNT                       ((int)(sizeof(tbl_disturd_num_item) / sizeof(tbl_disturd_num_item[0])))

//搞个数字item，创建时遍历一下
static const disturd_num_item_t tbl_disturd_num_item[] = {
    /*   res_addr,                        num_cnt,   num_id,                        val,   x,      y,     visible_en*/
    {UI_BUF_0FONT_FONT_NUM_24_BIN,          5,       COMPO_ID_NUM_DISP_START,        0,    125,    390,   false},
    {UI_BUF_0FONT_FONT_NUM_24_BIN,          5,       COMPO_ID_NUM_DISP_END,          0,    345,    390,   false},
};

#define DISURD_DISP_TXT_ITEM_CNT    ((int)(sizeof(disturd_disp_txt_item) / sizeof(disturd_disp_txt_item[0])))

typedef struct disturd_disp_txt_item_t_ {
    u8 str_id;
    u16 btn_id;
    s16 x;
    s16 y;
    bool visible_en;
} disturd_disp_txt_item_t;

//文字item，创建时遍历一下
static const disturd_disp_txt_item_t disturd_disp_txt_item[] = {
    {STR_DISTURD_TIM_START,     COMPO_ID_TXT_START,          40,     310,    false},
    {STR_DISTURD_TIM_END,       COMPO_ID_TXT_END,            260,    310,    false},
    {STR_DISTURD_ALL,           COMPO_ID_TXT_ALL,            60,     135,    true},
    {STR_DISTURD_TIM,           COMPO_ID_TXT_TIM,            60,     225,    true},
};

//勿扰模式页面
compo_form_t *func_set_sub_disturd_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    // //设置标题栏
    // compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    // compo_form_set_title_center(frm, true);
    // compo_form_set_title(frm, i18n[STR_SETTING_DISTURD]);

    // //创建按钮
    // compo_button_t *btn;
    // for (u8 idx_btn = 0; idx_btn < DISURD_DISP_BTN_ITEM_CNT; idx_btn++) {
    //     if (idx_btn < 2) {
    //         btn = compo_button_create_by_image(frm, UI_BUF_COMMON_SWITCH_BUTTON_OFF_BIN);
    //         compo_setid(btn, tbl_disturd_disp_btn_item[idx_btn].btn_id);
    //         compo_button_set_pos(btn, tbl_disturd_disp_btn_item[idx_btn].x, tbl_disturd_disp_btn_item[idx_btn].y);
    //     } else {
    //         btn = compo_button_create(frm);
    //         compo_setid(btn, tbl_disturd_disp_btn_item[idx_btn].btn_id);
    //         compo_button_set_location(btn, tbl_disturd_disp_btn_item[idx_btn].x, tbl_disturd_disp_btn_item[idx_btn].y, tbl_disturd_disp_btn_item[idx_btn].h, tbl_disturd_disp_btn_item[idx_btn].l);

    //         // compo_shape_t *shape_bg = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    //         // compo_shape_set_color(shape_bg, COLOR_BLUE);
    //         // compo_shape_set_location(shape_bg, tbl_disturd_disp_btn_item[idx_btn].x, tbl_disturd_disp_btn_item[idx_btn].y, tbl_disturd_disp_btn_item[idx_btn].h, tbl_disturd_disp_btn_item[idx_btn].l);
    //     }
    // }

    // //创建文本
    // compo_textbox_t *textbox;
    // for (int i=0; i < DISURD_DISP_TXT_ITEM_CNT; i++) {
    //     textbox = compo_textbox_create(frm, 20);
    //     compo_setid(textbox, COMPO_ID_TXT_ALL + i);
    //     compo_textbox_set_pos(textbox, disturd_disp_txt_item[i].x, disturd_disp_txt_item[i].y);
    //     compo_textbox_set_align_center(textbox, false);
    //     compo_textbox_set(textbox, i18n[disturd_disp_txt_item[i].str_id]);
    //     compo_textbox_set_visible(textbox, disturd_disp_txt_item[i].visible_en);
    // }

    // //获取显示时间
    // u32 hour_start = sys_cb.disturd_start_time_sec / 3600;
    // u32 min_start  = (sys_cb.disturd_start_time_sec % 3600) / 60;
    // u32 hour_end   = sys_cb.disturd_end_time_sec / 3600;
    // u32 min_end  = (sys_cb.disturd_end_time_sec % 3600) / 60;

    // //创建数字
    // compo_textbox_t *num_txt;
    // char start_time_buff[12];
    // char end_time_buff[12];
    // for (u8 idx = 0; idx < DISTURD_NUM_ITEM_CNT; idx++) {
    //     num_txt = compo_textbox_create(frm, tbl_disturd_num_item[idx].num_cnt);
    //     compo_textbox_set_font(num_txt, tbl_disturd_num_item[idx].res_addr);
    //     compo_setid(num_txt, tbl_disturd_num_item[idx].num_id);
    //     compo_textbox_set_pos(num_txt, tbl_disturd_num_item[idx].x, tbl_disturd_num_item[idx].y);
    //     compo_textbox_set_visible(num_txt, tbl_disturd_num_item[idx].visible_en);

    //     if (tbl_disturd_num_item[idx].num_id == COMPO_ID_NUM_DISP_START) {
    //         snprintf(start_time_buff, sizeof(start_time_buff), "%02ld:%02ld", hour_start, min_start);
    //         compo_textbox_set(num_txt, start_time_buff);
    //     } else if (tbl_disturd_num_item[idx].num_id == COMPO_ID_NUM_DISP_END) {
    //         snprintf(end_time_buff, sizeof(end_time_buff), "%02ld:%02ld", hour_end, min_end);
    //         compo_textbox_set(num_txt, end_time_buff);
    //     }
    // }

    return frm;
}

//勿扰模式事件处理
static void func_set_sub_disturd_process(void)
{
    func_process();
}

//更新显示勿扰模式界面
static void func_set_sub_disturd_disp(void)
{
    // //获取按钮组件的地址
    // compo_button_t *btn_allday  = compo_getobj_byid(COMPO_ID_BIN_ALLDAY_ON);
    // compo_button_t *btn_timing  = compo_getobj_byid(COMPO_ID_BIN_TIMING_ON);
    // compo_button_t *btn_start_time  = compo_getobj_byid(COMPO_ID_BIN_START_TIME);
    // compo_button_t *btn_end_time  = compo_getobj_byid(COMPO_ID_BIN_END_TIME);
    // //获取文本组件的地址
    // compo_textbox_t *txt_disp[DISURD_DISP_TXT_ITEM_CNT - 2];
    // compo_textbox_t *num_disp[DISTURD_NUM_ITEM_CNT];

    // for (int i=0; i<DISURD_DISP_TXT_ITEM_CNT - 2; i++) {
    //     txt_disp[i] = compo_getobj_byid(COMPO_ID_TXT_ALL + i);
    //     num_disp[i] = compo_getobj_byid(COMPO_ID_NUM_DISP_START + i);
    // }

    // //显示界面各个组件
    // if (sys_cb.disturd_adl) {
    //     compo_button_set_bgimg(btn_allday, UI_BUF_COMMON_SWITCH_BUTTON_ON_BIN);
    // } else {
    //     compo_button_set_bgimg(btn_allday, UI_BUF_COMMON_SWITCH_BUTTON_OFF_BIN);
    // }

    // if (sys_cb.disturd_tim == 0) {
    //     compo_button_set_bgimg(btn_timing, UI_BUF_COMMON_SWITCH_BUTTON_OFF_BIN);
    //     compo_button_set_visible(btn_start_time, false);
    //     compo_button_set_visible(btn_end_time, false);
    //     for (int i=0; i<DISURD_DISP_TXT_ITEM_CNT - 2; i++) {
    //         compo_textbox_set_visible(txt_disp[i], false);
    //         compo_textbox_set_visible(num_disp[i], false);
    //     }
    // } else {
    //     compo_button_set_bgimg(btn_timing, UI_BUF_COMMON_SWITCH_BUTTON_ON_BIN);
    //     compo_button_set_visible(btn_start_time, true);
    //     compo_button_set_visible(btn_end_time, true);
    //     for (int i=0; i<DISURD_DISP_TXT_ITEM_CNT - 2; i++) {
    //         compo_textbox_set_visible(txt_disp[i], true);
    //         compo_textbox_set_visible(num_disp[i], true);
    //     }
    // }
}

//单击按钮
static void func_disturd_button_click(void)
{
    u8 ret = 0;
    int id = compo_get_button_id();

    switch(id) {
    case COMPO_ID_BIN_ALLDAY_ON:
        if (!sys_cb.disturd_adl) {
            ret = msgbox((char *)i18n[STR_DISTURD_ALL], NULL , MSGBOX_MODE_BTN_YESNO, MSGBOX_MSG_TYPE_NONE);
            if (ret == MSGBOX_RES_YES) {
                sys_cb.disturd_adl = 1;
            }
        } else {
            ret = 0;
            sys_cb.disturd_adl = 0;
        }
        break;

    case COMPO_ID_BIN_TIMING_ON:
        if (!sys_cb.disturd_tim) {
            ret = msgbox((char *)i18n[STR_DISTURD_TIM], NULL , MSGBOX_MODE_BTN_YESNO, MSGBOX_MSG_TYPE_NONE);
            if (ret == MSGBOX_RES_YES) {
                sys_cb.disturd_tim = 1;
            }
        } else {
            ret = 0;
            sys_cb.disturd_tim = 0;
        }
        break;

    case COMPO_ID_BIN_START_TIME:
        if (sys_cb.disturd_tim) {
            sys_cb.disturd_sel = 0;
            func_cb.sta = FUNC_DISTURD_SUB_SET;
            task_stack_pop();
        }
        break;

    case COMPO_ID_BIN_END_TIME:
        if (sys_cb.disturd_tim) {
            sys_cb.disturd_sel = 1;
            func_cb.sta = FUNC_DISTURD_SUB_SET;
            task_stack_pop();
        }
        break;

    default:
    break;
    }

    func_set_sub_disturd_disp();
}

//勿扰模式能消息处理
static void func_set_sub_disturd_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_disturd_button_click();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入勿扰模式功能
void func_set_sub_disturd_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_disturd_t));
    func_cb.frm_main = func_set_sub_disturd_form_create();
    func_set_sub_disturd_disp();
}

//退出勿扰模式功能
void func_set_sub_disturd_exit(void)
{
    func_cb.last = FUNC_SET_SUB_DISTURD;
}

//勿扰模式功能
void func_set_sub_disturd(void)
{
    printf("%s\n", __func__);
    func_set_sub_disturd_enter();
    while (func_cb.sta == FUNC_SET_SUB_DISTURD) {
        func_set_sub_disturd_process();
        func_set_sub_disturd_message(msg_dequeue());
    }
    func_set_sub_disturd_exit();
}
