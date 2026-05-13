#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


typedef struct {
    volatile u16 lcd_width;
    volatile u16 lcd_height;
    volatile u8 *gui_buff;
    volatile u32 buff0_idx;
    volatile u32 buff1_idx;
    volatile u32 buff_idx;
    volatile bool lcd_is_rotate;
}gui_share_t;

gui_share_t gui_share_cb AT(.share_gui.buff);

#if GUI_SPU_PSRAM
static u8 psram_gui_buff[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2*2]  AT(.psram_gui.buff);
AT(.com_text.gpu_draw)
bool gui_is_user_psram(void){return GUI_SPU_PSRAM;}

AT(.com_text.gpu_draw)
u16 gui_lcd_width(void){return GUI_SCREEN_WIDTH;}

AT(.com_text.gpu_draw)
u16 gui_lcd_height(void){return GUI_SCREEN_HEIGHT;}

AT(.com_text.gpu_draw)
u8 *gui_flush_lcd_buff_get(void)
{
    u8 idx = 0;

    if((gui_share_cb.buff0_idx == 0x00) && (gui_share_cb.buff1_idx == 0x00)){
        return NULL;
    }

    if(gui_share_cb.buff1_idx && gui_share_cb.buff0_idx){
        idx = (gui_share_cb.buff0_idx < gui_share_cb.buff1_idx) ? 0 : 1;
    }else if(gui_share_cb.buff0_idx){
        idx = 0;
    }else {
        idx = 1;
    }

    return (u8*)gui_share_cb.gui_buff + idx*gui_share_cb.lcd_width*gui_share_cb.lcd_height*2;
}


AT(.com_text.gpu_draw)
void gui_flush_lcd_buff_put(u8* buf)
{
    if(buf == NULL){
        return;
    }

    if(buf == gui_share_cb.gui_buff){
        gui_share_cb.buff0_idx = 0;
    }else{
        gui_share_cb.buff1_idx = 0;
    }
}
#endif

bool keep_ram_tbl_restore(void);
bool keep_ram_tbl_load(void);

//推屏缓存大小(双份)
#define GUI_LINES_BUF_SIZE              (GUI_SCREEN_WIDTH * GUI_LINES_CNT * 2 * 2)
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
#define GUI_ALI_LINES_BUF_SIZE          (GUI_SCREEN_WIDTH * 4 * 2 * 2)
#endif

//以下缓存大小一般情况下不需要修改
#define GUI_ELE_BUF_SIZE    (4096 + 1536 + 7168 * FUNC_GAME_TETRIS_EN) //element缓存
#define GUI_WGT_BUF_SIZE    (13748 + 2636 + 2048 * FUNC_GAME_TETRIS_EN)   //widget缓存

#define GUI_MAXSIZE_TEMPBUF             0x6000                  //中间临时计算缓存32k
#define GUI_MAXSIZE_TEMPBUF2            0x2000                  //中间计算缓存2
#define GUI_MAXSIZE_PARBUF              8096                    //PAR解码缓存
#define GUI_MAX_FONT_SIZE               128                     //单个字最大尺寸

static u8 gui_lines_buf[GUI_LINES_BUF_SIZE] AT(.disp.buf);      //推屏缓存(双份)
static u8 gui_element_buf[GUI_ELE_BUF_SIZE * 2] AT(.disp.ele_buf);  //Element Buf
static u8 gui_widget_buf[GUI_WGT_BUF_SIZE] AT(.disp.wdg_buf);       //Widgets Buf
static u8 gui_temp_buf[GUI_MAXSIZE_TEMPBUF] AT(.disp.temp_buf);      //中间计算缓存
static u8 gui_temp_buf2[GUI_MAXSIZE_TEMPBUF2] AT(.disp.temp_buf2);    //中间计算缓存2，放不同RAM加大带宽
#if GUI_USE_SCREENSHOOT
u8 cur_scbuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2+8] AT(.psram_buf.lcd);
u8 next_scbuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2+8] AT(.psram_buf.lcd);
#if GUI_USE_BLUR
u8 blur_obuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2+8] AT(.psram_buf.lcd);
u8 blur_tbuf[GUI_SCREEN_WIDTH*GUI_SCREEN_HEIGHT*2*2] AT(.psram_buf.lcd);
#endif
#endif
static u8 gpu_init_flag = 0;

