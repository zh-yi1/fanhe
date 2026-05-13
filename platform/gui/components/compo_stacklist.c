#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

static void compo_stacklist_symm_refresh_pic(compo_stacklist_t *stacklist, bool frist);


/**
 * @brief 创建一个堆叠的列表
 * @param[in] frm : 窗体指针
 * @param[in] disp_count : 显示列表上的图片数量
 * @param[in] style : 风格样式：对称、顺序
 * @param[in] dir : 显示方向：横向、纵向
 * @return 返回组件句柄
 **/
compo_stacklist_t *compo_stacklist_create(compo_form_t *frm, u8 disp_count, u8 style, u8 dir)
{
    u32 i;
    compo_stacklist_t *stacklist = compo_create(frm, COMPO_TYPE_STACKLIST);

    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    stacklist->main_page = page;

    if (disp_count > STACKLIST_DISP_COUNT_MAX) {
        halt(HALT_GUI_COMPO_STACKLIST_COUNT);
    }
    if (style != STACKLIST_SYMM_STYLE && dir != STACKLIST_HOR) {
        halt(HALT_GUI_COMPO_STACKLIST_COUNT);
    }

    u8 mid_index = disp_count / 2;

    for(i = 0; i < disp_count; i++) {
        if (i == mid_index) {
            continue ;
        }
        stacklist->icon_item[i*2] = widget_icon_create(stacklist->main_page, 0);
        widget_set_visible(stacklist->icon_item[i*2], false);
        stacklist->icon_item[i*2 + 1] = widget_icon_create(stacklist->main_page, 0);
        widget_set_visible(stacklist->icon_item[i*2 + 1], false);
    }
    stacklist->icon_item[mid_index*2] = widget_icon_create(stacklist->main_page, 0);
    widget_set_visible((widget_t *)stacklist->icon_item[mid_index*2], false);
    stacklist->icon_item[mid_index*2 + 1] = widget_icon_create(stacklist->main_page, 0);
    widget_set_visible((widget_t *)stacklist->icon_item[mid_index*2 + 1], false);

    stacklist->disp_count = disp_count;
    stacklist->disp_index = 0;
    stacklist->dir = dir;
    stacklist->style = style;
    stacklist->icon_pos_gap = GUI_SCREEN_WIDTH / 8;  //default
    stacklist->icon_zoom_per = 25;                   //default 

    return stacklist;
}

/**
 * @brief 设置列表图片
 * @param[in] stacklist : 组件句柄
 * @param[in] res_addr_item : 图片地址集合
 * @param[in] res_addr_count : 图片数量
 * @return 无
 **/
void compo_stacklist_set_pic(compo_stacklist_t *stacklist, u32 *res_addr_item, u8 res_addr_count)
{
    if (res_addr_count > STACKLIST_PIC_COUNT_MAX) {
        res_addr_count = STACKLIST_PIC_COUNT_MAX;
    }
    memset(stacklist->pic_addr, 0, sizeof(stacklist->pic_addr));
    memcpy(stacklist->pic_addr, res_addr_item, sizeof(stacklist->pic_addr[0]) * res_addr_count);
    stacklist->res_addr_count = res_addr_count;
    area_t area = gui_image_get_size(stacklist->pic_addr[0]);
    stacklist->pic_w = area.wid;
    stacklist->pic_h = area.hei;
    compo_stacklist_symm_refresh_pic(stacklist, true);
}

/**
 * @brief 设置堆叠图片间中心位置的间隔和缩放比例
 * @param[in] stacklist : 组件句柄
 * @param[in] pic_gap : 图片间的间隔
 * @param[in] zoom_per : 图片间间隔的缩放比例（例如对称式堆叠列表，相对于中间图，往两端间隔缩小25%）
 * @return 无
 **/
void compo_stacklist_set_user_para(compo_stacklist_t *stacklist, u8 pic_gap, u8 zoom_per)
{
    stacklist->icon_pos_gap = pic_gap;
    stacklist->icon_zoom_per = zoom_per;
}

/**
 * @brief 设置拖拽标志
 * @param[in] stacklist : 组件句柄
 * @param[in] drag_flag : 拖拽标志，true or false
 * @return 无
 **/
void compo_stacklist_set_drag(compo_stacklist_t *stacklist, bool drag_flag)
{
    stacklist->ctr.flag_drag = drag_flag;
}

/**
 * @brief 对称式堆叠图片缩放表
 * @param[in] stacklist : 组件句柄
 * @param[out] zoom_table : 缩放比例表
 * @return 无
 **/
static void compo_stacklist_symm_zoom_table_get(compo_stacklist_t *stacklist, u8 *zoom_table)
{
    u8 mid_index = stacklist->disp_count / 2;

    zoom_table[mid_index] = 100;
    for(u8 i = 0; i <= mid_index; i++) {
        if ((100 - i * stacklist->icon_zoom_per) <= 0) {
            break;
        }
        zoom_table[mid_index - i] = 100 - i * stacklist->icon_zoom_per;
        TRACE("zoom_table[%d]:%d\n",mid_index - i, zoom_table[mid_index - i]);
    }
    for (u8 i = mid_index; i < stacklist->disp_count; i++) {
        if ((100 - stacklist->icon_zoom_per * (i - mid_index)) <= 0) {
            break;
        }
        zoom_table[i] = 100 - stacklist->icon_zoom_per * (i - mid_index);
        TRACE("zoom_table[%d]:%d\n",i,zoom_table[i]);
    }
}

