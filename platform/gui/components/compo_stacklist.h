#ifndef __COMPO_STACKLIST_H
#define __COMPO_STACKLIST_H

#define STACKLIST_DISP_COUNT_MAX    3  //堆叠列表最多显示3个图标
#define STACKLIST_PIC_COUNT_MAX     10 //支持最大图标数


enum 
{
    STACKLIST_SYMM_STYLE,        //对称样式
    STACKLIST_SEQU_UP_STYLE,     //顺序样式(左面或上面在上)
    STACKLIST_SEQU_DOWN_STYLE,   //顺序样式(右面或下面在上)
};

enum 
{
    STACKLIST_HOR,        //横向
    STACKLIST_VER,        //纵向
};

typedef struct _stacklist_ctr_t
{
    s16 last_offset;
    bool flag_drag;
}stacklist_ctr_t;


typedef struct compo_stacklist_t_ {
    COMPO_STRUCT_COMMON;
    widget_page_t *main_page;
    widget_icon_t *icon_item[STACKLIST_DISP_COUNT_MAX * 2];
    
    u32 pic_addr[STACKLIST_PIC_COUNT_MAX];    
    u16 res_addr_count;
    u16 pic_w;
    u16 pic_h;
    u8 cur_index;   
    u8 disp_count;
    bool disp_index;

    //control
    stacklist_ctr_t ctr;

    //config
    u8 style; 
    u8 dir;
    u8 icon_pos_gap;
    u8 icon_zoom_per;
} compo_stacklist_t;



/**
 * @brief 创建一个堆叠的列表
 * @param[in] frm : 窗体指针
 * @param[in] disp_count : 显示列表上的图片数量
 * @param[in] style : 风格样式：对称、顺序
 * @param[in] dir : 显示方向：横向、纵向
 * @return 返回组件句柄
 **/
compo_stacklist_t *compo_stacklist_create(compo_form_t *frm, u8 disp_count, u8 style, u8 dir);

/**
 * @brief 设置列表图片
 * @param[in] stacklist : 组件句柄
 * @param[in] res_addr_item : 图片地址集合
 * @param[in] res_addr_count : 图片数量
 * @return 无
 **/
void compo_stacklist_set_pic(compo_stacklist_t *stacklist, u32 *res_addr_item, u8 res_addr_count);

/**
 * @brief 设置堆叠图片间中心位置的间隔和缩放比例
 * @param[in] stacklist : 组件句柄
 * @param[in] pic_gap : 图片间的间隔
 * @param[in] zoom_per : 图片间间隔的缩放比例（例如对称式堆叠列表，相对于中间图，往两端间隔缩小25%）
 * @return 无
 **/
void compo_stacklist_set_user_para(compo_stacklist_t *stacklist, u8 pic_gap, u8 zoom_per);

/**
 * @brief 设置拖拽标志
 * @param[in] stacklist : 组件句柄
 * @param[in] drag_flag : 拖拽标志，true or false
 * @return 无
 **/
void compo_stacklist_set_drag(compo_stacklist_t *stacklist, bool drag_flag);

/**
 * @brief 对称式堆叠列表拖拽处理
 * @param[in] stacklist : 组件句柄
 * @param[in] offset : 移动偏移量
 * @return 无
 **/
void compo_stacklist_symm_pic_move(compo_stacklist_t *stacklist, s16 offset);

/**
 * @brief 堆叠列表拖拽处理
 * @param[in] stacklist : 组件句柄
 * @return 无
 **/
void compo_stacklist_move(compo_stacklist_t *stacklist);


#endif