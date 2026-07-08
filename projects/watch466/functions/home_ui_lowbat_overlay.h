#ifndef _HOME_UI_LOWBAT_OVERLAY_H
#define _HOME_UI_LOWBAT_OVERLAY_H

#include "include.h"

#if ELUNCHBOX_PANEL_EN

/** 低电量(<25%)提示图 didian.bin：显示 5s → 隐藏 60s → 循环 */
void home_ui_lowbat_overlay_show(void);
void home_ui_lowbat_overlay_hide(void);
void home_ui_lowbat_overlay_bring_front(void);
void home_ui_lowbat_overlay_reset(void);
bool home_ui_lowbat_overlay_is_visible(void);

/** 主循环轮询：低电时显示 5s → 隐藏 60s → 循环 */
void home_ui_lowbat_poll(void);

#else

static inline void home_ui_lowbat_overlay_show(void) {}
static inline void home_ui_lowbat_overlay_hide(void) {}
static inline void home_ui_lowbat_overlay_bring_front(void) {}
static inline void home_ui_lowbat_overlay_reset(void) {}
static inline bool home_ui_lowbat_overlay_is_visible(void) { return false; }
static inline void home_ui_lowbat_poll(void) {}

#endif

#endif
