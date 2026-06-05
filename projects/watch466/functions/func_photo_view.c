#include "include.h"
#include "func.h"

#if PHOTO_VIEW_EN

//ID
enum {
    COMPO_ID_JPG = 1,
    COMPO_ID_BTN_NEXT,
    COMPO_ID_BTN_PREV,
    COMPO_ID_BTN_VOL_UP,
    COMPO_ID_BTN_VOL_DOWN,
    COMPO_ID_MASK_PRO_BG,
    COMPO_ID_MASK_PRO_FG,

    COMPO_ID_MASK_NEXT,
    COMPO_ID_MASK_PREV,

    COMPO_ID_TEXT_NEXT,
    COMPO_ID_TEXT_PREV,
    COMPO_ID_MAX,
};

typedef struct f_photo_view_t_{
    u8 btn_alpha;
    u32 btn_tick;
} f_photo_view_t;

const jpg_list_t jpg_list[] = {
//    [0] = {UI_BUF_JPG_1_JPG, UI_LEN_JPG_1_JPG},
//    [1] = {UI_BUF_JPG_2_JPG, UI_LEN_JPG_2_JPG},
//    [2] = {UI_BUF_JPG_3_JPG, UI_LEN_JPG_3_JPG},
};

//创建活动记录窗体
compo_form_t *func_photo_view_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    compo_jpg_t* jpg = compo_jpg_create(frm, 0, 0);
    compo_jpg_set_flash_res_list(jpg, jpg_list, sizeof(jpg_list)/sizeof(jpg_list_t));
    compo_setid(jpg, COMPO_ID_JPG);
    compo_jpg_set_size(jpg, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_jpg_set_pos(jpg, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    compo_button_t *btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_NEXT);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X+80, 400, 90, 60);

    btn = compo_button_create(frm);
    compo_setid(btn, COMPO_ID_BTN_PREV);
    compo_button_set_location(btn, GUI_SCREEN_CENTER_X-80, 400, 90, 60);


    //创建遮罩层
    compo_shape_t *masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_GRAY);
    compo_shape_set_radius(masklayer, 30);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X+80, 400, 90, 60);
    compo_shape_set_alpha(masklayer, 255);
    compo_setid(masklayer, COMPO_ID_MASK_NEXT);
    compo_shape_set_visible(masklayer, false);


    masklayer = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(masklayer, COLOR_GRAY);
    compo_shape_set_radius(masklayer, 30);
    compo_shape_set_location(masklayer, GUI_SCREEN_CENTER_X-80, 400, 90, 60);
    compo_shape_set_alpha(masklayer, 255);
    compo_setid(masklayer, COMPO_ID_MASK_PREV);
    compo_shape_set_visible(masklayer, false);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X+80, 395, 90, 50);
    compo_textbox_set(txt, "Next");
    compo_setid(txt, COMPO_ID_TEXT_NEXT);
    compo_textbox_set_visible(txt, false);

    //新建文本
    txt = compo_textbox_create(frm, 16);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X-80, 395, 90, 50);
    compo_textbox_set(txt, "Prev");
    compo_setid(txt, COMPO_ID_TEXT_PREV);
    compo_textbox_set_visible(txt, false);

    return frm;
}

void func_photo_play_btn_ui_set_alpha(u8 alpha)
{
    f_photo_view_t *f_photo_view = (f_photo_view_t *)func_cb.f_cb;
    u8 num  = COMPO_ID_MAX - COMPO_ID_MASK_NEXT;
    f_photo_view->btn_alpha = alpha;
    for(u8 i=0; i<num; i++) {
        if (i < 2) {
            compo_shape_t *masklayer = compo_getobj_byid(COMPO_ID_MASK_NEXT+i);
            compo_shape_set_visible(masklayer, f_photo_view->btn_alpha > 0);
            compo_shape_set_alpha(masklayer, f_photo_view->btn_alpha);
        } else {
            compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_MASK_NEXT+i);
            compo_textbox_set_visible(txt, f_photo_view->btn_alpha > 0);
            compo_textbox_set_alpha(txt, f_photo_view->btn_alpha);
        }
    }
}

void func_photo_play_btn_ui_process(void)
{
    f_photo_view_t *f_photo_view = (f_photo_view_t *)func_cb.f_cb;
    if (f_photo_view->btn_alpha == 0 || (tick_get() - f_photo_view->btn_tick < 2000)) {
        return;
    }
    static u32 ticks = 0;
    if (tick_check_expire(ticks, 1)) {
        ticks = tick_get();
        func_photo_play_btn_ui_set_alpha(f_photo_view->btn_alpha);
        f_photo_view->btn_alpha--;
    }
}

//活动记录功能事件处理
static void func_photo_view_process(void)
{
    compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
    compo_jpg_process(jpg);
    func_photo_play_btn_ui_process();
    func_process();
}

