#include "include.h"
#include "bsp_pt8028_key.h"

#ifndef USER_PT8028_KEY
#define USER_PT8028_KEY                 0
#endif
#ifndef PT8028_FLAG_ACTIVE_LOW
#define PT8028_FLAG_ACTIVE_LOW          1
#endif
#ifndef PT8028_BCD_STABLE_CNT
#define PT8028_BCD_STABLE_CNT           2
#endif
#ifndef PT8028_PRESS_SETTLE_SCANS
#define PT8028_PRESS_SETTLE_SCANS       2
#endif
#ifndef PT8028_BCD_SAMPLE_CNT
#define PT8028_BCD_SAMPLE_CNT           5
#endif

#if USER_PT8028_KEY
#include "port_pt8028_key.h"

typedef struct {
    u8  press_key;
    u8  press_tch;
    u8  press_bcd;
    u8  last_out_flag;
    u8  press_active;
    u8  press_settle;
    u16 pending_ku;
    u8  bcd_hist[8];
} pt8028_cb_t;

typedef struct {
    volatile u8 pending;
    volatile u8 edge;
    volatile u8 bcd;
    volatile u8 out_flag;
} pt8028_edge_log_t;

typedef struct {
    volatile u8 pending;
    volatile u8 tch;
    volatile u8 key;
    volatile u16 ku;
    volatile u8 bcd;
    volatile u8 out_flag;
} pt8028_key_log_t;

static pt8028_cb_t pt8028_cb AT(.buf.pt8028);
static pt8028_edge_log_t pt8028_edge_log AT(.buf.pt8028);
static pt8028_key_log_t pt8028_key_log AT(.buf.pt8028);

static char pt8028_last_key_str[12] AT(.buf.pt8028) = "none";
static u8 pt8028_sample_bcd AT(.buf.pt8028) = 0xff;
static u8 pt8028_stable_cnt AT(.buf.pt8028);

AT(.com_rodata.bsp.pt8028)
static const char * const tbl_pt8028_short_name[8] = {
    "锁", "热", "减", "模式", "确认", "开", "加", "约",
};

/* 表2：OUT_FLAG=0 有键；OUT_FLAG=1 且 D=111 为空闲，不是 TCH7 */
AT(.com_text.bsp.pt8028)
static u8 pt8028_read_out_flag(void)
{
    u8 flag = bsp_gpio_get_sta(PT8028_GPIO_OUT_FLAG) ? 1 : 0;

#if PT8028_FLAG_ACTIVE_LOW
    return flag;
#else
    return flag ? 0 : 1;
#endif
}

