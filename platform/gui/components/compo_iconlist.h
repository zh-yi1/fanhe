#ifndef _COMPO_ICONLIST_H
#define _COMPO_ICONLIST_H

//图标集样式
enum {
    ICONLIST_STYLE_HONEYCOMB,
    ICONLIST_STYLE_WATERFALL,
    ICONLIST_STYLE_SUDOKU,
    ICONLIST_STYLE_SUDOKU_HRZ,  //横向九宫格
    ICONLIST_STYLE_RING,        //环形菜单
    ICONLIST_STYLE_GRID,        //栅格菜单

    //客户要求新菜单风格
    ICONLIST_STYLE_CUM_SUDOKU,   //九宫格(边缘图标会缩小)
    ICONLIST_STYLE_CUM_GRID,     //华为栅格菜单风格
    ICONLIST_STYLE_CUM_HEXAGON,  //六边型菜单风格

    ICONLIST_STYLE_CUM_HONEYGRID, //网格蜂窝（华为风格）
};

typedef struct compo_iconlist_t_ {
    COMPO_STRUCT_COMMON;
    widget_page_t *page;
    widget_icon_t *sel_icon;    //选择的ICON
    s16 sel_idx;                //选择的idx
    u8 style;                   //图标集样式

    u32 icon_size;              //中央图标大小(与纵向大小相等)，放大256倍
    u32 icon_widhalf;           //横向宽度的一半，放大256倍
    u32 icon_lineheight;        //纵向行高，放大256倍
    u32 icon_min_size;          //最小显示尺寸
    u16 icon_org_size;          //图标原始大小
    u16 icon_total_size;        //icon_size + icon_space
    u16 icon_space;             //图标间隙
    u16 icon_threshold;         //多大以内的图标不进行缩放(平铺展示)
    u16 icon_zoomout_size;      //平铺时的大小

    s16 ofs_x;                  //焦点的坐标相对偏移
    s16 ofs_y;
    s16 max_r;                  //最大半径
	s16 max_hei;				//最大高度
	s16 ln_hei;					//行高
    s16 ratio_base;
    s16 ratio;                  //缩小速度

    widget_icon_t *focus_icon;  //焦点的图标
    s16 focus_icon_x;           //焦点的图标相对坐标
    s16 focus_icon_y;
    u8 focus_icon_idx;          //焦点的图标编号
    u8 total_cnt;               //图标个数
    u8 rcnt;                    //蜂窝圈数

    u8 idx_time;                //加入时分秒针元素
    widget_page_t *page_time;
    widget_icon_t *time_bg;
    widget_image_t *hour;
    widget_image_t *min;
    widget_image_t *sec;
    s16 start_angle;

    /*九宫格*/
    s16 icon0_x;                //左上角图标坐标（相对屏幕）
    s16 icon0_y;
    u16 icon_space_y;           //图标间距（横向间距用icon_space）
    s16 ofs_max;              //滑动到底端时的ofs_y/ofs_x
    s16 slide_min;            //回弹前焦点超过边界的允许值
    s16 slide_max;
    u8 total_line;              //图标行数


    /* 华为栅格 */
    area_t scale_area_min;
    area_t scale_area_max;
    point_t gofs_min;
    point_t gofs_max;

} compo_iconlist_t;

enum COMPO_ICONLIST_TIME_TYPE {
    COMPO_ICONLIST_TIME_TYPE_HOUR,
    COMPO_ICONLIST_TIME_TYPE_MIN,
    COMPO_ICONLIST_TIME_TYPE_SEC,
};

/**
 * @brief 创建一个图标集合
 * @param[in] frm : 窗体指针
 * @param[in] style : ICONLIST_STYLE_HONEYCOMB, 蜂窝
                      ICONLIST_STYLE_WATERFALL, 瀑布
 * @return 返回图标集指针
 **/
compo_iconlist_t *compo_iconlist_create(compo_form_t *frm, u8 style);

/**
 * @brief 修改Page边缘大小
 * @param[in] iconlist : 图标集指针
 * @param[in] width : page的宽度
 * @param[in] height : page的高度
 **/
