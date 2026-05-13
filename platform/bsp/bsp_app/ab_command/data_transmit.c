#include "include.h"
#include "ab_common.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)

u8 watch_market_get_diy_index(void);

///数据传输类
void dial_updata_stuta(u8 value)
{
//    market_dial_info_t *dial_p = &app_data.market_dial_info;
//    if(value == true) {
//        dial_p->enter_updata = true;
//        gcover_break_recover_clear_sleep();
//
//        sys_cb.cover_index = GCOVER_DIAL_UPDATA;
//        msg_queue_detach(EVT_WATCH_SET_COVER, false); //20221021 wang  修改息屏指针弹OTA卡死问题
//        gui_set_cover_index(GCOVER_DIAL_UPDATA);
//        gui_clr();				//清除表盘控件，防止擦除后导致显示花屏
//    } else {
//        if(dial_p->enter_updata){
//            dial_p->enter_updata = false;
//        }
//
//        if (value == false) {
//            //切到表盘市场
//            sys_cb.watch_index_save = sys_cb.watch_index;
//            sys_cb.watch_index = watch_market_get_diy_index();
//            app_data.sector0.watch_diy_cb.flag = 0x01;
//            printf("%s: %d %d\n", __func__, sys_cb.watch_index, app_data.sector0.watch_diy_cb.flag);
//        }
//    }

}

void wach_diy_info(u8 data_type)
{
//    u8 upload_buf[12] = {0x00, 0xe0, 0x02, 0x00, 0x07, 0x00};//表盘市场
//    upload_buf[6] = data_type;
//    if (data_type == 0x01) {//屏幕信息
////        upload_buf[5] = 0x07;
//        upload_buf[7] = WATCH_TYPE;
//        buf_get_u16(&upload_buf[8], LCD_WIDTH_SIZE);
//        buf_get_u16(&upload_buf[10], LCD_HEIGHT_SIZE);
//    }
//    app_protocol_tx(upload_buf, upload_buf[4] + INSTRUCT_HEADER_LEN);
}

u8 watch_diy_config(uint8_t *buf, uint16_t len)
{
//    market_dial_info_t *dial_p = &app_data.market_dial_info;
//    watch_diy_t *watch_diy_p = &app_data.sector0.watch_diy_cb;
//
//    dial_p->enter_updata_times = 0;
//
//    ble_update_conn_param(4, 0, 400);
//    printf("------>watch_diy_update_conn_param\n");
//    delay_5ms(10);
//    //进入表盘更新
//    dial_p->package_num = 0;
//    dial_p->package_total = (buf[7] << 8) + buf[6];
//    dial_p->bin_size = (buf[11] << 24) + (buf[10] << 16) + (buf[9] << 8) + buf[8];
//    dial_p->progress_per = 0;
//    dial_p->mtu_size = (buf[13] << 8) + buf[12];
//    dial_p->dial_type = buf[14] & 0x01;
//    watch_diy_p->location = buf[16];
//    watch_diy_p->color = (buf[20] << 8) + buf[19];
//    watch_diy_p->flag = 0x00;
//    // dial_p->enter_updata = true;
//    dial_updata_stuta(true);
//
//    printf("pk[%d] bin[%d] mtu[%d] type[%d] local[%d] color[%x]\n",
//           dial_p->package_total, dial_p->bin_size, dial_p->mtu_size, dial_p->dial_type, watch_diy_p->location, watch_diy_p->color);
//
//    //表盘市场擦除
//    for (u8 i = 0; i < FLASH_MARKET_DIAL_SIZE/4096; i++) {
//        os_spiflash_erase(FLASH_MARKET_DIAL_ADDR + i * 4096);
//    }
//
    return 0;
}

u16 market_check_sum_get(u8 *buf, u16 len)
{
    return (check_sum_get(buf, len) - buf[14] - buf[15]);   //剔除校验和本身的值
}

void watch_diy_transmit(u8 *buf, uint16_t len)
{
//    u8 response = 0;
//    market_dial_info_t *dial_p = &app_data.market_dial_info;
//    watch_diy_t *watch_diy_p = &app_data.sector0.watch_diy_cb;
//
//    uint32_t bin_num = 0;
//    dial_p->package_num = (buf[7] << 8) + buf[6];
//    u16 data_sum = (buf[15] << 8) + buf[14];
//    dial_p->check_data = market_check_sum_get(buf, len);
//    watch_diy_p->flag = 0x00;
//    dial_p->progress_per = buf[12];
//
//    bin_num = (buf[11] << 24) + (buf[10] << 16) + (buf[9] << 8) + (buf[8]); //bin文件位置
////    printf("package_num[%d] prog[%d] bin_num[%d] data_sum[%x]\n", dial_p->package_num, dial_p->progress_per, bin_num, data_sum);
//
//    if ((dial_p->package_num == 1) && (sys_cb.cover_index != GCOVER_DIAL_UPDATA)) {
//        gui_set_cover_index(GCOVER_DIAL_UPDATA);
//    } else if (dial_p->package_num == dial_p->package_total && !sys_cb.charge_sta) {//充电时不用break
//        gcover_break_recover();
//    }
//
//    if (dial_p->check_data == data_sum) {//校验成功
////        printf("data_sum ok\n");
//        if (dial_p->package_num == 90) {//test
//            buf[13] = 0x01;
//        }
//
//        if (buf[13] == 0x00) {//继续更新
////            printf("watch_dial continue\n");
//        } else if (buf[13] == 0x01) {//更新结束
//            if (dial_p->dial_type) {
//                watch_diy_p->flag = 0x02;
//            } else {
//                watch_diy_p->flag = 0x01;
//            }
//            watch_diy_p->addr = FLASH_MARKET_DIAL_ADDR;
////            printf("watch_dial end, data_len[%d] space[%d]\n", bin_num + buf[4] - 11, FLASH_MARKET_DIAL_SIZE);
//			if(dial_p->bin_size == (bin_num + buf[4] - 11)){//传输完成
//				dial_updata_stuta(false);
//			}
//        } else if (buf[13] == 0x02) {//传输退出
//            dial_updata_stuta(0x03);
////            printf("watch_dial exit\n");
//        }
//
//        if (bin_num + buf[4] - 11 < FLASH_MARKET_DIAL_SIZE) {//数据长度合法
//            os_spiflash_program(&buf[16], FLASH_MARKET_DIAL_ADDR + bin_num, len - 16);
//        } else {
//            response = CMD_RESULT_LEN_INVALID;
//            printf("[err] watch_dial overflow\n");
//        }
//
//    } else {
//        response = CMD_RESULT_CHECK_ERROR;
//        printf("[err] watch_dial sum fail\n");
//    }
//
//    watch_diy_transmit_rsp(buf, response);
}

#endif
