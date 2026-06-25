#include "include.h"
#include "heat_display_reg.h"

static heat_display_cb_t heat_display_cb;
static heat_display_info_t heat_display_last;
static bool heat_display_has_last;

static void heat_display_notify(void)
{
    if (heat_display_cb != NULL) {
        heat_display_cb(&heat_display_last);
    }
}

void heat_display_register(heat_display_cb_t cb)
{
    heat_display_cb = cb;
}

void heat_display_unregister(void)
{
    heat_display_cb = NULL;
}

bool heat_display_get_last(heat_display_info_t *out)
{
    if (out == NULL || !heat_display_has_last) {
        return false;
    }
    *out = heat_display_last;
    return true;
}

void heat_display_show_schedule(u32 schedule_min)
{
    if (schedule_min > 5999) {
        schedule_min = 5999;
    }

    if (heat_display_has_last && heat_display_last.schedule_min == schedule_min) {
        return;
    }

    heat_display_last.schedule_min = schedule_min;
    heat_display_has_last = true;
    heat_display_notify();
}

void heat_display_show(u32 remain_min, u16 temp_f)
{
    bool changed;

    if (remain_min > 5999) {
        remain_min = 5999;
    }
    if (temp_f > 999) {
        temp_f = 999;
    }

    changed = (!heat_display_has_last
               || heat_display_last.remain_min != remain_min
               || heat_display_last.temp_f != temp_f);
    if (!changed) {
        return;
    }

    heat_display_last.remain_min = remain_min;
    heat_display_last.temp_f = temp_f;
    heat_display_has_last = true;
    heat_display_notify();
}

void heat_display_show_all(u32 schedule_min, u32 remain_min, u16 temp_f)
{
    bool changed;

    if (schedule_min > 5999) {
        schedule_min = 5999;
    }
    if (remain_min > 5999) {
        remain_min = 5999;
    }
    if (temp_f > 999) {
        temp_f = 999;
    }

    changed = (!heat_display_has_last
               || heat_display_last.schedule_min != schedule_min
               || heat_display_last.remain_min != remain_min
               || heat_display_last.temp_f != temp_f);
    if (!changed) {
        return;
    }

    heat_display_last.schedule_min = schedule_min;
    heat_display_last.remain_min = remain_min;
    heat_display_last.temp_f = temp_f;
    heat_display_has_last = true;
    heat_display_notify();
}