/**
 * @brief 对称式堆叠的图片刷新
 * @param[in] stacklist : 组件句柄
 * @param[in] frist : 是否首次刷新
 * @return 无
 **/
static void compo_stacklist_symm_refresh_pic(compo_stacklist_t *stacklist, bool frist)
{
    u8 i;
    u8 mid_index = stacklist->disp_count / 2;
    u8 widget_index;
    int8_t icon_index;
    rect_t rect;

    u8 zoom_table[STACKLIST_DISP_COUNT_MAX] = {0};
    compo_stacklist_symm_zoom_table_get(stacklist, zoom_table);

    if (!frist) {
        stacklist->disp_index = !stacklist->disp_index;
    } else {   //首次刷新位置
        for (i = 0; i < stacklist->disp_count; i++) {
            if (STACKLIST_HOR == stacklist->dir) {
                rect.y = GUI_SCREEN_CENTER_Y;
                rect.x = GUI_SCREEN_CENTER_X + (i - mid_index) * stacklist->icon_pos_gap;
            } else {
                rect.x = GUI_SCREEN_CENTER_X;
                rect.y = GUI_SCREEN_CENTER_Y + (i - mid_index) * stacklist->icon_pos_gap;
            }

            widget_index = 2 * i + stacklist->disp_index;
            widget_set_pos(stacklist->icon_item[widget_index], rect.x, rect.y);
        }
    }


    TRACE("disp_count:%d,res_addr_count:%d\n",stacklist->disp_count, stacklist->res_addr_count);
    for (i = 0; i < stacklist->disp_count; i++) {
        //定位图像
        icon_index = stacklist->cur_index - mid_index + i;
        icon_index = (stacklist->res_addr_count + icon_index) % stacklist->res_addr_count;
        TRACE("icon_index:%d\n",icon_index);

        //显示隐藏的
        widget_index = 2 * i + stacklist->disp_index;
        rect_t old_rect = widget_get_location((widget_t *)stacklist->icon_item[widget_index]);
        widget_icon_set(stacklist->icon_item[widget_index], stacklist->pic_addr[icon_index]);
        rect_t new_rect = widget_get_location((widget_t *)stacklist->icon_item[widget_index]);
        if (100 != zoom_table[i]) {
            if (old_rect.wid != new_rect.wid || old_rect.hei != new_rect.hei) {
                rect.wid = zoom_table[i] * new_rect.wid / 100;
                rect.hei = zoom_table[i] * new_rect.hei / 100;
                widget_set_size(stacklist->icon_item[widget_index], rect.wid, rect.hei);
            }
        }
        widget_set_alpha((widget_t *)stacklist->icon_item[widget_index], 0xff);
        widget_set_visible((widget_t *)stacklist->icon_item[widget_index], true);
        
        //隐藏原有显示的并归位(同当前显示的坐标一致)
        rect = widget_get_location((widget_t *)stacklist->icon_item[widget_index]);
        widget_index = 2 * i + (!stacklist->disp_index);
        widget_set_alpha((widget_t *)stacklist->icon_item[widget_index], 0xff);
        widget_set_visible((widget_t *)stacklist->icon_item[widget_index], false);
        widget_set_location((widget_t *)stacklist->icon_item[widget_index], rect.x, rect.y, rect.wid, rect.hei);
    }

    if(!stacklist->disp_index) {
        compo_stacklist_symm_refresh_pic(stacklist, false);
    }
}

/**
 * @brief 对称式堆叠列表拖拽处理
 * @param[in] stacklist : 组件句柄
 * @param[in] offset : 移动偏移量
 * @return 无
 **/
