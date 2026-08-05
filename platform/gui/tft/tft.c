#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


static tft_cb_t tft_cb;

tft_cb_t* tft_get_tft_cb(void)
{
    return &tft_cb;
}

/* ---- 撕裂排查仪表 (量完请把 TFT_TE_DBG_EN 关掉) --------------------------
 * TICK0 = XOSC26M / 64 = 406.25kHz, 1 count = 2.46us, 一个 16.67ms 的 TE 周期 = 6772 count
 * TICK0CNT 在 tft_te_refresh() 里清零, 所以:
 *   TE 中断里读到的 = 上次"起画"到本次 TE 的间隔  (te_mode 0 下起画就在 TE 中断里 => 约等于 TE 周期)
 *   tft_frame_end 里读到的 = 本帧从起画到推完的实际耗时(含被主循环打断的时间)
 * 判据: frame 耗时若接近或超过 TE 周期(6772), 说明推屏被主循环拖到跨帧, 撕裂是必然的 */
#ifndef TFT_TE_DBG_EN
#define TFT_TE_DBG_EN                   0
#endif

#if TFT_TE_DBG_EN
static u16 tft_dbg_te_min = 0xFFFF, tft_dbg_te_max;
static u16 tft_dbg_frame_min = 0xFFFF, tft_dbg_frame_max;
static u32 tft_dbg_te_cnt, tft_dbg_frame_cnt;
static bool tft_dbg_skip;               //printf 自身会阻塞主循环几 ms, 丢掉它污染的那一帧

//主循环调用, 每 60 帧(约1秒)打印一次并清零
void tft_dbg_report(void)
{
    if (tft_dbg_frame_cnt < 60) {
        return;
    }
    printf("TE: te=%d..%d cnt=%d | frame=%d..%d cnt=%d (1cnt=2.46us, 1TE=%d)\n",
           tft_dbg_te_min, tft_dbg_te_max, (int)tft_dbg_te_cnt,
           tft_dbg_frame_min, tft_dbg_frame_max, (int)tft_dbg_frame_cnt,
           (int)((XOSC_CLK_HZ / 1000) * TFT_TE_CYCLE) / 64);
    tft_dbg_te_min = tft_dbg_frame_min = 0xFFFF;
    tft_dbg_te_max = tft_dbg_frame_max = 0;
    tft_dbg_te_cnt = tft_dbg_frame_cnt = 0;
    tft_dbg_skip = true;
}
#endif

AT(.com_text.tft_spi)
static void tft_te_refresh(void)
{
    TICK0CNT = 0;
    compo_cb.rtc_cnt = RTCCNT;
    compo_cb.rtc_cnt2 = RTCCON2;
    compo_cb.rtc_update = true;
#if GUI_USE_SCREENSHOOT
    if (!gui_get_screenshot())
#endif
	{
        os_gui_draw();
    }
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

#if TFT_TE_DBG_EN
        {
            u16 t = TICK0CNT;           //须在 tft_te_refresh() 清零 TICK0 之前采样
            if (t < tft_dbg_te_min) tft_dbg_te_min = t;
            if (t > tft_dbg_te_max) tft_dbg_te_max = t;
            tft_dbg_te_cnt++;
        }
#endif
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
                tft_te_refresh();
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
#if TFT_TE_DBG_EN
    {
        u16 t = TICK0CNT;               //本帧从起画到推完的实际耗时
        if (tft_dbg_skip) {
            tft_dbg_skip = false;       //丢掉被上一次 printf 污染的那一帧
        } else {
            if (t < tft_dbg_frame_min) tft_dbg_frame_min = t;
            if (t > tft_dbg_frame_max) tft_dbg_frame_max = t;
        }
        tft_dbg_frame_cnt++;
    }
#endif
    tft_cb.flag_in_frame = false;
    if (tft_cb.tft_bglight_kick) {
        tft_cb.tft_bglight_kick = false;
        tft_cb.te_bglight_cnt = 3; //3TE后打开背光
    }
    if (tft_cb.tft_set_baud_kick) {
        tft_cb.tft_set_baud_kick = false;
        LCDSPIBAUD = tft_cb.despi_baud;
    }
}

//背光亮度初始设置检测
void tft_bglight_frist_set_check(void)
{
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
