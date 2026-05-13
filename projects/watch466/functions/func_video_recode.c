#include "include.h"
#include "func.h"

#if VIDEO_RECODE_TAKE_PHOTO_EN


//void img_gui_buff_flush(void);
//
//#define VEDIO_NUM_LIST              ((int)(sizeof(vedio_bg_list)/sizeof(vedio_bg_list[0])))
//#define DELAY_TIMES_1S              1000
//#define HIDE_VEDIO_PROGRESS_TIME    2000

//ui资源结构体
//typedef struct func_menu_list_info_t_ {
//    u32 res_addr;
//    s16 x;
//    s16 y;
//    s16 wid;
//    s16 hei;
//} func_menu_list_info_t;

//背景
//static func_menu_list_info_t vedio_bg_list[] = {
//    {0,                   GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_1_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_2_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_3_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_4_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_5_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_6_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_7_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 60, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_8_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 60, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_9_BIN,  GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 40, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//    {UI_BUF_PHOTO_10_BIN, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 70, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT},
//};

#define VIDEO_RECODING_RES      UI_BUF_CAMERA_RECODING_BIN//UI_BUF_VIDEO_RECODING
#define VIDEO_START_RECODE_RES  UI_BUF_CAMERA_START_RECODE_BIN//UI_BUF_VIDEO_RECODE

#define VIDEO_WATERMASK_OPEN_RES   UI_BUF_CAMERA_WATERMASK_OPEN_BIN//UI_BUF_VIDEO_RECODING
#define VIDEO_WATERMASK_CLOSE_RES  UI_BUF_CAMERA_WATERMASK_CLOSE_BIN//UI_BUF_VIDEO_RECODE

//ID
enum {
    COMPO_ID_CAMERA = 1,

    //录像按钮
    COMPO_ID_AUTO_ALPHA_START,
    COMPO_ID_BTN_RECODE,  //录像按钮
    COMPO_ID_BTN_WATERMASK, //水印按钮
    COMPO_ID_AUTO_ALPHA_END,

    COMPO_ID_SHAPE_SD_UNMOUNT,
    COMPO_ID_TXT_SD_UNMOUNT,
};


typedef struct f_video_recode_t_{
    u8 btn_alpha;
    u32 btn_tick;
} f_video_recode_t;


//创建活动记录窗体
compo_form_t *func_video_recode_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    compo_camera_t* camera = compo_camera_create(frm);
    compo_camera_set_size(camera, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_camera_set_pos(camera, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_setid(camera, COMPO_ID_CAMERA);

    compo_button_t* btn = compo_button_create_by_image(frm, VIDEO_START_RECODE_RES);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT - gui_image_get_size(VIDEO_START_RECODE_RES).hei/2 - 10);
    compo_button_set_alpha(btn, 255);
    compo_setid(btn, COMPO_ID_BTN_RECODE);
    compo_button_set_visible(btn, false);

    btn = compo_button_create_by_image(frm, VIDEO_WATERMASK_CLOSE_RES);
    compo_button_set_pos(btn, gui_image_get_size(VIDEO_START_RECODE_RES).wid/2 + 10, GUI_SCREEN_HEIGHT - gui_image_get_size(VIDEO_START_RECODE_RES).hei/2 - 10);
    compo_button_set_alpha(btn, 255);
    compo_setid(btn, COMPO_ID_BTN_WATERMASK);
    compo_button_set_visible(btn, false);


    compo_shape_t* shape = compo_shape_create(frm, 0);
    compo_shape_set_location(shape, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH-50, GUI_SCREEN_HEIGHT/3);
    compo_shape_set_alpha(shape, 255);
    compo_shape_set_color(shape, COLOR_GRAY);
    compo_shape_set_radius(shape, 20);
    compo_shape_set_visible(shape, false);
    compo_setid(shape, COMPO_ID_SHAPE_SD_UNMOUNT);

    compo_textbox_t* txt = compo_textbox_create(frm, strlen(i18n[STR_SD_PLEASE_FORMAT]));
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH-50, GUI_SCREEN_HEIGHT/3);
    compo_textbox_set(txt, i18n[STR_SD_PLEASE_FORMAT]);
    compo_textbox_set_visible(txt, false);
    compo_setid(txt, COMPO_ID_TXT_SD_UNMOUNT);

    return frm;
}

void func_video_recode_btn_ui_set_alpha(u8 alpha)
{
    f_video_recode_t* f_video_recode = (f_video_recode_t*) func_cb.f_cb;
    f_video_recode->btn_alpha = alpha;

    for (int i=0; i<COMPO_ID_AUTO_ALPHA_END-COMPO_ID_AUTO_ALPHA_START-1; i++) {
        compo_button_t* btn = compo_getobj_byid(COMPO_ID_AUTO_ALPHA_START+1+i);
        compo_button_set_visible(btn, f_video_recode->btn_alpha > 0);
        compo_button_set_alpha(btn, f_video_recode->btn_alpha);
    }
}

