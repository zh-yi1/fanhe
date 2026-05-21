#ifndef _FUNC_CLOCK_H
#define _FUNC_CLOCK_H

//表盘预览类型
#define PREVIEW_ROWBOX_STYLE        1
#define PREVIEW_STACKLIST_STYLE     2
#define PREVIEW_ROTARY_STYLE        3                       //立体转盘效果

#define DP_HEADER_FORMAT   0x4657
#define DP_HEADER          sizeof(dp_header_t)
#define DP_RES_HEADER      sizeof(dp_res_t)
#define UI_BUF_WATCH_DIY   0xffffffff

enum {
    FUNC_CLOCK_MAIN,
    FUNC_CLOCK_SUB_DROPDOWN,
    FUNC_CLOCK_SUB_PULLUP,
    FUNC_CLOCK_SUB_SIDE,
    FUNC_CLOCK_SUB_ROTARY,
};

enum {
    FUNC_CLOCK_NORMAL,
    FUNC_CLOCK_CUBE,
    FUNC_CLOCK_BUTTERFLY,
};

typedef struct f_clock_t_ {
    u8 sta;
    u8 mode;
    u8 switch_to;
    u8 animation_id;
    compo_form_t *sub_frm;
    void *sub_cb;
    compo_shape_t *masklayer;
    compo_picturebox_t *blur_pic;
    bool flag_video_start;
    u32 tick_video_start;
    u32 user_data;
    u32 user_data2;
} f_clock_t;




//主窗体
void func_clock_sub_process(void);
void func_clock_sub_message(size_msg_t msg);

//子菜单
void func_clock_sub_dropdown(void);
void func_clock_sub_pullup(void);
void func_clock_sub_side(void);

//蝴蝶表盘
compo_form_t *func_clock_butterfly_form_create(void);
void func_clock_butterfly_set_light_visible(bool visible);

//表盘转盘
void func_clock_sub_rotary(void);
u32 func_clock_get_dialplate_cube_idx(void);
u32 func_clock_get_dialplate_butterfly_idx(void);

//自屏幕底部区域上滑进入足球菜单（长划，非短划）
#define FUNC_CLOCK_SWIPE_UP_BOTTOM_ZONE    (GUI_SCREEN_HEIGHT - GUI_SCREEN_HEIGHT / 10)
void func_clock_swipe_up_to_football_menu(void);

#endif
