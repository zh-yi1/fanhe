#include "include.h"
#include "func_key_lock.h"
#include "home_ui_lock_overlay.h"

#if ELUNCHBOX_PANEL_EN

void home_ui_lock_overlay_show(bool unlock_icon)
{
    func_lock_page_show(unlock_icon);
}

void home_ui_lock_overlay_hide(void)
{
    func_lock_page_hide();
}

#endif
