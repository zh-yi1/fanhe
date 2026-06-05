#include "include.h"
#include "func.h"
#include "func_music.h"

#if FUNC_MUSIC_EN

enum {
    COMPO_ID_BTN_PREV = 1,
    COMPO_ID_BTN_NEXT,
    COMPO_ID_BTN_PLAY,
    COMPO_ID_BTN_VOL_UP,
    COMPO_ID_BTN_VOL_DOWN,
    COMPO_ID_TXT_MUSIC_NAME,
    COMPO_ID_TXT_MUSIC_TIME,
    COMPO_ID_PIC_MUSIC_VOL,
    COMPO_ID_PLAY_PROC,
};

typedef struct f_music_t_ {
    u8 vol;
    bool sd_music_sta;
    bool first_enter;
} f_music_t;

void func_switch_to_menu(void);
void bsp_emit_start(void);
void emit_fix_cnt_clr(void);

//static bool sd_music_sta;

#if MUSIC_LRC_EN
static void func_show_lrc(char *lrc, u32 mtime)
{
    printf("func_show_lrc:[%02d:%02d.%d] %s\n", mtime/600, (mtime%600)/10, mtime%10, lrc); //todo: set txt comp
}
#endif

bool func_is_music(void)
{
    return (func_cb.sta == FUNC_MUSIC);
}

static void func_music_init(void)
{
    f_music_t *f_msc = (f_music_t *)func_cb.f_cb;
    f_msc->sd_music_sta = true;
    bsp_sd_disk_mount(24);

    memset(&msc_cb, 0, sizeof(msc_cb));
    if(bsp_music_scan_disk(DEV_SDCARD) == false){
//        func_cb.sta = FUNC_NULL;
        f_msc->sd_music_sta = false;
        return;
    }

    func_cb.mp3_res_play = bsp_func_music_mp3_res_play;
    param_msc_num_read();
//    bsp_music_idx_play(msc_cb.file_num);
    f_msc->first_enter = true;
    msc_cb.brkpt_flag = 1;
#if MUSIC_LRC_EN
    msc_cb.lrc_show_callback = func_show_lrc;
#endif
    sys_cb.local_music_sta = true;
    bsp_sys_unmute();
    sleep_only_gui_off_en();
}


void func_music_set_tilte_name(void *buf)
{
    compo_textbox_t *tilte_txt = compo_getobj_byid(COMPO_ID_TXT_MUSIC_NAME);
    compo_textbox_set(tilte_txt, buf);
}

void func_music_set_music_time_text(void)
{
    char time_str[20];
    if (!msc_cb.pause && msc_cb.alltime.min != 0xff) {
        compo_textbox_t *time_txt = compo_getobj_byid(COMPO_ID_TXT_MUSIC_TIME);
        sprintf(time_str, "%02d:%02d / %02d:%02d", msc_cb.curtime.min, msc_cb.curtime.sec, msc_cb.alltime.min, msc_cb.alltime.sec);
        compo_textbox_set(time_txt, time_str);
    }
}

//bool func_music_is_play(void)
//{
//    return sys_cb.local_music_sta;
//}

void func_music_set_play_btn_pic(void)
{
    if (sys_cb.local_music_sta == false) {
        return;
    }
    compo_arc_t *arc = compo_getobj_byid(COMPO_ID_PLAY_PROC);
    compo_button_t *btn = compo_getobj_byid(COMPO_ID_BTN_PLAY);
    if (msc_cb.pause) {
        // compo_button_set_bgimg(btn, UI_BUF_MUSIC_PLAY_BIN);
        // compo_arc_set_visible(arc, false);
    } else {
    //    compo_button_set_bgimg(btn, UI_BUF_MUSIC_PAUSE_BIN);
        compo_arc_set_visible(arc, true);
    }
}

void func_music_set_no_card_ui(void)
{
    char time_str[20];
    compo_textbox_t *tilte_txt = compo_getobj_byid(COMPO_ID_TXT_MUSIC_NAME);
    compo_textbox_set(tilte_txt, i18n[STR_SD_MUSIC_INSET_TF]);
    compo_textbox_t *time_txt = compo_getobj_byid(COMPO_ID_TXT_MUSIC_TIME);
    sprintf(time_str, "%02d:%02d / %02d:%02d", 0, 0, 0, 0);
    compo_textbox_set(time_txt, time_str);
    func_music_set_play_btn_pic();
}


