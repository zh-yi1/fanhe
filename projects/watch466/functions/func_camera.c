#include "include.h"
#include "func.h"

#if FUNC_CAMERA_TRANS_EN

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//组件ID
enum{
    //按钮
    COMPO_ID_BTN_TAKE_A_PIC = 1,
    COMPO_ID_BTN_START = 1,
    COMPO_ID_PIC,
    COMPO_ID_MASK,
};

typedef struct f_picture_t_ {
    compo_shape_t *mask;
    u32 mask_tick;
    u8 phone_status;        //用于区分HID还是APP控制
} f_camera_t;

#define JPEG_WID            160
#define JPEG_HEI            192
#define JPEG_ROTATION       0       //旋转角度 0:0°，1:90°，2:180°，3:270°

#define JPEG_RX_BUF_SIZE    5*1024
#define JPEG2RGB_BUF_SIZE   JPEG_WID*JPEG_HEI*2+8

#define JPEG_LOCAL_TEST     0

#if JPEG_LOCAL_TEST
//AT(.jpeg_tbuf)
u8 t_buf1[JPEG_RX_BUF_SIZE] = {

};
#endif

static u8 jpeg_rx_buf[2][JPEG_RX_BUF_SIZE];// AT(.jpeg_rx);
void func_camera_set(u8 status);

typedef struct{
    u8 rgb_buf[JPEG2RGB_BUF_SIZE];
    u16 wptr;
    u8 *buf;

    bool flag;
} f_camera_jpeg_t;
f_camera_jpeg_t f_camera_jpeg;// AT(.jpeg_rgb);

//按钮
enum{
    CAMERA_CLOSE = 0,
    CAMERA_OPEN,
    CAMERA_TAKE_PHOTOS,
    CAMERA_TRANS_START,
};

static u32 my_tick = 0;


void func_camera_jpeg_rx(u8 *buf, u16 len)
{
    if (buf[0] == 0xaa && buf[1] == 0x55) {
        f_camera_t *f_camera = (f_camera_t *)func_cb.f_cb;
        u8 status = buf[2];
        bool res = buf[3];
        if (res) {
            switch(status) {
                case CAMERA_OPEN:
                    f_camera->phone_status = CAMERA_OPEN;
                    printf("RECV PHONE CAMERA_OPEN\n");
                    ble_update_conn_param(20, 0, 500);     //interval: 400*1.25ms = 500ms
                    func_camera_set(CAMERA_TRANS_START);
                    break;

                case CAMERA_CLOSE:
                    f_camera->phone_status = CAMERA_CLOSE;
                    printf("RECV PHONE CAMERA_CLOSE\n");
                    func_cb.sta = func_cb.last;
                    break;

                default:
                    printf("UNDEFINED STATUS!\n");
                    break;
            }
        }

    } else {

        if (f_camera_jpeg.wptr == 0) {
            my_tick = tick_get();
        }

        memcpy(f_camera_jpeg.buf + f_camera_jpeg.wptr, buf, len - 1);
        f_camera_jpeg.wptr += len - 1;

        //结束
        if (buf[len - 1] == 0) {
    //        printf("rx tick:%d, total:%d\n", tick_get() - my_tick, f_camera_jpeg.wptr);
            f_camera_jpeg.wptr = 0;
            f_camera_jpeg.flag ^= 1;

            f_camera_jpeg.buf = jpeg_rx_buf[f_camera_jpeg.flag];
            msg_enqueue(EVT_JPEG_DECODE);

        }
    }

}

void func_camera_set(u8 status)
{

    if (!ble_is_connect()) {
        printf("please connect ble!\n");
        return;
    }

    u8 buf[13] = {0xaa, 0x55, status, JPEG_WID & 0xff, JPEG_WID >> 8, JPEG_HEI & 0xff, JPEG_HEI >> 8, JPEG_ROTATION, JPEG_RX_BUF_SIZE & 0xff, JPEG_RX_BUF_SIZE >> 8, 0, 0, 0};
    u8 size = sizeof(buf);
    switch(status) {
        case CAMERA_OPEN:
            break;

        case CAMERA_TRANS_START:
        case CAMERA_TAKE_PHOTOS:
        case CAMERA_CLOSE:
            buf[3] = 0;
            size = 4;
            break;

        default:
            printf("UNDEFINED STATUS!\n");
            break;
    }
    app_protocol_tx(buf, size);
}

//单击按钮
static void func_camera_button_click(void)
{
    int id = compo_get_button_id();
    switch (id) {
    case COMPO_ID_BTN_START:
        {
            f_camera_t *f_camera = (f_camera_t *)func_cb.f_cb;
            printf("COMPO_ID_BTN_START\n");
            func_camera_set(CAMERA_TAKE_PHOTOS);
            if (func_cb.sta == FUNC_CAMERA) {
                compo_shape_set_visible(f_camera->mask, true);
                f_camera->mask_tick = tick_get();
            }
        }
        break;

    default:
        break;
    }
}

int jpeg_decode(void *out_buf, int out_size_max, void *in_buf, int in_size);


//创建窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_camera_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

