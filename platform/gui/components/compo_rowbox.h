#ifndef _COMPO_ROWBOX_H
#define _COMPO_ROWBOX_H

#define IMG_MAX         7

enum COMPO_ROWBOX_STYLE {
    COMPO_ROWBOX_STYLE_NORMAL,             //普通样式
    COMPO_ROWBOX_STYLE_PREVIEW,            //预览图
    COMPO_ROWBOX_STYLE_ARC,                //弧形
};

typedef struct compo_rowbox_move_cb_t_ {
    u32 tick;
    s32 focus_x;
    s32 focus_dx;
    s32 focus_xstep;
    int moveto_idx;                 //设定自动移到的目标菜单
    s32 moveto_x;                   //设定自动移到的y坐标
    s32 first_x;                    //首图标坐标
    s32 last_x;                     //尾图标坐标
    bool flag_drag;                 //开始拖动
    bool flag_move_auto;            //自动移到坐标
} compo_rowbox_move_cb_t;

typedef struct compo_rowbox_t_ {
    COMPO_STRUCT_COMMON;
    compo_rowbox_move_cb_t *mcb;   //移动控制

    widget_page_t *page;
    widget_image_t *img[IMG_MAX];   //记录图片地址
    s16 img_idx[IMG_MAX];           //记录图片对应的idx
    widget_rect_t *rect[IMG_MAX];   //背景框（橱窗效果）

    const u32 *res_tbl;            //记录图片资源的地址
    u32 (*get_addr)(u32 base_addr); //用于表盘解析地址
    u8 style;                       //样式

    area_t img_area;                //记录图像原始的大小

    u8 img_cnt;                     //图片总计数
    u16 row_space;                  //空白间隙
    u16 row_width;                  //图片宽度
    u16 row_center_x;               //图片x轴中心位置
    u16 row_width_total;            //图片+空白

    u16 cycle_width;                //滑动一圈回到原点经过的距离
    u16 total_width;                //图片拼接总长度
    s16 ofs_x;                      //x偏移值
    s16 ofs_y;                      //y偏移值

    s16 focus_icon_idx;             //焦点的图标编号
    bool flag_cycle;                //是否循环滑动

    //u8 focus_img_idx;


    u8 margin_en;
    u8 margin_radius;
    u8 margin_width;
    u16 margin_color;


} compo_rowbox_t;

/**
 * @brief 创建一个列表（横向）控件
 * @param[in] frm : 窗体指针
 * @return 返回列表（横向）指针
 **/
compo_rowbox_t *compo_rowbox_create(compo_form_t *frm);

/**
 * @brief 设置列表（横向）控件内容
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] *res_tbl : 存放图像资源的地址
 * @param[in] img_cnt : 图像数量
 **/
void compo_rowbox_set(compo_rowbox_t *rowbox, const u32 *res_tbl, u8 img_cnt);

/**
 * @brief 根据编号获取x坐标
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] idx : 菜单编号
 * @return x坐标
 **/
int compo_rowbox_getx_byidx(compo_rowbox_t *rowbox, int idx);

/**
 * @brief 按x坐标设置列表（横向）焦点
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] x : x坐标
 **/
void compo_rowbox_set_focus(compo_rowbox_t *rowbox, s16 x);

/**
 * @brief 将选择的图标设为焦点
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] idx : 图标编号
 **/
void compo_rowbox_set_focus_byidx(compo_rowbox_t *rowbox, int idx);

/**
 * @brief 设置列表（横向）样式
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] style : 样式
 **/
void compo_rowbox_set_style(compo_rowbox_t *rowbox, u8 style);

/**
 * @brief 设置获取资源地址的回调函数
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] get_addr : 回调函数
 **/
void compo_rowbox_set_addrhook(compo_rowbox_t *rowbox, u32 (*get_addr)(u32 idx));


/**
 * @brief 更新列表框Widget
 * @param[in] rowbox : 列表（横向）指针
 **/
void compo_rowbox_update(compo_rowbox_t *rowbox);

/**
 * @brief 列表框拖动与移动初始化
 * @param[in] rowbox : 列表（横向）指针
 **/
void compo_rowbox_move_init(compo_rowbox_t *rowbox);

/**
 * @brief 列表框拖动与移动处理
 * @param[in] rowbox : 列表（横向）指针
 **/
void compo_rowbox_move(compo_rowbox_t *rowbox);

 /**
 * @brief 按坐标选择菜单项
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] pt : 坐标
 * @return 返回菜单项索引
 **/
int compo_rowbox_select(compo_rowbox_t *rowbox, point_t pt);

/**
 * @brief 是否使能列表循环滑动功能
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] cycle : true 使能循环滑动
                      false 关闭循环滑动
 **/
void compo_rowbox_cycle_en(compo_rowbox_t *rowbox, bool cycle);



/**
 * @brief 设置列表（横向）控件内容
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] margin_en : 是否使能边框
 * @param[in] margin_width : 边框的宽度
 * @param[in] margin_radius : 边框的半径
 * @param[in] margin_color : 边框的颜色
 * @return 0成功，其他失败
 **/
int compo_rowbox_set_margin(compo_rowbox_t *rowbox, u8 margin_en, u8 margin_width, u16 margin_radius, u16 margin_color);

/**
 * @brief 设置列表图标的纵向偏移
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] ofs_y : 相对于屏幕中央的纵向偏移
 **/
void compo_rowbox_set_ofs_y(compo_rowbox_t *rowbox, s16 ofs_y);

/**
 * @brief 获取焦点序号
 * @param[in] rowbox : 列表（横向）指针
 * @return 返回焦点序号（如果是负数则转为正数）
 **/
s16 compo_rowbox_get_focus_idx(compo_rowbox_t *rowbox);

/**
 * @brief 按索引选择图标
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] idx : 编号索引
 * @return 返回图标指针
 **/
widget_icon_t *compo_rowbox_select_byidx(compo_rowbox_t *rowbox, int idx);

#endif
