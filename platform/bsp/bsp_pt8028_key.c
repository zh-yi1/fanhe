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
#define PT8028_PRESS_SETTLE_SCANS       0
#endif
#ifndef PT8028_RELEASE_HOLD_SCANS
#define PT8028_RELEASE_HOLD_SCANS       4
#endif
#ifndef PT8028_POLL_REINIT_MS
#define PT8028_POLL_REINIT_MS           500
#endif
#ifndef PT8028_BCD_SAMPLE_CNT
#define PT8028_BCD_SAMPLE_CNT           3
#endif
#ifndef PT8028_BCD_SAMPLE_US
#define PT8028_BCD_SAMPLE_US            0
#endif
#ifndef PT8028_TCH7_CONFIRM_SCANS
#define PT8028_TCH7_CONFIRM_SCANS       12
#endif

#if USER_PT8028_KEY
#include "port_pt8028_key.h"

/*
 * 表2 / 规格书（PE1=OUT_FLAG, PE2=D0, PE3=D1, PE4=D2）：
 *   上电/空闲 OUT_FLAG=1, D=111 → 不是键
 *   按下     OUT_FLAG=0         → 读 FLAG/D2/D1/D0 编码判 TCH0~7
 *   释放     OUT_FLAG=1         → Hold = 按下锁存副本；下次按下沿清空重录
 *   无长按/短按，一律按下触发
 */
