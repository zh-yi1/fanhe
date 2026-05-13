#include "include.h"
#include "api.h"

char redial_buf[2][32] AT(.btmem.hfhs);

void bt_incall_set_number(const char *number);

void bt_redial_init(void)
{
    memset(redial_buf, 0, 64);
}

void bt_redial_reset(uint8_t index)
{
    memset(redial_buf[index], 0, 32);
}

void bt_update_redial_number(uint8_t index, char *buf, u32 len)
{
    if (len < 32) {
        memset(redial_buf[index], 0, 32);
        memcpy(redial_buf[index], buf, len);
    }
}

const char *bt_get_last_call_number(uint8_t index)
{
    if (redial_buf[index][0]) {
        return (const char *)redial_buf[index];
    } else {
        return NULL;
    }
}

const char *hfp_get_last_call_number(uint8_t index)
{
    return bt_get_last_call_number(index);
}

bool hfp_is_list_curr_calls(void)
{
    return true;
}

/**
 * 拨打出去待电话号码通知
 * 比如手机端拨打电话出去，会主动把电话告知设备端
 * idx, type 两个参数不用理会，协议端对其形参
 * number 电话号码字符串
 * len 电话长度
 */
void hfp_hf_emit_curr_calls(u8 idx, u8 type, const char * number, u32 len)
{
    bt_update_redial_number(idx, (char *)number, len);
    hfp_hf_call_notice(BT_NOTICE_CALL_NUMBER);
}

/************************使用示例************************/
////按键发起回拨号码
//bt_call_redial_number();
//delay_5ms(10); //延迟一下，等它发送完毕
//
//此函数返回需要回拨的号码，例如“10086”
char * hfp_get_outgoing_number(void)
{
    //return "10086";
    return sys_cb.outgoing_number;
}

////发送自定义ATCMD
//bt_hfp_send_at_cmd();
//delay_5ms(10); //延迟一下，等它发送完毕
//
 //此函数返回需要发送的ATCMD
char *hfp_get_at_cmd(void)
{
//    return "AT+VTS=1"; //例如，通话过程发起号码键"1"
#if BT_HFP_GET_TIME_EN
    return "AT+CCLK?\r";//获取IOS手机时间（安卓暂不支持），获取回调函数hfp_get_time
#else
    return "ATD10000;"; //也可以，发起回拨号码"10086"
#endif
}

//获取IOS手机时间（安卓暂不支持）,需要先发送"AT+CCLK?\r"AT命令
//void hfp_notice_network_time(u8 *buf, u16 len)
//{
//    char cache[12];
//    memcpy(cache, buf, 12);
//    printf("%s\n", cache);
//}

uint hfp_get_mic_gain(void)
{
    return 9;
}

uint hfp_get_spk_gain(void)
{
    return (sys_cb.hfp_vol>15)? 15 : sys_cb.hfp_vol;
}

#if BT_HFP_BAT_REPORT_EN
uint hfp_get_bat_level(void)
{
    //计算方法：level = (实测电压 - 关机电压) / ((满电电压 - 关机电压) / 10)
    u16 bat_off = LPWR_OFF_VBAT;
    if (bat_off > sys_cb.vbat) {
        return 0;
    }
    if (bat_off < 2800) {
        bat_off = 2800;
    }
    uint bat_level = (sys_cb.vbat - bat_off) / ((4200 - bat_off) / 10);
    if (bat_level) {
        bat_level--;
    }
//    printf("bat level: %d %d\n", sys_cb.vbat, bat_level);
    return bat_level;
}
#endif

#if BT_HFP_GET_TIME_EN
////获取IOS手机时间（安卓暂不支持）,需要先发送"AT+CCLK?\r"AT命令
void bt_get_time(char *ptr, uint32_t len, uint8_t format);
void hfp_notice_network_time(u8 *buf, u16 len)
{
    bt_get_time((char *)buf, len, 1);
}
#endif
