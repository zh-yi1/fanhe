#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define TEST_EN                 0      //假数据测试


#if (SENSOR_STEP_SEL == SENSOR_STEP_SC7A20)

#define STEPS_DATA_LIMIT        "99999"
#define KCAL_DATA_LIMIT         "99999 千卡"
#define DISTANCE_DATA_LIMIT     "99999.9 千米"

enum
{
    ACTIVITY_STEPS,
    ACTIVITY_KCAL,
    ACTIVITY_DIST,
};

enum
{
	//文本框
	COMPO_ID_TXT_STEPS = 1,
	COMPO_ID_TXT_KCAL,
	COMPO_ID_TXT_DIATANCE,
};

typedef struct f_activity_t_ {
    uint32_t last_steps;
    uint32_t last_kcal;
    uint32_t last_distance;
} f_activity_t;

#if TEST_EN
static int cur_steps = 0, cur_kcal = 0, cur_distance = 0;
#endif


//创建活动记录窗体
compo_form_t *func_activity_form_create(void)
{
    component_t *compo;
    uint32_t res_bin[] = {
        UI_BUF_ACTIVITY_STEP_BIN,
        UI_BUF_ACTIVITY_KCAL_BIN,
        UI_BUF_ACTIVITY_KM_BIN,
    };

    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_ACTIVITY_RECORD]);

    //step_pic
    compo = (component_t *)compo_picturebox_create(frm, res_bin[ACTIVITY_STEPS]);
    compo_picturebox_set_pos((compo_picturebox_t *)compo, GUI_SCREEN_CENTER_X, 102);

    //kcal_pic
    compo = (component_t *)compo_picturebox_create(frm, res_bin[ACTIVITY_KCAL]);
    compo_picturebox_set_pos((compo_picturebox_t *)compo, 70, 260);

    //km_pic
    compo = (component_t *)compo_picturebox_create(frm, res_bin[ACTIVITY_DIST]);
    compo_picturebox_set_pos((compo_picturebox_t *)compo, 70, 330);

    //steps text
    compo = (component_t *)compo_textbox_create(frm, sizeof(STEPS_DATA_LIMIT));
    compo_setid(compo, COMPO_ID_TXT_STEPS);
    compo_textbox_set_pos((compo_textbox_t *)compo, GUI_SCREEN_CENTER_X, 171);

    //kcal text
    compo = (component_t *)compo_textbox_create(frm, sizeof(KCAL_DATA_LIMIT));
    compo_textbox_set_align_center((compo_textbox_t *)compo, false);
    compo_setid(compo, COMPO_ID_TXT_KCAL);
    compo_textbox_set_pos((compo_textbox_t *)compo, 117, 260);

    //distance text
    compo = (component_t *)compo_textbox_create(frm, sizeof(DISTANCE_DATA_LIMIT));
    compo_textbox_set_align_center((compo_textbox_t *)compo, false);
    compo_setid(compo, COMPO_ID_TXT_DIATANCE);
    compo_textbox_set_pos((compo_textbox_t *)compo, 117, 330);

    return frm;
}

//刷新活动数据
static void func_activity_data_refresh(void)
{
    compo_textbox_t *txt_compo;
    f_activity_t *f_activity = (f_activity_t *)func_cb.f_cb;
    char buf[16] = {0};

    //获取当前值(从存储接口或外设接口获取)
#if (!TEST_EN)
    int cur_steps = bsp_sensor_step_cur_steps_get();
    int cur_kcal = bsp_sensor_step_cur_kcal_get();
    int cur_distance = bsp_sensor_step_cur_distance_get();
#endif

    if (f_activity->last_steps != cur_steps) {
        txt_compo = compo_getobj_byid(COMPO_ID_TXT_STEPS);
        snprintf(buf, sizeof(buf), "%d", cur_steps);
        compo_textbox_set(txt_compo, buf);
        f_activity->last_steps = cur_steps;
    }

    if (f_activity->last_kcal != cur_kcal) {
        txt_compo = compo_getobj_byid(COMPO_ID_TXT_KCAL);
        snprintf(buf, sizeof(buf), "%d %s", cur_kcal / 10, i18n[STR_KCAL]);
        compo_textbox_set(txt_compo, buf);
        f_activity->last_kcal = cur_kcal;
    }

    if (f_activity->last_distance != cur_distance) {
        txt_compo = compo_getobj_byid(COMPO_ID_TXT_DIATANCE);
        snprintf(buf, sizeof(buf), "%d.%d %s", cur_distance / 10, cur_distance % 10, i18n[STR_KM]);
        compo_textbox_set(txt_compo, buf);
        f_activity->last_distance = cur_distance;
    }
}

//活动记录功能事件处理
static void func_activity_process(void)
{

#if TEST_EN
    static uint32_t tick = 0;
    if(tick_check_expire(tick, 100)) {
        tick = tick_get();
        cur_steps += 1;
        cur_kcal += 2;
        cur_distance += 3;
    }
#endif
    func_activity_data_refresh();
    func_process();
}

//活动记录功能消息处理
static void func_activity_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入活动记录控制功能
void func_activity_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_activity_t));
    f_activity_t *f_activity = (f_activity_t *)func_cb.f_cb;
    f_activity->last_steps = 0xffffffff;
    f_activity->last_kcal = 0xffffffff;
    f_activity->last_distance = 0xffffffff;

    func_cb.frm_main = func_activity_form_create();
}