typedef struct {
    u8  press_key;
    u8  press_tch;
    u8  last_out_flag;
    u8  press_active;
    u16 pending_ku;
    u8  bcd_hist[8];
    u8  press_snap_tch;
    u8  press_settle_left;
    u8  release_hold_left;
    u8  release_hold_hist[8];
    u32 press_tick;
    u8  release_done;
#if ELUNCHBOX_PANEL_EN
    u8  release_tch;
    u8  release_pending;
    u8  res_key_pending;
    u8  press_bcd;          /* 按下期间最后一次 BCD(OUT_FLAG=0) */
    u8  session_tch;        /* 本次按下 OUT_FLAG=0 时锁定的键值 */
    u8  press_pending;      /* 按下沿事件待取 */
    u8  press_emitted;      /* 本次按下已触发按下逻辑 */
    u8  home_act_pending;   /* 释放后待 Home 取走的动作 */
    /* 表2 会话锁存：按下记 FLAG/D2/D1/D0；释放 Hold=按下副本；下次按下清空 */
    u8  press_ln_valid;
    u8  press_ln_flag;
    u8  press_ln_d0;
    u8  press_ln_d1;
    u8  press_ln_d2;
    u8  press_ln_bcd;
    u8  hold_ln_valid;
    u8  hold_ln_flag;
    u8  hold_ln_d0;
    u8  hold_ln_d1;
    u8  hold_ln_d2;
    u8  hold_ln_bcd;
    u8  press_d0_min;       /* 按下期间 D0 是否出现过 0 */
    u8  press_d1_min;
    u8  press_d2_min;
    u8  press_pe_min;       /* 按下期间 GPIOE[4:0] 最小值 */
#endif
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
static u8 pt8028_gpio_ok AT(.buf.pt8028);
#if ELUNCHBOX_PANEL_EN
static u8 pt8028_home_msg_block AT(.buf.pt8028);
#endif

static char pt8028_last_key_str[12] AT(.buf.pt8028) = "none";

#define PT8028_GPIO_PE_MASK             (BIT(1) | BIT(2) | BIT(3) | BIT(4))

#if ELUNCHBOX_PANEL_EN
typedef struct {
    u8 out_flag;
    u8 bcd;
    u8 d0;
    u8 d1;
    u8 d2;
} pt8028_lines_t;

static void pt8028_gpio_bcd_ensure(void)
{
    if (pt8028_pe_gpio_ok()) {
        pt8028_gpio_ok = 1;
        return;
    }
    pt8028_gpio_ok = 0;
    pt8028_port_gpio_init();
}

/*
 * PE1=FLAG, PE2=D0, PE3=D1, PE4=D2
 * 读前 reclaim + GPIOE 一次快照（与逻辑分析仪采样时刻一致）。
 */
AT(.com_text.bsp.pt8028)
static pt8028_lines_t pt8028_read_lines(void)
{
    pt8028_lines_t ln;
    u32 pe;

#if ELUNCHBOX_PANEL_EN
    if (!pt8028_pe_gpio_ok()) {
    }
    pt8028_pe_reclaim_fast();
#endif
    pe = GPIOE;
    ln.d0 = bsp_gpio_get_sta(PT8028_GPIO_D0) ? 1 : 0;
    ln.d1 = bsp_gpio_get_sta(PT8028_GPIO_D1) ? 1 : 0;
    ln.d2 = bsp_gpio_get_sta(PT8028_GPIO_D2) ? 1 : 0;
    ln.bcd = (ln.d2 << 2) | (ln.d1 << 1) | ln.d0;
#if PT8028_FLAG_ACTIVE_LOW
    ln.out_flag = (pe >> 1) & 1;
#else
    ln.out_flag = ((pe >> 1) & 1) ? 0 : 1;
#endif
    return ln;
}

static bool pt8028_is_pressed(void);

#if ELUNCHBOX_PANEL_EN
AT(.com_text.bsp.pt8028)
static void pt8028_press_track_lines(pt8028_lines_t ln)
{
    pt8028_cb.press_d0_min &= ln.d0;
    pt8028_cb.press_d1_min &= ln.d1;
    pt8028_cb.press_d2_min &= ln.d2;
    pt8028_cb.press_pe_min &= (u8)(GPIOE & 0x1f);
}
#endif

static u8 pt8028_read_out_flag(void)
{
    return pt8028_read_lines().out_flag;
}

static u8 pt8028_read_bcd_raw(void)
{
    return pt8028_read_lines().bcd;
}

/* 恢复 D 线上下拉（按 PT8028_GPIO_BCD_PULL 配置） */
AT(.com_text.bsp.pt8028)
static void pt8028_bcd_pull_restore(void)
{
#if PT8028_GPIO_BCD_PULL == GPIOxPU300
    GPIOEPU300 |= (BIT(2) | BIT(3) | BIT(4));
#elif PT8028_GPIO_BCD_PULL == GPIOxPU200K
    GPIOEPU200K |= (BIT(2) | BIT(3) | BIT(4));
#elif PT8028_GPIO_BCD_PULL == GPIOxPU
    GPIOEPU |= (BIT(2) | BIT(3) | BIT(4));
#endif
}

/* 释放 D 线上下拉，高阻采样 */
AT(.com_text.bsp.pt8028)
static void pt8028_bcd_pull_off(void)
{
    GPIOEPU     &= ~(BIT(2) | BIT(3) | BIT(4));
    GPIOEPD     &= ~(BIT(2) | BIT(3) | BIT(4));
    GPIOEPU200K &= ~(BIT(2) | BIT(3) | BIT(4));
    GPIOEPD200K &= ~(BIT(2) | BIT(3) | BIT(4));
    GPIOEPU300  &= ~(BIT(2) | BIT(3) | BIT(4));
    GPIOEPD300  &= ~(BIT(2) | BIT(3) | BIT(4));
}

/*
 * OUT_FLAG=0/1 时读 BCD：直接读 GPIOE 多数表决。
 * 本板 PT8028 推挽输出 BCD，禁止 pull_off —— 高阻时 D2(PE4) 易浮成 1，
 * 会把模式键 011(TCH3) 误读成 111(TCH7)。
 */
AT(.com_text.bsp.pt8028)
static u8 pt8028_bcd_vote_lines(u8 sample_cnt, u8 tch_max)
{
    u8 hist[8];
    u8 i, bcd, best, best_cnt, t;

    memset(hist, 0, sizeof(hist));
#if ELUNCHBOX_PANEL_EN
    if (pt8028_is_pressed()) {
        pt8028_bcd_pull_off();
    }
#endif
    for (i = 0; i < sample_cnt; i++) {
        bcd = pt8028_read_lines().bcd;
        if (bcd <= PT8028_KEY_TCH7) {
            hist[bcd]++;
        }
#if PT8028_BCD_SAMPLE_US > 0
        if ((i + 1) < sample_cnt) {
            delay_us(PT8028_BCD_SAMPLE_US);
        }
#endif
    }
#if ELUNCHBOX_PANEL_EN
    if (pt8028_is_pressed()) {
        pt8028_bcd_pull_restore();
    }
#endif

    best = 0xff;
    best_cnt = 0;
    for (t = 0; t <= tch_max; t++) {
        if (hist[t] > best_cnt) {
            best_cnt = hist[t];
            best = t;
        }
    }
    if (best_cnt == 0) {
        return 0xff;
    }
    return best;
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_read_bcd_raw_vote(void)
{
    return pt8028_bcd_vote_lines(PT8028_BCD_SAMPLE_CNT, PT8028_KEY_TCH7);
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_read_bcd_pressed(void)
{
    return pt8028_read_bcd_raw_vote();
}

static u8 pt8028_press_tch_resolve(void);
static u8 pt8028_table_to_tch(u8 out_flag, u8 bcd, u8 press_saw_tch7);
static bool pt8028_hist_has_tch06(void);
static bool pt8028_hist_tch7_only(void);
static u8 pt8028_hist_best_tch06(void);

/* 下次按下沿：清空上次按下/Hold 锁存 */
AT(.com_text.bsp.pt8028)
static void pt8028_session_clear(void)
{
    pt8028_cb.press_ln_valid = 0;
    pt8028_cb.press_ln_flag = 1;
    pt8028_cb.press_ln_d0 = 1;
    pt8028_cb.press_ln_d1 = 1;
    pt8028_cb.press_ln_d2 = 1;
    pt8028_cb.press_ln_bcd = PT8028_KEY_TCH7;
    pt8028_cb.hold_ln_valid = 0;
    pt8028_cb.hold_ln_flag = 1;
    pt8028_cb.hold_ln_d0 = 1;
    pt8028_cb.hold_ln_d1 = 1;
    pt8028_cb.hold_ln_d2 = 1;
    pt8028_cb.hold_ln_bcd = PT8028_KEY_TCH7;
}

/* OUT_FLAG=0 期间：把当前稳定 BCD 写入按下锁存（含 D2/D1/D0 各位） */
AT(.com_text.bsp.pt8028)
static void pt8028_session_press_latch(u8 bcd)
{
    if (bcd > PT8028_KEY_TCH7) {
        return;
    }
    pt8028_cb.press_ln_valid = 1;
    pt8028_cb.press_ln_flag = 0;
    pt8028_cb.press_ln_d0 = bcd & 1;
    pt8028_cb.press_ln_d1 = (bcd >> 1) & 1;
    pt8028_cb.press_ln_d2 = (bcd >> 2) & 1;
    pt8028_cb.press_ln_bcd = bcd;
}

/*
 * 表2：FLAG 变 0 后 D 线可能仍为空闲 111，须等 BCD 更新。
 * TCH0~6 一旦出现即锁存；111 仅在没有 TCH0~6 且连续多次稳定时才当 TCH7。
 */
AT(.com_text.bsp.pt8028)
static void pt8028_session_press_update(u8 bcd)
{
    if (bcd <= PT8028_KEY_TCH6) {
        pt8028_cb.bcd_hist[bcd]++;
        pt8028_cb.press_snap_tch = bcd;
        pt8028_session_press_latch(bcd);
        pt8028_cb.press_bcd = bcd;
        return;
    }
    if (bcd == PT8028_KEY_TCH7) {
        pt8028_cb.bcd_hist[PT8028_KEY_TCH7]++;
        if (!pt8028_hist_has_tch06() &&
            pt8028_cb.bcd_hist[PT8028_KEY_TCH7] >= PT8028_TCH7_CONFIRM_SCANS) {
            pt8028_session_press_latch(PT8028_KEY_TCH7);
            pt8028_cb.press_bcd = PT8028_KEY_TCH7;
        }
    }
}

/* 释放沿：Hold = 按下锁存；无效时读 OUT_FLAG=1 的 D 线 Hold（表2） */
AT(.com_text.bsp.pt8028)
static void pt8028_session_hold_latch(void)
{
    pt8028_lines_t ln;

    if (pt8028_cb.press_ln_valid) {
        pt8028_cb.hold_ln_valid = 1;
        pt8028_cb.hold_ln_flag = 1;
        pt8028_cb.hold_ln_d0 = pt8028_cb.press_ln_d0;
        pt8028_cb.hold_ln_d1 = pt8028_cb.press_ln_d1;
        pt8028_cb.hold_ln_d2 = pt8028_cb.press_ln_d2;
        pt8028_cb.hold_ln_bcd = pt8028_cb.press_ln_bcd;
        return;
    }

    ln = pt8028_read_lines();
    if (ln.out_flag != 1) {
        return;
    }
    pt8028_cb.hold_ln_valid = 1;
    pt8028_cb.hold_ln_flag = 1;
    pt8028_cb.hold_ln_d0 = ln.d0;
    pt8028_cb.hold_ln_d1 = ln.d1;
    pt8028_cb.hold_ln_d2 = ln.d2;
    pt8028_cb.hold_ln_bcd = ln.bcd;
}

/* 释放判键：优先用 Hold 锁存（按下时记录的电平） */
AT(.com_text.bsp.pt8028)
static u8 pt8028_session_hold_tch(void)
{
    u8 bcd;
    u8 tch;
    u8 saw_tch7;

    if (!pt8028_cb.hold_ln_valid) {
        return 0xff;
    }
    bcd = pt8028_cb.hold_ln_bcd;
    saw_tch7 = (pt8028_cb.bcd_hist[PT8028_KEY_TCH7] >= PT8028_BCD_STABLE_CNT);
    tch = pt8028_table_to_tch(1, bcd, saw_tch7);
    if (tch <= PT8028_KEY_TCH6) {
        return tch;
    }
#if FUNC_RESERVATION_UI_EN
    if (tch == PT8028_KEY_TCH7) {
        return tch;
    }
#endif
    if (bcd <= PT8028_KEY_TCH6) {
        return bcd;
    }
    return 0xff;
}

/* OUT_FLAG=1 释放沿 Hold：直接连采，111 为空闲非键 */
AT(.com_text.bsp.pt8028)
static u8 pt8028_read_bcd_hold(void)
{
    u8 hold;

    hold = pt8028_bcd_vote_lines(PT8028_HOLD_READ_CNT, PT8028_KEY_TCH6);
    if (hold != 0xff) {
        return hold;
    }
    /* Hold=111：用按下阶段 hist（含 111→TCH3 remap 后的统计） */
    return pt8028_press_tch_resolve();
}

/*
 * 表2：OUT_FLAG=0 时 BCD 即键值；OUT_FLAG=1 时 D=Hold。
 * OUT_FLAG=1 且 D=111 为上电/空闲，不是 TCH7（除非按下阶段确认为 TCH7）。
 */
AT(.com_text.bsp.pt8028)
static u8 pt8028_table_to_tch(u8 out_flag, u8 bcd, u8 press_saw_tch7)
{
    if (out_flag == 0) {
        if (bcd <= PT8028_KEY_TCH7) {
            return bcd;
        }
        return 0xff;
    }

    if (bcd <= PT8028_KEY_TCH6) {
        return bcd;
    }

    if (bcd == PT8028_KEY_TCH7) {
        if (press_saw_tch7) {
            return PT8028_KEY_TCH7;
        }
        return 0xff;
    }

    return 0xff;
}

/* 表2：Hold 锁存 BCD 即 TCH；TCH3=模式 Tab，TCH4=确认 */
AT(.com_text.bsp.pt8028)
static u8 pt8028_home_act_from_tch(u8 tch)
{
    if (tch == PT8028_KEY_TCH3) {
        return PT8028_HOME_ACT_MODE;
    }
    if (tch == PT8028_KEY_TCH4) {
        return PT8028_HOME_ACT_CONFIRM;
    }
    return PT8028_HOME_ACT_NONE;
}

/* OUT_FLAG=0 阶段：按下 hist 多数表决（hist 存 remap 后 TCH） */
AT(.com_text.bsp.pt8028)
static u8 pt8028_press_tch_resolve(void)
{
    u8 tch;

    tch = pt8028_hist_best_tch06();
    if (tch <= PT8028_KEY_TCH6) {
        return tch;
    }
    if (pt8028_cb.press_snap_tch <= PT8028_KEY_TCH6) {
        return pt8028_cb.press_snap_tch;
    }
    if (pt8028_cb.press_bcd <= PT8028_KEY_TCH6) {
        return pt8028_cb.press_bcd;
    }
    return 0xff;
}

/*
 * 释放沿：OUT_FLAG=1 读 Hold；111 为空闲。
 * Hold 无效时回退到按下阶段(OUT_FLAG=0)采样结果。
 */
AT(.com_text.bsp.pt8028)
static u8 pt8028_release_tch_resolve(u8 hold_raw)
{
    u8 tch;
    u8 saw_tch7;

    saw_tch7 = (pt8028_cb.bcd_hist[PT8028_KEY_TCH7] >= PT8028_BCD_STABLE_CNT);
    tch = pt8028_table_to_tch(1, hold_raw, saw_tch7);
    if (tch <= PT8028_KEY_TCH6) {
        return tch;
    }
#if FUNC_RESERVATION_UI_EN
    if (tch == PT8028_KEY_TCH7) {
        return tch;
    }
#endif
    tch = pt8028_press_tch_resolve();
#if !FUNC_RESERVATION_UI_EN
    if (tch == PT8028_KEY_TCH7) {
        tch = 0xff;
    }
#endif
    return tch;
}

#ifndef PT8028_HOLD_READ_CNT
#define PT8028_HOLD_READ_CNT            8
#endif

/* 释放沿：OUT_FLAG=1 立即连读 Hold，多数表决（高阻） */
AT(.com_text.bsp.pt8028)
static u8 pt8028_hold_read_maj(void)
{
    return pt8028_read_bcd_hold();
}

/* OUT_FLAG=0 时 BCD 输出即键值（表2） */
AT(.com_text.bsp.pt8028)
static u8 pt8028_elunchbox_press_tch_get(void)
{
#if FUNC_RESERVATION_UI_EN
    if (pt8028_hist_tch7_only()) {
        return PT8028_KEY_TCH7;
    }
#endif
    return pt8028_press_tch_resolve();
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_elunchbox_release_tch(void)
{
    u8 tch;

    if (!pt8028_cb.press_ln_valid) {
        tch = pt8028_hist_best_tch06();
        if (tch <= PT8028_KEY_TCH6) {
            pt8028_session_press_latch(tch);
        } else if (pt8028_hist_tch7_only()) {
            pt8028_session_press_latch(PT8028_KEY_TCH7);
        }
    }
    pt8028_session_hold_latch();
    return pt8028_session_hold_tch();
}
#else
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
static u8 pt8028_read_bcd_raw(void)
{
    u8 d0 = bsp_gpio_get_sta(PT8028_GPIO_D0) ? 1 : 0;
    u8 d1 = bsp_gpio_get_sta(PT8028_GPIO_D1) ? 1 : 0;
    u8 d2 = bsp_gpio_get_sta(PT8028_GPIO_D2) ? 1 : 0;

    return (d2 << 2) | (d1 << 1) | d0;
}
#endif

AT(.com_text.bsp.pt8028)
static bool pt8028_is_pressed(void)
{
    return pt8028_read_out_flag() == 0;
}

AT(.com_text.bsp.pt8028)
static bool pt8028_tch_valid(u8 tch)
{
    return (tch <= PT8028_KEY_TCH7);
}

AT(.com_text.bsp.pt8028)
static bool pt8028_bcd_is_idle(u8 bcd, u8 out_flag)
{
    return (out_flag == 1 && bcd == PT8028_KEY_TCH7);
}

AT(.com_text.bsp.pt8028)
static void pt8028_hist_clear(void)
{
    memset(pt8028_cb.bcd_hist, 0, sizeof(pt8028_cb.bcd_hist));
}

AT(.com_text.bsp.pt8028)
static bool pt8028_hist_has_tch06(void)
{
    u8 i;

    for (i = 0; i <= PT8028_KEY_TCH6; i++) {
        if (pt8028_cb.bcd_hist[i] > 0) {
            return true;
        }
    }
    return false;
}

AT(.com_text.bsp.pt8028)
static void pt8028_hist_accum(u8 *hist, u8 bcd)
{
    if (bcd > PT8028_KEY_TCH7) {
        return;
    }
    hist[bcd]++;
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_hist_pick_tch06(const u8 *hist)
{
    u8 i;
    u8 best = 0xff;
    u8 best_cnt = 0;

    for (i = 0; i <= PT8028_KEY_TCH6; i++) {
        if (hist[i] > best_cnt) {
            best_cnt = hist[i];
            best = i;
        }
    }
    if (best_cnt > 0) {
        return best;
    }
    return 0xff;
}

/* OUT_FLAG=0 的每次扫描：仅在按下期间读 BCD（表2） */
AT(.com_text.bsp.pt8028)
static void pt8028_press_sample(void)
{
    u8 bcd;

    if (!pt8028_is_pressed()) {
        return;
    }

#if ELUNCHBOX_PANEL_EN
    pt8028_gpio_bcd_ensure();
#endif

    if (pt8028_cb.press_settle_left > 0) {
        pt8028_cb.press_settle_left--;
        return;
    }

#if ELUNCHBOX_PANEL_EN
    bcd = pt8028_read_bcd_raw_vote();
    if (bcd == 0xff) {
        return;
    }
    pt8028_session_press_update(bcd);
#else
    u8 i;

    for (i = 0; i < PT8028_BCD_SAMPLE_CNT; i++) {
        bcd = pt8028_read_bcd_raw();
        if (bcd <= PT8028_KEY_TCH6) {
            pt8028_cb.bcd_hist[bcd]++;
            pt8028_cb.press_snap_tch = bcd;
            continue;
        }
        if (bcd == PT8028_KEY_TCH7) {
            pt8028_cb.bcd_hist[PT8028_KEY_TCH7]++;
        }
    }
#endif
}

AT(.com_text.bsp.pt8028)
static void pt8028_release_hold_sample(void)
{
    u8 i, bcd;

    for (i = 0; i < PT8028_BCD_SAMPLE_CNT; i++) {
        bcd = pt8028_read_bcd_raw();
        pt8028_hist_accum(pt8028_cb.release_hold_hist, bcd);
    }
}

AT(.com_text.bsp.pt8028)
static u8 pt8028_hist_best_tch06(void)
{
    u8 i;
    u8 best = 0xff;
    u8 best_cnt = 0;

    for (i = 0; i <= PT8028_KEY_TCH6; i++) {
        if (pt8028_cb.bcd_hist[i] > best_cnt) {
            best_cnt = pt8028_cb.bcd_hist[i];
            best = i;
        }
    }
    if (best_cnt > 0) {
        return best;
    }
    return 0xff;
}

AT(.com_text.bsp.pt8028)
static bool pt8028_hist_tch7_only(void)
{
    if (pt8028_hist_has_tch06()) {
        return false;
    }
    return (pt8028_cb.bcd_hist[PT8028_KEY_TCH7] >= PT8028_BCD_STABLE_CNT);
}

/* 松手 Hold：OUT_FLAG=1 时 D 应 Hold TCH0~6；111 为空闲态不是键 */
AT(.com_text.bsp.pt8028)
static u8 pt8028_hold_hist_to_tch(const u8 *hold_hist)
{
    u8 tch;

    tch = pt8028_hist_pick_tch06(hold_hist);
    if (tch <= PT8028_KEY_TCH6) {
        return tch;
    }
    return 0xff;
}

/*
 * 松手解析：饭盒按表2读 Hold；非饭盒走 hist/snap。
 */
AT(.com_text.bsp.pt8028)
static u8 pt8028_release_tch_get(void)
{
    u8 tch;
    u8 hold;

#if ELUNCHBOX_PANEL_EN
    return pt8028_elunchbox_release_tch();
#else
    if (pt8028_cb.press_snap_tch <= PT8028_KEY_TCH6) {
        return pt8028_cb.press_snap_tch;
    }

    tch = pt8028_hist_best_tch06();
    if (tch <= PT8028_KEY_TCH6) {
        return tch;
    }

    hold = pt8028_hold_hist_to_tch(pt8028_cb.release_hold_hist);
    if (hold <= PT8028_KEY_TCH6) {
        return hold;
    }

    if (pt8028_hist_tch7_only()) {
        return PT8028_KEY_TCH7;
    }
    return 0xff;
#endif
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
static void pt8028_emit_press(u8 tch)
{
    u8 key;

    if (tch > PT8028_KEY_TCH7) {
        return;
    }
#if !FUNC_RESERVATION_UI_EN
    if (tch == PT8028_KEY_TCH7) {
        return;
    }
#endif
    key = tbl_pt8028_bcd_to_key[tch];
    pt8028_cb.session_tch = tch;
    pt8028_cb.press_tch = tch;
    pt8028_cb.press_key = key;
    pt8028_cb.press_pending = 1;
#if ELUNCHBOX_PANEL_EN
    if (pt8028_cb.press_ln_valid) {
        pt8028_queue_edge(0, pt8028_cb.press_ln_bcd, pt8028_cb.press_ln_flag);
        pt8028_queue_key(tch, key, (u16)(key | KEY_SHORT),
                         pt8028_cb.press_ln_bcd, pt8028_cb.press_ln_flag);
    } else
#endif
    {
        pt8028_queue_edge(0, tch, 0);
        pt8028_queue_key(tch, key, (u16)(key | KEY_SHORT), tch, 0);
    }
}

AT(.com_text.bsp.pt8028)
static void pt8028_emit_release(u8 tch)
{
    u8 key;

    if (tch > PT8028_KEY_TCH6) {
        return;
    }
    key = tbl_pt8028_bcd_to_key[tch];
    pt8028_cb.release_tch = tch;
    pt8028_cb.release_pending = 1;
    pt8028_cb.pending_ku = (u16)(key | KEY_SHORT_UP);
#if ELUNCHBOX_PANEL_EN
    if (pt8028_cb.hold_ln_valid) {
        pt8028_queue_edge(1, pt8028_cb.hold_ln_bcd, pt8028_cb.hold_ln_flag);
        pt8028_queue_key(tch, key, pt8028_cb.pending_ku,
                         pt8028_cb.hold_ln_bcd, pt8028_cb.hold_ln_flag);
    } else
#endif
    {
        pt8028_queue_edge(1, tch, 1);
        pt8028_queue_key(tch, key, pt8028_cb.pending_ku, tch, 1);
    }
}

AT(.com_text.bsp.pt8028)
static void pt8028_emit_short_up(u8 tch)
{
    u8 key;

    if (!pt8028_tch_valid(tch)) {
        if (pt8028_cb.press_snap_tch <= PT8028_KEY_TCH6) {
            tch = pt8028_cb.press_snap_tch;
        } else if (pt8028_cb.press_tch <= PT8028_KEY_TCH6) {
            tch = pt8028_cb.press_tch;
        } else {
            pt8028_key_reject_print(pt8028_read_bcd_raw(), tch,
                                    pt8028_hist_best_tch06(), 1);
            return;
        }
    }

#if ELUNCHBOX_PANEL_EN
    if (tch == PT8028_KEY_TCH7) {
#if FUNC_RESERVATION_UI_EN
        if (pt8028_hist_tch7_only()) {
            pt8028_cb.res_key_pending = 1;
            pt8028_cb.pending_ku = NO_KEY;
            pt8028_queue_key(PT8028_KEY_TCH7, KEY_NEXT, NO_KEY, tch, 1);
            return;
        }
#endif
        pt8028_key_reject_print(tch, tch, pt8028_hist_best_tch06(), 1);
        return;
    }
#endif

    key = tbl_pt8028_bcd_to_key[tch];
    pt8028_cb.press_tch = tch;
    pt8028_cb.press_key = key;
    pt8028_cb.pending_ku = (u16)(key | KEY_SHORT_UP);
#if ELUNCHBOX_PANEL_EN
    pt8028_emit_release(tch);
#else
    pt8028_cb.release_tch = tch;
    pt8028_cb.release_pending = 1;
    pt8028_queue_key(tch, key, pt8028_cb.pending_ku, tch, 1);
#endif
}

AT(.com_text.bsp.pt8028)
static void pt8028_release_finish(void)
{
    u8 release_tch;
    u8 hold_bcd;

    release_tch = pt8028_release_tch_get();
#if ELUNCHBOX_PANEL_EN
    hold_bcd = pt8028_read_lines().bcd;
#else
    hold_bcd = release_tch;
#endif
    pt8028_queue_edge(1, (release_tch <= PT8028_KEY_TCH6) ? release_tch : hold_bcd,
                      pt8028_read_out_flag());
    if (pt8028_cb.press_active && release_tch <= PT8028_KEY_TCH6) {
        pt8028_emit_short_up(release_tch);
    }
    pt8028_cb.press_active = 0;
    pt8028_cb.release_done = 1;
    pt8028_cb.release_hold_left = 0;
    memset(pt8028_cb.release_hold_hist, 0, sizeof(pt8028_cb.release_hold_hist));
#if ELUNCHBOX_PANEL_EN
    pt8028_cb.press_bcd = 0xff;
#endif
    pt8028_hist_clear();
    pt8028_cb.press_tch = PT8028_KEY_NONE;
    pt8028_cb.press_key = NO_KEY;
    pt8028_cb.press_snap_tch = PT8028_KEY_NONE;
}

AT(.com_text.bsp.pt8028)
void pt8028_gpio_mark_configured(void)
{
    pt8028_gpio_ok = 1;
}

AT(.com_text.bsp.pt8028)
void pt8028_gpio_invalidate(void)
{
    pt8028_gpio_ok = 0;
}

AT(.com_text.bsp.pt8028)
void pt8028_gpio_ensure(void)
{
    if (pt8028_gpio_ok && pt8028_pe_gpio_ok()) {
        return;
    }
    pt8028_gpio_ok = 0;
    pt8028_port_gpio_init();
}

AT(.text.bsp.pt8028)
void pt8028_gpio_ensure_periodic(void)
{
    static u32 last_chk_ms;
    static u32 last_gpioede_pe;
    u32 cur;

    if (!tick_check_expire(last_chk_ms, 500)) {
        return;
    }
    last_chk_ms = tick_get();

    if (pt8028_pe_gpio_ok()) {
        pt8028_gpio_ok = 1;
        last_gpioede_pe = GPIOEDE & PT8028_GPIO_PE_MASK;
        return;
    }

    cur = GPIOEDE & PT8028_GPIO_PE_MASK;
    if (cur != last_gpioede_pe) {
        last_gpioede_pe = cur;
    }
    pt8028_gpio_ok = 0;
    pt8028_gpio_ensure();
}

AT(.text.key.init)
void pt8028_key_init(void)
{
#if ELUNCHBOX_PANEL_EN
    FUNCMCON0 = (FUNCMCON0 & ~0xF) | SD0MAP_NONE;
    FUNCMCON3 = (FUNCMCON3 & ~0xF) | SD1MAP_NONE;
#endif
    memset(&pt8028_cb, 0, sizeof(pt8028_cb));
    pt8028_cb.press_tch = PT8028_KEY_NONE;
    pt8028_cb.press_key = NO_KEY;
    pt8028_cb.press_snap_tch = PT8028_KEY_NONE;
    pt8028_cb.last_out_flag = pt8028_read_out_flag();
#if ELUNCHBOX_PANEL_EN
    pt8028_release_clear();
    pt8028_port_gpio_dump();
#endif
    pt8028_gpio_ensure();
}

AT(.com_text.bsp.pt8028)
u8 get_pt8028_key(void)
{
#if ELUNCHBOX_PANEL_EN
    pt8028_lines_t ln;
    u8 press_tch;

    pt8028_gpio_bcd_ensure();
    ln = pt8028_read_lines();

    if (ln.out_flag == 0) {
        if (pt8028_cb.last_out_flag == 1) {
            pt8028_gpio_ok = 0;
            pt8028_port_gpio_init();
            pt8028_cb.press_active = 1;
            pt8028_cb.release_done = 0;
            pt8028_cb.press_emitted = 0;
            pt8028_cb.press_tick = tick_get();
            pt8028_cb.press_settle_left = PT8028_PRESS_SETTLE_SCANS;
            pt8028_cb.press_bcd = 0xff;
            pt8028_cb.home_act_pending = PT8028_HOME_ACT_NONE;
            pt8028_cb.session_tch = 0xff;
            pt8028_cb.press_d0_min = 1;
            pt8028_cb.press_d1_min = 1;
            pt8028_cb.press_d2_min = 1;
            pt8028_cb.press_pe_min = 0x1f;
            pt8028_session_clear();
            pt8028_hist_clear();
            pt8028_cb.press_snap_tch = PT8028_KEY_NONE;
            if (ln.bcd <= PT8028_KEY_TCH6) {
                pt8028_session_press_latch(ln.bcd);
                pt8028_cb.press_bcd = ln.bcd;
            }
        }
        if (pt8028_cb.press_active) {
            pt8028_press_track_lines(ln);
            pt8028_press_sample();
            if (!pt8028_cb.press_emitted && pt8028_cb.press_settle_left == 0) {
                press_tch = pt8028_elunchbox_press_tch_get();
#if !FUNC_RESERVATION_UI_EN
                if (press_tch == PT8028_KEY_TCH7) {
                    press_tch = 0xff;
                }
#endif
                if (press_tch <= PT8028_KEY_TCH7) {
                    pt8028_emit_press(press_tch);
                    pt8028_cb.press_emitted = 1;
                }
            }
        }
    } else {
        if (pt8028_cb.last_out_flag == 0 && pt8028_cb.press_active &&
            !pt8028_cb.release_done) {
            u8 tch;
            u8 act;

            tch = pt8028_elunchbox_release_tch();
            if (!pt8028_cb.press_emitted && tch <= PT8028_KEY_TCH6) {
                pt8028_emit_press(tch);
                pt8028_cb.press_emitted = 1;
            }
            act = pt8028_home_act_from_tch(tch);
            if (act != PT8028_HOME_ACT_NONE) {
                pt8028_cb.home_act_pending = act;
            }
            if (tch <= PT8028_KEY_TCH6) {
                pt8028_emit_release(tch);
            } else if (!pt8028_cb.press_emitted) {
                /* Hold 无效且按下阶段无有效键 */
            } else {
                pt8028_key_reject_print(ln.bcd, pt8028_cb.press_bcd,
                                        tch, ln.out_flag);
            }
            pt8028_cb.press_active = 0;
            pt8028_cb.release_done = 1;
            pt8028_cb.press_emitted = 0;
            pt8028_cb.session_tch = 0xff;
            pt8028_cb.press_bcd = 0xff;
            pt8028_hist_clear();
            pt8028_cb.press_tch = PT8028_KEY_NONE;
            pt8028_cb.press_key = NO_KEY;
            pt8028_cb.press_snap_tch = PT8028_KEY_NONE;
        } else if (pt8028_cb.last_out_flag == 0) {
            pt8028_cb.press_active = 0;
        }
    }

    pt8028_cb.last_out_flag = ln.out_flag;
    return NO_KEY;
#else
    u8 out_flag = pt8028_read_out_flag();
    u8 key_val = NO_KEY;
    u8 live_tch;

    if (pt8028_is_pressed()) {
        if (pt8028_cb.last_out_flag == 1) {
            pt8028_cb.press_active = 1;
            pt8028_cb.release_done = 0;
            pt8028_cb.press_emitted = 0;
            pt8028_cb.press_settle_left = PT8028_PRESS_SETTLE_SCANS;
            pt8028_hist_clear();
            pt8028_cb.press_snap_tch = PT8028_KEY_NONE;
            pt8028_queue_edge(0, pt8028_read_bcd_raw(), out_flag);
        }
        pt8028_press_sample();
        live_tch = pt8028_hist_best_tch06();
        if (live_tch <= PT8028_KEY_TCH6) {
            pt8028_cb.press_tch = live_tch;
            pt8028_cb.press_key = tbl_pt8028_bcd_to_key[live_tch];
            key_val = pt8028_cb.press_key;
        }
    } else {
        if (pt8028_cb.last_out_flag == 0 && pt8028_cb.press_active &&
            !pt8028_cb.release_done) {
            u8 release_tch = pt8028_release_tch_get();

            pt8028_queue_edge(1, release_tch, out_flag);
            pt8028_release_finish();
        } else if (pt8028_cb.last_out_flag == 0) {
            pt8028_cb.press_active = 0;
        }
        key_val = NO_KEY;
    }

    pt8028_cb.last_out_flag = out_flag;
    return key_val;
#endif
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
           (pt8028_cb.release_hold_left > 0) ||
           (pt8028_cb.pending_ku != NO_KEY);
}

AT(.com_text.bsp.pt8028)
u8 pt8028_get_press_tch(void)
{
#if ELUNCHBOX_PANEL_EN
    pt8028_lines_t ln;

    if (!pt8028_is_pressed()) {
        return PT8028_KEY_NONE;
    }
    ln = pt8028_read_lines();
    if (ln.out_flag == 0) {
        return pt8028_read_bcd_pressed();
    }
    return pt8028_table_to_tch(ln.out_flag, ln.bcd, 0);
#else
    u8 tch;

    if (!pt8028_is_pressed()) {
        return PT8028_KEY_NONE;
    }
    if (pt8028_cb.press_snap_tch <= PT8028_KEY_TCH6) {
        return pt8028_cb.press_snap_tch;
    }
    tch = pt8028_hist_best_tch06();
    if (tch <= PT8028_KEY_TCH6) {
        return tch;
    }
    return PT8028_KEY_NONE;
#endif
}

AT(.com_text.bsp.pt8028)
u8 pt8028_get_led_tch(void)
{
    u8 tch;

    if (!pt8028_is_pressed()) {
        return PT8028_KEY_NONE;
    }
    tch = pt8028_get_press_tch();
    if (tch <= PT8028_KEY_TCH6) {
        return tch;
    }
    if (pt8028_hist_tch7_only()) {
        return PT8028_KEY_TCH7;
    }
    return PT8028_KEY_NONE;
}

AT(.text.bsp.pt8028)
void pt8028_get_raw_state(u8 *out_flag, u8 *bcd, u8 *d0, u8 *d1, u8 *d2)
{
#if ELUNCHBOX_PANEL_EN
    pt8028_lines_t ln;

    ln = pt8028_read_lines();
    if (ln.out_flag == 0 && pt8028_cb.press_ln_valid) {
        if (out_flag) {
            *out_flag = pt8028_cb.press_ln_flag;
        }
        if (d0) {
            *d0 = pt8028_cb.press_ln_d0;
        }
        if (d1) {
            *d1 = pt8028_cb.press_ln_d1;
        }
        if (d2) {
            *d2 = pt8028_cb.press_ln_d2;
        }
        if (bcd) {
            *bcd = pt8028_cb.press_ln_bcd;
        }
        return;
    }
    if (ln.out_flag == 1 && pt8028_cb.hold_ln_valid &&
        pt8028_cb.last_out_flag == 0) {
        if (out_flag) {
            *out_flag = pt8028_cb.hold_ln_flag;
        }
        if (d0) {
            *d0 = pt8028_cb.hold_ln_d0;
        }
        if (d1) {
            *d1 = pt8028_cb.hold_ln_d1;
        }
        if (d2) {
            *d2 = pt8028_cb.hold_ln_d2;
        }
        if (bcd) {
            *bcd = pt8028_cb.hold_ln_bcd;
        }
        return;
    }
    if (out_flag) {
        *out_flag = ln.out_flag;
    }
    if (d0) {
        *d0 = ln.d0;
    }
    if (d1) {
        *d1 = ln.d1;
    }
    if (d2) {
        *d2 = ln.d2;
    }
    if (bcd) {
        *bcd = ln.bcd;
    }
    return;
#else
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
#endif
}

const char *pt8028_get_last_key_str(void)
{
    return pt8028_last_key_str;
}

AT(.text.bsp.pt8028)
void pt8028_log_flush(void)
{
#if PT8028_KEY_DEBUG
    {
        static u8 last_flag = 0xff;
        static u8 press_logged;
        pt8028_lines_t ln;
        u8 flag, bcd, d0, d1, d2;

        ln = pt8028_read_lines();
        flag = ln.out_flag;

        /* 下降沿：OUT_FLAG=0 且 BCD 锁存有效 */
        if (flag == 0 && pt8028_cb.press_ln_valid && !press_logged) {
            u8 lf, lb, ld0, ld1, ld2;

            lf = pt8028_cb.press_ln_flag;
            ld2 = pt8028_cb.press_ln_d2;
            ld1 = pt8028_cb.press_ln_d1;
            ld0 = pt8028_cb.press_ln_d0;
            lb = pt8028_cb.press_ln_bcd;
            printf("PT8028 下降沿: FLAG,D2,D1,D0=%d,%d,%d,%d TCH%d live=%d,%d,%d,%d PE=0x%02x\n",
                   lf, ld2, ld1, ld0, lb,
                   ln.out_flag, ln.d2, ln.d1, ln.d0, (u8)(GPIOE & 0x1f));
            press_logged = 1;
        }

        /* 上升沿：释放 Hold */
        if (flag == 1 && last_flag == 0 && pt8028_cb.hold_ln_valid) {
            flag = pt8028_cb.hold_ln_flag;
            d2 = pt8028_cb.hold_ln_d2;
            d1 = pt8028_cb.hold_ln_d1;
            d0 = pt8028_cb.hold_ln_d0;
            bcd = pt8028_cb.hold_ln_bcd;
            printf("PT8028 上升沿: FLAG,D2,D1,D0=%d,%d,%d,%d TCH%d Hold live=%d,%d,%d,%d\n",
                   flag, d2, d1, d0, bcd,
                   ln.out_flag, ln.d2, ln.d1, ln.d0);
            press_logged = 0;
        }

        if (flag == 1 && last_flag == 0 && !pt8028_cb.hold_ln_valid) {
            printf("PT8028 上升沿: 未锁存 live=%d,%d,%d,%d d_min=%d,%d,%d pe_min=0x%02x h7=%d FC3=0x%x DIR=0x%02x\n",
                   ln.out_flag, ln.d2, ln.d1, ln.d0,
                   pt8028_cb.press_d0_min, pt8028_cb.press_d1_min, pt8028_cb.press_d2_min,
                   pt8028_cb.press_pe_min,
                   pt8028_cb.bcd_hist[PT8028_KEY_TCH7],
                   (u8)(FUNCMCON3 & 0x0f), (u8)(GPIOEDIR & 0x1f));
            press_logged = 0;
        }

        if (flag != last_flag) {
            if (flag == 1 && pt8028_bcd_is_idle(ln.bcd, 1) &&
                !(last_flag == 0 && pt8028_cb.hold_ln_valid)) {
                printf("PT8028 chg: FLAG,D2,D1,D0=%d,%d,%d,%d TCH%d idle\n",
                       ln.out_flag, ln.d2, ln.d1, ln.d0, ln.bcd);
            }
            if (flag == 0) {
                press_logged = 0;
            }
            last_flag = flag;
        }
    }
#endif

    if (pt8028_edge_log.pending) {
        pt8028_edge_log_t edge = pt8028_edge_log;

        pt8028_edge_log.pending = 0;
        pt8028_key_edge_print(edge.edge, edge.bcd, edge.out_flag);
    }

    if (pt8028_key_log.pending) {
        pt8028_key_log_t key = pt8028_key_log;

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

    if (tick_check_expire(last_tick, PT8028_POLL_REINIT_MS)) {
        last_tick = tick_get();
        if (!pt8028_pe_gpio_ok()) {
            pt8028_gpio_ok = 0;
            pt8028_port_gpio_init();
        }
    }
}

#if ELUNCHBOX_PANEL_EN
AT(.text.bsp.pt8028)
void pt8028_release_clear(void)
{
    pt8028_cb.release_pending = 0;
    pt8028_cb.release_tch = 0xff;
    pt8028_cb.press_pending = 0;
    pt8028_cb.press_emitted = 0;
    pt8028_cb.session_tch = 0xff;
    pt8028_cb.res_key_pending = 0;
    pt8028_cb.press_bcd = 0xff;
    pt8028_cb.home_act_pending = PT8028_HOME_ACT_NONE;
    pt8028_session_clear();
}

AT(.com_text.bsp.pt8028)
u8 pt8028_take_home_action(void)
{
    u8 act;

    act = pt8028_cb.home_act_pending;
    pt8028_cb.home_act_pending = PT8028_HOME_ACT_NONE;
    return act;
}

AT(.com_text.bsp.pt8028)
u8 pt8028_take_press_tch(void)
{
    u8 tch;

    if (!pt8028_cb.press_pending) {
        return 0xff;
    }
    tch = pt8028_cb.session_tch;
    pt8028_cb.press_pending = 0;
    return tch;
}

AT(.com_text.bsp.pt8028)
bool pt8028_take_res_key_pending(void)
{
    if (!pt8028_cb.res_key_pending) {
        return false;
    }
    pt8028_cb.res_key_pending = 0;
    return true;
}

AT(.com_text.bsp.pt8028)
u8 pt8028_take_release_tch(void)
{
    u8 tch;

    if (!pt8028_cb.release_pending) {
        return 0xff;
    }
    tch = pt8028_cb.release_tch;
    pt8028_cb.release_pending = 0;
    pt8028_cb.release_tch = 0xff;
    return tch;
}

AT(.text.bsp.pt8028)
void pt8028_set_home_msg_block(u8 en)
{
    pt8028_home_msg_block = en ? 1 : 0;
}

AT(.text.bsp.pt8028)
void pt8028_key_scan(void)
{
    u16 key;

    if (!pt8028_gpio_ok) {
        pt8028_gpio_ensure();
    }
    get_pt8028_key();

    key = pt8028_pop_short_up();
    if (key == NO_KEY) {
        return;
    }
    if (pt8028_home_msg_block) {
        return;
    }
    if (sys_cb.gui_sleep_sta) {
        sys_cb.gui_need_wakeup = 1;
    }
    msg_enqueue(key);
    reset_sleep_delay_all();
}
#endif

#endif // USER_PT8028_KEY
