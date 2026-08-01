#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


static tft_cb_t tft_cb;

/* 背光 kick 超时兜底: ELUNCHBOX 冷启动靠 "首帧推完+3TE" 点背光,
 * 但 0x35=0x00 时屏不输出 TE 脉冲, te_bglight_cnt 永不递减, 背光死锁。
 * 首帧推完(kick)后超时未点亮则强制完成。 */
#define TFT_BGLIGHT_KICK_TIMEOUT_MS     300
static u32 bglight_kick_tick;           /* kick 置 te_bglight_cnt 的时刻 */

/* ---- TE 帧同步门控 (撕裂消除) ----
 * TE 中断只标帧边界, 不再调 os_gui_draw —— ISR 推屏与主循环 gui_process
 * 双推 SPI 总线冲突是上次 6b99f86 使能 TE 后切图卡死的根因。
 * 推屏统一由主循环经 tft_te_frame_gate() 在 TE 时隙后执行。 */
#define TFT_TE_GATE_TIMEOUT_MS          80      /* TE 同步下最长等帧边界, 超时放行防卡帧 */
static volatile bool te_frame_ready;            /* TE 帧边界就绪 (ISR 置位/主循环消费) */
static volatile u32  te_pulse_cnt;              /* TE 脉冲计数 (证真+背光 kick 递减用) */

/* ---- TE 脉冲形态 (实测 2026-08-01) ----
 * 0x35=0x00: 无脉冲。0x01: 19246/s = 320.8行 × 60Hz → 行脉冲仅在
 * 有效行输出, V-blank(~28行, ~1.4ms)静默 → 有可识别的帧间隙。
 * 帧相位实验: 行边沿保持完整 tft_te_refresh(rtc快照/te_frame_ready
 * 节奏与旧行为逐位一致), 仅 TICK0CNT 清零挪到帧间隙后首个边沿,
 * 使 tft_te_getnorm 输出真实帧内相位 (0..100), 让 GUI 库内
 * te_margin(默认30) 起绘门控真正对齐帧界 (撕裂治本)。
 * 保险: 开机 10s 内若推帧 FPS=0 而 TE 脉冲正常, 自动退回逐边沿
 * 清零 (旧行为) 并打印, 不会再黑屏。 */
#define TFT_TE_PULSE_DEBUG              1       /* 每秒打印 TE:脉冲 norm push, 定位后关闭 */
#define TFT_TE_EDGE_GAP_TICKS           100     /* 边沿间隔>此值判帧间隙 (行≈21tick, 隙≈570tick) */
static volatile bool te_phase_clear;            /* true=仅帧界清TICK0(实验) false=逐边沿(旧行为) */
volatile u16 tft_push_cnt;                      /* set_window 推送计数 (i80 不走 frame_start/end,
                                                 * 由驱动 set_window 递增; 诊断+保险) */
static u16 te_edge_last;                        /* 上一边沿 TICK0CNT (仅ISR访问) */

tft_cb_t* tft_get_tft_cb(void)
{
    return &tft_cb;
}

/* rtc 快照 + 帧就绪标志 (与旧逐边沿行为一致的部分, 不含 TICK0CNT 清零) */
AT(.com_text.tft_spi)
static void te_edge_mark(void)
{
    compo_cb.rtc_cnt = RTCCNT;
    compo_cb.rtc_cnt2 = RTCCON2;
    compo_cb.rtc_update = true;
#if GUI_USE_SCREENSHOOT
    if (!gui_get_screenshot())
#endif
	{
        te_frame_ready = true;      /* 帧边界就绪, 主循环门控消费 */
    }
}

AT(.com_text.tft_spi)
static void tft_te_refresh(void)
{
    TICK0CNT = 0;
    te_edge_mark();
}

AT(.com_text.tft_spi)
void tft_bglight_en(void)
{
    LCD_BL_EN();            //LCD BL(开背光)
}

AT(.com_text.tft_spi)
void tft_te_isr(void)
{
#if (PORT_TFT_INT != IO_NONE)
    if (WKUPEDG & BIT(16+PORT_TFT_INT_VECTOR)) {
        WKUPCPND = BIT(16+PORT_TFT_INT_VECTOR);
        te_pulse_cnt++;

        bool flag_mode_nochange = true;
        if (tft_cb.te_mode != tft_cb.te_mode_next) {
            tft_cb.te_mode = tft_cb.te_mode_next;
            tft_cb.despi_baud = (tft_cb.te_mode > 0) ? tft_cb.despi_baud1 : tft_cb.despi_baud2;
            if (tft_cb.flag_in_frame) {
                tft_cb.tft_set_baud_kick = true;
            } else {
                LCDSPIBAUD = tft_cb.despi_baud;
            }
            flag_mode_nochange = false;
        }
        if (tft_cb.te_mode == 1) {
            //1TE MODE
            TICK1CNT = 0;
            TICK1CON |= BIT(0);         //TICK EN
        } else {
            //>1TE MODE, 如果Mode change停一个TE
            if (flag_mode_nochange) {
                u16 now = (u16)TICK0CNT;
                u16 delta = now - te_edge_last;
                te_edge_last = now;
                if (!te_phase_clear || delta > TFT_TE_EDGE_GAP_TICKS) {
                    /* 帧间隙后首个边沿(或旧行为模式): 清 TICK0 定帧相位 */
                    te_edge_last = 0;
                    tft_te_refresh();
                } else {
                    /* 行边沿: 快照/就绪与旧行为一致, 仅不清 TICK0CNT */
                    te_edge_mark();
                }
            }
        }
        //延时打开背光
        if (tft_cb.te_bglight_cnt > 0) {
            tft_cb.te_bglight_cnt--;
            if (tft_cb.te_bglight_cnt == 0) {
                tft_cb.tft_bglight_first_set = true;
            }
        }
    }
#endif
}

