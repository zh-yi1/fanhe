#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define WEEKS_LIST_CNT                       ((int)(sizeof(tbl_weeks_list) / sizeof(tbl_weeks_list[0])))

//组件ID
enum {
    //列表
    COMPO_ID_LISTBOX = 1,

    //按键
	COMPO_ID_BTN_REPETAT_NO,
	COMPO_ID_BTN_REPETAT_YES,

    //图像
	COMPO_ID_PIC_REPETAT_NO_CLICK,
	COMPO_ID_PIC_REPETAT_YES_CLICK,

};

typedef struct f_alarm_clock_sub_repeat_t_ {
    compo_listbox_t *listbox;
    compo_listbox_move_cb_t mcb;
} f_alarm_clock_sub_repeat_t;

static const compo_listbox_item_t tbl_weeks_list[] = {
    {STR_ALARM_CLOCK_MON, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK_MON},
    {STR_ALARM_CLOCK_TUE, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK_TUE},
    {STR_ALARM_CLOCK_WED, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK_WED},
    {STR_ALARM_CLOCK_THU, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK_THU},
    {STR_ALARM_CLOCK_FRI, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK_FRI},
    {STR_ALARM_CLOCK_SAT, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK_SAT},
    {STR_ALARM_CLOCK_SUN, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_ACLOCK_SUN},
};

//创建闹钟--重复窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_alarm_clock_sub_repeat_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_ALARM_CLOCK_REPEAT]);

    //新建列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE_NORMAL);
    compo_listbox_set(listbox, tbl_weeks_list, WEEKS_LIST_CNT);
    compo_listbox_set_location(listbox, 160+50, 165, 320+100, 200);
    compo_listbox_set_item_height(listbox, 60);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_sta_icon(listbox, UI_BUF_COMMON_SELECT_YES_BIN, UI_BUF_COMMON_SELECT_NO_BIN); //UI_BUF_COMMON_SELECT_ON_BIN  //UI_BUF_COMMON_SELECT_BIN
    compo_listbox_set_bithook(listbox, bsp_sys_get_ctlbit);
    compo_listbox_set_focus_byidx(listbox, 2);

    for (int i=0; i<7; i++) {       //获取每个闹钟设置的星期
        bsp_sys_set_ctlbit(SYS_CTL_ACLOCK_MON + i, (sys_cb.alarm_week_sel[sys_cb.alarm_idx] >> i) & 1);
    }

    compo_listbox_update(listbox);

	//新建按钮
	compo_button_t *btn;
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_NO_BIN);//UI_BUF_COMMON_NO_BIN
    compo_setid(btn, COMPO_ID_BTN_REPETAT_NO);
    compo_button_set_pos(btn, 87+60, 321);

    btn = compo_button_create_by_image(frm, UI_BUF_ALARM_CLOCK_YES_BIN);
    compo_setid(btn, COMPO_ID_BTN_REPETAT_YES);
    compo_button_set_pos(btn, 233+60, 321);

    //新建图像
   compo_picturebox_t *pic_click = compo_picturebox_create(frm, UI_BUF_COMMON_SELECT_YES_BIN);
   compo_setid(pic_click, COMPO_ID_PIC_REPETAT_NO_CLICK);
   compo_picturebox_set_pos(pic_click, 87, 321);
   compo_picturebox_set_visible(pic_click, false);

   pic_click = compo_picturebox_create(frm, UI_BUF_COMMON_SELECT_NO_BIN);
   compo_setid(pic_click, COMPO_ID_PIC_REPETAT_YES_CLICK);
   compo_picturebox_set_pos(pic_click, 233, 321);
   compo_picturebox_set_visible(pic_click, false);
    return frm;
}

//触摸按钮效果处理
static void func_alarm_clock_sub_repeat_button_touch_handle(void)
{

    int id = compo_get_button_id();

    compo_button_t *btn_no = compo_getobj_byid(COMPO_ID_BTN_REPETAT_NO);
    compo_button_t *btn_yes = compo_getobj_byid(COMPO_ID_BTN_REPETAT_YES);


    switch (id) {
    case COMPO_ID_BTN_REPETAT_NO:

        compo_button_set_alpha(btn_no, UI_BTN_CLICK_EFFECT_ALPHA);
        break;
    case COMPO_ID_BTN_REPETAT_YES:

        compo_button_set_alpha(btn_yes, UI_BTN_CLICK_EFFECT_ALPHA);
        break;

    default:
        break;
    }

}

//释放按钮效果处理
static void func_alarm_clock_sub_repeat_button_release_handle(void)
{

    compo_button_t *btn_no = compo_getobj_byid(COMPO_ID_BTN_REPETAT_NO);
    compo_button_t *btn_yes = compo_getobj_byid(COMPO_ID_BTN_REPETAT_YES);

    compo_button_set_alpha(btn_no, UI_BTN_NORMAL_EFFECT_ALPHA);
    compo_button_set_alpha(btn_yes, UI_BTN_NORMAL_EFFECT_ALPHA);
}

