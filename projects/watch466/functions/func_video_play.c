#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

typedef struct f_video_play_t_ {
    u8 btn_visible;
    u32 btn_tick;
} f_video_play_t;

enum {
    COMPO_ID_VIDEO = 1,
    COMPO_ID_BTN_NEXT,
    COMPO_ID_BTN_PREV,
    COMPO_ID_BTN_VOL_UP,
    COMPO_ID_BTN_VOL_DOWN,
    COMPO_ID_MAX,
};

u32 api_video_play_get_times(void);
u32 api_video_play_get_total_times(void);
void func_video_play_init(bool type);
void avi_audio_exsit_set(bool have);

bool func_is_video_play(void)
{
    return (func_cb.sta == FUNC_VIDEO_PLAY);
}

void func_video_play_btn_ui_set_visible(bool visible)
{
    f_video_play_t *f_video_play = (f_video_play_t *)func_cb.f_cb;
    u8 num = COMPO_ID_MAX - COMPO_ID_BTN_NEXT;
    for(u8 i=0;i<num;i++) {
        compo_button_t *btn = compo_getobj_byid(COMPO_ID_BTN_NEXT+i);
        compo_button_set_visible(btn, visible);
    }

    f_video_play->btn_visible = visible;
}

const video_list_t video_list[] = {
//    [0] = {UI_BUF_AVI_1_AVI, UI_LEN_AVI_1_AVI},
};

//创建指南针窗体
compo_form_t *func_video_play_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    compo_video_t *video = compo_video_create(frm, 0);
    compo_video_set_flash_res_list(video, video_list, sizeof(video_list)/sizeof(video_list_t));
    compo_setid(video, COMPO_ID_VIDEO);
    compo_video_set_size(video, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    compo_video_set_pos(video, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);

    // compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_VOLUME_DOWN_CLICK_BIN);
    // compo_setid(btn, COMPO_ID_BTN_VOL_DOWN);
    // compo_button_set_pos(btn, 80, 110);
    // compo_button_set_alpha(btn, 127);

    // btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_VOLUME_UP_CLICK_BIN);
    // compo_setid(btn, COMPO_ID_BTN_VOL_UP);
    // compo_button_set_pos(btn, 386, 110);
    // compo_button_set_alpha(btn, 127);

    // btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_PREV_CLICK_BIN);
    // compo_setid(btn, COMPO_ID_BTN_PREV);
    // compo_button_set_pos(btn, 80, GUI_SCREEN_HEIGHT - 110);
    // compo_button_set_alpha(btn, 127);

    // btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_NEXT_CLICK_BIN);
    // compo_setid(btn, COMPO_ID_BTN_NEXT);
    // compo_button_set_pos(btn, 386, GUI_SCREEN_HEIGHT - 110);
    // compo_button_set_alpha(btn, 127);



    return frm;
}

void func_video_play_btn_ui_process(void)
{
    f_video_play_t *f_video_play = (f_video_play_t *)func_cb.f_cb;

    if (tick_check_expire(f_video_play->btn_tick, 2000) && f_video_play->btn_visible) {
        func_video_play_btn_ui_set_visible(false);
    }

}

void func_video_play_video_control(bool next)
{
    compo_video_t *video = compo_getobj_byid(COMPO_ID_VIDEO);

    if (!compo_video_play_control(video, next)) {
        func_cb.sta = task_stack_pop();
    }
}

void func_video_play_video_process(void)
{
    if (api_video_play_get_times() >= api_video_play_get_total_times()) {
        func_video_play_video_control(true);
    }
}

//指南针功能事件处理
static void func_video_play_process(void)
{
    compo_video_t *video = compo_getobj_byid(COMPO_ID_VIDEO);
    compo_video_process(video);
    func_video_play_btn_ui_process();
    func_video_play_video_process();
    func_process();
}

void func_video_play_btn_handle(void)
{
    f_video_play_t *f_video_play = (f_video_play_t *)func_cb.f_cb;
    f_video_play->btn_tick = tick_get();

    if (!f_video_play->btn_visible) {
        func_video_play_btn_ui_set_visible(true);
        return;
    }

    int id = compo_get_button_id();

    switch (id) {
    case COMPO_ID_BTN_NEXT:
        func_video_play_video_control(true);
        break;

    case COMPO_ID_BTN_PREV:
        func_video_play_video_control(false);
        break;

    case COMPO_ID_BTN_VOL_UP:
        bsp_set_volume(bsp_volume_inc(sys_cb.vol));
        break;

    case COMPO_ID_BTN_VOL_DOWN:
        bsp_set_volume(bsp_volume_dec(sys_cb.vol));
        break;

    default:
        break;
    }
}