void func_photo_play_btn_handle(void)
{
    f_photo_view_t *f_photo_view = (f_photo_view_t *)func_cb.f_cb;
    f_photo_view->btn_tick = tick_get();
    if (f_photo_view->btn_alpha < 255) {
        func_photo_play_btn_ui_set_alpha(255);
        return;
    }

    int id = compo_get_button_id();
    switch (id) {
    case COMPO_ID_BTN_NEXT:
        {
            compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
            if (!compo_jpg_view_control(jpg, true)) {
                func_cb.sta = task_stack_pop();
            }
        }
        break;

    case COMPO_ID_BTN_PREV:
        {
            compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
            if (!compo_jpg_view_control(jpg, false)) {
                func_cb.sta = task_stack_pop();
            }
        }
        break;

    default:
        break;
    }
}

//活动记录功能消息处理
static void func_photo_view_message(size_msg_t msg)
{
//    f_photo_view_t *f_photo_view = (f_photo_view_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_photo_play_btn_handle();
        break;

    case MSG_QDEC_BACKWARD: {
        printf("MSG_QDEC_BACKWARD\n");
        compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
        compo_jpg_view_scale_control(jpg, true);
    }   break;

    case MSG_QDEC_FORWARD:
        {
            printf("MSG_QDEC_FORWARD\n");
            compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
            compo_jpg_view_scale_control(jpg, false);
        }
        break;

    case KU_LEFT: {
        printf("KU_LEFT\n");
        compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
        if (!compo_jpg_view_control(jpg, true)) {
            func_cb.sta = task_stack_pop();
        }
    }   break;

    case KU_RIGHT: {
        printf("KU_RIGHT\n");
        compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
        if (!compo_jpg_view_control(jpg, false)) {
            func_cb.sta = task_stack_pop();
        }
    }   break;

    case EVT_SD_CARD_INTER: {
        printf("EVT_SD_CARD_INTER\n");
        compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
        os_gui_draw_w4_done();
        compo_jpg_exit(jpg);
        compo_jpg_set_view_style(jpg, COMPO_JPG_TYPE_SD_FATFS);
        const char* path[COMPO_JPG_SUPPORT_DISK_NUM] = {
            [COMPO_JPG_TYPE_SD_FATFS]       = "B:\\PIC",
            [COMPO_JPG_TYPE_FLASH_FATFS]    = "A:\\PIC",
        };
        compo_jpg_view_init_control(jpg, path, true, true);
    }   break;

    case EVT_SD_CARD_OUT: {
        printf("EVT_SD_CARD_OUT\n");
        compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
        os_gui_draw_w4_done();
        compo_jpg_exit(jpg);
        const char* path[COMPO_JPG_SUPPORT_DISK_NUM] = {
            [COMPO_JPG_TYPE_SD_FATFS]       = "B:\\PIC",
            [COMPO_JPG_TYPE_FLASH_FATFS]    = "A:\\PIC",
        };
        compo_jpg_view_init_control(jpg, path, false, true);
    }   break;

//#if PSRAM_SIZE == 0
    case KU_BACK:                           ///使用sram 进行 jpg显示退出只能直接退，不能有动画
    case MSG_CTP_SHORT_RIGHT:
        printf("MSG_CTP_SHORT_RIGHT\n");
        func_directly_back_to();
        break;
//#endif // PSRAM_SIZE

    default:
        func_message(msg);
        break;
    }
}

void func_photo_view_enter(void)
{
    sleep_only_gui_off_en();
    func_cb.frm_main = func_photo_view_form_create();
    func_cb.f_cb = func_zalloc(sizeof(f_photo_view_t));
    compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);

    const char* path[COMPO_JPG_SUPPORT_DISK_NUM] = {
        [COMPO_JPG_TYPE_SD_FATFS]       = "B:\\PIC",
        [COMPO_JPG_TYPE_FLASH_FATFS]    = "A:\\PIC",
    };
    compo_jpg_view_init_control(jpg, path, true, true);
}


void func_photo_view_exit(void)
{
    printf("%s\n", __func__);
    os_gui_draw_w4_done();
    compo_jpg_t* jpg = compo_getobj_byid(COMPO_ID_JPG);
    compo_jpg_exit(jpg);
    const char* path[COMPO_JPG_SUPPORT_DISK_NUM] = {
        [COMPO_JPG_TYPE_SD_FATFS]       = "B:\\PIC",
        [COMPO_JPG_TYPE_FLASH_FATFS]    = "A:\\PIC",
    };
    compo_jpg_view_init_control(jpg, path, false, false);
    sleep_only_gui_off_dis();
}


void func_photo_view(void)
{
    printf("%s\n", __func__);
    func_photo_view_enter();
    while (func_cb.sta == FUNC_PHOTO_VIEW) {
        func_photo_view_process();
        func_photo_view_message(msg_dequeue());
    }
    func_photo_view_exit();
}
#endif // PHOTO_VIEW_EN
