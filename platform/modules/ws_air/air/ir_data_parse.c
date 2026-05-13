#include "asr_config.h"

#if (ASR_SELECT == ASR_WS_AIR && IR_AIR_FUNC)

uint8_t  p_remote_dat[1024] AT(.ir_rmt.dat);
uint16_t pwm_out[1024]      AT(.ir_pwm.out);
int32_t  buf_prd[1024]      AT(.ir_prd.rcv);


uint8_t  isd_search_type;
uint16_t remote_num, remote_id, remote_handle, remote_data_len, remote_brand;
uint8_t  remote_index = 0; //空调品牌的第几组
int32_t  remote_data_offset;
_ISD_REMOTE_DATA isd_remote_data;

static uint8_t rmt_first_init; //匹配过的空调，二次上电，不允许直接使用 下一套遥控码，需先进行匹配后 再使用


int32_t buf_prd_init[] AT(.kws_ir.bufPrd) = {

    9079, 4475, 618, 1622, 629, 516, 650, 509, 652, 1625, 614, 1646, 552, 531, 657, 1630, 629, 515, 662, 524,
    660, 524, 672, 520, 661, 1641, 652, 534, 662, 534, 662, 523, 662, 535, 664, 524, 675, 515, 618, 559, 648,
    511, 641, 526, 657, 1622, 615, 520, 656, 515, 640, 519, 658, 522, 644, 515, 657, 527, 642, 1626, 614, 523,
    656, 1634, 636, 522, 660, 535, 662, 1641, 652, 534, 661, 20085, 618, 1638, 630, 535, 661, 524, 662, 534,
    662, 1641, 652, 534, 662, 535, 650, 534, 662, 535, 664, 524, 664, 508, 654, 503, 650, 496, 644, 1639, 630,
    508, 637, 528, 646, 507, 653, 522, 658, 513, 650, 530, 597, 520, 650, 516, 661, 515, 657, 524, 662, 512, 662,
    524, 664, 532, 661, 535, 653, 535, 662, 1638, 662, 1638, 653, 1641, 641, 40205, 9067, 4441, 662, 1639, 662, 523,
    664, 535, 664, 1641, 615, 1623, 630, 520, 661, 1609, 636, 513, 652, 524, 646, 526, 656, 526, 649, 1629, 617, 527,
    656, 513, 652, 532, 658, 531, 664, 524, 664, 528, 664, 523, 661, 535, 662, 534, 653, 1639, 662, 534, 664, 524,
    664, 516, 624, 505, 661, 512, 656, 507, 653, 1631, 629, 1630, 629, 1626, 629, 508, 660, 524, 624, 1665, 615, 522,
    661, 20061, 626, 517, 656, 509, 660, 520, 658, 511, 648, 523, 654, 523, 652, 512, 660, 522, 658, 511, 661, 528,
    661, 530, 652, 535, 661, 534, 652, 532, 662, 532, 662, 524, 662, 534, 664, 524, 664, 535, 637, 526, 637, 1622,
    630, 523, 661, 512, 644, 523, 660, 505, 654, 522, 660, 517, 656, 504, 654, 475, 642, 519, 660, 1637, 587, 1633, 634, 0,
};

void asr_timer_cnt_clear(void);

/**
 * @brief 在码库读取遥控器数据
 * @param [in] {device_type}     设备号
 * @return 0: OK    -1:error
 */
AT(.kws_ir.com)
int isd_read_remote_code(unsigned char device_type)
{
    if(device_type == 0) {
        printf("goto _func_end\n");
        goto _func_end;
    }

    asr_start_stop(0); //关闭语音识别

    //读取某一品牌的空调索引范围数据
    memset(p_remote_dat, 0, sizeof(p_remote_dat));

    int32_t i, j = 0;
    for (i = remote_data_offset; i < remote_data_len + remote_data_offset; i++) {
        p_remote_dat[j] = air_remote_dat[i];
        // printf("%02X\n", p_remote_dat[j]);
        j++;
    }
    // printf("\n");

_func_end:
    isd_remote_data.offset = remote_data_offset;
    isd_remote_data.len = remote_data_len;
    isd_remote_data.brand_id = remote_brand;
    isd_remote_data.brand_index = remote_index;

    air_flash_write();

    return 0;
}