//指南针功能消息处理
static void func_video_play_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_video_play_btn_handle();
        break;

    case KU_BACK:
        printf("KU_BACK\n");

        api_video_play_pause();
        break;

    case MSG_QDEC_BACKWARD:
        printf("MSG_QDEC_BACKWARD\n");
        api_video_play_set_times(api_video_play_get_times() + 3000);
        msg_queue_detach(MSG_QDEC_FORWARD, 0);
        break;

    case MSG_QDEC_FORWARD:
        printf("MSG_QDEC_FORWARD\n");
        if(api_video_play_get_times() > 3000){
            api_video_play_set_times(api_video_play_get_times() - 3000);
            msg_queue_detach(MSG_QDEC_BACKWARD, 0);
        }
        break;

    case KU_LEFT:
        printf("KU_LEFT\n");
        func_video_play_video_control(true);
        break;

    case KU_RIGHT:
        printf("KU_RIGHT\n");
        func_video_play_video_control(false);
        break;

    case EVT_SD_CARD_INTER: {
        printf("EVT_SD_CARD_INTER\n");
        compo_video_t *video = compo_getobj_byid(COMPO_ID_VIDEO);
        os_gui_draw_w4_done();
        compo_video_exit(video);
        compo_video_set_play_style(video, COMPO_JPG_TYPE_SD_FATFS);
        const char* path[] = {
            [COMPO_VIDEO_TYPE_SD_FATFS]         = "B:\\DCIM",
            [COMPO_VIDEO_TYPE_FLASH_FATFS]      = "A:\\DCIM",
        };
        if (!compo_video_play_init_control(video, path, true, true)) {
            func_cb.sta = task_stack_pop();
        }
    }   break;

    case EVT_SD_CARD_OUT: {
        printf("EVT_SD_CARD_OUT\n");
        compo_video_t *video = compo_getobj_byid(COMPO_ID_VIDEO);
        os_gui_draw_w4_done();
        compo_video_exit(video);
        const char* path[] = {
            [COMPO_VIDEO_TYPE_SD_FATFS]         = "B:\\DCIM",
            [COMPO_VIDEO_TYPE_FLASH_FATFS]      = "A:\\DCIM",
        };

        if (!compo_video_play_init_control(video, path, false, true)) {
            func_cb.sta = task_stack_pop();
        }
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

//进入指南针功能
void func_video_play_enter(void)
{
    printf("%s\n", __func__);

    sleep_only_gui_off_en();
    func_cb.f_cb = func_zalloc(sizeof(f_video_play_t));
    f_video_play_t *f_video_play = (f_video_play_t *)func_cb.f_cb;

    func_cb.frm_main = func_video_play_form_create();
    f_video_play->btn_tick = tick_get();
    f_video_play->btn_visible = true;
    compo_video_t *video = compo_getobj_byid(COMPO_ID_VIDEO);

    const char* path[] = {
        [COMPO_VIDEO_TYPE_SD_FATFS]         = "B:\\DCIM",
        [COMPO_VIDEO_TYPE_FLASH_FATFS]      = "A:\\DCIM",
    };

    if (!compo_video_play_init_control(video, path, true, true)) {
        func_cb.sta = task_stack_pop();
    }
}

//退出指南针功能
void func_video_play_exit(void)
{
    printf("%s\n", __func__);
    func_cb.last = FUNC_VIDEO_PLAY;
    compo_video_t *video = compo_getobj_byid(COMPO_ID_VIDEO);
    compo_video_exit(video);

    const char* path[] = {
        [COMPO_VIDEO_TYPE_SD_FATFS]         = "B:\\DCIM",
        [COMPO_VIDEO_TYPE_FLASH_FATFS]      = "A:\\DCIM",
    };
    compo_video_play_init_control(video, path, false, false);
    sleep_only_gui_off_dis();
//    printf("avi_audio_exsit_get:%d, bsp_bt_disp_status:%d\n", avi_audio_exsit_get(), bsp_bt_disp_status());
    if (avi_audio_exsit_get()) {
        avi_audio_exsit_set(false);
        if (sbc_is_bypass() /*&& bsp_bt_disp_status() > BT_STA_CONNECTED*/) {
            printf("-->enable\n");
            bt_audio_enable();
        } else {
            bsp_sys_mute();
        }
    }

}

//指南针功能
void func_video_play(void)
{
    printf("%s\n", __func__);
    func_video_play_enter();
    while (func_cb.sta == FUNC_VIDEO_PLAY) {
        func_video_play_process();
        func_video_play_message(msg_dequeue());
    }
    func_video_play_exit();
}