AT(.com_text.bsp.pt8028)
static bool pt8028_is_pressed(void)
{
    return pt8028_read_out_flag() == 0;
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_read_bcd_raw(void)
{
    u8 d0 = bsp_gpio_get_sta(PT8028_GPIO_D0) ? 1 : 0;
    u8 d1 = bsp_gpio_get_sta(PT8028_GPIO_D1) ? 1 : 0;
    u8 d2 = bsp_gpio_get_sta(PT8028_GPIO_D2) ? 1 : 0;

    return (d2 << 2) | (d1 << 1) | d0;
}

AT(.com_text.bsp.pt8028)
static bool pt8028_bcd_pressed_valid(u8 bcd)
{
    return (bcd <= PT8028_KEY_TCH7);
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_hist_best(const u8 *hist)
{
    u8 i;
    u8 best_bcd = 0xff;
    u8 best_cnt = 0;

    for (i = 0; i <= PT8028_KEY_TCH7; i++) {
        if (hist[i] > best_cnt) {
            best_cnt = hist[i];
            best_bcd = i;
        }
    }
    return best_bcd;
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_read_bcd_pressed(void)
{
    u8 i;
    u8 hist[8];
    u8 bcd;

    if (!pt8028_is_pressed()) {
        return 0xff;
    }

    memset(hist, 0, sizeof(hist));
    for (i = 0; i < PT8028_BCD_SAMPLE_CNT; i++) {
        bcd = pt8028_read_bcd_raw();
        if (pt8028_bcd_pressed_valid(bcd)) {
            hist[bcd]++;
        }
    }

    bcd = pt8028_hist_best(hist);
    if (pt8028_bcd_pressed_valid(bcd)) {
        return bcd;
    }
    return pt8028_read_bcd_raw();
}

AT(.com_text.bsp.pt8028)
static bool pt8028_bcd_is_stable(void)
{
    return (pt8028_stable_cnt >= PT8028_BCD_STABLE_CNT);
}

AT(.com_text.bsp.pt8028)
static void pt8028_bcd_track_stable(u8 bcd)
{
    if (!pt8028_bcd_pressed_valid(bcd)) {
        pt8028_sample_bcd = 0xff;
        pt8028_stable_cnt = 0;
        return;
    }
    if (bcd == pt8028_sample_bcd) {
        if (pt8028_stable_cnt < 255) {
            pt8028_stable_cnt++;
        }
    } else {
        pt8028_sample_bcd = bcd;
        pt8028_stable_cnt = 1;
    }
    if (pt8028_bcd_is_stable()) {
        pt8028_cb.press_bcd = bcd;
        pt8028_cb.bcd_hist[bcd]++;
    }
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_bcd_pick_release(void)
{
    u8 hist_bcd = pt8028_hist_best(pt8028_cb.bcd_hist);

    if (pt8028_bcd_pressed_valid(pt8028_cb.press_bcd)) {
        return pt8028_cb.press_bcd;
    }
    if (pt8028_bcd_is_stable() && pt8028_bcd_pressed_valid(pt8028_sample_bcd)) {
        return pt8028_sample_bcd;
    }
    hist_bcd = pt8028_hist_best(pt8028_cb.bcd_hist);
    if (pt8028_bcd_pressed_valid(hist_bcd) && hist_bcd != PT8028_KEY_TCH7) {
        return hist_bcd;
    }
    return 0xff;
}

AT(.com_text.bsp.pt8028)
static void pt8028_hist_clear(void)
{
    memset(pt8028_cb.bcd_hist, 0, sizeof(pt8028_cb.bcd_hist));
}

AT(.com_text.bsp.pt8028)
static void pt8028_queue_edge(u8 edge, u8 bcd, u8 out_flag)
{
    pt8028_edge_log.edge = edge;
    pt8028_edge_log.bcd = bcd;
    pt8028_edge_log.out_flag = out_flag;
    pt8028_edge_log.pending = 1;
}

AT(.com_text.bsp.pt8028)
static void pt8028_queue_key(u8 tch, u8 key, u16 ku, u8 bcd, u8 out_flag)
{
    pt8028_key_log.tch = tch;
    pt8028_key_log.key = key;
    pt8028_key_log.ku = ku;
    pt8028_key_log.bcd = bcd;
    pt8028_key_log.out_flag = out_flag;
    pt8028_key_log.pending = 1;
}

AT(.com_text.bsp.pt8028)
static void pt8028_emit_short_up(u8 out_flag)
{
    u8 key;
    u8 bcd = pt8028_bcd_pick_release();

    if (!pt8028_bcd_pressed_valid(bcd)) {
        pt8028_key_reject_print(0xff, pt8028_cb.press_bcd,
                                pt8028_hist_best(pt8028_cb.bcd_hist), out_flag);
        return;
    }

    key = tbl_pt8028_bcd_to_key[bcd];
    pt8028_cb.press_tch = bcd;
    pt8028_cb.press_key = key;
    pt8028_cb.pending_ku = (u16)(key | KEY_SHORT_UP);
    pt8028_queue_key(bcd, key, pt8028_cb.pending_ku, bcd, out_flag);

    if (bcd <= PT8028_KEY_TCH7) {
        strcpy(pt8028_last_key_str, tbl_pt8028_short_name[bcd]);
    }
}

AT(.text.bsp.pt8028)
void pt8028_gpio_ensure(void)
{
    /* bsp_io_init/gui 可能清掉 GPIOE，须 4 脚 DE 全置位才认为有效 */
    const u32 pe_mask = (BIT(1) | BIT(2) | BIT(3) | BIT(4));

    if ((GPIOEDE & pe_mask) != pe_mask) {
        pt8028_port_gpio_init();
    }
}

AT(.text.key.init)
void pt8028_key_init(void)
{
#if ELUNCHBOX_PANEL_EN
    FUNCMCON0 = (FUNCMCON0 & ~0xF) | SD0MAP_NONE;
#endif
    memset(&pt8028_cb, 0, sizeof(pt8028_cb));
    pt8028_cb.press_tch = PT8028_KEY_NONE;
    pt8028_cb.press_key = NO_KEY;
    pt8028_cb.press_bcd = 0xff;
    pt8028_cb.last_out_flag = pt8028_read_out_flag();
    pt8028_gpio_ensure();
}

AT(.com_text.bsp.pt8028)
u8 get_pt8028_key(void)
{
    u8 out_flag = pt8028_read_out_flag();
    u8 key_val = NO_KEY;
    u8 bcd;

    pt8028_gpio_ensure();

    if (pt8028_is_pressed()) {
        if (pt8028_cb.last_out_flag == 1) {
            pt8028_cb.press_active = 1;
            pt8028_cb.press_bcd = 0xff;
            pt8028_cb.press_settle = PT8028_PRESS_SETTLE_SCANS;
            pt8028_sample_bcd = 0xff;
            pt8028_stable_cnt = 0;
            pt8028_hist_clear();
            bcd = pt8028_read_bcd_pressed();
            pt8028_queue_edge(0, bcd, out_flag);
        }
        if (pt8028_cb.press_settle > 0) {
            pt8028_cb.press_settle--;
        } else {
            bcd = pt8028_read_bcd_pressed();
            pt8028_bcd_track_stable(bcd);
        }
        if (pt8028_bcd_is_stable() && pt8028_bcd_pressed_valid(pt8028_cb.press_bcd)) {
            pt8028_cb.press_tch = pt8028_cb.press_bcd;
            pt8028_cb.press_key = tbl_pt8028_bcd_to_key[pt8028_cb.press_bcd];
            key_val = pt8028_cb.press_key;
        }
    } else {
        if (pt8028_cb.last_out_flag == 0) {
            if (!pt8028_bcd_pressed_valid(pt8028_cb.press_bcd) &&
                pt8028_bcd_is_stable() &&
                pt8028_bcd_pressed_valid(pt8028_sample_bcd)) {
                pt8028_cb.press_bcd = pt8028_sample_bcd;
            }
            pt8028_queue_edge(1, pt8028_cb.press_bcd, out_flag);
            if (pt8028_cb.press_active) {
                pt8028_emit_short_up(out_flag);
            }
        }
        pt8028_cb.press_active = 0;
        pt8028_cb.press_settle = 0;
        pt8028_sample_bcd = 0xff;
        pt8028_stable_cnt = 0;
        pt8028_cb.press_bcd = 0xff;
        pt8028_cb.press_tch = PT8028_KEY_NONE;
        pt8028_cb.press_key = NO_KEY;
        pt8028_hist_clear();
        key_val = NO_KEY;
    }

    pt8028_cb.last_out_flag = out_flag;
    return key_val;
}

AT(.com_text.bsp.pt8028)
u16 pt8028_pop_short_up(void)
{
    u16 ku = pt8028_cb.pending_ku;

    pt8028_cb.pending_ku = NO_KEY;
    return ku;
}

AT(.com_text.bsp.pt8028)
bool pt8028_key_busy(void)
{
    return (pt8028_cb.press_active && pt8028_is_pressed()) ||
           (pt8028_cb.pending_ku != NO_KEY);
}

AT(.com_text.bsp.pt8028)
u8 pt8028_get_press_tch(void)
{
    if (!pt8028_is_pressed() || pt8028_cb.press_settle > 0 || !pt8028_bcd_is_stable()) {
        return PT8028_KEY_NONE;
    }
    if (pt8028_bcd_pressed_valid(pt8028_cb.press_bcd)) {
        return pt8028_cb.press_bcd;
    }
    return PT8028_KEY_NONE;
}

AT(.com_text.bsp.pt8028)
u8 pt8028_get_led_tch(void)
{
    u8 bcd;

    /* LED：settle 后即时多数表决，不要求 full stable（避免长时间不亮） */
    if (!pt8028_is_pressed() || pt8028_cb.press_settle > 0) {
        return PT8028_KEY_NONE;
    }

    bcd = pt8028_read_bcd_pressed();
    if (bcd <= PT8028_KEY_TCH6) {
        /* TCH1(001) 易为过渡毛刺，稍等 1~2 次扫描 */
        if (bcd == PT8028_KEY_TCH1 && pt8028_stable_cnt < 2) {
            if (pt8028_bcd_pressed_valid(pt8028_cb.press_bcd) &&
                pt8028_cb.press_bcd != PT8028_KEY_TCH1) {
                return pt8028_cb.press_bcd;
            }
            return PT8028_KEY_NONE;
        }
        return bcd;
    }
    if (bcd == PT8028_KEY_TCH7 && pt8028_bcd_is_stable()) {
        return bcd;
    }
    if (pt8028_bcd_pressed_valid(pt8028_cb.press_bcd)) {
        return pt8028_cb.press_bcd;
    }
    return PT8028_KEY_NONE;
}

AT(.text.bsp.pt8028)
void pt8028_get_raw_state(u8 *out_flag, u8 *bcd, u8 *d0, u8 *d1, u8 *d2)
{
    u8 td0 = bsp_gpio_get_sta(PT8028_GPIO_D0) ? 1 : 0;
    u8 td1 = bsp_gpio_get_sta(PT8028_GPIO_D1) ? 1 : 0;
    u8 td2 = bsp_gpio_get_sta(PT8028_GPIO_D2) ? 1 : 0;
    u8 tflag = bsp_gpio_get_sta(PT8028_GPIO_OUT_FLAG) ? 1 : 0;

    if (d0) {
        *d0 = td0;
    }
    if (d1) {
        *d1 = td1;
    }
    if (d2) {
        *d2 = td2;
    }
    if (bcd) {
        *bcd = (td2 << 2) | (td1 << 1) | td0;
    }
    if (out_flag) {
#if PT8028_FLAG_ACTIVE_LOW
        *out_flag = tflag;
#else
        *out_flag = tflag ? 0 : 1;
#endif
    }
}

const char *pt8028_get_last_key_str(void)
{
    return pt8028_last_key_str;
}

AT(.text.bsp.pt8028)
void pt8028_log_flush(void)
{
    pt8028_edge_log_t edge;
    pt8028_key_log_t key;

#if PT8028_KEY_DEBUG
    {
        static u8 last_flag = 0xff;
        u8 flag, bcd, d0, d1, d2;

        pt8028_get_raw_state(&flag, &bcd, &d0, &d1, &d2);
        if (flag != last_flag) {
            printf("PT8028 chg: FLAG,D2,D1,D0=%d,%d,%d,%d TCH%d\n",
                   flag, d2, d1, d0, bcd);
            last_flag = flag;
        }
    }
#endif

    if (pt8028_edge_log.pending) {
        edge.pending = pt8028_edge_log.pending;
        edge.edge = pt8028_edge_log.edge;
        edge.bcd = pt8028_edge_log.bcd;
        edge.out_flag = pt8028_edge_log.out_flag;
        pt8028_edge_log.pending = 0;
        pt8028_key_edge_print(edge.edge, edge.bcd, edge.out_flag);
    }

    if (pt8028_key_log.pending) {
        key.pending = pt8028_key_log.pending;
        key.tch = pt8028_key_log.tch;
        key.key = pt8028_key_log.key;
        key.ku = pt8028_key_log.ku;
        key.bcd = pt8028_key_log.bcd;
        key.out_flag = pt8028_key_log.out_flag;
        pt8028_key_log.pending = 0;
        pt8028_key_name_print(key.tch, key.key, key.ku, key.bcd, key.out_flag);
    }
}

AT(.text.bsp.pt8028)
void pt8028_gpio_monitor(void)
{
#if PT8028_GPIO_MONITOR_EN
    static u8 last_flag = 0xff;
    static u32 last_tick;
    u8 flag, bcd, d0, d1, d2;

    pt8028_get_raw_state(&flag, &bcd, &d0, &d1, &d2);
    if (flag != last_flag && tick_check_expire(last_tick, 200)) {
        printf("PT8028 raw: FLAG,D2,D1,D0=%d,%d,%d,%d TCH%d\n",
               flag, d2, d1, d0, bcd);
        last_flag = flag;
        last_tick = tick_get();
    }
#endif
}

AT(.text.bsp.pt8028)
void pt8028_poll_reinit(void)
{
    static u32 last_tick;

    if (tick_check_expire(last_tick, 500)) {
        last_tick = tick_get();
        pt8028_port_gpio_init();
    }
}

#endif // USER_PT8028_KEY