//	创建图片
	compo_picturebox_t *pic = compo_picturebox_create(frm, 0);
	compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_setid(pic, COMPO_ID_PIC);

    //新建按钮
    compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_ICON_CLOCK_BG_BIN);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 150);
    compo_setid(btn, COMPO_ID_BTN_START);

    if (func_cb.sta == FUNC_CAMERA) {
        f_camera_t *f_camera = (f_camera_t *)func_cb.f_cb;
        f_camera->mask = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
        compo_shape_set_location(f_camera->mask, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
        compo_shape_set_color(f_camera->mask, COLOR_BLACK);
        compo_shape_set_visible(f_camera->mask, false);
    }

    return frm;
}

//拍照事件处理
static void func_camera_process(void)
{
    f_camera_t *f_camera = (f_camera_t *)func_cb.f_cb;
    if (compo_shape_get_visible(f_camera->mask) && tick_check_expire(f_camera->mask_tick, 100)) {
        compo_shape_set_visible(f_camera->mask, false);
    }

    func_process();
}

//拍照消息处理
static void func_camera_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_camera_button_click();
        break;

    //不需要点击mask效果这里return回去
    case MSG_CTP_TOUCH:
        break;


    case KU_LEFT:
        {
            #if JPEG_LOCAL_TEST
            compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_PIC);
            jpeg_decode(f_picture_jpeg.rgb_buf, JPEG2RGB_BUF_SIZE, t_buf2, JPEG_RX_BUF_SIZE);
            my_tick = 0;
            gui_set_ram_check(f_picture_jpeg.rgb_buf, __func__);      //检测ram对不对
            compo_picturebox_set_ram(pic, f_picture_jpeg.rgb_buf);
            compo_picturebox_set_size(pic, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
            #else
            func_camera_set(CAMERA_TRANS_START);
            #endif
        }
        break;

    case EVT_JPEG_DECODE:
        {
            int res = 0;
            func_camera_set(CAMERA_TRANS_START);
            compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_PIC);

            #if JPEG_LOCAL_TEST
            res = jpeg_decode(f_picture_jpeg.rgb_buf, JPEG2RGB_BUF_SIZE, t_buf1, JPEG_RX_BUF_SIZE);
            #else
            res = jpeg_decode(f_camera_jpeg.rgb_buf, JPEG2RGB_BUF_SIZE, jpeg_rx_buf[!f_camera_jpeg.flag], JPEG_RX_BUF_SIZE);
            #endif
            memset(&jpeg_rx_buf[!f_camera_jpeg.flag], 0, JPEG_RX_BUF_SIZE);

//            printf("### jpeg_decode time:%d, flag:%d\n", tick_get() - my_tick, f_picture_jpeg.flag);
            my_tick = 0;
            if (res > 0 ) {
//                printf("### set\n");
                gui_set_ram_check(f_camera_jpeg.rgb_buf, __func__);      //检测ram对不对
                compo_picturebox_set_ram(pic, f_camera_jpeg.rgb_buf);
                compo_picturebox_set_size(pic, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
            } else {
//                printf("### JPEG DECODE err:%d\n", res);
            }
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入音量页面
static void func_camera_enter(void)
{
    func_camera_set(CAMERA_OPEN);

    func_cb.f_cb = func_zalloc(sizeof(f_camera_t));
    func_cb.frm_main = func_camera_form_create();
    memset(&f_camera_jpeg, 0, sizeof(f_camera_jpeg_t));
    f_camera_jpeg.buf = jpeg_rx_buf[f_camera_jpeg.flag];
//    delay_5ms(200);

}

//退出拍照页面
static void func_camera_exit(void)
{
    func_camera_set(CAMERA_CLOSE);
    func_cb.last = FUNC_CAMERA;
}

//拍照页面
void func_camera(void)
{
    printf("%s\n", __func__);
    func_camera_enter();
//    msg_enqueue(EVT_JPEG_DECODE);
    while (func_cb.sta == FUNC_CAMERA) {
        func_camera_process();
        func_camera_message(msg_dequeue());
    }
    func_camera_exit();
}
#else
typedef struct f_camera_t_ {

} f_camera_t;

//创建相机窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_camera_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);
	compo_form_add_image(frm, UI_BUF_CAMERA_CAMERA_BIN, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_CAMERA]);

	//新建按钮
	compo_button_t *btn;
    btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BUTTON2_BIN);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 418);

	//创建文本
	compo_textbox_t *txt_start = compo_textbox_create(frm, 10);
	compo_textbox_set_pos(txt_start, GUI_SCREEN_CENTER_X, 418);
    compo_textbox_set(txt_start, i18n[STR_START]);

    return frm;
}

//相机功能事件处理
static void func_camera_process(void)
{
    func_process();
}

//相机功能消息处理
static void func_camera_message(size_msg_t msg)
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

//进入相机功能
void func_camera_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_camera_t));
    func_cb.frm_main = func_camera_form_create();
}

//退出相机功能
void func_camera_exit(void)
{
    func_cb.last = FUNC_CAMERA;
}

//相机功能
void func_camera(void)
{
    printf("%s\n", __func__);
    func_camera_enter();
    while (func_cb.sta == FUNC_CAMERA) {
        func_camera_process();
        func_camera_message(msg_dequeue());
    }
    func_camera_exit();
}

#endif