//GUI初始化配置表
static const gui_init_param_t tbl_gui_init_param = {
    .screen_width = GUI_SCREEN_WIDTH,
    .screen_height = GUI_SCREEN_HEIGHT,
    .element_buf = gui_element_buf,
    .widget_buf = gui_widget_buf,
    .element_buf_size = GUI_ELE_BUF_SIZE,
    .widget_buf_size = GUI_WGT_BUF_SIZE,
    .temp_buf = gui_temp_buf,
    .temp_buf_size = GUI_MAXSIZE_TEMPBUF,
    .lines_buf = gui_lines_buf,
    .lines_buf_size = GUI_LINES_BUF_SIZE,
    .lines_count = GUI_LINES_CNT,
    .maxsize_parbuf = GUI_MAXSIZE_PARBUF,
    .font_res_addr = UI_BUF_FONT_SYS,
    .max_font_size = GUI_MAX_FONT_SIZE,
    .font_wspace = GUI_FONT_W_SPACE,
    .font_hspace = GUI_FONT_H_SPACE,
    .temp_buf2 = gui_temp_buf2,
    .temp_buf2_size = GUI_MAXSIZE_TEMPBUF2,
};

bool gui_set_ram_check(void* ptr, const char* func_name)
{
    //检测ram对不对
    u8* ram = (u8*)ptr;
    if (GET_LE32(&ram[0]) != 0x24150 || GET_LE16(&ram[4]) == 0 || GET_LE16(&ram[6]) == 0) {
        printf("%s<0x%x>:%d*%d\n", func_name, GET_LE32(&ram[0]), GET_LE16(&ram[4]), GET_LE16(&ram[6]));
        printf("show err\n");
        return false;
    }

//    printf("%s<0x%x>:%d*%d\n", func_name, GET_LE32(&ram[0]), GET_LE16(&ram[4]), GET_LE16(&ram[6]));
    return true;
}

extern u32 __psram_vma;

void* psram_switch_cache(void* ptr)
{
    if ((u32)ptr >= (u32)&__psram_vma) {
        return (void*)((u32)ptr | BIT(28));
    }
    return ptr;
}

void gui_sw_init(void)
{
    if (!gpu_init_flag) {           //GPU没有初始化没有办法初始化GUI
        return;
    }

    if (sys_cb.frist_gui_sw_init) {
        if (!sys_cb.gpu_exit_ever) {       //gpu没有退出过,不需要重新初始化GUI
            return;
        }
    }

    printf("%s\n", __func__);
    sys_cb.frist_gui_sw_init = true;
    os_gui_init(&tbl_gui_init_param);
    compos_init();

    if (sys_cb.gpu_exit_ever) {     //restore ram
        keep_ram_tbl_restore();
    }

    sys_cb.gpu_exit_ever = false;
}


//GPU初始化
void gpu_init(void)
{
    if (!gpu_init_flag) {
        gpu_pg_on();
        RSTCON0 &= ~BIT(10);
        CLKGAT3 |= BIT(0);          //GPU CLK EN
        RSTCON0 |= BIT(10);         //GPU Release

        if (UI_BUF_0GPU_GPU_BIN >= UI_EX_ADDR_BASE(0)) {
#if FLASH_EXTERNAL_EN
            spi1flash_read((void *)0x310000, UI_BUF_0GPU_GPU_BIN - UI_EX_ADDR_BASE(0), UI_LEN_0GPU_GPU_BIN);
#endif
        } else {
            os_spiflash_read((void *)0x310000, UI_BUF_0GPU_GPU_BIN, UI_LEN_0GPU_GPU_BIN);
        }

        CLKGAT3 |= BIT(10) | BIT(11) | BIT(12) | BIT(13); //wdt10/tick1x/tmr1x/wpt10 clk en
        CLKGAT0 |= BIT(23);         //SPI2
        CLKGAT2 |= BIT(28) | BIT(29);   //GPDMA
        CLKGAT3 |= BIT(1);          //GPU RV CLK EN
        gpu_init_flag = 1;
        printf("%s\n", __func__);
        gui_sw_init();
    }

}

void gpu_exit(void)
{
    if (keep_ram_tbl_load() == false) {
        // 先这样，直接不给休眠了，退出界面清所有信息方式运行 todo
        printf("keep_ram_tbl_load fail exit sleep\n");
    }

    CLKGAT3 &= ~BIT(1);
    CLKGAT3 &= ~BIT(0);
    RSTCON0 &= ~BIT(10);
    CLKGAT3 &= ~(BIT(10) | BIT(11) | BIT(12) | BIT(13));
    CLKGAT0 &= ~BIT(23);
    RSTCON0 &= ~BIT(10);
    gpu_pg_off();
    gpu_init_flag = 0;
    sys_cb.gpu_exit_ever = true;
    void sys_clk_gpu_limti_close(void);
    sys_clk_gpu_limti_close();
    printf("%s\n", __func__);
}

u8 is_gpu_init(void)
{
    return  gpu_init_flag;
}


//GUI相关初始化
void gui_init(void)
{
//    led_pg_on();
#if (GUI_SELECT == GUI_VGS_640)
// PE9拉低，否则供电 1.8V纹波较大

    gpio_t gpio;
	bsp_gpio_cfg_init(&gpio, IO_PE9);
    if (gpio.sfr) {
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
        gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
    }
    GPIOECLR = BIT(9);
#endif

    LCD_POWER_EN();
    gpu_init();
#if CTP_SELECT != CTP_NO
    ctp_init();
#endif // CTP_SELECT
    tft_init();

    sys_cb.sleep_en = 1;            //允许进休眠
    sys_cb.gui_sleep_sta = 0;

}

