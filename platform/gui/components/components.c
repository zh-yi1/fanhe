#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

compo_cb_t compo_cb AT(.disp.compo);
static u8 compos_buf[COMPO_BUF_SIZE] AT(.disp.compo);              //Components Buf

//获取组件结构体Size
static int compo_get_size(int type)
{
    int compo_size;
    switch (type) {
    case COMPO_TYPE_FORM:
        compo_size = sizeof(compo_form_t);
        break;

    case COMPO_TYPE_PICTUREBOX:
        compo_size = sizeof(compo_picturebox_t);
        break;

    case COMPO_TYPE_LABEL:
        compo_size = sizeof(compo_label_t);
        break;

    case COMPO_TYPE_TEXTBOX:
        compo_size = sizeof(compo_textbox_t);
        break;

    case COMPO_TYPE_SHAPE:
        compo_size = sizeof(compo_shape_t);
        break;

    case COMPO_TYPE_BUTTON:
        compo_size = sizeof(compo_button_t);
        break;

    case COMPO_TYPE_NUMBER:
        compo_size = sizeof(compo_number_t);
        break;

    case COMPO_TYPE_DATETIME:
        compo_size = sizeof(compo_datetime_t);
        break;

    case COMPO_TYPE_QRCODEBOX:
        compo_size = sizeof(compo_qrcodebox_t);
        break;

    case COMPO_TYPE_LISTBOX:
        compo_size = sizeof(compo_listbox_t);
        break;

    case COMPO_TYPE_STACKLIST:
        compo_size = sizeof(compo_stacklist_t);
        break;

    case COMPO_TYPE_ROWBOX:
        compo_size = sizeof(compo_rowbox_t);
        break;

    case COMPO_TYPE_ICONLIST:
        compo_size = sizeof(compo_iconlist_t);
        break;

    case COMPO_TYPE_ROTARY:
        compo_size = sizeof(compo_rotary_t);
        break;

    case COMPO_TYPE_FOOTBALL:
        compo_size = sizeof(compo_football_t);
        break;

    case COMPO_TYPE_CUBE:
        compo_size = sizeof(compo_cube_t);
        break;

    case COMPO_TYPE_DISKLIST:
        compo_size = sizeof(compo_disklist_t);
        break;

    case COMPO_TYPE_KALEIDOSCOPE:
        compo_size = sizeof(compo_kaleidoscope_t);
        break;

    case COMPO_TYPE_ANIMATION:
        compo_size = sizeof(compo_animation_t);
        break;

#if VIDEO_PLAY_EN
    case COMPO_TYPE_VIDEO:
        compo_size = sizeof(compo_video_t);
        break;
#endif

    case COMPO_TYPE_JPG:
        compo_size = sizeof(compo_jpg_t);
        break;

    case COMPO_TYPE_CAMERA:
        compo_size = sizeof(compo_camera_t);
        break;

    case COMPO_TYPE_RINGS:
        compo_size = sizeof(compo_rings_t);
        break;

    case COMPO_TYPE_ARC:
        compo_size = sizeof(compo_arc_t);
        break;

    case COMPO_TYPE_CHARTBOX:
        compo_size = sizeof(compo_chartbox_t);
        break;

    case COMPO_TYPE_CARDBOX:
        compo_size = sizeof(compo_cardbox_t);
        break;

    case COMPO_TYPE_BUTTERFLY:
        compo_size = sizeof(compo_butterfly_t);
        break;

    case COMPO_TYPE_SCROLLBAR:
        compo_size = sizeof(compo_scroll_t);
        break;

//    case COMPO_TYPE_NUMLIST:
//        compo_size = sizeof(compo_numlist_t);
//        break;

//    case COMPO_TYPE_TXTLIST:
//        compo_size = sizeof(compo_txtlist_t);
//        break;

    case COMPO_TYPE_WINDMILL:
        compo_size = sizeof(compo_windmill_t);
        break;

    case COMPO_TYPE_GIF:
        compo_size = sizeof(compo_gif_t);
        break;

    case COMPO_TYPE_FINSH:
        compo_size = sizeof(compo_fish_t);
        break;

    default:
        TRACE("Unknow Compo Type: %d\n", type);
        halt(HALT_GUI_COMPO_UNKNOW_TYPE);
        return 0;
    }
    return compo_size;
}

//查询当前compo buf
bool compo_in_buf0(void *compo)
{
    u8 *compo_buf = compo;
    if (compo_buf >= compo_cb.buf0 && compo_buf < compo_cb.buf0 + compo_cb.buf0_size) {
        return true;
    } else if (compo_buf >= compo_cb.buf1 && compo_buf < compo_cb.buf1 + compo_cb.buf1_size) {
        return false;
    } else {
        halt(HALT_GUI_COMPO_PTR);
        return false;
    }
}

//获取第一个
void *compo_get_head(void)
{
    return compo_pool_get_bottom();
}

//获取下一个
component_t *compo_get_next(component_t *compo)
{
    if (compo == NULL) {
        halt(HALT_GUI_COMPO_GET_NEXT);
    }
    int compo_size = compo_get_size(compo->type);
    u8* compo_next = (u8 *)compo + compo_size;
    if (compo_in_buf0(compo)) {
        if (compo_next < (u8 *)compo_cb.buf0 + compo_cb.buf0_pos) {
            return (component_t *)compo_next;
        } else if (compo_cb.top_num == 1 && compo_cb.buf1_pos > 0) {
            return (component_t *)compo_cb.buf1;
        } else {
            return NULL;
        }
    } else {
        if (compo_next < (u8 *)compo_cb.buf1 + compo_cb.buf1_pos) {
            return (component_t *)compo_next;
        } else if (compo_cb.top_num == 0 && compo_cb.buf0_pos > 0) {
            return (component_t *)compo_cb.buf0;
        } else {
            return NULL;
        }
    }
}

