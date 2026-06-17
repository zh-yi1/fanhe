#include "include.h"
#include "func.h"
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if AVI_DVP_DEMOLIST

enum {
    COMPO_ID_LISTBOX = 1,
};

typedef struct f_3d_showlist_t_ {
    compo_listbox_t* listbox;
    compo_listbox_move_cb_t mcb;
} f_3d_showlist_t;


#define TBL_3D_SHOWLIST_CNT     ((int)(sizeof(tbl_3d_showlist)/sizeof(tbl_3d_showlist[0])))
static const compo_listbox_item_t tbl_3d_showlist[] = {
    {
#if VIDEO_PLAY_EN
        STR_VIDEO_PLAY,
#else
        STR_UNKNOWN,
#endif // VIDEO_PLAY_EN
        //UI_BUF_SETTING_ABOUT_BIN,

#if VIDEO_PLAY_EN
        .func_sta = FUNC_VIDEO_PLAY
#else
        .func_sta = FUNC_NULL
#endif // VIDEO_PLAY_EN
    },

    {
#if PHOTO_VIEW_EN
        STR_PHOTO_VIEW,
#else
        STR_UNKNOWN,
#endif // PHOTO_VIEW_EN
        //UI_BUF_SETTING_ABOUT_BIN,
#if PHOTO_VIEW_EN
        .func_sta = FUNC_PHOTO_VIEW
#else
        .func_sta = FUNC_NULL
#endif // PHOTO_VIEW_EN
    },

    {

        STR_GIF_VIEW,
        //UI_BUF_SETTING_ABOUT_BIN,
        .func_sta = FUNC_GIF
    },

    {
#if VIDEO_RECODE_TAKE_PHOTO_EN
        STR_VIDEO_RECODE,
#else
        STR_UNKNOWN,
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
       // UI_BUF_SETTING_ABOUT_BIN,
#if VIDEO_RECODE_TAKE_PHOTO_EN
        .func_sta = FUNC_VIDEO_RECODE
#else
        .func_sta = FUNC_NULL
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
    },
    {
#if VIDEO_RECODE_TAKE_PHOTO_EN
        STR_TAKE_PHOTO,
#else
        STR_UNKNOWN,
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
       // UI_BUF_SETTING_ABOUT_BIN,
#if VIDEO_RECODE_TAKE_PHOTO_EN
        .func_sta = FUNC_TAKE_PHOTO
#else
        .func_sta = FUNC_NULL
#endif // VIDEO_RECODE_TAKE_PHOTO_EN
    },
};

compo_form_t *func_video_showlist_form_create(void)
{
    compo_form_t* frm = compo_form_create(true);

    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_VIDEO_MODE]);

    compo_listbox_t* listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE);
    compo_listbox_set_rect_size(listbox, GUI_SCREEN_WIDTH, 120, 60);
    compo_listbox_set_rect_color(listbox, make_color(29, 29, 29));
    compo_listbox_set_rect_visible(listbox, true);
    compo_listbox_set(listbox, tbl_3d_showlist, TBL_3D_SHOWLIST_CNT);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_focus(listbox, 140);
    compo_listbox_update(listbox);

    return frm;
}

static void func_3d_showlist_click(void)
{
    printf("%s\n", __func__);
    int icon_idx;
    f_3d_showlist_t *f_set = (f_3d_showlist_t*)func_cb.f_cb;
    compo_listbox_t *listbox = f_set->listbox;
    u8 func_sta;

    icon_idx = compo_listbox_select(listbox, ctp_get_sxy());
    printf("%s->idx[%d]\n", __func__, icon_idx);
    if (icon_idx < 0 || icon_idx >= TBL_3D_SHOWLIST_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    func_sta = tbl_3d_showlist[icon_idx].func_sta;
    printf("%s->sta[%d]\n", __func__, func_sta);

    //切入应用 直接转跳防止两个列表转跳死机情况
    if (func_sta > 0) {
//        compo_form_t *frm = func_create_form(func_sta);
//        func_switching(FUNC_SWITCH_ZOOM_FADE | FUNC_SWITCH_AUTO, listbox->sel_icon);
//        compo_form_destroy(frm);
        func_cb.sta = func_sta;
    }
}

static void func_3d_showlist_process(void)
{
    f_3d_showlist_t* f_set = (f_3d_showlist_t*)func_cb.f_cb;
    compo_listbox_move(f_set->listbox);
    func_process();
}

void func_video_showlist_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_3d_showlist_t));
    func_cb.frm_main = func_video_showlist_form_create();

    f_3d_showlist_t* f_set = (f_3d_showlist_t*)func_cb.f_cb;
    f_set->listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_t* listbox = f_set->listbox;
    if (listbox->type != COMPO_TYPE_LISTBOX) {
        halt(HALT_GUI_COMPO_LISTBOX_TYPE);
    }
    listbox->mcb = &f_set->mcb;        //建立移动控制块，退出时需要释放

    compo_listbox_move_init_modify(listbox, listbox->ofs_y, compo_listbox_gety_byidx(listbox, TBL_3D_SHOWLIST_CNT - 2));
    func_cb.enter_tick = tick_get();

}

static void func_3d_showlist_message(size_msg_t msg)
{
    f_3d_showlist_t* f_set = (f_3d_showlist_t*)func_cb.f_cb;
    compo_listbox_t* listbox = f_set->listbox;

    if (!func_cb.flag_sort && compo_listbox_message(listbox, msg)) {
        return;
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        func_3d_showlist_click();
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

void func_video_showlist_exit(void)
{
    printf("%s\n", __func__);
    func_cb.last = FUNC_VIDEO_SHOWLIST;
}

void func_video_showlist(void)
{
    func_video_showlist_enter();
    while(func_cb.sta == FUNC_VIDEO_SHOWLIST) {
        func_3d_showlist_process();
        func_3d_showlist_message(msg_dequeue());
    }
    func_video_showlist_exit();
}

#endif // EN_3D_SHOWLIST
