#ifndef _FUNC_LOWBAT_H
#define _FUNC_LOWBAT_H

#include "include.h"

#if ELUNCHBOX_PANEL_EN && ELUNCHBOX_LOWBAT_MODE_EN

compo_form_t *func_lowbat_form_create(void);
void func_lowbat_enter(void);
void func_lowbat_exit(void);
void func_lowbat(void);

bool elunchbox_lowbat_active(void);
void elunchbox_lowbat_poll(void);
void elunchbox_lowbat_feed_dp(u8 *data, u16 len);
void elunchbox_lowbat_reset(void);
bool elunchbox_lowbat_should_block_ui_route(void);

#else

static inline compo_form_t *func_lowbat_form_create(void) { return NULL; }
static inline void func_lowbat_enter(void) {}
static inline void func_lowbat_exit(void) {}
static inline void func_lowbat(void) {}
static inline bool elunchbox_lowbat_active(void) { return false; }
static inline void elunchbox_lowbat_poll(void) {}
static inline void elunchbox_lowbat_feed_dp(u8 *data, u16 len) { (void)data; (void)len; }
static inline void elunchbox_lowbat_reset(void) {}
static inline bool elunchbox_lowbat_should_block_ui_route(void) { return false; }

#endif

#endif