//新建一个组件
void *compo_create(compo_form_t *parent, u8 type)
{
    if (parent == NULL) {
        halt(HALT_GUI_COMPO_CREATE);
    }
    component_t *compo;
    int compo_size = compo_get_size(type);
    u8 *parent_buf = (u8 *)parent;
    if (parent_buf == compo_cb.buf0) {
        if (compo_cb.buf0_pos + compo_size > compo_cb.buf0_size) {
            TRACE("Compo Size Exceed\n");
            halt(HALT_GUI_COMPO_BUF_EXCEED);
        }
        compo = (component_t *)(compo_cb.buf0 + compo_cb.buf0_pos);
        compo_cb.buf0_pos += compo_size;
//        TRACE("compo_buff0[%d/%d]\n", compo_cb.buf0_pos, compo_cb.buf0_size);
    } else if (parent_buf == compo_cb.buf1) {
        if (compo_cb.buf1_pos + compo_size > compo_cb.buf1_size) {
            TRACE("Compo Size Exceed\n");
            halt(HALT_GUI_COMPO_BUF_EXCEED);
        }
        compo = (component_t *)(compo_cb.buf1 + compo_cb.buf1_pos);
        compo_cb.buf1_pos += compo_size;
//        TRACE("compo_buff1[%d/%d]\n", compo_cb.buf1_pos, compo_cb.buf1_size);
    } else {
        halt(HALT_GUI_COMPO_CREATE);
        return NULL;
    }
    memset(compo, 0, compo_size);
    compo->type = type;
    return compo;
}

//设置组件绑定数据
void compo_bonddata(void *compo, u8 bond_data)
{
    if (compo == NULL) {
        halt(HALT_GUI_COMPO_POOL_BONDDATA);
    }
    ((component_t *)compo)->bond_data = bond_data;
    compo_cb.rtc_update = true;
}

//设置组件ID
void compo_setid(void *compo, u16 id)
{
    if (compo == NULL) {
        halt(HALT_GUI_COMPO_POOL_SETID);
    }
    ((component_t *)compo)->id = id;
}

//新建一个组件池
void *compo_pool_create(bool flag_top)
{
    compo_form_t *frm;
    if (compo_cb.buf0_pos == 0) {
        compo_cb.buf0_pos = sizeof(compo_form_t);
        compo_cb.top_num = flag_top ? 0 : 1;
        frm = (compo_form_t *)compo_cb.buf0;
    } else if (compo_cb.buf1_pos == 0) {
        compo_cb.buf1_pos = sizeof(compo_form_t);
        compo_cb.top_num = flag_top ? 1 : 0;
        frm = (compo_form_t *)compo_cb.buf1;
    } else {
        halt(HALT_GUI_COMPO_POOL_CREATE);
        return NULL;
    }
    memset(frm, 0, sizeof(compo_form_t));
    return frm;
}

//清空组件池
void compo_pool_clear(compo_form_t *compo)
{
    u8 *compo_buf = (u8 *)compo;
    if (compo_cb.buf0 == compo_buf) {
        compo_cb.buf0_pos = 0;
    } else if (compo_cb.buf1 == compo_buf) {
        compo_cb.buf1_pos = 0;
    } else {
        halt(HALT_GUI_COMPO_POOL_CLEAR);
    }
}

//获取组件池顶上一个
compo_form_t *compo_pool_get_top(void)
{
    if (compo_cb.top_num == 0) {
        if (compo_cb.buf0_pos >= sizeof(compo_form_t)) {
            return (compo_form_t *)compo_cb.buf0;
        }
        if (compo_cb.buf1_pos >= sizeof(compo_form_t)) {
            return (compo_form_t *)compo_cb.buf1;
        }
    } else {
        if (compo_cb.buf1_pos >= sizeof(compo_form_t)) {
            return (compo_form_t *)compo_cb.buf1;
        }
        if (compo_cb.buf0_pos >= sizeof(compo_form_t)) {
            return (compo_form_t *)compo_cb.buf0;
        }
    }
    return NULL;
}

//获取组件池底下一个
compo_form_t *compo_pool_get_bottom(void)
{
    if (compo_cb.top_num == 0) {
        if (compo_cb.buf1_pos >= sizeof(compo_form_t)) {
            return (compo_form_t *)compo_cb.buf1;
        }
        if (compo_cb.buf0_pos >= sizeof(compo_form_t)) {
            return (compo_form_t *)compo_cb.buf0;
        }
    } else {
        if (compo_cb.buf0_pos >= sizeof(compo_form_t)) {
            return (compo_form_t *)compo_cb.buf0;
        }
        if (compo_cb.buf1_pos >= sizeof(compo_form_t)) {
            return (compo_form_t *)compo_cb.buf1;
        }
    }
    return NULL;
}

//将窗体置顶
void compo_pool_set_top(compo_form_t *frm)
{
    u8 *ptr = (u8 *)frm;
    if (ptr == compo_cb.buf0) {
        compo_cb.top_num = 0;
    } else if (ptr == compo_cb.buf1) {
        compo_cb.top_num = 1;
    } else {
        halt(HALT_GUI_COMPO_POOL_SETTOP);
    }
    widget_pool_set_top(compo_cb.top_num);
}

//初始化组件Buffer
void compos_init(void)
{
    memset(&compo_cb, 0, sizeof(compo_cb));
    compo_cb.buf0 = compos_buf;
    compo_cb.buf0_size = COMPO_BUF_SIZE / 2;
    compo_cb.buf1 = compos_buf + COMPO_BUF_SIZE / 2;
    compo_cb.buf1_size = COMPO_BUF_SIZE / 2;
}
