#ifndef _HOME_UI_LOWBAT_OVERLAY_H
#define _HOME_UI_LOWBAT_OVERLAY_H

#include "include.h"
#include "func_lowbat.h"

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_LOWBAT_MODE_EN

static inline void home_ui_lowbat_overlay_show(void) {}
static inline void home_ui_lowbat_overlay_hide(void) {}
static inline void home_ui_lowbat_overlay_bring_front(void) {}
static inline bool home_ui_lowbat_overlay_is_visible(void)
{
    return elunchbox_lowbat_active();
}

static inline void home_ui_lowbat_overlay_reset(void)
{
    elunchbox_lowbat_reset();
}

static inline void home_ui_lowbat_poll(void)
{
    elunchbox_lowbat_poll();
}

#else

static inline void home_ui_lowbat_overlay_show(void) {}
static inline void home_ui_lowbat_overlay_hide(void) {}
static inline void home_ui_lowbat_overlay_bring_front(void) {}
static inline void home_ui_lowbat_overlay_reset(void) {}
static inline bool home_ui_lowbat_overlay_is_visible(void) { return false; }
static inline void home_ui_lowbat_poll(void) {}

#endif

#endif
