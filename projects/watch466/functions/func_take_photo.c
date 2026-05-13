
#include "include.h"
#include "func.h"

#if VIDEO_RECODE_TAKE_PHOTO_EN



static bool camera_take_photo_ble_send(u8 *buf, u32 len)       //相机ble发送数据
{
#if (USE_APP_TYPE == APP_AB_LINK)
    return user_common_ble_send(buf, len, ble_app_ab_link_send_packet);
#endif // USE_APP_TYPE
    return true;
}


//#define VIDEO_RECODING_RES      UI_BUF_CAMERA_RECODING_BIN//UI_BUF_VIDEO_RECODING
#define VIDEO_START_RECODE_RES  UI_BUF_CAMERA_START_PHOTO_BIN//UI_BUF_VIDEO_RECODE

#define VIDEO_WATERMASK_OPEN_RES   UI_BUF_CAMERA_WATERMASK_OPEN_BIN//UI_BUF_VIDEO_RECODING
#define VIDEO_WATERMASK_CLOSE_RES  UI_BUF_CAMERA_WATERMASK_CLOSE_BIN//UI_BUF_VIDEO_RECODE


//ID
enum {
    COMPO_ID_CAMERA = 1,

    //录像按钮
    COMPO_ID_AUTO_ALPHA_START,
    COMPO_ID_BTN_TAKE_PHOTO,  //拍照按钮
    COMPO_ID_BTN_WATERMASK, //水印按钮
    COMPO_ID_AUTO_ALPHA_END,
};

typedef struct f_take_photo_t_{
    u8 btn_alpha;
    u32 btn_tick;
} f_take_photo_t;

//创建活动记录窗体
compo_form_t *func_take_photo_form_create(void)
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
    compo_setid(btn, COMPO_ID_BTN_TAKE_PHOTO);
    compo_button_set_visible(btn, false);

    btn = compo_button_create_by_image(frm, VIDEO_WATERMASK_CLOSE_RES);
    compo_button_set_pos(btn, gui_image_get_size(VIDEO_START_RECODE_RES).wid/2 + 10, GUI_SCREEN_HEIGHT - gui_image_get_size(VIDEO_START_RECODE_RES).hei/2 - 10);
    compo_button_set_alpha(btn, 255);
    compo_setid(btn, COMPO_ID_BTN_WATERMASK);
    compo_button_set_visible(btn, false);

    return frm;
}


void func_take_photo_btn_ui_set_alpha(u8 alpha)
{
    f_take_photo_t* f_take_photo = (f_take_photo_t*) func_cb.f_cb;
    f_take_photo->btn_alpha = alpha;

    for (int i=0; i<COMPO_ID_AUTO_ALPHA_END-COMPO_ID_AUTO_ALPHA_START-1; i++) {
        compo_button_t* btn = compo_getobj_byid(COMPO_ID_AUTO_ALPHA_START+1+i);
        compo_button_set_visible(btn, f_take_photo->btn_alpha > 0);
        compo_button_set_alpha(btn, f_take_photo->btn_alpha);
    }
}

void func_take_photo_btn_ui_process(void)
{
    f_take_photo_t* f_take_photo = (f_take_photo_t*) func_cb.f_cb;
    compo_button_t* btn_watermask = compo_getobj_byid(COMPO_ID_BTN_WATERMASK);
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);


    if (compo_camera_is_working(camera)) {
        compo_button_set_visible(btn_watermask, false);
        return;
    } else {
        compo_button_set_visible(btn_watermask, f_take_photo->btn_alpha > 0);
    }

    if (f_take_photo->btn_alpha == 0 || (tick_get() - f_take_photo->btn_tick < 2000)) {
        return;
    }

    static u32 ticks = 0;
    if (tick_check_expire(ticks, 1)) {
        ticks = tick_get();
        func_take_photo_btn_ui_set_alpha(f_take_photo->btn_alpha);
        f_take_photo->btn_alpha--;
    }
}

void func_take_photo_btn_habdle(void)
{
    f_take_photo_t* f_take_photo = (f_take_photo_t*) func_cb.f_cb;

    f_take_photo->btn_tick = tick_get();
    if (f_take_photo->btn_alpha < 255) {
        func_take_photo_btn_ui_set_alpha(255);
        return;
    }

    int id = compo_get_button_id();
    compo_button_t* btn_watermask = compo_getobj_byid(COMPO_ID_BTN_WATERMASK);
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);

    switch (id) {
    case COMPO_ID_BTN_TAKE_PHOTO: {
        compo_camera_work_do(camera);
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
static void func_take_photo_process(void)
{
//    f_take_photo_t *take_photo_cb = (f_take_photo_t *)func_cb.f_cb;
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);
    compo_camera_view_frame_process(camera);
    func_take_photo_btn_ui_process();
    func_process();
}

//活动记录功能消息处理
static void func_take_photo_message(size_msg_t msg)
{
//    f_take_photo_t *take_photo_cb = (f_take_photo_t *)func_cb.f_cb;
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);

    switch (msg) {
    case MSG_CTP_CLICK:
        func_take_photo_btn_habdle();
        break;

    case KU_BACK: {//拍照
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


    case EVT_SD_CARD_INTER:
    {
        printf("EVT_SD_CARD_INTER\n");
        compo_camera_init_control(camera, 1, COMPO_CAMERA_SD_CARD_INTER);
    } break;

    case EVT_SD_CARD_OUT: {
        printf("EVT_SD_CARD_OUT\n");
        compo_camera_init_control(camera, 1, COMPO_CAMERA_SD_CARD_OUT);
    }   break;

//#if PSRAM_SIZE == 0
    case MSG_CTP_SHORT_RIGHT:           ///使用sram 进行 jpg显示退出只能直接退，不能有动画
        printf("MSG_CTP_SHORT_RIGHT\n");
        func_directly_back_to();
        break;
//#endif // PSRAM_SIZE

    default:
        if (!bsp_take_photo_is_busy()) {
            func_message(msg);
        }
        break;
    }
}

void func_take_photo_enter(void)
{
    sleep_only_gui_off_en();
    func_cb.f_cb = func_zalloc(sizeof(f_take_photo_t));
    func_cb.frm_main = func_take_photo_form_create();
    gui_set_te_margin(5);
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);
    compo_camera_init_control(camera, 1, COMPO_CAMERA_INIT);

    compo_camera_set_take_photo_ble_send(camera, camera_take_photo_ble_send);
}


void func_take_photo_exit(void)
{
    os_gui_draw_w4_done();
    compo_camera_t* camera = compo_getobj_byid(COMPO_ID_CAMERA);
    compo_camera_init_control(camera, 1, COMPO_CAMERA_EXIT);
    gui_set_te_margin(30);
    sleep_only_gui_off_dis();
    func_cb.last = FUNC_TAKE_PHOTO;
}


void func_take_photo(void)
{
    printf("%s\n", __func__);
    func_take_photo_enter();
    while (func_cb.sta == FUNC_TAKE_PHOTO) {
        func_take_photo_process();
        func_take_photo_message(msg_dequeue());
    }
    func_take_photo_exit();
}

#endif // VIDEO_RECODE_TAKE_PHOTO_EN
