#include "include.h"
#include "func.h"

//正常模式下
static bool compo_listbox_normal_message(compo_listbox_t *listbox, size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_DOWN:
        //任何方向短划，开启拖动
        compo_listbox_move_control(listbox, COMPO_LISTBOX_MOVE_CMD_DRAG);
        return true;

    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_LEFT:
        if (listbox->style == COMPO_LISTBOX_STYLE_MENU_FOURGRID) {
            compo_listbox_move_control(listbox, COMPO_LISTBOX_MOVE_CMD_DRAG);
            return true;
        }
        return false;

    case MSG_QDEC_FORWARD:
        //向前滚动菜单
        compo_listbox_move_control(listbox, COMPO_LISTBOX_MOVE_CMD_FORWARD);
        return true;

    case MSG_QDEC_BACKWARD:
        //向后滚动菜单
        compo_listbox_move_control(listbox, COMPO_LISTBOX_MOVE_CMD_BACKWARD);
        return true;

    default:
        return false;
    }
}

//拖动过程中，只响应部分消息
static bool compo_listbox_drag_message(compo_listbox_t *listbox, size_msg_t msg)
{
    switch (msg) {
    default:
        evt_message(msg);
        break;
    }
    return true;
}

//自动移动中，只响应部分消息
static bool compo_listbox_move_message(compo_listbox_t *listbox, size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_TOUCH:
        //移动过程中，触屏停止。重新进入到开始拖动模式
        compo_listbox_move_control(listbox, COMPO_LISTBOX_MOVE_CMD_DRAG);
        break;

    case MSG_QDEC_FORWARD:
        //向前滚动菜单
        compo_listbox_move_control(listbox, COMPO_LISTBOX_MOVE_CMD_FORWARD);
        break;

    case MSG_QDEC_BACKWARD:
        //向后滚动菜单
        compo_listbox_move_control(listbox, COMPO_LISTBOX_MOVE_CMD_BACKWARD);
        break;

    default:
        evt_message(msg);
        break;
    }
    return true;
}

//列表框消息处理
bool compo_listbox_message(compo_listbox_t *listbox, size_msg_t msg)
{
    u8 sta = compo_listbox_get_sta(listbox);
    switch (sta) {
    case COMPO_LISTBOX_STA_IDLE:
        //正常模式下
        return compo_listbox_normal_message(listbox, msg);

    case COMPO_LISTBOX_STA_DARG:
        //拖动过程中，只响应部分消息
        return compo_listbox_drag_message(listbox, msg);

    case COMPO_LISTBOX_STA_MOVE:
        //自动移动中，只响应部分消息
        return compo_listbox_move_message(listbox, msg);

    default:
        halt(HALT_GUI_COMPO_LISTBOX_MOVE_STA);
        return true;
    }
}