//推屏帧门控: 当前TE仅用于背光kick递减, 推屏时序暂不改变
bool tft_te_frame_gate(void)
{
    return true;    //TODO: TE同步验证通过后启用帧边界对齐
}

AT(.com_text.tft_spi)
void tick_te_isr(void)
{
    if (TICK1CON & BIT(9)) {
        TICK1CON &= ~BIT(0);
        TICK1CPND = BIT(9);
        tft_te_refresh();
#if (PORT_TFT_INT == IO_NONE)
        TICK1CON |= BIT(0);
        //延时打开背光
        if (tft_cb.te_bglight_cnt > 0) {
            tft_cb.te_bglight_cnt--;
            if (tft_cb.te_bglight_cnt == 0) {
                tft_cb.tft_bglight_first_set = true;
            }
        }
#endif
    }
}

//将TE量化到0~100, 由于TE中断不及时，可能偶尔会超过100
AT(.com_text.tft_spi)
int tft_te_getnorm(void)
{
#if (PORT_TFT_INT == IO_NONE)
    return 0;
#else
    return TICK0CNT * 100 * 64 / (int)((XOSC_CLK_HZ / 1000) * TFT_TE_CYCLE);
#endif
}

//设置1TE / 2TE的波特率
void tft_set_baud(u8 baud1, u8 baud2)
{
    tft_cb.despi_baud1 = baud1;
    tft_cb.despi_baud2 = baud2;
    tft_cb.despi_baud = tft_cb.te_mode ? tft_cb.despi_baud1 : tft_cb.despi_baud2;
}

AT(.com_text.tft_spi)
void tft_frame_start(void)
{
    if (LCDCON == 0) {
        return;
    }

    tft_cb.flag_in_frame = true;
    tft_write_data_start();
}

AT(.com_text.tft_spi)
void tft_frame_end(void)
{
    if (LCDCON == 0) {
        return;
    }

    tft_write_end();
    tft_cb.flag_in_frame = false;
    if (tft_cb.tft_bglight_kick) {
        tft_cb.tft_bglight_kick = false;
        tft_cb.te_bglight_cnt = 3; //3TE后打开背光
        bglight_kick_tick = tick_get();
    }
    if (tft_cb.tft_set_baud_kick) {
        tft_cb.tft_set_baud_kick = false;
        LCDSPIBAUD = tft_cb.despi_baud;
    }
}

//背光亮度初始设置检测
void tft_bglight_frist_set_check(void)
{
#if TFT_TE_PULSE_DEBUG
    /* TE:脉冲/s norm:帧内相位 push:set_window推送次数 ph:相位实验开 */
    static u32 te_dbg_tick;
    static u8 te_dbg_boot_chk = 15;
    static u8 te_dbg_zero_sec;
    if (tick_check_expire(te_dbg_tick, 1000)) {
        te_dbg_tick = tick_get();
        printf("TE:%d/s norm:%d push:%d%s\n", (int)te_pulse_cnt, tft_te_getnorm(),
               tft_push_cnt, te_phase_clear ? " ph" : "");
        /* 保险: TE 稳态行频(排除开机首秒~59k噪声)下连续 3 秒零推送
         * → 相位实验退回逐边沿清零。只在开机 15s 内检测 */
        if (te_dbg_boot_chk > 0) {
            te_dbg_boot_chk--;
            if (te_phase_clear && tft_push_cnt == 0
                && te_pulse_cnt > 15000 && te_pulse_cnt < 25000) {
                if (++te_dbg_zero_sec >= 3) {
                    te_phase_clear = false;
                    printf("TE phase clear OFF (no push)\n");
                }
            } else {
                te_dbg_zero_sec = 0;
            }
        }
        te_pulse_cnt = 0;
        tft_push_cnt = 0;
    }
#endif
    /* kick 后 TE 一直没来(0x35=0x00 无TE脉冲), 超时强制点亮 */
    if (tft_cb.te_bglight_cnt > 0
        && tick_check_expire(bglight_kick_tick, TFT_BGLIGHT_KICK_TIMEOUT_MS)) {
        tft_cb.te_bglight_cnt = 0;
        tft_cb.tft_bglight_first_set = true;
    }
    if(!tft_cb.tft_bglight_first_set) {
        return ;
    }
    tft_cb.tft_bglight_first_set = false;

    tft_cb.tft_bglight_last_duty = 0;
    //todo:后续根据客户定制调整
    if(0 == tft_cb.tft_bglight_duty)
    {
        tft_cb.tft_bglight_duty = GUI_DEFAULT_BK;
        sys_cb.light_level = 5;
    }
    lcd_drv_set_brightness(tft_cb.tft_bglight_duty);
}

