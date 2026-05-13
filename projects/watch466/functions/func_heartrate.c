#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FUNC_HR_SIMULATE_DATA_EN  0

//组件ID;
enum {
	COMPO_ID_NUM_HEARTRATE_MAX = 1,
	COMPO_ID_NUM_HEARTRATE_CUR,
	COMPO_ID_NUM_HEARTRATE_MIN,
	COMPO_ID_TEXT_HEARTRATE_TIME,
	COMPO_ID_CHART,
};

enum
{
    CHART_ANIM_STA_IDLE,
    CHART_ANIM_STA_START,
    CHART_ANIM_STA_END,
};

#define HEARTRATE_TEXT_ITEM_CNT	3

typedef struct f_heartrate_t_ {
	page_tp_move_t ptm;
	u8 last_hr;
	u8 max_hr;
	u8 min_hr;
} f_heartrate_t;

#if FUNC_HR_SIMULATE_DATA_EN
static const uint16_t total_hr_value[] = {10,20,30,40,50,60,70};
#endif


//创建心率窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_heartrate_form_create(void)
{
    //新建窗体
    component_t *compo;
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_HEART_RATE]);

    //设置内容
    compo_form_add_image(frm, UI_BUF_HEART_RATE_HR_BIN, GUI_SCREEN_CENTER_X, 145);
    compo_form_add_image(frm, UI_BUF_HEART_RATE_HR_BG_BIN, GUI_SCREEN_CENTER_X, 370);

	compo_form_add_image(frm, UI_BUF_HEART_RATE_UP_BIN, 141, 289);
    compo_form_add_image(frm, UI_BUF_HEART_RATE_DOWN_BIN, 400, 289);

	//创建文本
	compo_textbox_t *txt = compo_textbox_create(frm, 10);
	compo_textbox_set_pos(txt, GUI_SCREEN_CENTER_X, 289);
	compo_textbox_set(txt, i18n[STR_TIME_PER_MIN]);

	//测量心率值
    s16 txt_val_x[3] = {100, GUI_SCREEN_CENTER_X, 355};
    s16 txt_val_y[3] = {289, GUI_SCREEN_CENTER_Y, 289};
    char* txt_val_test[3] = {"90", "85", "80"};
    for (u8 idx = 0; idx < HEARTRATE_TEXT_ITEM_CNT; idx++) {
        compo_textbox_t *txt_val = compo_textbox_create(frm, 3);
        compo_textbox_set_font(txt_val, UI_BUF_0FONT_FONT_NUM_38_BIN);
        compo_setid(txt_val, COMPO_ID_NUM_HEARTRATE_MAX + idx);
        // compo_textbox_set_align_center(txt_val, false);
        compo_textbox_set_pos(txt_val, txt_val_x[idx], txt_val_y[idx]);
        compo_textbox_set(txt_val, txt_val_test[idx]);
    }

	//心率详情
	compo_form_add_image(frm, UI_BUF_HEART_RATE_CHART_BG_BIN, GUI_SCREEN_CENTER_X, 650);

    //创建图表
    compo = (component_t *)compo_chartbox_create(frm, CHART_TYPE_BAR, 7);
    compo_chartbox_set_location((compo_chartbox_t *)compo, 180+70, 572+20, 230, 150);
    compo_chartbox_set_pixel((compo_chartbox_t *)compo, 2);
    compo_setid((compo_chartbox_t *)compo, COMPO_ID_CHART);

    return frm;
}

// 刷新心率
static void func_heartrate_refresh(void)
{
	compo_textbox_t *txt = NULL;
	f_heartrate_t *f_heartrate = (f_heartrate_t *)func_cb.f_cb;

	u8 cur_hr = bsp_sensor_hrs_data_get();
	static char hr_buf[4];
	memset(hr_buf, 0, sizeof(hr_buf));

	//实时测量心率
	if (f_heartrate->last_hr != cur_hr) {
		f_heartrate->last_hr = cur_hr;
		txt = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_NUM_HEARTRATE_CUR);
        snprintf(hr_buf, sizeof(hr_buf), "%d", cur_hr);

        compo_textbox_set(txt, hr_buf);

		if (f_heartrate->max_hr <= cur_hr) {
			f_heartrate->max_hr = cur_hr;
			txt = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_NUM_HEARTRATE_MAX);
			snprintf(hr_buf, sizeof(hr_buf), "%d", f_heartrate->max_hr);
            compo_textbox_set(txt, hr_buf);
			return;
		}

		if (f_heartrate->min_hr == 0 || ((f_heartrate->min_hr > cur_hr) && (cur_hr != 0))) {
			f_heartrate->min_hr = cur_hr;
			txt = (compo_textbox_t *)compo_getobj_byid(COMPO_ID_NUM_HEARTRATE_MIN);
			snprintf(hr_buf, sizeof(hr_buf), "%d", f_heartrate->min_hr);
            compo_textbox_set(txt, hr_buf);
		}
	}
}

//心率功能事件处理
static void func_heartrate_process(void)
{
    f_heartrate_t *f_heartrate = (f_heartrate_t *)func_cb.f_cb;
    compo_page_move_process(&f_heartrate->ptm);
    func_heartrate_refresh();
    func_process();
}

//心率功能消息处理
static void func_heartrate_message(size_msg_t msg)
{
	f_heartrate_t *f_heartrate = (f_heartrate_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_TOUCH:
        compo_page_move_touch_handler(&f_heartrate->ptm);
        break;

    case MSG_CTP_CLICK:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
    case MSG_CTP_LONG:
        break;

	case MSG_QDEC_BACKWARD:
        if (func_cb.flag_sort) {    //快捷应用状态下不滚动页面
            func_message(msg);
        } else {
            compo_page_move_set_by_pages(&f_heartrate->ptm, -1);
        }
		break;

	case MSG_QDEC_FORWARD:
        if (func_cb.flag_sort) {
            func_message(msg);
        } else {
            compo_page_move_set_by_pages(&f_heartrate->ptm, 1);
        }
		break;

    default:
        func_message(msg);
        break;
    }
}

//进入心率功能
void func_heartrate_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_heartrate_t));
	func_cb.frm_main = func_heartrate_form_create();

	f_heartrate_t *f_heartrate = (f_heartrate_t *)func_cb.f_cb;
    page_move_info_t info = {
        .title_used = true,
        .page_size = GUI_SCREEN_HEIGHT - (GUI_SCREEN_HEIGHT / 8),
        .page_count = 2,
        .jump_perc = 20,
        .quick_jump_perc = 100,
        .up_over_perc = 10,
        .down_over_perc = 10,
    };
    compo_page_move_init(&f_heartrate->ptm, func_cb.frm_main->page_body, &info);
	bsp_sensor_hr_init(0);
}

//退出心率功能
void func_heartrate_exit(void)
{
	bsp_sensor_hr_stop();
}

//心率功能
void func_heartrate(void)
{
    printf("%s\n", __func__);
    func_heartrate_enter();
    while (func_cb.sta == FUNC_HEARTRATE) {
        func_heartrate_process();
        func_heartrate_message(msg_dequeue());
    }
    func_heartrate_exit();
}
