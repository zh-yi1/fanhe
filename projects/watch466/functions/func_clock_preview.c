#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define CUR_PREVIEW_STYLE           PREVIEW_ROTARY_STYLE

extern u8 func_clock_get_max_dialplate_num(void);
extern u16 func_clock_header_phrase(u32 base_addr);
extern const u32 dialplate_info[];

//根据表盘的基地址获取对应的预览图
u32 func_clock_preview_get_addr(u32 base_addr)
{
    uitool_header_t uitool_header;
    os_spiflash_read(&uitool_header, base_addr, UITOOL_HEADER);
	for(u16 i=0; i<uitool_header.num; i++) {
        uitool_res_t uitool_res = {0};
        os_spiflash_read(&uitool_res, base_addr + UITOOL_HEADER + i * UITOOL_RES_HEADER, UITOOL_RES_HEADER);
        u32 res_addr = base_addr + uitool_res.res_addr;
        //预览图
        if (uitool_res.res_type == UITOOL_TYPE_IMAGE && uitool_res.bond_type == COMPO_BOND_IMAGE_CLOCK_PREVIEW) {
            return res_addr;
        }
    }

    return 0;
}

//获取表盘预览类型
u16 func_clock_preview_get_type(void)
{
    return CUR_PREVIEW_STYLE;
}


#if (CUR_PREVIEW_STYLE == PREVIEW_ROWBOX_STYLE)

//组件ID
enum {
    //列表(横向)
    COMPO_ID_ROWBOX = 1,

};

typedef struct f_clock_preview_t_ {

} f_clock_preview_t;


//创建表盘预览窗体
compo_form_t *func_clock_preview_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title(frm, i18n[STR_CLOCK_PREVIEW]);
    compo_form_set_title_center(frm, true);

    //创建预览图
    compo_rowbox_t *rowbox = compo_rowbox_create(frm);
    compo_rowbox_set_style(rowbox, COMPO_ROWBOX_STYLE_PREVIEW);			//设置预览样式，用于表盘时需要在set前调用

	u8 dialplate_max_num = func_clock_get_max_dialplate_num();
    compo_rowbox_set(rowbox, dialplate_info, dialplate_max_num);
    compo_setid(rowbox, COMPO_ID_ROWBOX);
    compo_rowbox_cycle_en(rowbox, true);                                //循环滚动

    compo_rowbox_set_focus_byidx(rowbox, sys_cb.dialplate_index);
    compo_rowbox_update(rowbox);

    return frm;
}

//表盘预览功能事件处理
static void func_clock_preview_process(void)
{
    compo_rowbox_t *rowbox = compo_getobj_byid(COMPO_ID_ROWBOX);
    compo_rowbox_move(rowbox);
    func_process();
}


//表盘预览功能消息处理
static void func_clock_preview_message(size_msg_t msg)
{
    compo_rowbox_t *rowbox = compo_getobj_byid(COMPO_ID_ROWBOX);
    compo_rowbox_move_cb_t *mcb = rowbox->mcb;
    int sel_idx;    //选择表盘的idx

    switch (msg) {
    case MSG_CTP_CLICK:
        sel_idx = compo_rowbox_select(rowbox, ctp_get_sxy());
        if (sel_idx >= 0) {
            sys_cb.dialplate_index = sel_idx;
            func_switch_to(FUNC_CLOCK, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);                    //切换回主时钟
        }
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
        mcb->flag_drag = true;
        mcb->flag_move_auto = false;
        break;

    case MSG_QDEC_FORWARD:
        //向前滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->moveto_idx = rowbox->focus_icon_idx;
        }
        if (mcb->moveto_idx <= rowbox->img_cnt - 1 || rowbox->flag_cycle) {
            mcb->moveto_idx++;
            mcb->moveto_x = compo_rowbox_getx_byidx(rowbox, mcb->moveto_idx);
        }
        break;

    case MSG_QDEC_BACKWARD:
        //向后滚动
        if (!mcb->flag_move_auto) {
            mcb->flag_move_auto = true;
            mcb->moveto_idx = rowbox->focus_icon_idx;
        }
        if (mcb->moveto_idx >= 0 || rowbox->flag_cycle) {
            mcb->moveto_idx--;
            mcb->moveto_x = compo_rowbox_getx_byidx(rowbox, mcb->moveto_idx);
        }
        break;

    case KU_BACK:
        sys_cb.dialplate_index = rowbox->img_idx[IMG_MAX/2];    //中间为焦点
        func_switch_to(FUNC_CLOCK, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入表盘预览功能
static void func_clock_preview_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_clock_preview_t));
    func_cb.frm_main = func_clock_preview_form_create();

    compo_rowbox_t *rowbox = compo_getobj_byid(COMPO_ID_ROWBOX);
    rowbox->mcb = func_zalloc(sizeof(compo_rowbox_move_cb_t));
    compo_rowbox_move_init(rowbox);
}

