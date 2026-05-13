#ifndef __ASR_CONFIG_H__
#define __ASR_CONFIG_H__

#include "include.h"

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

#include "func_api_air.h"
#include "ir.h"
#include "ir_index.h"
#include "wanson_ir.h"



#define AIR_SEARCH_TIME     5 //空调搜索一次间隔时间 单位:S

enum{
    AIR_OFF,
    AIR_SEARCH,
};


typedef struct ISD_REMOTE_DATA{
    int32_t offset;
    int32_t len;
    int16_t brand_id;
    int8_t  brand_index;
}_ISD_REMOTE_DATA;
extern _ISD_REMOTE_DATA isd_remote_data;
extern AIR_STA_T air_sta_data;


extern uint8_t  p_remote_dat[]; //遥控码数据
extern uint16_t pwm_out[];
extern int32_t  buf_prd[];


extern uint16_t remote_num, remote_id, remote_handle;
extern uint16_t remote_data_len;
extern int32_t  remote_data_offset;



extern void air_flash_write(void);
extern int air_flash_read(void);
extern void asr_start_stop(uint8_t flag);

#endif //ASR_WS_AIR

#endif //__ASR_CONFIG_H__
 