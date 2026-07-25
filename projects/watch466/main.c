#include "include.h"
#include "demo/spi1flash_demo.h"
#include "demo/flash_fatfs_demo.h"

const uint8_t *bt_rf_get_inq_param(void)
{
    return NULL;
}

u8 get_chip_package(void)
{
    return 0;
}

const uint8_t *bt_rf_get_param(void)
{
    //优先使用FT参数，其次自定义参数，最后是库预置参数
    if(xcfg_cb.ft_rf_param_en && bt_get_ft_trim_value(&xcfg_cb.rf_pa_gain)) {
        return (const uint8_t *)&xcfg_cb.rf_pa_gain;
    } else if(xcfg_cb.bt_rf_param_en) {
        return (const uint8_t *)&xcfg_cb.rf_pa_gain;
    }

    return NULL;
}

#if	DONGLE_AUTH_EN
bool check_uid_entryption(void);

const u8 tbl_secret_key[16] = {0xA8, 0xE7, 0xB7, 0x74, 0xD1, 0x04, 0x97, 0x3E, 0x8E, 0xB4, 0x7E, 0x4A, 0x36, 0x99, 0xF2, 0xD7};
const u8* get_alg_tbl(void)
{
    return tbl_secret_key;
}

const u8* get_soft_key(void)
{
    return xcfg_cb.soft_key;
}
#endif

//默认OS_THREAD_MAIN_PRIORITY 30
#if ASR_SELECT
AT(.text.startup.init)
u32 system_get_main_stack_priopity(void)
{
    return 27;
}
#endif

#if	ASR_USBKEY_PSD
WEAK const u8* get_soft_key(void)
{
#if ASR_USBKEY_PSD
    return xcfg_cb.asr_soft_key;
#endif
    return NULL;
}
#endif

void run_test()
{
#if FLASH_EXTERNAL_EN
    printf("\t**spi1flash_demo**\n");
    spi1flash_demo();
#endif // FLASH_EXTERNAL_EN

#if FLASH_DISK_EN
    printf("\t**flash_fatfs_demo**\n");
    flash_fatfs_demo();
#endif // FLASH_DISK_EN
}


//正常启动Main函数
int main(void)
{
    printf("Hello **AB5790**,main start\n");
    u32 rst_reason, rtccon10;

    rst_reason = LVDCON;
    rtccon10 = RTCCON10;
    printf("Hello **AB5790**: %08x, CPUID: %d\n", rst_reason, CPUID);

    if (rst_reason & BIT(24)) {
        bsp_rtc_recode_set(1);
        printf("SW reset\n");
    } else if (rst_reason & BIT(19)) {
        if (rtccon10 & BIT(10)) {
            printf("WKO10S reset\n");
        } else if (rtccon10 & BIT(4)) {
            printf("INBOX reset\n");
        } else if (rtccon10 & BIT(15)) {
            printf("VUSB4S reset\n");
        } else {
            printf("RTC_WDT reset\n");
        }
    } else if (rst_reason & BIT(18)) {
        bsp_rtc_recode_set(1);
        printf("WKUP reset\n");
    } else if (rst_reason & BIT(17) || rtccon10 & BIT(3)) {
        bsp_rtc_recode_set(1);
        printf("VUSB reset\n");
    } else if (rst_reason & BIT(16)) {
        bsp_rtc_recode_set(1);
        printf("WDT reset\n");
    }

#if ASR_SELECT
    thread_asr_create();
#endif //ASR_STACK_EN

    bsp_sys_init();
#if	DONGLE_AUTH_EN
    if (check_uid_entryption() == false) {
        printf("Dongle authorization verification failed!\n");
    } else {
        printf("Dongle authorization verification successful!\n");
    }
#endif

    bsp_flash_disk_mount();

#if FUNC_LUNCHBOX_UART_EN
    lunchbox_uart_init(LB_BAUD);
    lunchbox_uart_init_handlers();  // 桥模式也需要注册handler(0x01/0x09/0x0a本地处理)
#if LB_SELFTEST_EN
    func_lunchbox_uart_test();
#endif
#endif

    func_run();
    return 0;
}

//升级完成
void update_complete(int mode)
{
    printf("update_complete: %d\n", mode);
    bsp_update_init();
    if (mode == 0) {
        WDT_DIS();
        while (1);
    }
    WDT_RST();
}