void func_music_set_vol_btn_pic(void)
{
    f_music_t *f_msc = (f_music_t *)func_cb.f_cb;
    compo_picturebox_t *vol_pic;

    if (f_msc->vol == sys_cb.vol) {
        return;
    }
    f_msc->vol = sys_cb.vol;

    vol_pic = compo_getobj_byid(COMPO_ID_PIC_MUSIC_VOL);
    switch (f_msc->vol){
        case 0:
            //compo_picturebox_set(vol_pic, UI_BUF_MUSIC_VOLUME1_BIN);
            compo_picturebox_set_visible(vol_pic, false);
            break;

        case 1 ... 3:
           // compo_picturebox_set(vol_pic, UI_BUF_MUSIC_VOLUME1_BIN);
            compo_picturebox_set_visible(vol_pic, true);
    //        compo_picturebox_set_pos(vol_pic, 110+gui_image_get_size(UI_BUF_MUSIC_VOLUME1_BIN).wid/2 ,335);
            break;

        case 4 ... 6:
           // compo_picturebox_set(vol_pic, UI_BUF_MUSIC_VOLUME2_BIN);
            compo_picturebox_set_visible(vol_pic, true);
           // compo_picturebox_set_pos(vol_pic, 110+gui_image_get_size(UI_BUF_MUSIC_VOLUME2_BIN).wid/2 ,335);

            break;
        case 7 ... 9:
       //     compo_picturebox_set(vol_pic, UI_BUF_MUSIC_VOLUME2_BIN);
            compo_picturebox_set_visible(vol_pic, true);
            //compo_picturebox_set_pos(vol_pic, 110+gui_image_get_size(UI_BUF_MUSIC_VOLUME2_BIN).wid/2 ,335);
            break;
        case 10 ... 11:
            // compo_picturebox_set(vol_pic, UI_BUF_MUSIC_VOLUME2_BIN);
            // compo_picturebox_set_visible(vol_pic, true);
            // compo_picturebox_set_pos(vol_pic, 110+gui_image_get_size(UI_BUF_MUSIC_VOLUME2_BIN).wid/2 ,335);
            break;
        case 12 ... 16:
            // compo_picturebox_set(vol_pic, UI_BUF_MUSIC_VOLUME2_BIN);
            // compo_picturebox_set_visible(vol_pic, true);
            // compo_picturebox_set_pos(vol_pic, 110+gui_image_get_size(UI_BUF_MUSIC_VOLUME2_BIN).wid/2 ,335);
            break;
    }
}

//创建本地音乐播放器窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_music_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE | COMPO_FORM_MODE_SHOW_TIME);
    compo_form_set_title(frm, i18n[STR_MUSIC]);

    //歌名
    compo_textbox_t *name_txt = compo_textbox_create(frm, 50);
    compo_textbox_set_location(name_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 100, GUI_SCREEN_WIDTH, 60);
    compo_setid(name_txt, COMPO_ID_TXT_MUSIC_NAME);
    compo_textbox_set(name_txt, "    ");

    //歌词
    compo_textbox_t *time_txt = compo_textbox_create(frm, 50);
    compo_textbox_set_location(time_txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 30, GUI_SCREEN_WIDTH, 60);
    compo_setid(time_txt, COMPO_ID_TXT_MUSIC_TIME);
    compo_textbox_set(time_txt, "  ");

    //新建按钮
    compo_button_t *btn;
    // btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_PREV_BIN);
    // compo_setid(btn, COMPO_ID_BTN_PREV);
    // compo_button_set_pos(btn, 53, 248);

    // btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_PLAY_BIN);
    // compo_setid(btn, COMPO_ID_BTN_PLAY);
    // compo_button_set_pos(btn, 160, 245);

    // btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_NEXT_BIN);
    // compo_setid(btn, COMPO_ID_BTN_NEXT);
    // compo_button_set_pos(btn, 267, 248);

    // btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_VOLUME_DOWN_BIN);
    // compo_setid(btn, COMPO_ID_BTN_VOL_DOWN);
    // compo_button_set_pos(btn, 62, 340);

    // btn = compo_button_create_by_image(frm, UI_BUF_MUSIC_VOLUME_UP_BIN);
    // compo_setid(btn, COMPO_ID_BTN_VOL_UP);
    // compo_button_set_pos(btn, 258, 340);

    // compo_picturebox_t *vol_pic = compo_picturebox_create(frm, UI_BUF_MUSIC_VOLUME1_BIN);
    // compo_setid(vol_pic, COMPO_ID_PIC_MUSIC_VOL);
    // compo_picturebox_set_pos(vol_pic, 110, 335);
    // widget_set_align_center(vol_pic->img, false);
    // compo_picturebox_set_visible(vol_pic, false);

    // //my test
    // compo_arc_t *arc = compo_arc_create(frm);
    // compo_setid(arc, COMPO_ID_PLAY_PROC);
    // compo_arc_set_alpha(arc, 0xff, 0);
    // compo_arc_set_location(arc, 160, 245, 111, 111);
    // compo_arc_set_width(arc, 3);
    // compo_arc_set_rotation(arc, 0);
    // compo_arc_set_angles(arc, 0, 3600);
    // compo_arc_set_color(arc, make_color(241, 64, 202), make_color(241, 64, 202));
    // compo_arc_set_value(arc, 0);

    return frm;
}

