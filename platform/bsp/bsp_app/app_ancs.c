#include "include.h"

#if LE_ANCS_CLIENT_EN

#define TRACE_EN    0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


static ble_ancs_msg_cb_t ancs_msg_cb;

const char * const appleAppID_table[] = {
    [ANCS_TYPE_NULL]                = " ",
    [ANCS_TYPE_INCOMINGCALL]        = "com.apple.mobilephone",        //wakeUpAppID 01, Incoming Call (Apple)
    [ANCS_TYPE_MISSEDCALL]          = "com.apple.mobilephone",        //wakeUpAppID 02, Missed Call (Apple)
    [ANCS_TYPE_MESSAGES]            = "com.apple.MobileSMS",          //wakeUpAppID 03, Messages (Apple)
    [ANCS_TYPE_MAIL]                = "com.apple.mobilemail",         //WakeUpAppID 04 , Mail (Apple)
    [ANCS_TYPE_CALENDAR]            = "com.apple.mobilecal",          //WakeUpAppID 05 , Calendar (Apple)
    [ANCS_TYPE_FACETIME]            = "com.apple.facetime",           //WakeUpAppID 06 , FaceTime (Apple)
    [ANCS_TYPE_QQ]                  = "com.tencent.mqq",              //WakeUpAppID 07 , QQ
    [ANCS_TYPE_SKYPE]               = "com.skype.skype",              //WakeUpAppID 08 , Skype
    [ANCS_TYPE_WECHAT]              = "com.tencent.xin",              //WakeUpAppID 09 , WeChat
    [ANCS_TYPE_WHATSAPP]            = "com.whatsapp.WhatsApp",        //WakeUpAppID 10 , WhatsApp
    [ANCS_TYPE_GMAIL]               = "com.google.Gmail",             //WakeUpAppID 11 , Gmail
    [ANCS_TYPE_HANGOUT]             = "com.google.hangouts",          //WakeUpAppID 12 , Hangout
    [ANCS_TYPE_INBOX]               = "com.google.inbox",             //WakeUpAppID 13 , Inbox
    [ANCS_TYPE_LINE]                = "jp.naver.line",                //WakeUpAppID 14 , Line
    [ANCS_TYPE_TWITTER]             = "com.atebits.Tweetie2",         //WakeUpAppID 15 , Twitter
    [ANCS_TYPE_FACEBOOK]            = "com.facebook.Facebook",        //WakeUpAppID 16 , Facebook
    [ANCS_TYPE_FACEBOOK_MESSENGER]  = "com.facebook.Messenger",       //WakeUpAppID 17 , Facebook Messenger
    [ANCS_TYPE_INSTAGRAM]           = "com.burbn.instagram",          //WakeUpAppID 18 , Instagram
    [ANCS_TYPE_WEIBO]               = "com.sina.weibo",               //WakeUpAppID 19 , WeiBo
    [ANCS_TYPE_KAKAOTALK]           = "com.iwilab.KakaoTaik",         //WakeUpAppID 20 , Kakaotalk
    [ANCS_TYPE_FACEBOOK_PAGE_MANAGER] = " ",                          //WakeUpAppID 21 , Facebook page manager
    [ANCS_TYPE_VIBER]               = "com.viber",                    //WakeUpAppID 22 , Viber
    [ANCS_TYPE_VKCLINET]            = "com.vk.vkclient",              //WakeUpAppID 23 , vkclient
    [ANCS_TYPE_TELEGRAM]            = "ph.telegra.Telegraph",         //WakeUpAppID 24 , Telegram
};

void ble_ancs_var_init(void)
{
    memset(&ancs_msg_cb, 0, sizeof(ancs_msg_cb));
}

ble_ancs_msg_cb_t *ble_app_ancs_get_msg(void)
{
    return &ancs_msg_cb;
}

