#ifndef _HOME_UI_LOCK_OVERLAY_H
#define _HOME_UI_LOCK_OVERLAY_H

#include "include.h"

#if ELUNCHBOX_PANEL_EN

/** 在 form 创建完成后预建 overlay 控件（隐藏），保证 z-order 与 compo 注册 */
void home_ui_lock_overlay_prepare(compo_form_t *frm);

/** 显示/隐藏锁键全屏提示（白半透明底 + 居中锁/解锁图标） */
void home_ui_lock_overlay_show(bool unlock_icon);
void home_ui_lock_overlay_hide(void);

/** 提示显示期间每帧置顶，避免被其它控件盖住 */
void home_ui_lock_overlay_bring_front(void);

/** 切页/销毁 form 后调用，下次 show 会重新挂载 */
void home_ui_lock_overlay_reset(void);

/** 查询 overlay 当前是否可见（用于 poll 中判断是否需重建） */
bool home_ui_lock_overlay_is_visible(void);

#else

static inline void home_ui_lock_overlay_prepare(compo_form_t *frm) { (void)frm; }
static inline void home_ui_lock_overlay_show(bool unlock_icon) { (void)unlock_icon; }
static inline void home_ui_lock_overlay_hide(void) {}
static inline void home_ui_lock_overlay_bring_front(void) {}
static inline void home_ui_lock_overlay_reset(void) {}
static inline bool home_ui_lock_overlay_is_visible(void) { return false; }

#endif

#endif
