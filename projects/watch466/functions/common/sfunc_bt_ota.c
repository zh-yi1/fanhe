#include "include.h"
#include "func.h"
#include "func_bt.h"

#if FUNC_BT_EN

AT(.text.func.btring)
void sfunc_bt_ota(void)
{
    printf("%s\n", __func__);
#if (ASR_SELECT && ASR_FULL_SCENE)
    bsp_asr_stop();
#endif
    bt_audio_bypass();
    ota_enter();
    while (bt_get_status() == BT_STA_OTA) {
        WDT_CLR();                  // OTA 期间喂狗，防止 Flash 擦写/蓝牙传输超时导致复位
        bt_thread_check_trigger();
        delay_5ms(10);              // 20ms→50ms：降低 CPU 占用，给充电检测等中断留出时间
    }
    ota_exit();
}

u16 ota_get_flash_size(void){
    return FLASH_SIZE>>16;
}

#endif //FUNC_BT_EN
