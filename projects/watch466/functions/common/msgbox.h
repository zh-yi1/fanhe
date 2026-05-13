#ifndef _MSGBOX_H
#define _MSGBOX_H

enum {
    MSGBOX_MODE_BTN_OK,
    MSGBOX_MODE_BTN_OKCANCEL,
    MSGBOX_MODE_BTN_YESNO,
    MSGBOX_MODE_BTN_DELETE, //消息推送删除按键
};

//msgbox返回值
enum {
    MSGBOX_RES_NONE,
    MSGBOX_RES_OK,          //确定
    MSGBOX_RES_CANCEL,      //取消
    MSGBOX_RES_ABORT,       //终止
    MSGBOX_RES_RETRY,       //重试
    MSGBOX_RES_INGNORE,     //忽略
    MSGBOX_RES_YES,         //是
    MSGBOX_RES_NO,          //否
    MSGBOX_RES_DELETE,      //删除
};

enum {
    MSGBOX_MSG_TYPE_NONE,
    MSGBOX_MSG_TYPE_WECHAT,   //微信消息
};

int msgbox(char *msg, char *title, int mode, char msg_type);

#endif