void func_video_recode_btn_ui_process(void)
{
    f_video_recode_t* f_video_recode = (f_video_recode_t*) func_cb.f_cb;
    compo_button_t* btn = compo_getobj_byid(COMPO_ID_BTN_RECODE);
    compo_button_t* btn_watermask = compo_getobj_byid(COMPO_ID_BTN_WATERMASK);
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);
    compo_textbox_t* txt = compo_getobj_byid(COMPO_ID_TXT_SD_UNMOUNT);
    compo_shape_t* shape = compo_getobj_byid(COMPO_ID_SHAPE_SD_UNMOUNT);

    if (camera->dev_sta & MASK_SD_MOUNT_STA) {
        compo_shape_set_visible(shape, false);
        compo_textbox_set_visible(txt, false);
    } else {
        compo_shape_set_visible(shape, true);
        compo_textbox_set_visible(txt, true);
    }

    if (bsp_video_recode_is_start()) {
//        func_video_recode_btn_ui_set_alpha(255);
        compo_button_set_visible(btn_watermask, false);
        return;
    } else {
        compo_button_set_bgimg(btn, VIDEO_START_RECODE_RES);
        compo_button_set_visible(btn_watermask, f_video_recode->btn_alpha > 0);
    }


    if (f_video_recode->btn_alpha == 0 || (tick_get() - f_video_recode->btn_tick < 2000)) {
        return;
    }

    static u32 ticks = 0;
    if (tick_check_expire(ticks, 1)) {
        ticks = tick_get();
        func_video_recode_btn_ui_set_alpha(f_video_recode->btn_alpha);
        f_video_recode->btn_alpha--;
    }
}

void func_video_recode_btn_habdle(void)
{
    f_video_recode_t* f_video_recode = (f_video_recode_t*) func_cb.f_cb;

    f_video_recode->btn_tick = tick_get();
    if (f_video_recode->btn_alpha < 255) {
        func_video_recode_btn_ui_set_alpha(255);
        return;
    }

    int id = compo_get_button_id();
    compo_button_t* btn = compo_getobj_byid(COMPO_ID_BTN_RECODE);
    compo_button_t* btn_watermask = compo_getobj_byid(COMPO_ID_BTN_WATERMASK);
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);

    switch (id) {
    case COMPO_ID_BTN_RECODE: {
        compo_camera_work_do(camera);
        if (bsp_video_recode_is_start()) {
            compo_button_set_bgimg(btn, VIDEO_RECODING_RES);
        } else {
            compo_button_set_bgimg(btn, VIDEO_START_RECODE_RES);
        }
    }   break;

    case COMPO_ID_BTN_WATERMASK: {
        if (sys_cb.datelabel) {
            sys_cb.datelabel = false;
            compo_button_set_bgimg(btn_watermask, VIDEO_WATERMASK_CLOSE_RES);
        } else {
            sys_cb.datelabel = true;
            compo_button_set_bgimg(btn_watermask, VIDEO_WATERMASK_OPEN_RES);
        }
    }   break;

    default:
        break;
    }
}


//活动记录功能事件处理
static void func_video_recode_process(void)
{

//    f_video_recode_t *video_recode_cb = (f_video_recode_t *)func_cb.f_cb;
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);
    compo_camera_view_frame_process(camera);
    func_video_recode_btn_ui_process();
    func_process();
}

//活动记录功能消息处理
static void func_video_recode_message(size_msg_t msg)
{
//    f_video_recode_t *video_recode_cb = (f_video_recode_t *)func_cb.f_cb;
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);

    switch (msg) {
    case MSG_CTP_CLICK:
        func_video_recode_btn_habdle();
        break;

    case KU_BACK: { //录像开始或结束
        printf("KU_BACK\n");
        compo_camera_work_do(camera);
    }   break;

    case MSG_QDEC_BACKWARD: {
        printf("MSG_QDEC_BACKWARD\n");
        compo_camera_zoom_do(camera, true);
    }   break;

    case MSG_QDEC_FORWARD: {
        printf("MSG_QDEC_FORWARD\n");
        compo_camera_zoom_do(camera, false);
    }   break;

    case EVT_SD_CARD_INTER: {
        printf("EVT_SD_CARD_INTER\n");
        compo_camera_init_control(camera, 1, COMPO_CAMERA_SD_CARD_INTER);
    }   break;

    case EVT_SD_CARD_OUT: {
        printf("EVT_SD_CARD_OUT\n");
        compo_camera_init_control(camera, 0, COMPO_CAMERA_SD_CARD_OUT);
    }   break;

//#if PSRAM_SIZE == 0
    case MSG_CTP_SHORT_RIGHT:           ///使用sram 进行 jpg显示退出只能直接退，不能有动画
        printf("MSG_CTP_SHORT_RIGHT\n");
        func_directly_back_to();
        break;
//#endif // PSRAM_SIZE

    default:
        func_message(msg);
        break;
    }
}

void func_video_recode_enter(void)
{
    sleep_only_gui_off_en();
    sys_cb.rfile_limit = 60*30;       //录像限制时间 sec
    func_cb.f_cb = func_zalloc(sizeof(f_video_recode_t));
    func_cb.frm_main = func_video_recode_form_create();
    gui_set_te_margin(5);
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);
    compo_camera_init_control(camera, 0, COMPO_CAMERA_INIT);
}


void func_video_recode_exit(void)
{
    os_gui_draw_w4_done();
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);
    compo_camera_init_control(camera, 0, COMPO_CAMERA_EXIT);
    gui_set_te_margin(30);
    sleep_only_gui_off_dis();
    func_cb.last = FUNC_VIDEO_RECODE;
}


void func_video_recode(void)
{
    printf("%s\n", __func__);
    func_video_recode_enter();
    while (func_cb.sta == FUNC_VIDEO_RECODE) {
        func_video_recode_process();
        func_video_recode_message(msg_dequeue());
    }
    func_video_recode_exit();
}
#endif // VIDEO_RECODE_TAKE_PHOTO_EN