void ble_app_watch_ancs_client_notifiy_process(u8 id, const char *att_name, const char *att_content)
{
    if (att_name == NULL || att_content == NULL) {
        TRACE("%s:error!", __func__);
        return;
    }

    if (ancs_msg_cb.msg_idx == ANCS_MSG_MAX_NUM) {
        ancs_msg_cb.msg_idx = 0;
    }
    u8 msg_idx = ancs_msg_cb.msg_idx;

    if (id == ANCS_ATT_TYPE_APP_NAME) {                                             //app名称
        int i = 0;
        for (i = 0; i < ANCS_TYPE_MAX; i++) {
            if (strcmp(att_content, appleAppID_table[i]) == 0) {
                break;
            }
        }
        ancs_msg_cb.app_sw_flag = true;
        ancs_msg_cb.msg_type[msg_idx] = i;
    } else if (ancs_msg_cb.app_sw_flag && id == ANCS_ATT_TYPE_TITLE) {               //消息标题
        char time[8];
        tm_t rtc_tm = rtc_clock_get();
        ancs_msg_cb.title_size = strlen(att_content);

        TRACE("time:%2d:%2d\n", rtc_tm.hour, rtc_tm.min);
        sprintf(time, "%2d:%2d", rtc_tm.hour, rtc_tm.min);
        // memset(ancs_msg_cb.msg_content[msg_idx], 0, ANCS_MSG_MAX_SIZE);
        memcpy(ancs_msg_cb.msg_title[msg_idx], time, sizeof(time));                 //时间作为一级标题
        memcpy(ancs_msg_cb.msg_content[msg_idx],(char *)att_content, ANCS_MSG_TITLE_SIZE);
    } else if (ancs_msg_cb.app_sw_flag && id == ANCS_ATT_TYPE_MESSAGE) {             //消息内容
        uint8_t message_len = strlen((char *)att_content);
        if( message_len + ancs_msg_cb.title_size + 1 + 1 > ANCS_MSG_MAX_SIZE -3) {
            message_len = ANCS_MSG_MAX_SIZE - ancs_msg_cb.title_size - 5; // 减5留点余量
        }
        ancs_msg_cb.msg_content[msg_idx][ancs_msg_cb.title_size] = ':';
        memcpy(&ancs_msg_cb.msg_content[msg_idx][ancs_msg_cb.title_size + 1], (char *)att_content, message_len + 1);
    } else if (ancs_msg_cb.app_sw_flag && id == ANCS_ATT_TYPE_DATE) {                //消息时间，单条完整消息接收完成，弹框显示
        TRACE("ancs_msg[%d]: time[%s] msg[%s]\n", msg_idx, ancs_msg_cb.msg_title[msg_idx], ancs_msg_cb.msg_content[msg_idx]);
        ancs_msg_cb.app_sw_flag = false;
        ancs_msg_cb.title_size = 0;
#if FLASHDB_EN
        msg_tsdb_append_user(appleAppID_table[ancs_msg_cb.msg_type[msg_idx]], "", (const char*)ancs_msg_cb.msg_content[msg_idx]);
#else
        ancs_msg_cb.msg_show_num = msg_idx;
        ancs_msg_cb.msg_idx++;
#endif

#if (USE_APP_TYPE == APP_NULL)
        {   //仅无APP测试用
            tm_t tm;
            u32 date, time;
            sscanf(att_content, "%dT%d", &date, &time);
            tm.year = date / 10000;
            tm.mon  = (date % 10000) / 100;
            tm.day  = date % 100;
            tm.hour = time / 10000;
            tm.min  = (time % 10000) / 100;
            tm.sec  = time % 100;
            RTCCNT = tm_to_time(tm);            //更新本地RTC时间
            TRACE("%4d.%2d.%2d, %2d:%2d:%2d\n", tm.year, tm.mon, tm.day, tm.hour, tm.min, tm.sec);
        }
#endif

    }
}

#endif // LE_ANCS_CLIENT_EN
