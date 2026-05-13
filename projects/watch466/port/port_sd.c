#include "include.h"

#if SD_SUPPORT_EN
static gpio_t sddet_gpio;

void sdcard_detect_init(void)
{
    gpio_t *g = &sddet_gpio;
    u8 io_num = SD_DETECT_IO;
    bsp_gpio_cfg_init(g, io_num);

    if (!io_num) {
        return;
    } else if (io_num == IO_MUX_SDCLK) {
        SD_MUX_DETECT_INIT();
    } else if (io_num <= IO_MAX_NUM) {
        g->sfr[GPIOxDE] |= BIT(g->num);
        g->sfr[GPIOxPU] |= BIT(g->num);
        g->sfr[GPIOxDIR] |= BIT(g->num);
    }
}

AT(.com_text.sdio)
bool sdcard_is_online(void)
{
    gpio_t *g = &sddet_gpio;
    u8 io_num = SD_DETECT_IO;

    if (!io_num) {
        return false;
    } else if (io_num == IO_MUX_SDCLK) {
        return SD_MUX_IS_ONLINE();
    } else {
        return (!(g->sfr[GPIOx] & BIT(g->num)));
    }
}

AT(.com_text.sdio)
bool is_det_sdcard_busy(void)
{
    u8 io_num = SD_DETECT_IO;

    //无SD检测
    if (!io_num) {
        return true;
    }

    if (io_num == IO_MUX_SDCLK) {           //普通IO或复用SDCLK检测
        return SD_MUX_IS_BUSY();
    }
    return false;
}

AT(.text.sdcard)
void sd_gpio_init(u8 type)
{
//    printf("sd_gpio_init [%d]\n", type);
#if SD_SOFT_DETECT_EN || (SD_DETECT_IO != IO_MUX_SDCLK)
    if (type != 0) {
        return;
    }
#endif

    if (type == 0) {
#if SD_SOFT_DETECT_EN || (SD_DETECT_IO != IO_MUX_SDCLK)
        SD_IO_INIT();
#else
        SD_MUX_IO_INIT();
#endif
#if SD_DATA_BUS_4BIT_EN
        SD_MULT_DATA_INIT();
#endif
    } else if (type == 1) {
        SD_CLK_DIR_OUT();
        SD_FCON_INIT();
    } else {
        if(SD_DETECT_IO != IO_MUX_SDCMD){
            SD_CLK_DIR_IN();
        }
    }
}

bool sd_4bit_en(void)
{
    return SD_DATA_BUS_4BIT_EN;
}

bool sd_ddr_en(void)
{
    return SD_DATA_BUS_DDR_EN * SD_DATA_BUS_4BIT_EN;
}

psfr_t sd_use_sfr(void)
{
//    printf("SD1CON INIT\n");
    return SD_SFR_BASE();
}

#else
AT(.text.sdcard)
void sd_gpio_init(u8 type)
{
}
AT(.com_text.sdio)
bool is_det_sdcard_busy(void){ return false;};
#endif          // MUSIC_SDCARD_EN
