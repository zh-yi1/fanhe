#include "include.h"
#include "func.h"

//正常模式下
static bool compo_rotary_normal_message(compo_rotary_t *rotary, size_msg_t msg)
{
    switch (msg) {
    case KU_BACK:
        //compo_rotary_set_rotation_byidx(rotary, func_cb.menu_style);
        compo_rotary_set_rotation_byidx(rotary, compo_rotary_get_idx(rotary));
        compo_rotary_update(rotary);
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_EXITING);
        return true;

    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_LEFT:
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_DRAG);
        return true;

    case MSG_QDEC_FORWARD:
        //向前滚动菜单
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_FORWARD);
        return true;

    case MSG_QDEC_BACKWARD:
        //向后滚动菜单
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_BACKWARD);
        return true;

    default:
        return false;
    }
}

//拖动过程中，只响应部分消息
static bool compo_rotary_drag_message(compo_rotary_t *rotary, size_msg_t msg)
{
    switch (msg) {
    default:
        evt_message(msg);
        break;
    }
    return true;
}

//自动移动中，只响应部分消息
static bool compo_rotary_move_message(compo_rotary_t *rotary, size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_TOUCH:
        //移动过程中，触屏停止。重新进入到开始拖动模式
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_DRAG);
        break;

    case MSG_QDEC_FORWARD:
        //向前滚动菜单
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_FORWARD);
        break;

    case MSG_QDEC_BACKWARD:
        //向后滚动菜单
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_BACKWARD);
        break;

    default:
        evt_message(msg);
        break;
    }
    return true;
}

//转盘消息处理
bool compo_rotary_message(compo_rotary_t *rotary, size_msg_t msg)
{
    u8 sta = compo_rotary_get_sta(rotary);
    switch (sta) {
    case COMPO_ROTARY_STA_IDLE:
        //正常模式下
        return compo_rotary_normal_message(rotary, msg);

    case COMPO_ROTARY_STA_ENTERING:
    case COMPO_ROTARY_STA_EXITING:
        //出入场动画
        return true;

    case COMPO_ROTARY_STA_DARG:
        //拖动过程中，只响应部分消息
        return compo_rotary_drag_message(rotary, msg);

    case COMPO_ROTARY_STA_MOVE:
        //自动移动中，只响应部分消息
        return compo_rotary_move_message(rotary, msg);

    case COMPO_ROTARY_STA_EXIT:
        //已出场
        return true;

    default:
        halt(HALT_GUI_COMPO_ROTARY_MOVE_STA);
        return true;
    }
}
