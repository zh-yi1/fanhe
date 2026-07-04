#ifndef _FUNC_NEW_HOME_H
#define _FUNC_NEW_HOME_H

#include "home_top_time_txt.h"

/* 新主页底部 Tab 枚举 */
enum {
    NEW_HOME_TAB_HEAT = 0,
    NEW_HOME_TAB_MODE,
    NEW_HOME_TAB_SETUP,
    NEW_HOME_TAB_CNT,
};

typedef struct f_new_home_t_ {
    u8 last_top_min;
    u8 last_top_sec;
    bool screen_locked;
    u8 cur_tab;                         /* 当前选中的 Tab (NEW_HOME_TAB_xxx) */
    home_top_time_txt_t top_time;
    compo_picturebox_t *pic_logo;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_lock;
    compo_picturebox_t *pic_bat;
    compo_picturebox_t *pic_tab_heat;   /* 底部 Tab 图标 */
    compo_picturebox_t *pic_tab_mode;
    compo_picturebox_t *pic_tab_setup;
    compo_textbox_t *txt_res_marquee;
#if ELUNCHBOX_PANEL_EN
    u8 display_stage;
    u8 pending_switch_sta;   /* 0=无；确认键延后到 process 末再切页（勿在扫键路径 switch） */
#endif
} f_new_home_t;

/* func_key_lock.c 使用与旧 Home 相同的锁图标接口 */
typedef f_new_home_t f_home_t;
void func_home_lock_icon_apply(f_home_t *f_home);

#if ELUNCHBOX_PANEL_EN
void func_home_gpu_detach_before_leave(f_new_home_t *f);
#endif

#endif /* _FUNC_NEW_HOME_H */