//退出表盘预览功能
static void func_clock_preview_exit(void)
{
    compo_rowbox_t *rowbox = compo_getobj_byid(COMPO_ID_ROWBOX);
    func_free(rowbox->mcb);

    func_cb.last = FUNC_CLOCK_PREVIEW;
}

//表盘预览功能
void func_clock_preview(void)
{
    printf("%s\n", __func__);
    func_clock_preview_enter();
    while (func_cb.sta == FUNC_CLOCK_PREVIEW) {
        func_clock_preview_process();
        func_clock_preview_message(msg_dequeue());
    }
    func_clock_preview_exit();
}

#endif //(CUR_PREVIEW_STYLE == PREVIEW_ROWBOX_STYLE)



#if(CUR_PREVIEW_STYLE == PREVIEW_STACKLIST_STYLE)

//组件ID
enum {
    //列表(横向)
    COMPO_ID_STACKLIST = 1,

};

typedef struct f_clock_preview_t_ {

} f_clock_preview_t;


//创建表盘预览窗体
compo_form_t *func_clock_preview_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title(frm, i18n[STR_CLOCK_PREVIEW]);
    compo_form_set_title_center(frm, true);

    //创建预览图
    compo_stacklist_t *stacklist = compo_stacklist_create(frm, 3, STACKLIST_SYMM_STYLE, STACKLIST_HOR);
    compo_setid(stacklist, COMPO_ID_STACKLIST);
    stacklist->cur_index = sys_cb.dialplate_index;

    //设置堆叠图片间中心位置的间隔和缩放比例
    compo_stacklist_set_user_para(stacklist, 60, 25);

    //设置图片
    u8 dial_count = func_clock_get_max_dialplate_num();
    u32 prev_dial_pic[STACKLIST_PIC_COUNT_MAX] = {0};
    for (u8 i = 0; i < dial_count; i++) {
        prev_dial_pic[i] = func_clock_preview_get_addr(dialplate_info[i]);
    }
    compo_stacklist_set_pic(stacklist, prev_dial_pic, dial_count);

    return frm;
}

//表盘预览功能事件处理
static void func_clock_preview_process(void)
{
    compo_stacklist_t *stacklist = compo_getobj_byid(COMPO_ID_STACKLIST);
    compo_stacklist_move(stacklist);
    func_process();
}

//表盘预览功能消息处理
static void func_clock_preview_message(size_msg_t msg)
{
    compo_stacklist_t *stacklist = compo_getobj_byid(COMPO_ID_STACKLIST);

    switch (msg) {
    case MSG_CTP_TOUCH:
            compo_stacklist_set_drag(stacklist, true);
         break;

    case MSG_CTP_CLICK:
    case KU_BACK:
            sys_cb.dialplate_index = stacklist->cur_index;
            func_switch_to(FUNC_CLOCK, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);      //切换回表盘界面
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_RIGHT:
        break;

    case MSG_QDEC_FORWARD:
        compo_stacklist_symm_pic_move(stacklist, 20);
        break;

    case MSG_QDEC_BACKWARD:
        compo_stacklist_symm_pic_move(stacklist, -20);
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入表盘预览功能
static void func_clock_preview_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_clock_preview_t));
    func_cb.frm_main = func_clock_preview_form_create();
}

//退出表盘预览功能
static void func_clock_preview_exit(void)
{
    func_cb.last = FUNC_CLOCK_PREVIEW;
}

//表盘预览功能
void func_clock_preview(void)
{
    printf("%s\n", __func__);
    func_clock_preview_enter();
    while (func_cb.sta == FUNC_CLOCK_PREVIEW) {
        func_clock_preview_process();
        func_clock_preview_message(msg_dequeue());
    }
    func_clock_preview_exit();
}

#endif //(CUR_PREVIEW_STYLE == PREVIEW_STACKLIST_STYLE)



#if (CUR_PREVIEW_STYLE == PREVIEW_ROTARY_STYLE)