//退出活动记录功能
void func_activity_exit(void)
{
    func_cb.last = FUNC_ACTIVITY;
}

//活动记录功能
void func_activity(void)
{
    printf("%s\n", __func__);
    func_activity_enter();
    while (func_cb.sta == FUNC_ACTIVITY) {
        func_activity_process();
        func_activity_message(msg_dequeue());
    }
    func_activity_exit();
}


#else

enum
{
    ARC_ANIM_STATUS_START = 0,
    ARC_ANIM_STATUS_FW,
    ARC_ANIM_STATUS_BW,
    ARC_ANIM_STATUS_END,
};

enum
{
	//文本框
	COMPO_ID_ARC_START = 1,
	COMPO_ID_ARC_END = 3,
};

typedef struct f_activity_t_ {
    uint32_t tick;
    uint32_t last_steps;
    uint32_t last_kcal;
    uint32_t last_distance;

    u16 value[3];
    u8 anim_status;
} f_activity_t;

// size, arc_w, rotation, s_angle, e_angle, content_color, bg_color
static const int16_t activity_arc_info[][7] = {
    {300, 30, 0,  0,    3600,   COLOR_BLUE, COLOR_WHITE},      //kcal
    {220, 30, 0,  3600, 0,    0x9772,     COLOR_WHITE},      //steps
    {140, 30, 0,  0,    3600,   0xfbea,     COLOR_WHITE},      //exervise
};
static const u16 activity_arc_value_goal[3] = {750, 750, 1000};

//创建活动记录窗体
compo_form_t *func_activity_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_ACTIVITY_RECORD]);

    //创建圆弧
    for (u8 i = 0; i < sizeof(activity_arc_info) / sizeof(activity_arc_info[0]); i++) {
        compo_arc_t *arc = compo_arc_create(frm);
        compo_setid(arc, COMPO_ID_ARC_START + i);
        compo_arc_set_alpha(arc, 0xff, 0);
        compo_arc_set_location(arc, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 40, activity_arc_info[i][0], activity_arc_info[i][0]);
        compo_arc_set_width(arc, activity_arc_info[i][1]);
        compo_arc_set_rotation(arc, activity_arc_info[i][2]);
        compo_arc_set_angles(arc, activity_arc_info[i][3], activity_arc_info[i][4]);
        compo_arc_set_color(arc, activity_arc_info[i][5], activity_arc_info[i][6]);
    }

    return frm;
}

//活动记录功能事件处理
static void func_activity_process(void)
{
    int i = 0;
    f_activity_t *f_activity = (f_activity_t *)func_cb.f_cb;

    func_process();
    if (ARC_ANIM_STATUS_END == f_activity->anim_status) {
        return ;
    }

    if(tick_check_expire(f_activity->tick, 10)) {
        f_activity->tick = tick_get();
        u8 count = sizeof(f_activity->value) / sizeof(f_activity->value[0]);

        switch (f_activity->anim_status) {
            case ARC_ANIM_STATUS_START:
                {
                    for (i = 0; i < count; i++) {
                        f_activity->value[i] = 0;
                    }
                    f_activity->anim_status = ARC_ANIM_STATUS_FW;
                }break;

            case ARC_ANIM_STATUS_FW:
                {
                    for (i = 0; i < count; i++) {
                        f_activity->value[i] += 10;
                        if (f_activity->value[i] > ARC_VALUE_MAX) {
                            f_activity->value[i] = ARC_VALUE_MAX;
                            f_activity->anim_status = ARC_ANIM_STATUS_BW;
                        }
                    }
                }break;

            case ARC_ANIM_STATUS_BW:
                {
                    for (i = 0; i < sizeof(f_activity->value) / sizeof(f_activity->value[0]); i++) {
                        u16 offset = (ARC_VALUE_MAX - activity_arc_value_goal[i]) / 20;
                        if (f_activity->value[i] < activity_arc_value_goal[i]) {
                            f_activity->value[i] = activity_arc_value_goal[i];
                            f_activity->anim_status = ARC_ANIM_STATUS_END;
                        } else {
                            f_activity->value[i] -= offset;
                        }
                    }
                }break;
        }
    }

    for (i = 0; i < sizeof(activity_arc_info) / sizeof(activity_arc_info[0]); i++) {
        compo_arc_t *arc = compo_getobj_byid(COMPO_ID_ARC_START + i);
        compo_arc_set_value(arc, f_activity->value[i]);
    }
}

//活动记录功能消息处理
static void func_activity_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入活动记录控制功能
void func_activity_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_activity_t));
    func_cb.frm_main = func_activity_form_create();
    f_activity_t *f_activity = (f_activity_t *)func_cb.f_cb;
    f_activity->last_steps = 0xffffffff;
    f_activity->last_kcal = 0xffffffff;
    f_activity->last_distance = 0xffffffff;
    f_activity->tick = tick_get();
}

//退出活动记录功能
void func_activity_exit(void)
{
    func_cb.last = FUNC_ACTIVITY;
}

//活动记录功能
void func_activity(void)
{
    printf("%s\n", __func__);
    func_activity_enter();
    while (func_cb.sta == FUNC_ACTIVITY) {
        func_activity_process();
        func_activity_message(msg_dequeue());
    }
    func_activity_exit();
}


#endif