AT(.kws_ir.com)
int isd_check_ir_dat(void)
{
    int32_t ret = 0;
    extern uint16_t ir_data_len;

    // 将获取到的红外遥控器数据,拿去空调索引文件(air_index)匹配
    ret = wanson_search_remote(air_index, sizeof(air_index), (int32_t*)&buf_prd,
        ir_data_len, 5, &remote_num, &remote_handle,
        &remote_id, &remote_data_offset, &remote_data_len);

    if (ret != 0) {
        printf("\r\nsearch_remote return error:ret=%d\r\n", ret);
        return -1;
    }
    printf(
        "search_remote: ret=%d, remote_num=%d, remote_handle=%d, remote_id=%d, remote_data_offset=%d, remote_data_len=%d\n",
        ret, remote_num, remote_handle, remote_id, remote_data_offset,
        remote_data_len);

    remote_index = 0;
    rmt_first_init = 0;

    isd_read_remote_code(0);
    air_remote_data_init();

    isd_search_type = 1;

    return 0;
}


AT(.kws_ir.com)
void isd_init_remote_data(void)
{
    asr_start_stop(0); //关闭语音识别

    int32_t ret = 0;

    ret = air_flash_read();

    for (u16 i = 0; i < 280; i++) {
        buf_prd[i] = buf_prd_init[i];
    }

    if (ret != 0) //没读到，使用 buf 默认码库
    {
        ret = wanson_search_remote(air_index, sizeof(air_index), (int32_t*)&buf_prd,
            1024, 5, &remote_num, &remote_handle,
            &remote_id, &remote_data_offset, &remote_data_len);
        if (ret != 0) {
            printf("\r\nsearch_remote return error:ret=%d\r\n", ret);
            return;
        }
        remote_brand = 0;
        remote_index = 0;
        isd_search_type = 1;
    } else {
        remote_data_offset = isd_remote_data.offset;
        remote_data_len = isd_remote_data.len;
        remote_brand = isd_remote_data.brand_id;
        remote_index = isd_remote_data.brand_index;

        rmt_first_init = 1;
    }
    printf(
        "search_remote: ret=%d, remote_num=%d, remote_handle=%d, remote_id=%d, remote_data_offset=%d, remote_data_len=%d\n",
        ret, remote_num, remote_handle, remote_id, remote_data_offset,
        remote_data_len);
    
    isd_read_remote_code(0);
    isd_search_type = 1;

    asr_start_stop(1); //语音识别
}


AT(.kws_ir.com)
int isd_brand_get_next_remote(uint8_t type)
{
    int ret;
    u8 lastrmt = 0;
    printf("type............... = %d \n", type);

    if(rmt_first_init) {
        printf("Please pair air first!!! \n");
        return 1;
    }
    if(remote_index >= remote_num) {
        printf("This is already the last remote code!!! \n");
        lastrmt = 1;
        goto _search_end;
    }

    if (type == 0) {
        ret = wanson_get_next_remote(air_index_brand, sizeof(air_index_brand), remote_handle, &remote_handle,
            &remote_id, &remote_data_offset, &remote_data_len);

    } else if (type == 1) {
        ret = wanson_get_next_remote(air_index, sizeof(air_index), remote_handle, &remote_handle,
            &remote_id, &remote_data_offset, &remote_data_len);
    } else {
        ret = wanson_get_remote_one_by_one(ir_all_index_buf, sizeof(ir_all_index_buf), &remote_num,
            &remote_handle, &remote_id, &remote_data_offset, &remote_data_len);
    }

    if (ret != 0) {
        printf("get_next_remote return error:ret=%d\n", ret);
        return 1;
    }
    if (type == 0) {
        remote_index++;
    }
    printf("curr remote_index: %d \n\n", remote_index);

_search_end:
    printf(
        "get_next_remote: ret=%d, remote_handle=%d, remote_id=%d, remote_data_offset=%d, remote_data_len=%d\n",
        ret, remote_handle, remote_id, remote_data_offset, remote_data_len);

    isd_read_remote_code(0);
    air_remote_data_init();

    event_air_power_on();
    isd_search_type = 0;

    if(lastrmt) {
        bsp_asr_tone_play(RES_BUF_ASR_94_MP3, RES_LEN_ASR_94_MP3);
        return 2;
    }
    return 0;
}