//单击按钮
static void func_alarm_clock_sub_repeat_button_click(void)
{
    int id = compo_get_button_id();

    int week_idx;
    f_alarm_clock_sub_repeat_t *f_aclock = (f_alarm_clock_sub_repeat_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_aclock->listbox;

    week_idx = compo_listbox_select(listbox, ctp_get_sxy());
    if (week_idx >= 0) {
        bsp_sys_reverse_ctlbit(tbl_weeks_list[week_idx].vidx);
        compo_listbox_update(listbox);
    }

    switch (id) {
    case COMPO_ID_BTN_REPETAT_NO:
        task_stack_pop();
        if (func_cb.last == FUNC_ALARM_CLOCK_SUB_EDIT) {
            func_cb.sta = FUNC_ALARM_CLOCK_SUB_EDIT;
            break;
        }
        sys_cb.alarm_week_sel[sys_cb.alarm_idx] = 0;
        func_cb.sta = FUNC_ALARM_CLOCK_SUB_SET;
        break;

    case COMPO_ID_BTN_REPETAT_YES:
        for (int i=0; i<7; i++) {
            sys_cb.alarm_week_sel[sys_cb.alarm_idx] &= ~BIT(i);
            sys_cb.alarm_week_sel[sys_cb.alarm_idx] |= (bsp_sys_get_ctlbit(SYS_CTL_ACLOCK_MON + i) << i);
        }
//        printf("%s-->alarm_week_sel[%d]:%x\n", __func__, sys_cb.alarm_idx, sys_cb.alarm_week_sel[sys_cb.alarm_idx]);
        if (func_cb.last == FUNC_ALARM_CLOCK_SUB_EDIT) {
            func_cb.sta = FUNC_ALARM_CLOCK_SUB_EDIT;
            task_stack_pop();
            break;
        }

        sys_cb.alarm_enable_cnt++;
        sys_cb.alarm_idx++;

        func_cb.sta = FUNC_ALARM_CLOCK;
        task_stack_pop();
        task_stack_pop();        //回退两个页面，出栈两次
        break;

    default:
        break;
    }

    func_alarm_clock_sub_repeat_button_release_handle();
}

//闹钟--重复功能事件处理
static void func_alarm_clock_sub_repeat_process(void)
{
    f_alarm_clock_sub_repeat_t *f_aclock = (f_alarm_clock_sub_repeat_t *)func_cb.f_cb;
    compo_listbox_move(f_aclock->listbox);
    func_process();
}


//闹钟--重复功能消息处理
static void func_alarm_clock_sub_repeat_message(size_msg_t msg)
{
    f_alarm_clock_sub_repeat_t *f_aclock = (f_alarm_clock_sub_repeat_t *)func_cb.f_cb;
    compo_listbox_t *listbox = f_aclock->listbox;

    if (compo_listbox_message(listbox, msg)) {
        func_alarm_clock_sub_repeat_button_release_handle();
        return;                                         //处理列表框信息
    }
    switch (msg) {
    case MSG_CTP_TOUCH:
        func_alarm_clock_sub_repeat_button_touch_handle();
        break;

	case MSG_CTP_CLICK:
        func_alarm_clock_sub_repeat_button_click();
        break;

    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_LONG:
        func_alarm_clock_sub_repeat_button_release_handle();
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_alarm_clock_sub_repeat_button_release_handle();
        func_message(msg);
        break;



    default:
        func_message(msg);
        break;
    }
}

//进入闹钟--重复功能
void func_alarm_clock_sub_repeat_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_alarm_clock_sub_repeat_t));
    func_cb.frm_main = func_alarm_clock_sub_repeat_form_create();

    f_alarm_clock_sub_repeat_t *f_aclock = (f_alarm_clock_sub_repeat_t *)func_cb.f_cb;
    f_aclock->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t *listbox = f_aclock->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_aclock->mcb;        //建立移动控制块，退出时需要释放

    s32 first_y = compo_listbox_gety_byidx(listbox, 2);
    s32 last_y = compo_listbox_gety_byidx(listbox, WEEKS_LIST_CNT - 1);
    compo_listbox_move_init_modify(listbox, first_y, last_y);

    func_cb.enter_tick = tick_get();

}

//退出闹钟--重复功能
void func_alarm_clock_sub_repeat_exit(void)
{
    func_cb.last = FUNC_ALARM_CLOCK_SUB_REPEAT;
}

//闹钟--重复功能
void func_alarm_clock_sub_repeat(void)
{
    printf("%s\n", __func__);
    func_alarm_clock_sub_repeat_enter();
    while (func_cb.sta == FUNC_ALARM_CLOCK_SUB_REPEAT) {
        func_alarm_clock_sub_repeat_process();
        func_alarm_clock_sub_repeat_message(msg_dequeue());
    }
    func_alarm_clock_sub_repeat_exit();
}