void compo_iconlist_set_size(compo_iconlist_t *iconlist, s16 width, s16 height);

/**
 * @brief 根据图标编号获取行号
          瀑布流模式下
 * @param[in] iconlist : 图标集指针
 * @param[in] idx : 图标编号
 * @return 行号
 **/
int compo_iconlist_getline_byidx(compo_iconlist_t *iconlist, int idx);

/**
 * @brief 向图标集合中添加图标
 * @param[in] iconlist : 图标集指针
 * @param[in] res_addr : 图像资源地址
 **/
void compo_iconlist_add(compo_iconlist_t *iconlist, u32 res_addr);

/**
 * @brief 向集合中添加时分秒针
          cx cy为旋转中心点, x y为目标坐标
 * @param[in] iconlist : 图标集指针
 * @param[in] type : COMPO_ICONLIST_TIME_TYPE_HOUR, 时
                     COMPO_ICONLIST_TIME_TYPE_MIN,  分
                     COMPO_ICONLIST_TIME_TYPE_SEC,  秒
 * @param[in] res_addr : 图像资源地址
 * @param[in] cx : x轴坐标
 * @param[in] cy : y轴坐标
 **/
void compo_iconlist_add_time(compo_iconlist_t *iconlist, u8 type, u32 res_addr, s16 cx, s16 cy);

/**
 * @brief 设置图标集合时钟序号（需要在compo_iconlist_add之后使用）
 * @param[in] iconlist : 图标集指针
 * @param[in] idx : 时钟序号
 **/
void compo_iconlist_set_time_idx(compo_iconlist_t *iconlist, u8 idx);

/**
 * @brief 设置图标集合图标大小
 * @param[in] iconlist : 图标集指针
 * @param[in] icon_size : 图标大小
 **/
void compo_iconlist_set_iconsize(compo_iconlist_t *iconlist, s16 icon_size);

/**
 * @brief 根据菜单xy编号获取坐标
 * @param[in] iconlist : 图标集指针
 * @param[in] ix : x轴坐标
 * @param[in] iy : y轴坐标
 * @return (x, y)坐标
 **/
point_t compo_iconlist_getxy_byipos(compo_iconlist_t *iconlist, int ix, int iy);

/**
 * @brief 将坐标对齐到最临近的点
 * @param[in] iconlist : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @return (x, y)坐标
 **/
point_t compo_iconlist_align_xy(compo_iconlist_t *iconlist, s32 x, s32 y);

/**
 * @brief 按坐标设置图标集合焦点坐标
 * @param[in] iconlist : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_iconlist_set_focus(compo_iconlist_t *iconlist, s16 x, s16 y);

/**
 * @brief 按索引设置图标集合焦点坐标
 * @param[in] iconlist : 图标集指针
 * @param[in] idx : 编号索引
 **/
void compo_iconlist_set_focus_byidx(compo_iconlist_t *iconlist, int idx);

/**
 * @brief 更新图标集合Widget
 * @param[in] iconlist : 图标集指针
 **/
void compo_iconlist_update(compo_iconlist_t *iconlist);

/**
 * @brief 按坐标选择图标
 * @param[in] iconlist : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @return 返回图标索引
 **/
int compo_iconlist_select(compo_iconlist_t *iconlist, s32 x, s32 y);

/**
 * @brief 按索引选择图标
 * @param[in] iconlist : 图标集指针
 * @param[in] idx : 编号索引
 * @return 返回图标指针
 **/
widget_icon_t *compo_iconlist_select_byidx(compo_iconlist_t *iconlist, int idx);

/**
 * @brief 获取选择图标的区域
 * @param[in] iconlist : 图标集指针
 * @return 返回图标坐标和大小
 **/

rect_t compo_iconlist_get_sel_rect(compo_iconlist_t *iconlist);
/**
 * @brief 设置图标集指针的起始角度
 * @param[in] iconlist : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_iconlist_set_start_angle(compo_iconlist_t *iconlist, s16 angle);

#endif