//触摸单击按钮
static void func_music_button_click(void)
{
    f_music_t *f_msc = (f_music_t *)func_cb.f_cb;
    int id = compo_get_button_id();
    switch (id) {
    case COMPO_ID_BTN_PREV:
        if (f_msc->sd_music_sta) {
            bsp_music_prev();
        }
        break;

    case COMPO_ID_BTN_NEXT:
        if (f_msc->sd_music_sta) {
            bsp_music_next();
        }
        break;

    case COMPO_ID_BTN_PLAY:
        printf("COMPO_ID_BTN_PLAY\n");
        if (f_msc->sd_music_sta) {
            bsp_music_play_pause();
        }
        func_music_set_play_btn_pic();
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

//本地音乐消息处理
static void func_music_message(size_msg_t msg)
{
    f_music_t *f_msc = (f_music_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_CLICK:
        func_music_button_click();                         //单击按钮
        break;

    case MSG_CTP_SHORT_RIGHT:
        printf("MSG_CTP_SHORT_RIGHT\n");
        bsp_music_pause();
        func_directly_back_to();
        break;

    case KU_BACK:
        func_switch_to_menu();
        break;

    case MSG_SYS_500MS:
        if (sys_cb.local_music_sta) {
            compo_arc_t *arc = compo_getobj_byid(COMPO_ID_PLAY_PROC);
            u16 total_time = music_get_total_time();
            u16 cur_time = music_get_cur_time() / 10;
            u16 proc = 0;
            if (total_time) {
                proc = ARC_VALUE_MAX * cur_time / total_time;
            }
//            printf("total:%d, cur:%d, proc:%d\n", total_time, cur_time, proc);
            compo_arc_set_value(arc, proc);
            func_music_set_play_btn_pic();

        }
        func_music_set_vol_btn_pic();
        func_music_set_music_time_text();
        break;


    case EVT_SD_CARD_OUT: {
        bsp_music_pause();
        func_music_set_no_card_ui();
        if (!sys_cb.local_music_sta || !MUSCI_BACKSTAGE_EN) {
            music_breakpoint_save();
            music_control(MUSIC_MSG_STOP);
            if (dev_is_online(DEV_SDCARD)) {
                sd0_stop(1);
            }
//    #if BT_BACKSTAGE_MUSIC_EN
//            bt_audio_enable();
//    #endif
            sys_cb.local_music_sta = false;
        }

        bsp_sd_disk_unmount();
        f_msc->sd_music_sta = false;
        sleep_only_gui_off_dis();
    } break;

    case EVT_SD_CARD_INTER: {
        func_music_init();
    } break;

    default:
        func_message(msg);
        break;
    }
}

AT(.text.func.music)
void func_music_process(void)
{
    f_music_t *f_msc = (f_music_t *)func_cb.f_cb;
    func_process();
    if (f_msc->sd_music_sta) {
        bsp_music_process(f_msc->first_enter);
        f_msc->first_enter = false;
    }
    if(msc_cb.fname_update){
        msc_cb.fname_update = 0;
        func_music_set_tilte_name(msc_cb.fname);
    }
}

void func_music_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_music_t));
    f_music_t *f_msc = (f_music_t *)func_cb.f_cb;
    func_cb.frm_main = func_music_form_create();
    func_music_set_play_btn_pic();

#if BT_BACKSTAGE_MUSIC_EN
    bt_audio_bypass();
    if (bt_is_playing()) {
        bt_music_pause();
    }
#endif

    msg_queue_clear();
    if (!dev_is_online(DEV_SDCARD)) {
//        func_cb.sta = FUNC_NULL;
        f_msc->sd_music_sta = false;
        func_music_set_no_card_ui();
        return;
    }
    func_music_init();

}

void func_music_exit(void)
{
    f_music_t *f_msc = (f_music_t *)func_cb.f_cb;
    if (!sys_cb.local_music_sta || !MUSCI_BACKSTAGE_EN) {
        music_breakpoint_save();
        music_control(MUSIC_MSG_STOP);
        if (dev_is_online(DEV_SDCARD)) {
            sd0_stop(1);
        }
#if BT_BACKSTAGE_MUSIC_EN
        bt_audio_enable();
#endif
        sys_cb.local_music_sta = false;
    }

    func_cb.last = FUNC_MUSIC;
    bsp_sd_disk_unmount();
    f_msc->sd_music_sta = false;
    bsp_sys_mute();

}


AT(.text.func.music)
void func_music(void)
{
    printf("%s\n", __func__);

    func_music_enter();

    while (func_cb.sta == FUNC_MUSIC) {
        func_music_process();
        func_music_message(msg_dequeue());
    }
    func_music_exit();
}
#endif // FUNC_MUSIC_EN
