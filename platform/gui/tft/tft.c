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

tft_cb_t* tft_get_tft_cb(void)
{
    return &tft_cb;
}

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
