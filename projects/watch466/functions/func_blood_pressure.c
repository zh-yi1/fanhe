#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//血压外设接口
static void sensor_blood_pressure_start(void);
static void sensor_blood_pressure_stop(void);
static void sensor_blood_pressure_data_get(uint8_t *sbp, uint8_t *dbp);
static bool sensor_blood_pressure_wear_sta_get(void);

//血压检测状态
enum {
	BP_STA_IDLE,		    //空闲
	BP_STA_WORKING,		    //检测中
	BP_STA_UNWEAR,			//未佩戴
};

//组件ID
enum {
	COMPO_ID_BTN = 1,
    COMPO_ID_PIC_BG,
	COMPO_ID_PIC_CLICK,
	COMPO_ID_TXT_TIPS,
    COMPO_ID_TXT_RESULT,
    COMPO_ID_PIC_UNWEAR,
};

typedef struct f_bloodpressure_t_ {
	uint8_t cur_sta;
    uint8_t sbp;
    uint8_t dbp;
    bool touch;
} f_bloodpressure_t;


//创建血压窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_bloodpressure_form_create(void)
{
    component_t *compo;

    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
    compo = (component_t *)compo_picturebox_create(frm, UI_BUF_ICON_BLOOD_PRESSURE_BIN);
    compo_picturebox_set_pos((compo_picturebox_t *)compo, GUI_SCREEN_CENTER_X, 210);
    compo_setid(compo, COMPO_ID_PIC_BG);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_BLOOD_PRESSURE]);

	//图像按钮
	compo = (component_t *)compo_button_create_by_image(frm, UI_BUF_COMMON_BUTTON_BIN);
    compo_setid(compo, COMPO_ID_BTN);
    compo_button_set_pos((compo_button_t *)compo, GUI_SCREEN_CENTER_X, 336);

    //按钮上的文本
    compo = (component_t *)compo_textbox_create(frm, 10);
    compo_setid(compo, COMPO_ID_TXT_TIPS);
    compo_textbox_set_pos((compo_textbox_t *)compo, GUI_SCREEN_CENTER_X, 336);
    compo_textbox_set((compo_textbox_t *)compo, i18n[STR_START]);

    //测量结果
    compo = (component_t *)compo_textbox_create(frm, 16);
    compo_setid(compo, COMPO_ID_TXT_RESULT);
    compo_textbox_set_pos((compo_textbox_t *)compo, GUI_SCREEN_CENTER_X, 260);
    compo_textbox_set_visible((compo_textbox_t *)compo, false);

    //未佩戴提示
    compo = (component_t *)compo_picturebox_create(frm, UI_BUF_BLOOD_OXYGEN_EXPLAIN_BIN);
    compo_setid(compo, COMPO_ID_PIC_UNWEAR);
    compo_picturebox_set_pos((compo_picturebox_t *)compo, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_visible((compo_picturebox_t *)compo, false);

    return frm;
}

//触摸按钮效果处理
static void func_bloodpressure_button_touch_handle(void)
{
    f_bloodpressure_t *f_bp = (f_bloodpressure_t *)func_cb.f_cb;
    int id = compo_get_button_id();

    if (COMPO_ID_BTN != id) {
        return ;
    }
    compo_button_t  *btn_click = compo_getobj_byid(COMPO_ID_BTN);
    compo_button_set_alpha(btn_click, UI_BTN_CLICK_EFFECT_ALPHA);

    f_bp->touch = true;
}

//释放按钮效果处理
static void func_bloodpressure_button_release_handle(void)
{
    compo_button_t  *btn_click = compo_getobj_byid(COMPO_ID_BTN);
    compo_button_set_alpha(btn_click, UI_BTN_NORMAL_EFFECT_ALPHA);
}

//单击按钮
static void func_bloodpressure_button_click(void)
{
    f_bloodpressure_t *f_bp = (f_bloodpressure_t *)func_cb.f_cb;
    component_t *compo;

    //未佩戴下退出
    if (BP_STA_UNWEAR == f_bp->cur_sta) {
        compo = compo_getobj_byid(COMPO_ID_PIC_UNWEAR);
        compo_picturebox_set_visible((compo_picturebox_t *)compo, false);
        compo = compo_getobj_byid(COMPO_ID_PIC_BG);
        compo_picturebox_set_visible((compo_picturebox_t *)compo, true);
    }

    int id = compo_get_button_id();
    if (COMPO_ID_BTN != id) {
        return ;
    }

    switch (f_bp->cur_sta) {
        case BP_STA_IDLE: {
            sensor_blood_pressure_stop();
            sensor_blood_pressure_start();
            compo = compo_getobj_byid(COMPO_ID_TXT_TIPS);
            compo_textbox_set((compo_textbox_t *)compo, i18n[STR_MEASURING]);
            f_bp->cur_sta = BP_STA_WORKING;
        }break;

        case BP_STA_WORKING:
        case BP_STA_UNWEAR: {
            sensor_blood_pressure_stop();
            compo = compo_getobj_byid(COMPO_ID_TXT_TIPS);
            compo_textbox_set((compo_textbox_t *)compo, i18n[STR_START]);
            f_bp->cur_sta = BP_STA_IDLE;
        }break;
    }
}

