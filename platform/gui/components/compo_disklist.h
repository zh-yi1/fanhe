#ifndef _COMPO_DISKLIST_H
#define _COMPO_DISKLIST_H

#define DISKLIST_MAX_ITEM_CNT            40

#define DISKLIST_ITEM_CNT                7       //显示项目个数

//菜单项定义
typedef struct compo_disklist_item_t_ {
    u32 str_idx;                    //文字
    u32 res_addr;                   //图标
    u8 func_sta;                    //任务序号
} compo_disklist_item_t;

typedef struct compo_disklist_t_ {
    COMPO_STRUCT_COMMON;
    widget_page_t *page;
    widget_icon_t *sel_icon;        //选择的ICON
    int sel_idx;                    //选择的idx

    compo_disklist_item_t const *item;
    int item_cnt;
    u16 icon_size;                  //图标大小(宽度)
    u16 icon_org_size;

    s32 ofs_deg;                    //焦点角度（10倍角）偏移
    u16 max_deg;                    //焦点为末尾图标时的deg*10，非无限旋转时使用
    u16 cycle_deg;                  //转动一圈回到原点经过的deg*10，无限旋转时使用
    s16 focus_icon_idx;             //焦点的图标编号

    bool focus_rem_en;              //焦点对360度取余，自动旋转结束后使能

    int sidx;                       //起始编号
    bool txt_roll_need_rst;         //文本滚动重置标志

    widget_icon_t *item_icon[DISKLIST_ITEM_CNT];
    widget_page_t *item_page_text[DISKLIST_ITEM_CNT];
    widget_text_t *item_text[DISKLIST_ITEM_CNT];
    compo_roll_cb_t roll_cb[DISKLIST_ITEM_CNT];
    s16 item_idx[DISKLIST_ITEM_CNT];


    u8 idx_time;				//加入时分秒针元素
	widget_page_t *page_time;
	widget_icon_t *time_bg;
	widget_image_t *hour;
	widget_image_t *min;
	widget_image_t *sec;
	s16 start_angle;
} compo_disklist_t;

/**
 * @brief 创建一个圆盘列表控件
 * @param[in] frm : 窗体指针
 * @return 返回圆盘列表指针
 **/
compo_disklist_t *compo_disklist_create(compo_form_t *frm);

/**
 * @brief 设置圆盘列表控件内容
 * @param[in] disklist : 圆盘列表指针
 * @param[in] item : 存放列表信息，图片、文字等
 * @param[in] item_cnt : 存放列表信息的数量
 **/
void compo_disklist_set(compo_disklist_t *disklist, compo_disklist_item_t const *item, int item_cnt);

/**
 * @brief 根据列表编号获取10倍角度
 * @param[in] disklist : 圆盘列表指针
 * @param[in] idx : 菜单编号
 * @return 所在角度*10
 **/
int compo_disklist_get_deg_byidx(compo_disklist_t *disklist, int idx);

/**
 * @brief 按旋转角度设置列表框焦点
 * @param[in] disklist : 圆盘列表指针
 * @param[in] deg : 角度*10
 **/
void compo_disklist_set_focus(compo_disklist_t *disklist, int deg);

/**
 * @brief 将选择的图标设为焦点
 * @param[in] disklist : 圆盘列表指针
 * @param[in] idx : 图标编号
 **/
void compo_disklist_set_focus_byidx(compo_disklist_t *disklist, int idx);

/**
 * @brief 更新列表框Widget
 * @param[in] disklist : 圆盘列表指针
 **/
void compo_disklist_update(compo_disklist_t *disklist);

/**
 * @brief 更新列表框Widget，同时重置所有文本滚动（item中str突变时需要调用，防止C208蓝屏）
 * @param[in] disklist : 圆盘列表指针
 **/
void compo_disklist_update_with_text_scroll_rst(compo_disklist_t *disklist);


/**
 * @brief 按坐标选择菜单项
 * @param[in] disklist : 圆盘列表指针
 * @param[in] x : x坐标
 * @param[in] y : y坐标
 * @return 返回菜单项索引
 **/
int compo_disklist_select(compo_disklist_t *disklist, int x, int y);

/**
 * @brief 按索引选择图标
 * @param[in] disklist : 圆盘列表指针
 * @param[in] idx : 编号
 * @return 返回图标指针
 **/
widget_icon_t *compo_disklist_select_byidx(compo_disklist_t *disklist, int idx);

/**
 * @brief 获取选择图标的区域
 * @param[in] disklist : 圆盘列表指针
 * @return 返回图标坐标和大小
 **/
rect_t compo_disklist_get_sel_rect(compo_disklist_t *disklist);

/**
 * @brief 设置圆盘列表控件图标大小
 * @param[in] disklist : 圆盘列表指针
 * @param[in] icon_size : 图标大小
 **/
void compo_disklist_set_iconsize(compo_disklist_t *disklist, u16 icon_size);

/**
 * @brief 向集合中添加时分秒针
          cx cy为旋转中心点, x y为目标坐标
 * @param[in] disklist : 图标集指针
 * @param[in] type : COMPO_ICONLIST_TIME_TYPE_HOUR, 时
                     COMPO_ICONLIST_TIME_TYPE_MIN,  分
                     COMPO_ICONLIST_TIME_TYPE_SEC,  秒
 * @param[in] res_addr : 图像资源地址
 * @param[in] cx : x轴坐标
 * @param[in] cy : y轴坐标
 **/
void compo_disklist_add_time(compo_disklist_t *disklist, u8 type, u32 res_addr, s16 cx, s16 cy);

/**
 * @brief 设置图标集指针的起始角度
 * @param[in] disklist : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_disklist_set_start_angle(compo_disklist_t *disklist, s16 angle);

/**
 * @brief 设置图标集合时钟序号
 * @param[in] disklist : 图标集指针
 * @param[in] idx : 时钟序号
 **/
void compo_disklist_set_time_idx(compo_disklist_t *disklist, u8 idx);


#endif
