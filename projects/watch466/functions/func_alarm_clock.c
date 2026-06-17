#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define ACLOCK_LIST_CNT                       ((int)(sizeof(tbl_aclock_list) / sizeof(tbl_aclock_list[0])))

//组件ID
enum {
    //列表
    COMPO_ID_LISTBOX = 1,

    //按键
	COMPO_ID_BTN_ADD,

    //图像
	//COMPO_ID_PIC_ADD_CLICK,

};

typedef struct f_alarm_clock_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;
} f_alarm_clock_t;

static const compo_listbox_item_t tbl_aclock_list[] = {
    {STR_ALARM_CLOCK1, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK1_ON},
    {STR_ALARM_CLOCK2, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK2_ON},
    {STR_ALARM_CLOCK3, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK3_ON},
};
compo_listbox_custom_item_t tbl_aclock_txt_list[ALARM_CLOCK_NUM_MAX] = {0};

//创建闹钟窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_alarm_clock_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_ALARM_CLOCK]);

    if (sys_cb.alarm_enable_cnt != 0) {
        //新建列表
        compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE_NORMAL);//COMPO_LISTBOX_STYLE_TITLE_NORMAL
        compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 90, 60);
        compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
        compo_listbox_set_rect_visible(listbox, true);
        compo_listbox_set(listbox, tbl_aclock_list, sys_cb.alarm_enable_cnt);
        compo_setid(listbox, COMPO_ID_LISTBOX);
        //compo_listbox_set_sta_icon(listbox, UI_BUF_ALARM_CLOCK_SELECT1_ON_BIN, UI_BUF_ALARM_CLOCK_SELECT1_BIN);
        compo_listbox_set_bithook(listbox, bsp_sys_get_ctlbit);
        if (sys_cb.alarm_enable_cnt == 1) {
            compo_listbox_set_location(listbox, GUI_SCREEN_CENTER_X, 120, GUI_SCREEN_WIDTH, 200);
            compo_listbox_set_focus(listbox, 40);
        } else if (sys_cb.alarm_enable_cnt == 2) {
            compo_listbox_set_location(listbox, GUI_SCREEN_CENTER_X, 140, GUI_SCREEN_WIDTH, 300);
            compo_listbox_set_focus_byidx(listbox, 0);
        } else {
            compo_listbox_set_location(listbox, GUI_SCREEN_CENTER_X, 185, GUI_SCREEN_WIDTH, 200);
            compo_listbox_set_focus(listbox, 165);
        }
        u8 hour, min;
        for (int i=0; i<sys_cb.alarm_enable_cnt; i++) {
            hour = sys_cb.alarm_total_sec[i] / 3600;
            min = (sys_cb.alarm_total_sec[i] % 3600) / 60;
            sprintf(tbl_aclock_txt_list[i].str_txt, "%s%d(%02d:%02d)", i18n[STR_ALARM], i+1, hour, min);
            printf("%s\n",tbl_aclock_txt_list[i].str_txt);
        }
        compo_listbox_set_text_modify(listbox, tbl_aclock_txt_list);
        compo_listbox_update(listbox);

    } else {
        //创建文本
        // compo_form_add_image(frm, UI_BUF_COMMON_BG_BIN, GUI_GET_SCREEN_CENTER_X, 140);
        // compo_textbox_t *txt_null = compo_textbox_create(frm, 10);
        // compo_textbox_set_pos(txt_null, GUI_GET_SCREEN_CENTER_X, 140);
        // compo_textbox_set(txt_null, i18n[STR_NO_ALARM]);
    }


	//新建按钮
	compo_button_t *btn;
    //btn = compo_button_create_by_image(frm, UI_BUF_ALARM_CLOCK_ADD_BIN);
    // compo_setid(btn, COMPO_ID_BTN_ADD);
    // compo_button_set_pos(btn, GUI_GET_SCREEN_CENTER_X, GUI_GET_SCREEN_CENTER_Y + GUI_GET_SCREEN_HEIGHT/4);

    return frm;
}

//触摸按钮效果处理
static void func_alarm_clock_button_touch_handle(void)
{

    int id = compo_get_button_id();
    compo_button_t *btn = compo_getobj_byid(COMPO_ID_BTN_ADD);


    switch (id) {
    case COMPO_ID_BTN_ADD:

        compo_button_set_alpha(btn, UI_BTN_CLICK_EFFECT_ALPHA);
        break;

    default:
        break;
    }

}

//释放按钮效果处理
static void func_alarm_clock_button_release_handle(void)
{
    compo_button_t *btn = compo_getobj_byid(COMPO_ID_BTN_ADD);

    compo_button_set_alpha(btn, UI_BTN_NORMAL_EFFECT_ALPHA);
}