//未佩戴和佩戴处理
static void func_bloodpressure_unwear_handle(bool wear)
{
    f_bloodpressure_t *f_bp = (f_bloodpressure_t *)func_cb.f_cb;
    component_t *compo;

    if (wear) {
        compo = compo_getobj_byid(COMPO_ID_PIC_UNWEAR);
        compo_picturebox_set_visible((compo_picturebox_t *)compo, false);

        compo = compo_getobj_byid(COMPO_ID_PIC_BG);
        compo_picturebox_set_visible((compo_picturebox_t *)compo, true);

        compo = compo_getobj_byid(COMPO_ID_TXT_TIPS);
        compo_textbox_set((compo_textbox_t *)compo, i18n[STR_MEASURING]);
    } else {
        compo = compo_getobj_byid(COMPO_ID_PIC_UNWEAR);
        compo_picturebox_set_visible((compo_picturebox_t *)compo, true);

        compo = compo_getobj_byid(COMPO_ID_PIC_BG);
        compo_picturebox_set_visible((compo_picturebox_t *)compo, false);

        compo = compo_getobj_byid(COMPO_ID_TXT_TIPS);
        compo_textbox_set((compo_textbox_t *)compo, i18n[STR_WEAR_CHECK]);

        f_bp->sbp = 0;
        f_bp->dbp = 0;
        compo = compo_getobj_byid(COMPO_ID_TXT_RESULT);
        compo_textbox_set_visible((compo_textbox_t *)compo, false);
    }
}

//血压功能事件处理
static void func_bloodpressure_process(void)
{
    f_bloodpressure_t *f_bp = (f_bloodpressure_t *)func_cb.f_cb;
    s32 dx, dy;

    //按钮释放效果
    if (f_bp->touch && !ctp_get_dxy(&dx, &dy)) {
        f_bp->touch = false;
        func_bloodpressure_button_release_handle();
    }

    if (BP_STA_WORKING == f_bp->cur_sta) {
        if (!sensor_blood_pressure_wear_sta_get()) { //未佩戴检测
            func_bloodpressure_unwear_handle(false);
            f_bp->cur_sta = BP_STA_UNWEAR;
        } else {
            //出值结果刷新
            uint8_t sbp = 0, dbp = 0;
            sensor_blood_pressure_data_get(&sbp, &dbp);
            char buf[16] = {0};
            if (f_bp->sbp != sbp || f_bp->dbp != dbp) {
                f_bp->sbp = sbp;
                f_bp->dbp = dbp;
                compo_textbox_t *compo_txt = compo_getobj_byid(COMPO_ID_TXT_RESULT);
                compo_textbox_set_visible(compo_txt, true);
                snprintf(buf, sizeof(buf), "sbp:%d dbp:%d", sbp, dbp);
                compo_textbox_set(compo_txt, buf);
            }
        }
    } else if(BP_STA_UNWEAR == f_bp->cur_sta) {    //佩戴检测
        if (sensor_blood_pressure_wear_sta_get()) {
            func_bloodpressure_unwear_handle(true);
            f_bp->cur_sta = BP_STA_WORKING;
        }
    }

    func_process();
}

//血压功能消息处理
static void func_bloodpressure_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_TOUCH:
        func_bloodpressure_button_touch_handle();
        break;

	case MSG_CTP_CLICK:
        func_bloodpressure_button_click();
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入血压功能
void func_bloodpressure_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_bloodpressure_t));
    func_cb.frm_main = func_bloodpressure_form_create();

    f_bloodpressure_t *f_bp = (f_bloodpressure_t *)func_cb.f_cb;
    f_bp->cur_sta = BP_STA_IDLE;
}

//退出血压功能
void func_bloodpressure_exit(void)
{
    func_cb.last = FUNC_BLOOD_PRESSURE;

    f_bloodpressure_t *f_bp = (f_bloodpressure_t *)func_cb.f_cb;
    if (BP_STA_IDLE != f_bp->cur_sta) {
        sensor_blood_pressure_stop();
    }
    func_cb.last = FUNC_BLOOD_PRESSURE;
}

//血压功能
void func_bloodpressure(void)
{
    printf("%s\n", __func__);
    func_bloodpressure_enter();
    while (func_cb.sta == FUNC_BLOOD_PRESSURE) {
        func_bloodpressure_process();
        func_bloodpressure_message(msg_dequeue());
    }
    func_bloodpressure_exit();
}

//血压外设接口
static void sensor_blood_pressure_start(void)
{
    bsp_sensor_hr_init(3);
    return ;
}

static void sensor_blood_pressure_stop(void)
{
    bsp_sensor_hr_stop();
    return ;
}

static void sensor_blood_pressure_data_get(uint8_t *sbp, uint8_t *dbp)
{
    bsp_sensor_bp_data_get(sbp, dbp);

    return ;
}

static bool sensor_blood_pressure_wear_sta_get(void)
{
    return bsp_sensor_hr_wear_sta_get();
}