/********************************************************************
 * 注意：使用该效果的表盘预览图在生成时需要使能旋转压缩
*********************************************************************/
#define ROTARY_RADIUS                   200                             //转盘半径

//组件ID
enum {
    //列表(横向)
    COMPO_ID_ROTARY = 1,

};

typedef struct f_clock_preview_t_ {
    compo_rotary_t *rotary;
} f_clock_preview_t;

compo_rotary_item_t prev_dial_item[STACKLIST_PIC_COUNT_MAX] = {0};

//创建表盘预览窗体
compo_form_t *func_clock_preview_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);
    //获取表盘预览图集合
    u8 dial_count = func_clock_get_max_dialplate_num();

    for (u8 i = 0; i < dial_count; i++) {
        prev_dial_item[i].res_addr = func_clock_preview_get_addr(dialplate_info[i]);
        prev_dial_item[i].str_idx = STR_NULL;
    }

    //创建背景
    compo_shape_t *rect = compo_shape_create(frm, COMPO_SHAPE_TYPE_RECTANGLE);
    compo_shape_set_color(rect, make_color(24, 24, 32));
    compo_shape_set_location(rect, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);

    //创建转盘组件
    compo_rotary_t *rotary = compo_rotary_create(frm, prev_dial_item, dial_count, ROTARY_RADIUS);
    compo_setid(rotary, COMPO_ID_ROTARY);
    compo_rotary_update(rotary);

    return frm;
}

//表盘预览功能事件处理
static void func_clock_preview_process(void)
{
    f_clock_preview_t *f_clock_preview = (f_clock_preview_t *)func_cb.f_cb;
    compo_rotary_t *rotary = f_clock_preview->rotary;
    compo_rotary_move(rotary);
    func_process();
    //printf("idx = %d, width = %d, height = %d\n", compo_rotary_get_idx(rotary),
    //       gui_image_get_size(prev_dial_item[compo_rotary_get_idx(rotary)].res_addr).wid,
    //       gui_image_get_size(prev_dial_item[compo_rotary_get_idx(rotary)].res_addr).hei);
    if (compo_rotary_get_sta(rotary) == COMPO_ROTARY_STA_EXIT) {
        func_cb.sta = FUNC_CLOCK;
    }
}

//表盘预览功能消息处理
static void func_clock_preview_message(size_msg_t msg)
{
    f_clock_preview_t *f_clock_preview = (f_clock_preview_t *)func_cb.f_cb;
    compo_rotary_t *rotary = f_clock_preview->rotary;

    switch (msg) {
    case KU_BACK:
        sys_cb.dialplate_index = compo_rotary_get_idx(rotary);
        break;
    default:
        break;
    }

    if (compo_rotary_message(rotary, msg)) {
        return;
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        sys_cb.dialplate_index = compo_rotary_get_idx(rotary);
        compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_EXITING);
        break;

    default:
        func_message(msg);
        break;
    }


}

//进入表盘预览功能
void func_clock_preview_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_clock_preview_t));
    func_cb.frm_main = func_clock_preview_form_create();
    tft_set_temode(1);
    f_clock_preview_t *f_clock_preview = (f_clock_preview_t *)func_cb.f_cb;
    f_clock_preview->rotary = compo_getobj_byid(COMPO_ID_ROTARY);
    compo_rotary_t *rotary = f_clock_preview->rotary;
    if (rotary->type != COMPO_TYPE_ROTARY) {
        halt(HALT_GUI_COMPO_ROTARY_TYPE);
    }
    compo_rotary_set_rotation_byidx(rotary, sys_cb.dialplate_index);
    compo_rotary_update(rotary);
    compo_rotary_move_control(rotary, COMPO_ROTARY_MOVE_CMD_ENTERING);      //入场
}

//退出表盘预览功能
void func_clock_preview_exit(void)
{
    tft_set_temode(DEFAULT_TE_MODE);
    func_cb.last = FUNC_CLOCK_PREVIEW;
}

//表盘预览功能
void func_clock_preview(void)
{
    printf("%s\n", __func__);
    func_clock_preview_enter();
    while (func_cb.sta == FUNC_CLOCK_PREVIEW) {
        func_clock_preview_process();
        func_clock_preview_message(msg_dequeue());
    }
    func_clock_preview_exit();
}


#endif // (CUR_PREVIEW_STYLE == PREVIEW_ROTARY_STYLE)