//单击按钮
static void func_alarm_clock_button_click(void)
{
    int id = compo_get_button_id();
    f_alarm_clock_t *f_aclock = (f_alarm_clock_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_aclock->listbox;
    printf("===>id [%d]\n", id);
    if (sys_cb.alarm_enable_cnt != 0) {
        int aclock_sel = compo_listbox_select(listbox, ctp_get_sxy());
        if (aclock_sel >= 0) {
           printf("===>aclock_sel:%d\n",aclock_sel);
            if (compo_listbox_is_sel_sta(listbox)) {
                bsp_sys_reverse_ctlbit(tbl_aclock_list[aclock_sel].vidx);
                compo_listbox_update(listbox);

                sys_cb.alarm_enable_sel ^= BIT(aclock_sel);
               printf("===>alarm_enable_sel:%d\n",sys_cb.alarm_enable_sel);
            } else {
                sys_cb.alarm_idx = aclock_sel;
                func_cb.sta = FUNC_ALARM_CLOCK_SUB_EDIT;
            }
        }
    }

    switch (id) {
    case COMPO_ID_BTN_ADD:
        if (sys_cb.alarm_enable_cnt == ALARM_CLOCK_NUM_MAX) {
            break;
        }
        sys_cb.alarm_idx = sys_cb.alarm_enable_cnt;         //最后一个闹钟idx
        func_cb.sta = FUNC_ALARM_CLOCK_SUB_SET;
        break;

    default:
        break;
    }

    func_alarm_clock_button_release_handle();
}

//闹钟功能事件处理
static void func_alarm_clock_process(void)
{
    f_alarm_clock_t *f_aclock = (f_alarm_clock_t *)func_cb.f_cb;
    if (sys_cb.alarm_enable_cnt != 0) {
        compo_listbox_move(f_aclock->listbox);
    }
    func_process();
}


//闹钟功能消息处理
static void func_alarm_clock_message(size_msg_t msg)
{
    f_alarm_clock_t *f_aclock = (f_alarm_clock_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_aclock->listbox;

    if (sys_cb.alarm_enable_cnt != 0 && compo_listbox_message(listbox, msg)) {
        func_alarm_clock_button_release_handle();
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_TOUCH:
        func_alarm_clock_button_touch_handle();
        break;

	case MSG_CTP_CLICK:
        func_alarm_clock_button_click();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_LONG:
        func_alarm_clock_button_release_handle();
        if (func_cb.flag_sort) {
            func_message(msg);
        }
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_alarm_clock_button_release_handle();
        func_message(msg);
        break;

    default:
        func_message(msg);
        break;
    }

}

//进入闹钟功能
void func_alarm_clock_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_alarm_clock_t));
    func_cb.frm_main = func_alarm_clock_form_create();

    if (sys_cb.alarm_enable_cnt != 0) {
        f_alarm_clock_t *f_aclock = (f_alarm_clock_t *)func_cb.f_cb;
        f_aclock->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
        compo_listbox_t *listbox = f_aclock->listbox;
        if (listbox->type != COMPO_TYPE_LISTBOX) {
            halt(HALT_GUI_COMPO_LISTBOX_TYPE);
        }
        listbox->mcb = &f_aclock->mcb;        //建立移动控制块，退出时需要释放
        s32 first_y = 0;
        s32 last_y = 0;
        if (sys_cb.alarm_enable_cnt == 1) {
            first_y = compo_listbox_gety_byidx(listbox, 0);
            last_y = compo_listbox_gety_byidx(listbox, 0);

        } else if (sys_cb.alarm_enable_cnt == 2) {
            first_y = compo_listbox_gety_byidx(listbox, 0);
            last_y = compo_listbox_gety_byidx(listbox, 0);
        } else {
            first_y = 165;
            last_y = compo_listbox_gety_byidx(listbox, ACLOCK_LIST_CNT - 1);
        }
        compo_listbox_move_init_modify(listbox, first_y, last_y);
        func_cb.enter_tick = tick_get();
    }
}

//退出闹钟功能
void func_alarm_clock_exit(void)
{
    func_cb.last = FUNC_ALARM_CLOCK;
}

//闹钟功能
void func_alarm_clock(void)
{
    printf("%s\n", __func__);
    func_alarm_clock_enter();
    while (func_cb.sta == FUNC_ALARM_CLOCK) {
        func_alarm_clock_process();
        func_alarm_clock_message(msg_dequeue());
    }
    func_alarm_clock_exit();
}
