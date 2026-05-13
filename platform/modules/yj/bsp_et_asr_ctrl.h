#ifndef __BSP_ET_ASR_CTRL_H
#define __BSP_ET_ASR_CTRL_H

#if (ASR_SELECT == ASR_YJ)

#if ET_WAKEUP_EN

void et_bsp_ctrl_main_init();
void et_bsp_ctrl_exit();
u32 et_asr_get_time_cnt();

void et_open_asr_record();
void et_close_asr_record();
#endif // ET_WAKEUP_EN

#endif //ASR_SELECT

#endif
