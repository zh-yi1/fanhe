#ifndef _APP_ANCS_H_
#define _APP_ANCS_H_

enum ANCS_TYPE {
    ANCS_TYPE_NULL                          = 0x00,
    ANCS_TYPE_INCOMINGCALL,
    ANCS_TYPE_MISSEDCALL,
    ANCS_TYPE_MESSAGES,
    ANCS_TYPE_MAIL,
    ANCS_TYPE_CALENDAR,
    ANCS_TYPE_FACETIME,
    ANCS_TYPE_QQ,
    ANCS_TYPE_SKYPE,
    ANCS_TYPE_WECHAT,
    ANCS_TYPE_WHATSAPP,
    ANCS_TYPE_GMAIL,
    ANCS_TYPE_HANGOUT,
    ANCS_TYPE_INBOX,
    ANCS_TYPE_LINE,
    ANCS_TYPE_TWITTER,
    ANCS_TYPE_FACEBOOK,
    ANCS_TYPE_FACEBOOK_MESSENGER,
    ANCS_TYPE_INSTAGRAM,
    ANCS_TYPE_WEIBO,
    ANCS_TYPE_KAKAOTALK,
    ANCS_TYPE_FACEBOOK_PAGE_MANAGER,
    ANCS_TYPE_VIBER,
    ANCS_TYPE_VKCLINET,
    ANCS_TYPE_TELEGRAM,
    ANCS_TYPE_MAX,
    ANCS_TYPE_SNAPCHAT,
    ANCS_TYPE_DINGTALK,
    ANCS_TYPE_ALIPAY,
    ANCS_TYPE_TIKTOP,
    ANCS_TYPE_LINKEDIN,
};

#define ANCS_MSG_MAX_NUM     3  //消息最大条数
//#define ANCS_MSG_MAX_SIZE    16 //消息最大长度
#define ANCS_MSG_MAX_SIZE    300 //消息最大长度
#define ANCS_MSG_TITLE_SIZE  16  //消息标题长度

typedef struct {
    u8 msg_idx;                                             //消息索引
    u8 msg_show_num;                                        //需要显示的具体消息编号
    bool app_sw_flag;                                       //app软件标志
    u8 title_size;
    u8 msg_type[ANCS_MSG_MAX_NUM];                          //消息类型
    u8 msg_title[ANCS_MSG_MAX_NUM][6];                      //消息标题
    u8 msg_content[ANCS_MSG_MAX_NUM][ANCS_MSG_MAX_SIZE];    //消息内容
} ble_ancs_msg_cb_t;

//ancs消息类型枚举
enum ANCS_ATT_TYPE {
    ANCS_ATT_TYPE_APP_NAME,
    ANCS_ATT_TYPE_TITLE,
    ANCS_ATT_TYPE_SUBTITLE,
    ANCS_ATT_TYPE_MESSAGE,
    ANCS_ATT_TYPE_MESSAGE_SIZE,
    ANCS_ATT_TYPE_DATE,
};

void ble_ancs_var_init(void);
//ancs cb
void ble_app_watch_ancs_client_notifiy_process(u8 id, const char *att_name, const char *att_content);
//get ancs msg
ble_ancs_msg_cb_t *ble_app_ancs_get_msg(void);

#endif // _APP_ANCS_H_