extern void air_time_start(uint8_t part, uint8_t en);

AT(.kws_ir.com)
void isd_get_next_remote(void)
{
    asr_timer_cnt_clear(); //搜索空调期间，不退出唤醒

    int ret;
    printf("remote_handle=%d\n", remote_handle);
    ret = wanson_get_next_remote(ir_all_index_buf, sizeof(ir_all_index_buf), remote_handle, &remote_handle,
        &remote_id, &remote_data_offset, &remote_data_len);

    if (ret != 0) {
        printf("get_next_remote return error:ret=%d\n", ret);
        air_time_start(2, 0);
        return;
    }
    printf(
        "isd_get_next_remote: ret=%d, remote_handle=%d, remote_id=%d, remote_data_offset=%d, remote_data_len=%d\n",
        ret, remote_handle, remote_id, remote_data_offset, remote_data_len);

    rmt_first_init = 0;

    isd_read_remote_code(0);
    air_remote_data_init();

    event_air_power_on();
    isd_search_type = 3;

    // 添加定时
    air_time_start(AIR_SEARCH, 1);
}

AT(.kws_ir.com)
void isd_exit_search_remote(void)
{
    air_time_start(AIR_SEARCH, 0);
}


AT(.kws_ir.com)
void isd_search_remote_onebyone(void)
{
    asr_timer_cnt_clear(); //搜索空调期间，不退出唤醒

    int ret;
    ret = wanson_get_remote_one_by_one(ir_all_index_buf, sizeof(ir_all_index_buf), &remote_num,
        &remote_handle, &remote_id, &remote_data_offset, &remote_data_len);
    if (ret != 0) {
        printf("get_remote_one_by_one return error:ret=%d\n", ret);
        return;
    }
    printf(
        "get_remote_one_by_one: ret=%d, remote_num=%d, remote_handle=%d, remote_id=%d, remote_data_offset=%d, remote_data_len=%d\n",
        ret, remote_num, remote_handle, remote_id, remote_data_offset,
        remote_data_len);

    rmt_first_init = 0;

    isd_read_remote_code(0);
    air_remote_data_init();

    event_air_power_on();
    isd_search_type = 3;

    // 添加定时
    air_time_start(AIR_SEARCH, 1);
}


AT(.kws_ir.com)
int isd_set_ir_brand(int index)
{
    const unsigned short brand_tab[] = {
        32, 27, 192, 197, 202, 97, 7, 37, 577, 182, 17, 207, 187, 2782, 257, 227,
        147, 487, 57, 2777, 2, 1577, 117, 149, 12, 102, 1437, 22, 3050, 1452, 212,
    };

    char ret;
    remote_brand = brand_tab[index];
    remote_index = 0;
    ret = wanson_get_remote_by_brand(air_index_brand, sizeof(air_index_brand), remote_brand, &remote_num,
        &remote_handle, &remote_id, &remote_data_offset, &remote_data_len);
    printf(
        "isd_set_ir_brand: ret=%d, remote_num=%d, remote_handle=%d, remote_id=%d, remote_data_offset=%d, remote_data_len=%d\n",
        ret, remote_num, remote_handle, remote_id, remote_data_offset,
        remote_data_len);
    if (ret != 0) {
        printf("kk_get_remote_by_brand failed\n");
        return ret;
    } else {
        // mos_sent_msg_event(MOS_MSG_WECHAT_SEND_BRAND, NULL);
    }
    rmt_first_init = 0;
    
    isd_read_remote_code(0);
    air_remote_data_init();
    
    isd_search_type = 0;
    
    return 0;
}


AT(.kws_ir.com)
void air_data_init()
{
    air_remote_data_init();
    isd_init_remote_data();
}

#endif //AIR_WS_AIR