//即将进入休眠时会关闭psram，正在使用截图的任务需要在这里切换到非截图的界面
void gui_sleep_psram_check(void)
{
#if GUI_USE_SCREENSHOOT
    if (func_cb.sta == FUNC_CLOCK || func_cb.sta == FUNC_CARD) {
        if (func_cb.sta == FUNC_CLOCK && ((f_clock_t *)func_cb.f_cb)->sta != FUNC_CLOCK_SUB_DROPDOWN) {
            return;
        }

        if (func_cb.sta == FUNC_CARD) {
            func_cb.sta = FUNC_CLOCK;
            task_stack_push(func_cb.sta);
        } else if (func_cb.sta == FUNC_CLOCK) {
            ((f_clock_t *)func_cb.f_cb)->sta = FUNC_CLOCK_MAIN;
        }
        compo_form_t *func_clock_form_create(void);
        compo_form_destroy(func_cb.frm_main);
        func_cb.frm_main = func_clock_form_create();
        os_gui_draw_force();
        delay_5ms(20);

    }
#endif
    os_gui_draw_w4_done();
    lcd_drv_clk_deregister();
}

void gui_sleep(bool is_gpu_exit)
{
    if (!sys_cb.gui_sleep_sta) {
        os_gui_draw_w4_done();      //关tft前要等当前帧刷完
        tft_exit();
#if CTP_SELECT != CTP_NO
        ctp_exit();
#endif // CTP_SELECT

        LCD_BL_DIS();
        if(!vddio_sleep_level) {
            LCD_POWER_DIS();
        }
        sys_cb.gui_sleep_sta = 1;
        sys_cb.gui_need_wakeup = 0;
        printf("gui_sleep\n");
    }
    if (is_gpu_exit) {
        gpu_exit();
    }
}

void gui_wakeup(void)
{
    if (sys_cb.gui_sleep_sta) {
        gpu_init();
//        led_pg_on();
#if (GUI_SELECT == GUI_VGS_640)
// PE9拉低，否则供电 1.8V纹波较大
    gpio_t gpio;
	bsp_gpio_cfg_init(&gpio, IO_PE9);
    if (gpio.sfr) {
        gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
        gpio.sfr[GPIOxSET] = BIT(gpio.num);
        gpio.sfr[GPIOxDE] |= BIT(gpio.num);
        gpio.sfr[GPIOxFEN] &= ~BIT(gpio.num);
    }
    GPIOECLR = BIT(9);
#endif
		bsp_pwm_wakeup();
        LCD_POWER_EN();
#if CTP_SELECT != CTP_NO
        ctp_init();
#endif // CTP_SELECT
        tft_init();
        gui_widget_refresh();
        sys_cb.gui_sleep_sta = 0;
        sys_cb.gui_need_wakeup = 1;
        printf("gui_wakeup\n");
    }
}

AT(.com_text.gui)
bool gui_get_auto_power_en(void)
{
    return GUI_AUTO_POWER_EN;
}

bool gui_font_get_align_top(void)
{
    return true;
}

////是否打开字库打印
//bool unicode_show_info(void)
//{
//    return true;
//}

bool qr_encode_use_malloc(void)
{
	return true;
}

#if GUI_USE_BLUR
//占用大小：8*max(w, h)
void *blur_buf_malloc(u32 size)
{
    return ab_malloc(size);
}

void blur_buf_free(void *p_buf)
{
    ab_free(p_buf);
}
#endif


//蓝屏
AT(.com_text.hwio)
void gui_halt(u32 halt_no)
{
    int i;
    tft_bglight_en();               //打开背光
    tft_frame_end();
    for (i=0; i<20000; i++) {
        asm("nop");                  //足够的延时，保证前面SPI推完
    }
    FUNCMCON0 = (1 << 4);           //SPI0 Map To G1
    tft_frame_start();
    for (i=0; i<GUI_SCREEN_HEIGHT; i++) {
        de_fill_rgb565(gui_lines_buf, COLOR_BLUE, GUI_SCREEN_WIDTH);
        if (GUI_SCREEN_WIDTH >= 96 && GUI_SCREEN_HEIGHT >= 10 &&
            i >= GUI_SCREEN_CENTER_Y - 5 && i < GUI_SCREEN_CENTER_Y + 5) {
            de_fill_num(gui_lines_buf + GUI_SCREEN_CENTER_X * 2 - 96, halt_no, i - (GUI_SCREEN_CENTER_Y - 5));
        }
        tft_spi_send(gui_lines_buf, GUI_SCREEN_WIDTH, 1);
    }
    tft_frame_end();
}