void compo_stacklist_symm_pic_move(compo_stacklist_t *stacklist, s16 offset)
{
    u8 mid_index = stacklist->disp_count / 2;
    s16 lift_border = GUI_SCREEN_CENTER_X - stacklist->icon_pos_gap;
    s16 right_border = GUI_SCREEN_CENTER_X + stacklist->icon_pos_gap;
    s16 size_offset = 0;
    int8_t change_dir = 0;
    int8_t icon_index = -1;

    u8 zoom_table[STACKLIST_DISP_COUNT_MAX] = {0};
    compo_stacklist_symm_zoom_table_get(stacklist, zoom_table);
    s16 wid_change = stacklist->pic_w - (zoom_table[mid_index - 1] * stacklist->pic_w / 100);


    for (u8 i = 0; i < stacklist->disp_count; i++) {
        u8 widget_index = 2 * i + stacklist->disp_index;
        u8 bk_widget_index = 2 * i + (!stacklist->disp_index);
        rect_t rect = widget_get_location(stacklist->icon_item[widget_index]);

        if (i < mid_index) {  
            //左侧          
            rect_t mid_rect = widget_get_location(stacklist->icon_item[2 * mid_index + stacklist->disp_index]);
            if (mid_rect.x < GUI_SCREEN_CENTER_X) {
                continue ;
            }
            rect.x += offset;
            if (rect.x > GUI_SCREEN_CENTER_X) {
                rect.x = GUI_SCREEN_CENTER_X;
                change_dir = 1;
                printf("change_dir:%d\n",change_dir);
            } else if (rect.x < lift_border) {
                rect.x = lift_border;
            }
            size_offset = (rect.x - lift_border) * wid_change / stacklist->icon_pos_gap;
            icon_index = stacklist->cur_index - mid_index + i - 1;
            icon_index = (stacklist->res_addr_count + icon_index) % stacklist->res_addr_count;
            
        } else if (i > mid_index) {  
            //右侧   
            rect_t mid_rect = widget_get_location(stacklist->icon_item[2 * mid_index + stacklist->disp_index]);
            if (mid_rect.x > GUI_SCREEN_CENTER_X) {
                continue ;
            }
            rect.x += offset;
            if (rect.x < GUI_SCREEN_CENTER_X) {
                rect.x = GUI_SCREEN_CENTER_X;
                change_dir = -1;
                printf("change_dir:%d\n",change_dir);
            } else if (rect.x > right_border) {
                rect.x = right_border;
            }
            size_offset = (right_border - rect.x) * wid_change / stacklist->icon_pos_gap;
            icon_index = stacklist->cur_index - mid_index + i + 1;
            icon_index = icon_index % stacklist->res_addr_count;
        } else {      
            //中间                  
            rect.x += offset;
            if (rect.x > right_border) {
                rect.x = right_border;
            } else if (rect.x < lift_border) {
                rect.x = lift_border;
            }
            size_offset = (right_border - rect.x) * wid_change / stacklist->icon_pos_gap;
            if (rect.x < GUI_SCREEN_CENTER_X) {
                size_offset = (rect.x - lift_border) * wid_change / stacklist->icon_pos_gap;
            }
        }

       
        if (icon_index >= 0) {
            //定位叠层图像
            if (!widget_get_visble(stacklist->icon_item[bk_widget_index])) {
                icon_index = (stacklist->res_addr_count + icon_index) % stacklist->res_addr_count;
                widget_icon_set(stacklist->icon_item[bk_widget_index], stacklist->pic_addr[icon_index]);
                rect_t bk_rect = widget_get_location(stacklist->icon_item[bk_widget_index]);
                bk_rect.wid = zoom_table[i] * bk_rect.wid / 100;
                bk_rect.hei = zoom_table[i] * bk_rect.hei / 100;
                widget_set_size(stacklist->icon_item[bk_widget_index], bk_rect.wid, bk_rect.hei);
                widget_set_visible((widget_t *)stacklist->icon_item[bk_widget_index], true);
            }
            //处理叠层透明度
            widget_set_alpha((widget_t *)stacklist->icon_item[bk_widget_index], size_offset * 0xff / wid_change);
            icon_index = -1;
        }
        
        //大小由位置作一阶线性处理
        rect.wid = zoom_table[mid_index - 1] * stacklist->pic_w / 100 + size_offset; 
        rect.hei = zoom_table[mid_index - 1] * stacklist->pic_h / 100 + size_offset;
        widget_set_location(stacklist->icon_item[widget_index], rect.x, rect.y, rect.wid, rect.hei);
    }

    if(change_dir) {
        if (change_dir > 0) {
            if (stacklist->cur_index) {
                stacklist->cur_index --;
            } else {
                stacklist->cur_index = stacklist->res_addr_count - 1;
            }
        } else {
            stacklist->cur_index ++;
            stacklist->cur_index %= stacklist->res_addr_count; 
        }
        compo_stacklist_symm_refresh_pic(stacklist, false);
        TRACE("stacklist->cur_index:%d\n",stacklist->cur_index);
    }
}

/**
 * @brief 堆叠列表拖拽处理
 * @param[in] stacklist : 组件句柄
 * @return 无
 **/
void compo_stacklist_move(compo_stacklist_t *stacklist)
{
    if (stacklist->ctr.flag_drag) {
        s32 dx;
        s32 dy;
    
        stacklist->ctr.flag_drag = ctp_get_dxy(&dx, &dy);
        if (stacklist->ctr.flag_drag) {
            if (stacklist->ctr.last_offset != dx) {
                s16 move_offset = dx - stacklist->ctr.last_offset;
                stacklist->ctr.last_offset = dx;

                switch (stacklist->style) {
                    case STACKLIST_SYMM_STYLE:
                        compo_stacklist_symm_pic_move(stacklist, move_offset);
                        break;
                    
                    case STACKLIST_SEQU_UP_STYLE:
                        break;

                    case STACKLIST_SEQU_DOWN_STYLE:
                        break;
                    
                    default:
                        break;
                }
            }
        } else {
            /* 松手矫正效果 */
            stacklist->disp_index = 0;
            compo_stacklist_symm_refresh_pic(stacklist, true);
            stacklist->ctr.last_offset = 0;
        }
    }
}

