#include "include.h"
#include "func.h"

/* 加热/模式页共用的中部倒计时状态 */

static u32 home_countdown_remain_sec;
static bool home_countdown_running;

void func_home_countdown_set(u8 hour, u8 min)
{
    if (hour > 99) {
        hour = 99;
    }
    if (min > 59) {
        min = 59;
    }
    home_countdown_remain_sec = (u32)hour * 3600 + (u32)min * 60;
}

void func_home_countdown_start(void)
{
    home_countdown_running = true;
}

void func_home_countdown_stop(void)
{
    home_countdown_running = false;
}

u32 func_home_countdown_remain_sec(void)
{
    return home_countdown_remain_sec;
}

bool func_home_heating_countdown_active(void)
{
    return home_countdown_running && home_countdown_remain_sec > 0;
}

void func_home_countdown_tick(void)
{
    if (home_countdown_running && home_countdown_remain_sec > 0) {
        home_countdown_remain_sec--;
        if (home_countdown_remain_sec == 0) {
            home_countdown_running = false;
        }
    }
}

void func_home_countdown_get_display(u8 *hour, u8 *min)
{
    u32 sec = home_countdown_remain_sec;
    u32 h = sec / 3600;

    if (h > 99) {
        h = 99;
    }
    if (hour) {
        *hour = (u8)h;
    }
    if (min) {
        *min = (u8)((sec % 3600) / 60);
    }
}
