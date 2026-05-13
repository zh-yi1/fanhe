#include "include.h"
#include "func.h"
#include "func_recorder.h"

#if FUNC_RECORDER_EN

enum {
    COMPO_ID_BTN_PLAY = 1,
    COMPO_ID_TXT_MUSIC_NAME,
    COMPO_ID_TXT_TIME_NAME,
};

typedef struct {
    u8 start;
    u32 start_sec;
    u32 frame_cnt;
    u8 rec_type;
} f_rec_t;

//static char fname_buf[100];

void func_recorder_set_play_btn_pic(u8 sta)
{
    compo_button_t *btn = compo_getobj_byid(COMPO_ID_BTN_PLAY);
    if (sta) {
        compo_button_set_bgimg(btn, UI_BUF_MUSIC_PAUSE_BIN);
    } else {
        compo_button_set_bgimg(btn, UI_BUF_MUSIC_PLAY_BIN);
    }
}

void func_recorder_set_title_name(void *buf, u16 id)
{
    compo_textbox_t *tilte_txt = compo_getobj_byid(id);
    compo_textbox_set(tilte_txt, buf);
}

void func_recorder_set_time_text(u32 cur_sec)
{
    char time_str[12];
    tm_t tm;
    f_rec_t *f_rec = (f_rec_t *)func_cb.f_cb;

    if (f_rec->start) {
        compo_textbox_t *tilte_txt = compo_getobj_byid(COMPO_ID_TXT_TIME_NAME);
        tm = time_to_tm(cur_sec - f_rec->start_sec);
        sprintf(time_str, "%02d:%02d:%02d", tm.hour, tm.min, tm.sec);
        compo_textbox_set(tilte_txt, time_str);
    }
}

//创建窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_recorder_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_RECORDER]);

    //text
    compo_textbox_t *name_txt = compo_textbox_create(frm, 50);
    compo_textbox_set_location(name_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 80, 0, 0);
    compo_textbox_set_autosize(name_txt, true);
    compo_setid(name_txt, COMPO_ID_TXT_MUSIC_NAME);
    compo_textbox_set(name_txt, "  ");

    compo_textbox_t *time_txt = compo_textbox_create(frm, 12);
    compo_textbox_set_location(time_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 20, 0, 0);
    compo_textbox_set_autosize(time_txt, true);
    compo_setid(time_txt, COMPO_ID_TXT_TIME_NAME);
    compo_textbox_set(time_txt, "  ");

    //新建按钮
    compo_button_t *btn;
    btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_PLAY_BIN);
    compo_setid(btn, COMPO_ID_BTN_PLAY);
    compo_button_set_pos(btn, 160, 300);

    return frm;
}

//触摸单击按钮
static void func_recorder_button_click(void)
{
    f_rec_t *f_rec = (f_rec_t *)func_cb.f_cb;
    int id = compo_get_button_id();
    switch (id) {

    case COMPO_ID_BTN_PLAY:
        u8 sta = get_bsp_record_sta();
        if (sta == REC_STOP) {
            bsp_record_start(false, f_rec->rec_type, FUNC_REC_SPR, FUNC_REC_BITRATE);
            f_rec->start_sec = RTCCNT;
            f_rec->start = 1;
            func_recorder_set_title_name(rec_file_get_name(), COMPO_ID_TXT_MUSIC_NAME);
        } else if (sta == REC_RECORDING) {
            f_rec->start = 0;
            bsp_record_pause();
        } else if (sta == REC_PAUSE) {
            bsp_record_continue();
            f_rec->start = 1;
            func_recorder_set_title_name(rec_file_get_name(), COMPO_ID_TXT_MUSIC_NAME);
        } else {
//            bsp_record_stop();
//            f_rec->start = 0;
//            func_recorder_set_title_name("  ", COMPO_ID_TXT_MUSIC_NAME);
//            func_recorder_set_title_name("  ", COMPO_ID_TXT_TIME_NAME);
        }
        func_recorder_set_play_btn_pic(f_rec->start);
        break;

    default:
        break;
    }
}

//本地音乐消息处理
static void func_recorder_message(size_msg_t msg)
{
    f_rec_t *f_rec = (f_rec_t *)func_cb.f_cb;

    switch (msg) {
    case MSG_CTP_CLICK:
        func_recorder_button_click();                       //单击按钮
        break;

    case MSG_SYS_500MS: {
            u8 sta = get_bsp_record_sta();
            if (sta == REC_RECORDING) {
                func_recorder_set_time_text(RTCCNT);                //录音时间
                func_recorder_set_play_btn_pic(1);
            } else {
                func_recorder_set_play_btn_pic(0);
            }
        } break;

    case KU_LEFT: {
            u8 sta = get_bsp_record_sta();
            if (sta != REC_STOP) {
                printf("KD_MODE\n");
                bsp_record_set_mode(!bsp_record_get_mode());
            }
        } break;

    case KU_BACK: {
            u8 sta = get_bsp_record_sta();
            if (sta == REC_STOP) {
                f_rec->rec_type = bsp_record_get_next_type(f_rec->rec_type);
                printf("Set Rec type:%d\n", f_rec->rec_type);
            }
        } break;

    default:
        func_message(msg);
        break;
    }
}


void func_recorder_process(void)
{
    func_process();
    bsp_record_process();
}

void func_recorder_enter(void)
{
    f_rec_t *f_rec;
    func_cb.f_cb = func_zalloc(sizeof(f_rec_t));
    func_cb.frm_main = func_recorder_form_create();
    f_rec = (f_rec_t *)func_cb.f_cb;
    f_rec->start = 0;
    func_recorder_set_play_btn_pic(0);
    f_rec->rec_type = bsp_record_get_type();
    bsp_record_enter();
}

void func_recorder_exit(void)
{
    bsp_record_exit();
    func_cb.last = FUNC_RECORDER;
}

void func_recorder(void)
{
    printf("%s\n", __func__);

    func_recorder_enter();

    while (func_cb.sta == FUNC_RECORDER) {
        func_recorder_process();
        func_recorder_message(msg_dequeue());
    }

    func_recorder_exit();
}

#endif  //FUNC_RECORDER_EN