void tft_bglight_open(void)
{
    LCD_BL_EN();
    tft_cb.tft_bglight_first_set = true;
    tft_bglight_frist_set_check();
}

void tft_bglight_force_on(void)
{
    tft_cb.te_bglight_cnt = 0;
    tft_cb.tft_bglight_kick = false;
    tft_cb.tft_bglight_first_set = true;
    if (tft_cb.tft_bglight_duty == 0) {
        tft_cb.tft_bglight_duty = GUI_DEFAULT_BK;
    }
    tft_bglight_frist_set_check();
    LCD_BL_EN();
}

//设置TE MODE
void tft_set_temode(u8 mode)
{
    tft_cb.te_mode_next = mode;
}


void tft_init(void)
{

#if (PORT_TFT_INT != IO_NONE)
    CLKGAT0 |= BIT(30);                                 //TICK0
    delay_us(1);                                        //set CLKGAT0需要时间生效
    TICK0CON = BIT(6) | BIT(5) | BIT(2);                //div64[6:4], xosc26m[3:1]
    TICK0PR = 0xFFFF;
    TICK0CNT = 0;
    TICK0CON |= BIT(0);                                 //TICK EN

    CLKGAT0 |= BIT(31);                                 //TICK1
    TICK1CON = BIT(7) | BIT(6) | BIT(5) | BIT(2);       //TIE, div64[6:4], xosc26m[3:1]
    TICK1PR = (int)((XOSC_CLK_HZ / 1000) * TFT_TE_CYCLE_DELAY) / 64;
    TICK1CNT = 0;
    sys_irq_init(IRQ_TE_TICK_VECTOR, 0, tick_te_isr);

    tft_cb.te_mode = 0;                                 //初始化
    tft_cb.te_mode_next = 0;
    tft_set_temode(DEFAULT_TE_MODE);

    /* 帧门控初始态: TE 未证实前自由推屏 */
    te_pulse_cnt = 0;
    te_frame_ready = false;
    te_edge_last = 0;
    te_phase_clear = true;          /* 帧相位实验开 (fps=0 保险会自动关) */
#else
    CLKGAT0 |= BIT(31);                                 //TICK1
    TICK1CON = BIT(7) | BIT(6) | BIT(5) | BIT(2);       //TIE, div64[6:4], xosc26m[3:1]
    TICK1PR = (int)((XOSC_CLK_HZ / 1000) * TFT_TE_CYCLE) / 64;
    TICK1CNT = 0;
    TICK1CON |= BIT(0);
    sys_irq_init(IRQ_TE_TICK_VECTOR, 0, tick_te_isr);
#endif

#if (GUI_SELECT == GUI_TFT_320_ST77916)
    lcd_drv_register(&lcd_320_st77916_drv);
#elif (GUI_SELECT == GUI_OLED_466_ICNA3310B)
    lcd_drv_register(&lcd_oled_466_icna3310b_drv);
#elif (GUI_SELECT == GUI_OLED_368_ST7801N)
    lcd_drv_register(&lcd_oled_368_st7801n_drv);
#elif (GUI_SELECT == GUI_TFT_240_ST789_i80)
    lcd_drv_register(&lcd_240_st7789V3_i80_drv);
#elif (GUI_SELECT == GUI_LCD_480_ST7283)
    lcd_drv_register(&lcd_480_st7283_drv);
#elif (GUI_SELECT == GUI_LCD_800_ST7265)
    lcd_drv_register(&lcd_800_st7265_drv);
#elif (GUI_SELECT == GUI_VGS_640)
    lcd_drv_register(&lcd_vga012a_drv);
#endif

    LCDSPIBAUD = 30;      //读ID建议在20M以内
    TRACE("TFT ID: %x\n", lcd_drv_read_id());
    LCDSPIBAUD = tft_cb.despi_baud;
    lcd_drv_init();
    lcd_drv_set_window(0, 0, GUI_SCREEN_WIDTH - 1, GUI_SCREEN_HEIGHT - 1);
    tft_cb.tft_bglight_kick = true; //延时打开背光
}

void tft_exit(void)
{
    if(!vddio_sleep_level) {
//        bsp_pwm_disable(PORT_TFT_BL); //关背光
        lcd_drv_set_brightness(0);
//    printf("%s:%d\n",__func__,__LINE__);
    }

    lcd_drv_deregister();

    TICK0CON = 0;
    TICK1CON = 0;
}
