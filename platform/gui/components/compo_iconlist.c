#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MAX_ICON_COUNT          61                                      //最大支持 5-6-7-8-9-8-7-6-5 六边形
#define MAX_ICON_LINE           9                                       //大小为 9 * 9

#define MIN_VALID_ICON_SIZE     (GUI_SCREEN_WIDTH / 10)                 //最小可点击的图标尺寸

#define ICON_SIZE_Q             8                                       //放大256倍
#define RATIO_SHIFT             8                                       //缩小速度
#define ICON_ROUND(x)           ((s16)(((x) + 128) >> ICON_SIZE_Q))
#define ICON_SQRT_3             ((int)(1.7320508075688772935274463415059 * (1 << ICON_SIZE_Q)))
#define GRID_ICON_SQRT_3        ((int)(1.9320508075688772935274463415059 * (1 << ICON_SIZE_Q)))
#define MIN_ICON_SIZE           (10 * (1 << ICON_SIZE_Q))               //最小图标尺寸
#define MIN_ICON_SIZE_2         (4 * (1 << ICON_SIZE_Q))

//蜂窝六边形坐标索引
static const s8 tbl_hexagon_pos[MAX_ICON_COUNT][2] = {
    {0, 0},                                                                 //0
    {2, 0},     {1, -1},    {-1, -1},   {-2, 0},    {-1, 1},    {1, 1},     //1 ~ 6
    {3, 1},     {4, 0},     {3, -1},    {2, -2},    {0, -2},    {-2, -2},   //7 ~ 18
    {-3, -1},   {-4, 0},    {-3, 1},    {-2, 2},    {0, 2},     {2, 2},
    {4, 2},     {5, 1},     {6, 0},     {5, -1},    {4, -2},    {3, -3},    //19 ~ 36
    {1, -3},    {-1, -3},   {-3, -3},   {-4, -2},   {-5, -1},   {-6, 0},
    {-5, 1},    {-4, 2},    {-3, 3},    {-1, 3},    {1, 3},     {3, 3},     //37 ~ 60
    {5, 3},     {6, 2},     {7, 1},     {8, 0},     {7, -1},    {6, -2},
    {5, -3},    {4, -4},    {2, -4},    {0, -4},    {-2, -4},   {-4, -4},
    {-5, -3},   {-6, -2},   {-7, -1},   {-8, 0},    {-7, 1},    {-6, 2},
    {-5, 3},    {-4, 4},    {-2, 4},    {0, 4},     {2, 4},     {4, 4},
};

//参考原始蜂窝做的华为风格坐标索引
static const s8 tbl_hexagon_grid_pos[MAX_ICON_COUNT][2] = {
    {0, 0},                                                                 //0
    {2, 0},     {0, -1},    {-2, -1},   {-2, 0},    {-2, 1},    {0, 1},     //1 ~ 6
    {2, 1},     {4, 0},     {2, -1},    {2, -2},    {0, -2},    {-2, -2},   //7 ~ 18
    {-4, -1},   {-4, 0},    {-4, 1},    {-2, 2},    {0, 2},     {2, 2},
    {4, 2},     {4, 1},     {6, 0},     {4, -1},    {4, -2},    {2, -3},    //19 ~ 36
    {0, -3},    {-2, -3},   {-4, -3},   {-4, -2},   {-6, -1},   {-6, 0},
    {-6, 1},    {-4, 2},    {-4, 3},    {-2, 3},    {0, 3},     {2, 3},     //37 ~ 60
    {4, 3},     {6, 2},     {6, 1},     {8, 0},     {6, -1},    {6, -2},
    {4, -3},    {4, -4},    {2, -4},    {0, -4},    {-2, -4},   {-4, -4},
    {-6, -3},   {-6, -2},   {-6, -1},   {-8, 0},    {-6, 1},    {-6, 2},
    {-6, 3},    {-4, 4},    {-2, 4},    {0, 4},     {2, 4},     {4, 4},
};


//瀑布流坐标索引
static const s8 tbl_hexagon_pos_wf[7][2] = {
    {-2, 0},  {0, 0},  {2, 0},  {-3, 1},  {-1, 1},  {1, 1},  {3, 1}
};

//华为风格菜单
#define MAX_GRID_ICON_COUNT      81
static const s8 tbl_gridbox_pos[MAX_GRID_ICON_COUNT][2] = {
    //0中心点
    {0,0},
    //第一圈
    {1,0},      {1,-1},
    {0,-1},     {-1,-1},
    {-1,0},     {-1,1},
    {0,1},      {1,1},
    //第二圈
    {2,1},      {2,0},      {2,-1},     {2,-2},
    {1,-2},     {0,-2},     {-1,-2},    {-2,-2},
    {-2,-1},    {-2,0},     {-2,1},     {-2,2},
    {-1,2},     {0,2},      {1,2},      {2,2},
    //第三圈
    {3,2},      {3,1},      {3,0},      {3,-1},     {3,-2},     {3,-3},
    {2,-3},     {1,-3},     {0,-3},     {-1,-3},    {-2,-3},    {-3,-3},
    {-3,-2},    {-3,-1},    {-3,0},     {-3,1},     {-3,2},     {-3,3},
    {-2,3},     {-1,3},     {0,3},      {1,3},      {2,3},      {3,3},
    //第四圈
    {4,3},      {4,2},      {4,1},      {4,0},      {4,-1},     {4,-2},     {4,-3},     {4,-4},
    {3,-4},     {2,-4},     {1,-4},     {0,-4},     {-1,-4},    {-2,-4},    {-3,-4},    {-4,-4},
    {-4,-3},    {-4,-2},    {-4,-1},    {-4,0},     {-4,1},     {-4,2},     {-4,3},     {-4,4},
    {-3,4},     {-2,4},     {-1,4},     {0,4},      {1,4},      {2,4},      {3,4},      {4,4},
    //第五圈
    //{5,4},
};

/**
 * @brief 创建一个图标集合
 * @param[in] frm : 窗体指针
 * @param[in] style : ICONLIST_STYLE_HONEYCOMB, 蜂窝
                      ICONLIST_STYLE_WATERFALL, 瀑布
                      ICONLIST_STYLE_SUDOKU，九宫格
                      ICONLIST_STYLE_SUDOKU_HRZ，横向九宫格
                      ICONLIST_STYLE_RING，环形菜单
 * @return 返回图标集指针
 **/
compo_iconlist_t *compo_iconlist_create(compo_form_t *frm, u8 style)
{
    compo_iconlist_t *iconlist = compo_create(frm, COMPO_TYPE_ICONLIST);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    iconlist->page = page;
    iconlist->style = style;
    switch (style) {
    case ICONLIST_STYLE_CUM_HONEYGRID:
    case ICONLIST_STYLE_HONEYCOMB:
        iconlist->icon_min_size = MIN_ICON_SIZE;
        break;

    case ICONLIST_STYLE_WATERFALL:
        iconlist->icon_min_size = MIN_ICON_SIZE_2;
        break;

    case ICONLIST_STYLE_SUDOKU:
    case ICONLIST_STYLE_SUDOKU_HRZ:
    case ICONLIST_STYLE_RING:
        break;

    case ICONLIST_STYLE_CUM_SUDOKU:
        iconlist->icon_min_size = 20;           //最小图标大小
        break;

    case ICONLIST_STYLE_GRID:
    case ICONLIST_STYLE_CUM_GRID:
        break;

    case ICONLIST_STYLE_CUM_HEXAGON:
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_STYLE);
        break;
    }
    return iconlist;
}

/**
 * @brief 修改Page边缘大小
 * @param[in] iconlist : 图标集指针
 * @param[in] width : page的宽度
 * @param[in] height : page的高度
 **/
void compo_iconlist_set_size(compo_iconlist_t *iconlist, s16 width, s16 height)
{
    widget_set_size(iconlist->page, width, height);
}

/**
 * @brief 根据图标编号获取行号
          瀑布流模式下
 * @param[in] iconlist : 图标集指针
 * @param[in] idx : 图标编号
 * @return 行号
 **/
int compo_iconlist_getline_byidx(compo_iconlist_t *iconlist, int idx)
{
    if (iconlist->style != ICONLIST_STYLE_WATERFALL) {
        halt(HALT_GUI_COMPO_ICONLIST_GETLINE);
    }
    const s8 *p = tbl_hexagon_pos_wf[idx % 7];
    return (p[1] + 2 * (idx / 7));
}

/**
 * @brief 向图标集合中添加图标
 * @param[in] iconlist : 图标集指针
 * @param[in] res_addr : 图像资源地址
 **/
void compo_iconlist_add(compo_iconlist_t *iconlist, u32 res_addr)
{
    if (iconlist == NULL) {
        halt(HALT_GUI_COMPO_ICONLIST_ADD);
    }
    widget_icon_t *icon = widget_icon_create(iconlist->page, res_addr);

    if (iconlist->total_cnt == 0) {
#if BOX_GUI_ROTATE_DISP
        rect_t location = rotate_widget_get_location(icon);
#else
        rect_t location = widget_get_location(icon);
#endif // BOX_GUI_ROTATE_DISP

        iconlist->time_bg = icon;                               //如果有时钟且没设置序号，默认第一个为时钟
        iconlist->icon_org_size = location.wid;
        if (iconlist->style == ICONLIST_STYLE_SUDOKU || iconlist->style == ICONLIST_STYLE_SUDOKU_HRZ) {
#if BOX_GUI_ROTATE_DISP
            iconlist->icon_space = GUI_SCREEN_HEIGHT / 3;  //平均分9格居中对齐
            iconlist->icon0_x = (iconlist->icon_space) / 2 - GUI_SCREEN_CENTER_Y;  //首图标坐标
            iconlist->icon_space_y = GUI_SCREEN_WIDTH / 3;
            iconlist->icon0_y = (iconlist->icon_space_y) / 2 - GUI_SCREEN_CENTER_X;
#else
            iconlist->icon_space = GUI_SCREEN_WIDTH / 3;  //平均分9格居中对齐
            iconlist->icon0_x = (iconlist->icon_space) / 2 - GUI_SCREEN_CENTER_X;  //首图标坐标
            iconlist->icon_space_y = GUI_SCREEN_HEIGHT / 3;
            iconlist->icon0_y = (iconlist->icon_space_y) / 2 - GUI_SCREEN_CENTER_Y;
#endif // BOX_GUI_ROTATE_DISP
            iconlist->ln_hei = iconlist->style == ICONLIST_STYLE_SUDOKU ? iconlist->icon_space_y : iconlist->icon_space;
            iconlist->slide_min = -(iconlist->ln_hei / 2);
            compo_iconlist_set_iconsize(iconlist, iconlist->icon_org_size);
        } else if (iconlist->style == ICONLIST_STYLE_RING) {
            iconlist->ln_hei = iconlist->icon_org_size;
            iconlist->slide_min = -(iconlist->ln_hei / 2);
        } else if (iconlist->style == ICONLIST_STYLE_HONEYCOMB || iconlist->style == ICONLIST_STYLE_WATERFALL) {  //蜂窝/瀑布流
            iconlist->icon_zoomout_size = iconlist->icon_org_size * 9 / 20;
            iconlist->ratio_base = iconlist->icon_org_size * 9 / 16;        //基础缩小速度，越大缩的越快
            iconlist->icon_space = iconlist->icon_org_size / 7;
            iconlist->icon_total_size = iconlist->icon_org_size + iconlist->icon_space;
            compo_iconlist_set_iconsize(iconlist, iconlist->icon_org_size);

        } else if (iconlist->style == ICONLIST_STYLE_CUM_HONEYGRID) {       //参考原始蜂窝做的华为风格
            iconlist->icon_zoomout_size = iconlist->icon_org_size * 9 / 20;
            iconlist->ratio_base = iconlist->icon_org_size * 9 / 16;        //基础缩小速度，越大缩的越快
            iconlist->icon_space = iconlist->icon_org_size / 7;
            iconlist->icon_total_size = iconlist->icon_org_size + iconlist->icon_space;
            compo_iconlist_set_iconsize(iconlist, iconlist->icon_org_size);

        } else if (iconlist->style == ICONLIST_STYLE_CUM_SUDOKU) {              //根据图标间距与屏幕大小自动适配图标大小, 保持九个图标显示在屏幕中心
            iconlist->icon_space_y = 20;
#if BOX_GUI_ROTATE_DISP
            iconlist->icon_size = (GUI_SCREEN_WIDTH - 4 * iconlist->icon_space_y) / 4;
            iconlist->icon_space = GUI_SCREEN_CENTER_X - 5 - (iconlist->icon_size * 3 / 2);
#else
            iconlist->icon_size = (GUI_SCREEN_HEIGHT - 4 * iconlist->icon_space_y) / 4;
            iconlist->icon_space = GUI_SCREEN_CENTER_X - 5 - (iconlist->icon_size * 3 / 2);
#endif // BOX_GUI_ROTATE_DISP
            iconlist->icon_total_size = iconlist->icon_size + iconlist->icon_space_y;
            iconlist->ln_hei = iconlist->icon_total_size * 2 + iconlist->icon_size;         //图标保持原大小区域
            iconlist->slide_min = -(iconlist->icon_total_size / 2);
        } else if (iconlist->style == ICONLIST_STYLE_GRID || iconlist->style == ICONLIST_STYLE_CUM_GRID) {
            if (iconlist->style == ICONLIST_STYLE_CUM_GRID) {
                iconlist->icon_org_size = location.wid;
            }
            iconlist->icon_size = iconlist->icon_org_size;
            iconlist->icon_space = iconlist->icon_size / 10;
            iconlist->icon_total_size = iconlist->icon_space + iconlist->icon_size;
            iconlist->scale_area_min.wid = iconlist->icon_total_size * 2 + iconlist->icon_size;
            iconlist->scale_area_min.hei = iconlist->scale_area_min.wid;
            iconlist->scale_area_max.wid = iconlist->icon_total_size * 4;
            iconlist->scale_area_max.hei = iconlist->scale_area_max.wid;
        } else if (iconlist->style == ICONLIST_STYLE_CUM_HEXAGON) {
            iconlist->icon_size = iconlist->icon_org_size;
#if BOX_GUI_ROTATE_DISP
            iconlist->icon_space = muls_shift16(GUI_SCREEN_CENTER_X- iconlist->icon_size / 2, COS(300)) - iconlist->icon_size;
#else
            iconlist->icon_space = muls_shift16(GUI_SCREEN_CENTER_X - iconlist->icon_size / 2, COS(300)) - iconlist->icon_size;
#endif // BOX_GUI_ROTATE_DISP
            iconlist->icon_space_y = iconlist->icon_space;
            iconlist->icon_total_size = iconlist->icon_size + iconlist->icon_space_y;
            iconlist->slide_min = -(iconlist->icon_total_size / 2);
        }
    }

    switch (iconlist->style) {
    case ICONLIST_STYLE_CUM_HONEYGRID:
    case ICONLIST_STYLE_HONEYCOMB:
        iconlist->rcnt = 0;
        if (iconlist->total_cnt <= 0) {
            iconlist->rcnt = 0;
        } else if (iconlist->total_cnt <= 6) {
            iconlist->rcnt = 1;
        } else if (iconlist->total_cnt <= 18) {
            iconlist->rcnt = 2;
        } else if (iconlist->total_cnt <= 36) {
            iconlist->rcnt = 3;
        } else if (iconlist->total_cnt <= 60) {
            iconlist->rcnt = 4;
        } else {
            halt(HALT_GUI_COMPO_ICONLIST_ADD);
        }
        iconlist->max_r = (iconlist->icon_org_size / 2) + iconlist->rcnt * iconlist->icon_total_size;
        break;

    case ICONLIST_STYLE_WATERFALL:
        if (iconlist->total_cnt >= 7) {
            iconlist->max_hei = compo_iconlist_getline_byidx(iconlist, iconlist->total_cnt) * iconlist->ln_hei;
        } else {
            iconlist->max_hei = 2 * iconlist->ln_hei;
        }
        break;

    case ICONLIST_STYLE_SUDOKU:
    case ICONLIST_STYLE_SUDOKU_HRZ:
    case ICONLIST_STYLE_RING:
        iconlist->total_line = (iconlist->total_cnt + 3) / 3;
        if (iconlist->style == ICONLIST_STYLE_RING) {
            iconlist->ofs_max = (iconlist->total_line - 1) * iconlist->ln_hei;
        } else {
            iconlist->ofs_max = iconlist->total_line > 3 ? (iconlist->total_line - 3) * iconlist->ln_hei : 0;
        }
        iconlist->slide_max = iconlist->ofs_max + iconlist->ln_hei / 2;
        break;

    case ICONLIST_STYLE_CUM_SUDOKU:
        iconlist->total_line = (iconlist->total_cnt + 3) / 3;
        iconlist->ofs_max = (iconlist->total_line - 1) * (iconlist->icon_size + iconlist->icon_space_y);
        iconlist->slide_max = iconlist->ofs_max + (iconlist->icon_size + iconlist->icon_space_y) / 2;
        break;

    case ICONLIST_STYLE_GRID:
    case ICONLIST_STYLE_CUM_GRID:
        if (iconlist->total_cnt <= 0) {
            iconlist->rcnt = 0;
        } else if (iconlist->total_cnt <= 3 * 3) {
            iconlist->rcnt = 1;
        } else if (iconlist->total_cnt <= 5 * 5) {
            iconlist->rcnt = 2;
        } else if (iconlist->total_cnt <= 7 * 7) {
            iconlist->rcnt = 3;
        } else if (iconlist->total_cnt <= 9 * 9) {
            iconlist->rcnt = 4;
        } else {
            halt(HALT_GUI);
        }

        iconlist->gofs_min.y = - (iconlist->rcnt * iconlist->icon_total_size);
        iconlist->gofs_max.y = iconlist->rcnt * iconlist->icon_total_size;
        iconlist->gofs_min.x = - (iconlist->rcnt * iconlist->icon_total_size);
        iconlist->gofs_max.x = iconlist->rcnt * iconlist->icon_total_size;

        iconlist->icon_zoomout_size = GUI_SCREEN_WIDTH / (iconlist->rcnt * 2 + 4) ;
        break;

    case ICONLIST_STYLE_CUM_HEXAGON:
        iconlist->total_line = (iconlist->total_cnt + 3) / 3;
        iconlist->ofs_max = (iconlist->total_line - 1) * iconlist->icon_total_size;
        iconlist->slide_max = iconlist->ofs_max + iconlist->icon_total_size / 2;
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_STYLE);
        return;
    }
    iconlist->total_cnt++;
}

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
void compo_iconlist_add_time(compo_iconlist_t *iconlist, u8 type, u32 res_addr, s16 cx, s16 cy)
{
    if (iconlist->page_time == NULL) {
        iconlist->page_time = widget_page_create(iconlist->page);
#if BOX_GUI_ROTATE_DISP
        rotate_widget_set_location(iconlist->page_time, 0, 0, iconlist->icon_org_size, iconlist->icon_org_size);
        rotate_widget_page_set_client(iconlist->page_time, iconlist->icon_org_size / 2, iconlist->icon_org_size / 2);
#else
        widget_set_location(iconlist->page_time, 0, 0, iconlist->icon_org_size, iconlist->icon_org_size);
        widget_page_set_client(iconlist->page_time, iconlist->icon_org_size / 2, iconlist->icon_org_size / 2);
#endif // BOX_GUI_ROTATE_DISP
    }
    widget_image_t *img = widget_image_create(iconlist->page_time, res_addr);
#if BOX_GUI_ROTATE_DISP
    ro_widget_set_pos(img, 0, 0, iconlist->icon_org_size, iconlist->icon_org_size);
    rotate_widget_image_set_rotation_center(img, cx, cy);
#else
    widget_image_set_rotation_center(img, cx, cy);
#endif // BOX_GUI_ROTATE_DISP


    switch (type) {
    case COMPO_ICONLIST_TIME_TYPE_HOUR:
        iconlist->hour = img;
        break;

    case COMPO_ICONLIST_TIME_TYPE_MIN:
        iconlist->min = img;
        break;

    case COMPO_ICONLIST_TIME_TYPE_SEC:
        iconlist->sec = img;
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_TIME_TYPE);
        return;
    }
    compo_cb.rtc_update = true;                 //创建时钟组件，需要同步更新时间
}

/**
 * @brief 设置图标集合时钟序号（需要在compo_iconlist_add之后使用）
 * @param[in] iconlist : 图标集指针
 * @param[in] idx : 时钟序号
 **/
void compo_iconlist_set_time_idx(compo_iconlist_t *iconlist, u8 idx)
{
    if (iconlist == NULL || iconlist->total_cnt <= idx) {
        halt(HALT_GUI_COMPO_ICONLIST_TIME_IDX);
    }

    u8 idx_tmp = 0;
    widget_t *widget = widget_get_next(iconlist->page);
    while (widget != NULL) {
        if (idx_tmp == idx) {
            iconlist->time_bg = widget;
            break;
        }
        widget = widget_get_next(widget);
        idx_tmp++;
    }
    iconlist->idx_time = idx;
}

/**
 * @brief 设置图标集合图标大小
          蜂窝/瀑布流模式下
 * @param[in] iconlist : 图标集指针
 * @param[in] icon_size : 图标大小
 **/
void compo_iconlist_set_iconsize(compo_iconlist_t *iconlist, s16 icon_size)
{
    const s8 *p;
    if (iconlist == NULL) {
        halt(HALT_GUI_COMPO_ICONLIST_SET_ICONSIZE);
    }
    if (iconlist->icon_org_size == 0 || icon_size == 0) {
        halt(HALT_GUI_COMPO_ICONLIST_ICONSIZE);
    }

    //计算图标大小
    switch (iconlist->style) {
    case ICONLIST_STYLE_HONEYCOMB:
    case ICONLIST_STYLE_WATERFALL:
        iconlist->icon_space = icon_size / 7;
        iconlist->icon_total_size = icon_size + iconlist->icon_space;
        iconlist->icon_size = icon_size << ICON_SIZE_Q;
        iconlist->icon_widhalf = (iconlist->icon_total_size << (ICON_SIZE_Q - 1));
        iconlist->icon_lineheight = (iconlist->icon_widhalf * ICON_SQRT_3) >> ICON_SIZE_Q;
        iconlist->max_r = (icon_size / 2) + iconlist->rcnt * iconlist->icon_total_size;
        iconlist->ln_hei = iconlist->icon_lineheight >> ICON_SIZE_Q;

        iconlist->ratio = iconlist->ratio_base + (icon_size - iconlist->icon_org_size) * 2;
        if (iconlist->ratio < 0) {
            iconlist->ratio = 0;
        }

        p = tbl_hexagon_pos[iconlist->focus_icon_idx];
        iconlist->ofs_x = ICON_ROUND((s32)p[0] * iconlist->icon_widhalf);
        iconlist->ofs_y = ICON_ROUND((s32)p[1] * iconlist->icon_lineheight);
        break;
    case ICONLIST_STYLE_CUM_HONEYGRID:
        //iconlist->icon_space = icon_size / 7;
        iconlist->icon_space = icon_size / 9;
        iconlist->icon_total_size = icon_size + iconlist->icon_space;
        iconlist->icon_size = icon_size << ICON_SIZE_Q;
        iconlist->icon_widhalf = (iconlist->icon_total_size << (ICON_SIZE_Q - 1));
        iconlist->icon_lineheight = (iconlist->icon_widhalf * GRID_ICON_SQRT_3) >> ICON_SIZE_Q;
        iconlist->max_r = (icon_size / 2) + iconlist->rcnt * iconlist->icon_total_size;
        iconlist->ln_hei = iconlist->icon_lineheight >> ICON_SIZE_Q;

        iconlist->ratio = iconlist->ratio_base + (icon_size - iconlist->icon_org_size) * 2;

        iconlist->ratio = (int)iconlist->ratio * 12 / 10;

        if (iconlist->ratio < 0) {
            iconlist->ratio = 0;
        }

        p = tbl_hexagon_grid_pos[iconlist->focus_icon_idx];
        iconlist->ofs_x = ICON_ROUND((s32)p[0] * iconlist->icon_widhalf);
        iconlist->ofs_y = ICON_ROUND((s32)p[1] * iconlist->icon_lineheight);
        break;

    case ICONLIST_STYLE_SUDOKU:
    case ICONLIST_STYLE_SUDOKU_HRZ:
        iconlist->icon_size = icon_size;
        break;

    case ICONLIST_STYLE_CUM_SUDOKU:
        #if 0
        iconlist->icon_space_y = 20;
        iconlist->icon_size = (GUI_SCREEN_HEIGHT - 4 * iconlist->icon_space_y) / 4;
        iconlist->icon_space = GUI_SCREEN_CENTER_X - 5 - (iconlist->icon_size * 3 / 2);
        iconlist->icon_total_size = iconlist->icon_size + iconlist->icon_space_y;
        iconlist->ln_hei = iconlist->icon_total_size * 2 + iconlist->icon_size;
        iconlist->total_line = (iconlist->total_cnt + 2) / 3;
        iconlist->ofs_max = (iconlist->total_line - 1) * (iconlist->icon_size + iconlist->icon_space_y);
        iconlist->slide_min = -(iconlist->icon_total_size / 2);
        iconlist->slide_max = iconlist->ofs_max + (iconlist->icon_size + iconlist->icon_space_y) / 2;
        #endif // 0
        break;

    case ICONLIST_STYLE_GRID:
    case ICONLIST_STYLE_CUM_GRID:
        iconlist->icon_size = icon_size;
        iconlist->icon_space = iconlist->icon_size / 10;
        iconlist->icon_total_size = iconlist->icon_space + iconlist->icon_size;

        iconlist->scale_area_min.wid = iconlist->icon_total_size * 2 + iconlist->icon_size;
        iconlist->scale_area_min.hei = iconlist->scale_area_min.wid;
        iconlist->scale_area_max.wid = iconlist->icon_total_size * 4;
        iconlist->scale_area_max.hei = iconlist->scale_area_max.wid;

        iconlist->gofs_min.y = - (iconlist->rcnt * iconlist->icon_total_size);
        iconlist->gofs_max.y = iconlist->rcnt * iconlist->icon_total_size;
        iconlist->gofs_min.x = - (iconlist->rcnt * iconlist->icon_total_size);
        iconlist->gofs_max.x = iconlist->rcnt * iconlist->icon_total_size;

        const s8 *p;
        p = tbl_gridbox_pos[iconlist->focus_icon_idx];
        iconlist->ofs_x = p[0] * iconlist->icon_total_size;
        iconlist->ofs_y = p[1] * iconlist->icon_total_size;

        break;

    case ICONLIST_STYLE_CUM_HEXAGON:
        iconlist->icon_size = icon_size;
        iconlist->icon_space = muls_shift16(GUI_SCREEN_CENTER_X - iconlist->icon_size / 2, COS(300)) - iconlist->icon_size;
        iconlist->icon_space_y = iconlist->icon_space;
        iconlist->icon_total_size = iconlist->icon_size + iconlist->icon_space_y;
        iconlist->total_line = (iconlist->total_cnt + 2) / 3;
        iconlist->ofs_max = (iconlist->total_line - 1) * iconlist->icon_total_size;
        iconlist->slide_max = iconlist->ofs_max + iconlist->icon_total_size / 2;
        iconlist->slide_min = -(iconlist->icon_total_size / 2);
        break;
    }
}

/**
 * @brief 根据菜单xy编号获取坐标
          蜂窝/瀑布流模式下
 * @param[in] iconlist : 图标集指针
 * @param[in] ix : x轴坐标
 * @param[in] iy : y轴坐标
 * @return (x, y)坐标
 **/
point_t compo_iconlist_getxy_byipos(compo_iconlist_t *iconlist, int ix, int iy)
{
    point_t pt;
    switch (iconlist->style) {
    case ICONLIST_STYLE_GRID:
    case ICONLIST_STYLE_CUM_GRID:
        pt.x = ix * iconlist->icon_total_size;
        pt.y = iy * iconlist->icon_total_size;
        break;

    case ICONLIST_STYLE_CUM_HONEYGRID:
        pt.x = (ix * iconlist->icon_total_size) >> 1;
        pt.y = (iy * iconlist->icon_total_size * GRID_ICON_SQRT_3) >> (ICON_SIZE_Q + 1);
        break;

    default:
        pt.x = (ix * iconlist->icon_total_size) >> 1;
        pt.y = (iy * iconlist->icon_total_size * ICON_SQRT_3) >> (ICON_SIZE_Q + 1);
        break;
    }
    return pt;
}

/**
 * @brief 将坐标对齐到最临近的点
 * @param[in] iconlist : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @return (x, y)坐标
 **/
point_t compo_iconlist_align_xy(compo_iconlist_t *iconlist, s32 x, s32 y)
{
    int ix, iy, d1, d2;
    point_t pt, pt2, pt3;
    area_t dx, dy;
    int pt_ix, pt2_ix;

    switch (iconlist->style) {
    case ICONLIST_STYLE_CUM_HONEYGRID:
    case ICONLIST_STYLE_HONEYCOMB:
        ix = (x << 1) / iconlist->icon_total_size;
        iy = (y << (ICON_SIZE_Q + 1)) / (iconlist->icon_total_size* ICON_SQRT_3);
        if ((ix + iy) & 1) {
            //坐标位于两个图标中心时，对齐到其中一个
            if (ix >= 0) {
                ix++;
            } else {
                ix--;
            }
        }
        pt = compo_iconlist_getxy_byipos(iconlist, ix, iy);
        d1 = (x - pt.x) * (x - pt.x) + (y - pt.y) * (y - pt.y);

        pt_ix = ix;
        //printf("d1 %d, pt_ix %d\n", d1, ix);

        //计算最近相邻点
        if (x >= 0) {
            ix++;
        } else {
            ix--;
        }
        if (y >= 0) {
            iy++;
        } else {
            iy--;
        }
        pt2 = compo_iconlist_getxy_byipos(iconlist, ix, iy);
        d2 = (x - pt2.x) * (x - pt2.x) + (y - pt2.y) * (y - pt2.y);

        pt2_ix = ix;
        //printf("d2 %d, pt2_ix %d\n", d2, ix);

        if (iconlist->style == ICONLIST_STYLE_CUM_HONEYGRID) {
            if (d1 < d2) {
                if (pt_ix % 2) {
                    if (x >= 0) {
                        pt.x += (iconlist->icon_total_size >> 1);
                    } else {
                        pt.x -= (iconlist->icon_total_size >> 1);
                    }
                }
                return pt;
            } else {
                if (pt2_ix % 2) {
                    if (x >= 0) {
                        pt2.x += (iconlist->icon_total_size >> 1);
                    } else {
                        pt2.x -= (iconlist->icon_total_size >> 1);
                    }
                }
                return pt2;
            }
        }

        break;

    case ICONLIST_STYLE_WATERFALL:
        iy = (y << (ICON_SIZE_Q + 1)) / (iconlist->icon_total_size* ICON_SQRT_3);
        pt = compo_iconlist_getxy_byipos(iconlist, 0, iy);
        d1 = abs_s(y - pt.y);
        if (y >= 0) {
            iy++;
        } else {
            iy--;
        }
        pt2 = compo_iconlist_getxy_byipos(iconlist, 0, iy);
        d2 = abs_s(y - pt2.y);
        break;

    case ICONLIST_STYLE_SUDOKU:
    case ICONLIST_STYLE_SUDOKU_HRZ:
    case ICONLIST_STYLE_RING:
        ix = iconlist->style == ICONLIST_STYLE_SUDOKU_HRZ ? x : y;  //xy复用一下变量
        iy = ix / iconlist->ln_hei * iconlist->ln_hei;
        iy = MIN(iconlist->ofs_max, MAX(0, iy));
        pt.x = iconlist->style == ICONLIST_STYLE_SUDOKU_HRZ ? iy : 0;
        pt.y = iconlist->style == ICONLIST_STYLE_SUDOKU_HRZ ? 0 : iy;
        d1 = abs_s(ix - iy);
        iy += ix > 0 ? iconlist->ln_hei : -iconlist->ln_hei;
        pt2.x = iconlist->style == ICONLIST_STYLE_SUDOKU_HRZ ? iy : 0;
        pt2.y = iconlist->style == ICONLIST_STYLE_SUDOKU_HRZ ? 0 : iy;
        d2 = abs_s(ix - iy);
        break;

    case ICONLIST_STYLE_CUM_SUDOKU:
        ix = y;
        iy = ix / iconlist->icon_total_size * iconlist->icon_total_size;
        iy = MIN(iconlist->ofs_max, MAX(0, iy));
        pt.x = 0;
        pt.y = iy;
        d1 = abs_s(ix - iy);

        iy += ix > 0 ? iconlist->icon_total_size : -iconlist->icon_total_size;
        pt2.x = 0;
        pt2.y = iy;
        d2 = abs_s(ix - iy);
        break;

    case ICONLIST_STYLE_GRID:
    case ICONLIST_STYLE_CUM_GRID:
        ix = x / iconlist->icon_total_size; //第几列
        ix = ix * iconlist->icon_total_size;
        iy = y / iconlist->icon_total_size; //第几行
        iy = iy * iconlist->icon_total_size;
        //限制
        ix = ix >= 0 ? MIN(iconlist->gofs_max.x, ix) : MAX(iconlist->gofs_min.x, ix);
        iy = iy >= 0 ? MIN(iconlist->gofs_max.y, iy) : MAX(iconlist->gofs_min.y, iy);
        pt.x = ix;
        pt.y = iy;
        dx.wid = abs_s(x - ix);
        dy.wid = abs_s(y - iy);

        ix = (x / iconlist->icon_total_size) + 1;
        ix = ix * iconlist->icon_total_size;
        iy = (y / iconlist->icon_total_size) + 1;
        iy = iy * iconlist->icon_total_size;
        ix = ix >= 0 ? MIN(iconlist->gofs_max.x, ix) : MAX(iconlist->gofs_min.x, ix);
        iy = iy >= 0 ? MIN(iconlist->gofs_max.y, iy) : MAX(iconlist->gofs_min.y, iy);
        pt2.x = ix;
        pt2.y = iy;
        dx.hei = abs_s(x - ix);
        dy.hei = abs_s(y - iy);

        if (dx.wid < dx.hei) {
            pt3.x = pt.x;
        } else {
            pt3.x = pt2.x;
        }

        if (dy.wid < dy.hei) {
            pt3.y = pt.y;
        } else {
            pt3.y = pt2.y;
        }

        return pt3;
        break;

    case ICONLIST_STYLE_CUM_HEXAGON:
        ix = y;
        iy = ix / (iconlist->icon_total_size / 2) * (iconlist->icon_total_size / 2);
        iy = MIN(iconlist->ofs_max, MAX(0, iy));
        pt.x = 0;
        pt.y = iy;
        d1 = abs_s(ix - iy);

        iy += ix > 0 ? (iconlist->icon_total_size / 2) : (-iconlist->icon_total_size / 2);
        pt2.x = 0;
        pt2.y = iy;
        d2 = abs_s(ix - iy);
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_STYLE);
        return pt;
    }

    //对比输出与目标更接近的点
    if (d1 < d2) {
        return pt;
    } else {
        return pt2;
    }
}

/**
 * @brief 按坐标设置图标集合焦点坐标
 * @param[in] iconlist : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_iconlist_set_focus(compo_iconlist_t *iconlist, s16 x, s16 y)
{
    int ix, iy, r;
    rect_t location;
    if (iconlist == NULL) {
        halt(HALT_GUI_COMPO_ICONLIST_SET_FOCUS);
    }
#if BOX_GUI_ROTATE_DISP
    location = rotate_widget_get_location(iconlist->page);
#else
    location = widget_get_location(iconlist->page);
#endif // BOX_GUI_ROTATE_DISP
    switch (iconlist->style) {
    case ICONLIST_STYLE_CUM_HONEYGRID:
    case ICONLIST_STYLE_HONEYCOMB:
		r = sqrt64(x * x + y * y);
		if (r > iconlist->max_r) {
			x = x * iconlist->max_r / r;
			y = y * iconlist->max_r / r;
		}
        break;

    case ICONLIST_STYLE_WATERFALL:
		r = iconlist->icon_org_size >> 1;
		if (x < -r) {
			x = -r;
		} else if (x > r) {
			x = r;
		}
		if (y < iconlist->ln_hei * 2 - r) {
			y = iconlist->ln_hei * 2 - r;
		} else if (y > iconlist->max_hei + r) {
			y = iconlist->max_hei + r;
		}
        break;

    case ICONLIST_STYLE_SUDOKU:
    case ICONLIST_STYLE_RING:
        x = 0;
        y = y > iconlist->slide_max ? iconlist->slide_max : (y < iconlist->slide_min ? iconlist->slide_min : y);
        break;

    case ICONLIST_STYLE_SUDOKU_HRZ:
        x = x > iconlist->slide_max ? iconlist->slide_max : (x < iconlist->slide_min ? iconlist->slide_min : x);
        y = 0;
        break;

    case ICONLIST_STYLE_CUM_SUDOKU:
        x = 0;
        y = y > iconlist->slide_max ? iconlist->slide_max : (y < iconlist->slide_min ? iconlist->slide_min : y);
        break;

    case ICONLIST_STYLE_GRID:
    case ICONLIST_STYLE_CUM_GRID:
        x = x > iconlist->gofs_max.x ? iconlist->gofs_max.x : (x < iconlist->gofs_min.x ? iconlist->gofs_min.x : x);
        y = y > iconlist->gofs_max.y ? iconlist->gofs_max.y : (y < iconlist->gofs_min.y ? iconlist->gofs_min.y : y);
        break;

    case ICONLIST_STYLE_CUM_HEXAGON:
        x = 0;
        y = y > iconlist->slide_max ? iconlist->slide_max : (y < iconlist->slide_min ? iconlist->slide_min : y);
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_STYLE);
        return;
    }
    iconlist->ofs_x = x;
    iconlist->ofs_y = y;
    ix = (location.wid >> 1) - iconlist->ofs_x;
    iy = (location.hei >> 1) - iconlist->ofs_y;
#if BOX_GUI_ROTATE_DISP
    rotate_widget_page_set_client(iconlist->page, ix, iy);
#else
    widget_page_set_client(iconlist->page, ix, iy);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 按索引设置图标集合焦点坐标
 * @param[in] iconlist : 图标集指针
 * @param[in] idx : 编号索引
 **/
void compo_iconlist_set_focus_byidx(compo_iconlist_t *iconlist, int idx)
{
    point_t pt;
    int ix, iy;
    const s8 *p;

    switch (iconlist->style) {
    case ICONLIST_STYLE_HONEYCOMB:
        p = tbl_hexagon_pos[idx];
        ix = p[0];
        iy = p[1];
        pt = compo_iconlist_getxy_byipos(iconlist, ix, iy);
        break;
    case ICONLIST_STYLE_CUM_HONEYGRID:
        p = tbl_hexagon_grid_pos[idx];
        ix = p[0];
        iy = p[1];
        pt = compo_iconlist_getxy_byipos(iconlist, ix, iy);
        break;

    case ICONLIST_STYLE_WATERFALL:
        ix = 0;
        iy = compo_iconlist_getline_byidx(iconlist, idx);
        pt = compo_iconlist_getxy_byipos(iconlist, ix, iy);
        break;

    case ICONLIST_STYLE_SUDOKU:
    case ICONLIST_STYLE_SUDOKU_HRZ:
        iy = idx / 3;  //第iy行显示在屏幕中间
        iy = iy == 0 ? 1 : (iy == iconlist->total_line - 1 ? iy - 1 : iy);  //首行/尾行向中间退一行
        iy = iy == 0 ? 1 : iy;  //只有两行图标时
        pt.y = iconlist->style == ICONLIST_STYLE_SUDOKU ? (iy - 1) * iconlist->ln_hei : 0;
        pt.x = iconlist->style == ICONLIST_STYLE_SUDOKU ? 0 : (iy - 1) * iconlist->ln_hei;
        break;

    case ICONLIST_STYLE_RING:
        pt.y = idx / 3 * iconlist->ln_hei;
        pt.x = 0;
        break;

    case ICONLIST_STYLE_CUM_SUDOKU:
        pt.y = idx / 3 * iconlist->icon_total_size;
        pt.x = 0;
        break;

    case ICONLIST_STYLE_GRID:
    case ICONLIST_STYLE_CUM_GRID:
        p = tbl_gridbox_pos[idx];
        ix = p[0];
        iy = p[1];
        pt = compo_iconlist_getxy_byipos(iconlist, ix, iy);
        break;

    case ICONLIST_STYLE_CUM_HEXAGON:
        pt.y = idx / 3 * iconlist->icon_total_size + (idx % 3 == 0 ? 0 : iconlist->icon_total_size / 2);
        pt.x = 0;
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_STYLE);
        return;
    }

    compo_iconlist_set_focus(iconlist, pt.x, pt.y);
}

/**
 * @brief 更新图标集合Widget
 * @param[in] iconlist : 图标集指针
 **/
void compo_iconlist_update(compo_iconlist_t *iconlist)
{
    widget_t *widget;
    int idx = 0;
    int win_w_half, win_h_half, dc_r;
    int edge_l, edge_r, edge_t, edge_b;
    int cx, cy, cr, dw;
    int min_ds;
    bool focus_flag;
#if BOX_GUI_ROTATE_DISP
    rect_t location = rotate_widget_get_location(iconlist->page);
#else
    rect_t location = widget_get_location(iconlist->page);
#endif // BOX_GUI_ROTATE_DISP

    cx = (s32)iconlist->ofs_x << ICON_SIZE_Q;                   //屏幕中心点坐标
    cy = (s32)iconlist->ofs_y << ICON_SIZE_Q;
    cr = iconlist->icon_size >> 1;                           //维持原大小的中心区域

    dw = iconlist->icon_widhalf * 2;                            //图标中心点间距

    win_w_half = (location.wid >> 1) << ICON_SIZE_Q;            //尺寸的一半
    win_h_half = (location.hei >> 1) << ICON_SIZE_Q;
    dc_r = (win_w_half < win_h_half) ? win_h_half : win_w_half;         //圆形半径(边缘处理)
    //dc_r += dc_r >> 3;
    min_ds = dc_r;


    if (iconlist->style == ICONLIST_STYLE_CUM_HONEYGRID) {
        edge_l = cx - win_w_half + (5 << ICON_SIZE_Q);
        edge_r = cx + win_w_half - (5 << ICON_SIZE_Q);
    } else {
        edge_l = cx - win_w_half + (10 << ICON_SIZE_Q);
        edge_r = cx + win_w_half - (10 << ICON_SIZE_Q);
    }
    edge_t = cy - win_h_half + (5 << ICON_SIZE_Q);
    edge_b = cy + win_h_half - (5 << ICON_SIZE_Q);

    if (iconlist->style > ICONLIST_STYLE_WATERFALL) {
        min_ds = max(location.wid >> 1, location.hei >> 1);
        if (iconlist->style == ICONLIST_STYLE_RING) {
            dw = (location.wid >> 1) - iconlist->icon_org_size / 2 - 10;  //左右图标离中间图标原始距离
            dc_r = iconlist->ln_hei * 3;  //dc_r之外的图标不作处理
        } else if(iconlist->style == ICONLIST_STYLE_GRID || iconlist->style == ICONLIST_STYLE_CUM_GRID) {
            min_ds = iconlist->icon_total_size;
        }
    }

    if (iconlist->style == ICONLIST_STYLE_CUM_HONEYGRID) {
        min_ds = dc_r;
    }

    iconlist->focus_icon = NULL;
    iconlist->focus_icon_idx = 0;
    iconlist->focus_icon_x = 0;
    iconlist->focus_icon_y = 0;
    widget = widget_get_next(iconlist->page);
    while (widget != NULL) {
        if (widget_is_icon(widget) && widget_get_parent(widget) == iconlist->page) {
            int ix, iy, iw, ir;
            int dx, dy, ds;
            int t, t0;
			const s8 *p;

            switch (iconlist->style) {
            case ICONLIST_STYLE_CUM_HONEYGRID:
            case ICONLIST_STYLE_HONEYCOMB:
            case ICONLIST_STYLE_WATERFALL:
                if (iconlist->style == ICONLIST_STYLE_HONEYCOMB) {
                    p = tbl_hexagon_pos[idx];
                    ix = (s32)p[0] * iconlist->icon_widhalf;
                    iy = (s32)p[1] * iconlist->icon_lineheight;
                } else if (iconlist->style == ICONLIST_STYLE_CUM_HONEYGRID) {
                    p = tbl_hexagon_grid_pos[idx];
                    ix = (s32)p[0] * iconlist->icon_widhalf;
                    iy = (s32)p[1] * iconlist->icon_lineheight;
                } else {
                    p = tbl_hexagon_pos_wf[idx % 7];
                    ix = (s32)p[0] * iconlist->icon_widhalf;
                    iy = ((s32)p[1] + 2 * (idx / 7)) * iconlist->icon_lineheight;
                }

                iw = iconlist->icon_size;

                dx = ix - cx;                                       //计算离屏幕中心点距离
                dy = iy - cy;
                ds = sqrt64(__builtin_muls(dx, dx) + __builtin_muls(dy, dy));

                if (iconlist->ratio == 0 || ds <= cr) {
                    ;                                                   //ir以内图标维持大小
                } else {
                    t = (ds - cr) * iconlist->ratio >> RATIO_SHIFT;    //缩小速度
                    iw -= t;
                    //printf("dw %d, ds %d, cr %d, (ds-cr) %d, t %d\n", dw, ds, cr, (ds - cr), t);

                    do {
                        if (iw <= 0) {
                            break;
                        }
                        t >>= 1;
                        t0 = ds;
                        while ((t0 - cr) > dw) {                        //去除内圈距离
                            t0 -= dw;
                            t += (t0 - cr) * iconlist->ratio >> RATIO_SHIFT;
                        };
                        ir = iw >> 1;                                   //图标半径

                        if (ds - t + ir > dc_r) {                       //斜对角半径处理
                            t0 = ds - t + ir - dc_r;
                            iw -= t0;
                            t += (t0 >> 1);
                            if (iw <= 0) {
                                break;
                            }
                        }
                        ix -= t * dx / ds;
                        iy -= t * dy / ds;

                        ir = iw >> 1;                                   //四边缘距离处理
                        if (ix - ir < edge_l) {
                            iw = ix - edge_l + ir;
                            ir = iw >> 1;
                            ix = edge_l + ir;
                        } else if (ix + ir > edge_r) {
                            iw = edge_r - ix + ir;
                            ir = iw >> 1;
                            ix = edge_r - ir;
                        }
                        if (iy - ir < edge_t) {
                            iw = iy - edge_t + ir;
                            ir = iw >> 1;
                            iy = edge_t + ir;
                        } else if (iy + ir > edge_b) {
                            iw = edge_b - iy + ir;
                            ir = iw >> 1;
                            iy = edge_b - ir;
                        }
                    } while (0);
                    if (iw < iconlist->icon_min_size) {
                        iw = 0;
                    }
                }

                //转换成最后坐标
                if (iw > 0) {
                    iw = ICON_ROUND(iw);
                    ix = ICON_ROUND(ix);
                    iy = ICON_ROUND(iy);
                } else {
                    iw = 0;
                }

                focus_flag = ds < min_ds;
                break;

            case ICONLIST_STYLE_SUDOKU:
                ix = iconlist->icon0_x + (idx % 3) * iconlist->icon_space;
                iy = iconlist->icon0_y + (idx / 3) * iconlist->icon_space_y;
                iw = iconlist->icon_size;
                ds = abs_s(iy - iconlist->ofs_y);
                focus_flag = ds < min_ds || (ds == min_ds && idx % 3 == 1);  //取中间行第1或第2个图标（第2优先）
                break;

            case ICONLIST_STYLE_SUDOKU_HRZ:
                ix = iconlist->icon0_x + (idx / 3) * iconlist->icon_space;
                iy = iconlist->icon0_y + (idx % 3) * iconlist->icon_space_y;
                iw = iconlist->icon_size;
                ds = abs_s(ix - iconlist->ofs_x);
                focus_flag = ds < min_ds || (ds == min_ds && idx % 3 == 1);
                break;

            case ICONLIST_STYLE_RING:
                iw = iconlist->icon_org_size;
                ix = 0;
                iy = idx / 3 * iconlist->ln_hei;
                dy = iy - iconlist->ofs_y;
                ds = abs_s(dy);  //所在行离焦点距离
                if (ds <= dc_r) {
                    iw -= (iw / 6) * ds / iconlist->ln_hei;  //中间图标大小
                    //if (ds <= iconlist->ln_hei) {
                    //    iw = iconlist->icon_org_size;
                    //} else {
                    //    iw -= (iw / 5) * ds / iconlist->ln_hei;  //中间图标大小
                    //}

                    if (!((idx % 3) & 1)) {
                        iw = iw * 9 / 10;
                        iw &= ~BIT(0);
                        if (idx % 3 == 0) {  //左图标
                            ix -= dw;
                            ix += dw / 2 * (ds * ds) / (dc_r * dc_r);  //圆心横向偏移
                        } else {  //右图标
                            ix += dw;
                            ix -= dw / 2 * (ds * ds) / (dc_r * dc_r);
                        }
                    }
                    if (ds > iconlist->ln_hei * 3 / 2) {  //1.5倍行高内不偏移
                        iy -= (dy / ds) * (ds - iconlist->ln_hei * 3 / 2) / 6;  //圆心纵向偏移
                    }
                }
                focus_flag = ds < min_ds || (ds == min_ds && idx % 3 == 1);
//                if (idx == 0) {
//                    TRACE(">>>\n");
//                }
//                TRACE("[%d %d; %d] ", ix, iy, iw);
//                if (idx % 3 == 2 || idx == iconlist->total_cnt - 1) {
//                    if (ds < iconlist->icon_org_size / 2) {
//                        TRACE("---[focus]");
//                    }
//                    TRACE("\n");
//                }
                break;

            case ICONLIST_STYLE_CUM_SUDOKU:
                iw = iconlist->icon_size;
                ix = 0;
                iy = idx / 3 * iconlist->icon_total_size;       //当前图标所在位置iy
                dy = iy - iconlist->ofs_y;
                ds = abs_s(dy);                                 //焦点距离
                focus_flag = ds < min_ds || (ds == min_ds && idx % 3 == 1);
                int dds = ds < iconlist->ln_hei / 2 ? 0 : (ds - iconlist->ln_hei / 2);        //ln_hei聚焦放大区域，当在区域内不进行缩小
                iw -= iw * dds / (GUI_SCREEN_CENTER_Y - iconlist->ln_hei / 2);              //缩放, 在放大区域才可正常缩放, 否则iw会变成负数

                if (!((idx % 3) & 1)) {
                    if (idx % 3 == 0) { //左图标
                        ix = -(iconlist->icon_space + iconlist->icon_size);
                    } else {    //右图标
                        ix = iconlist->icon_space + iconlist->icon_size;
                    }
                }

                if (iw < iconlist->icon_size && ds <= GUI_SCREEN_CENTER_Y) {
                    if (iw < iconlist->icon_min_size) {
                        iw = iconlist->icon_min_size;
                    }
                    iy -= (dy / ds) * (iconlist->icon_size - iw) / 2;
                }
                //printf("icon [%d], ixyw[%d,%d,%d], dshei[%d,%d]\n",idx, ix, iy, iw, ds, iconlist->ln_hei / 2);

                break;

            case ICONLIST_STYLE_GRID:
            case ICONLIST_STYLE_CUM_GRID:
                p = tbl_gridbox_pos[idx];
                iw = iconlist->icon_size;
                ix = (s32)p[0] * iconlist->icon_total_size;
                iy = (s32)p[1] * iconlist->icon_total_size;


                int sdx = ix - iconlist->ofs_x;
                int sdy = iy - iconlist->ofs_y;
                dx = abs_s(sdx);        //焦点x距离
                dy = abs_s(sdy);        //焦点y距离
                ds = sqrt64(__builtin_muls(dx, dx) + __builtin_muls(dy, dy));

                if (iconlist->style == ICONLIST_STYLE_CUM_GRID) {       //华为菜单风格
                    if (iw == iconlist->icon_org_size) {
                        int ddx = (dx <= iconlist->scale_area_max.wid / 2) &&
                                  (dx > iconlist->scale_area_min.wid / 2) ?
                                  (dx - iconlist->scale_area_min.wid / 2) : 0;

                        int ddy = (dy <= iconlist->scale_area_max.hei / 2) &&
                                  (dy > iconlist->scale_area_min.hei / 2) ?
                                  (dy - iconlist->scale_area_min.hei / 2) : 0;

                        //iw -= ddx == 0 ? (ddy == 0 ? 0 : ddy) : ddx;


                        iw -= iw * ddx / (GUI_SCREEN_CENTER_X - iconlist->scale_area_min.wid / 2) +
                              iw * ddy / (GUI_SCREEN_CENTER_X - iconlist->scale_area_min.hei / 2);


                        if (dx > iconlist->scale_area_min.wid / 2 && dx <= iconlist->scale_area_max.wid / 2) {
                            if (iw < iconlist->icon_org_size/4) {
                                iw = iconlist->icon_org_size/4;
                            }

                            if (dy == 0 && iw < iconlist->icon_org_size/2) {
                                iw = iconlist->icon_org_size/2;
                            }
                            if (dy > iconlist->scale_area_min.hei / 2 && dy <= iconlist->scale_area_max.hei / 2) {
                                int tmp = iconlist->icon_size - iconlist->icon_size * ddx / (GUI_SCREEN_CENTER_X - iconlist->scale_area_min.wid / 2);
                                if (tmp < iconlist->icon_org_size/4) {
                                    tmp = iconlist->icon_org_size/4;
                                }
                                ix -= (sdx / dx) * (iconlist->icon_size - tmp) / 2;
                            } else {
                                ix -= (sdx / dx) * (iconlist->icon_size - iw) / 2;
                            }

                        }

                        if (dy > iconlist->scale_area_min.hei / 2 && dy <= iconlist->scale_area_max.hei / 2) {
                            if (iw < iconlist->icon_org_size/4) {
                                iw = iconlist->icon_org_size/4;
                            }

                            if (dx == 0 && iw < iconlist->icon_org_size/2) {
                                iw = iconlist->icon_org_size/2;
                            }

                            if (dx > iconlist->scale_area_min.wid / 2 && dx <= iconlist->scale_area_max.wid / 2) {
                                int tmp = iconlist->icon_size - iconlist->icon_size * ddy / (GUI_SCREEN_CENTER_X - iconlist->scale_area_min.hei / 2);
                                if (tmp < iconlist->icon_org_size/4) {
                                    tmp = iconlist->icon_org_size/4;
                                }
                                iy -= (sdy / dy) * (iconlist->icon_size - tmp) / 2;
                            } else {
                                iy -= (sdy / dy) * (iconlist->icon_size - iw) / 2;
                            }

                        }

                        if (dx > iconlist->scale_area_max.wid / 2 || dy > iconlist->scale_area_max.hei / 2) {
                            iw = 0;
                        }

                        if (dx >= iconlist->scale_area_max.wid / 2 && dy >= iconlist->scale_area_max.hei / 2) {
                            iw = 0;
                        }

                        /*
                        printf("idx[%d] dxy[%d,%d] scale[%d,%d,%d,%d] dd[%d,%d,%d] ixy[%d,%d]\n", idx, dx, dy,
                               gridbox->scale_area_max.wid / 2, gridbox->scale_area_min.wid / 2,
                               gridbox->scale_area_max.hei / 2, gridbox->scale_area_min.hei / 2,
                               ddx, ddy, iw, ix, iy);
                        */
                    }
                }

                focus_flag = ds < min_ds;

                break;

            case ICONLIST_STYLE_CUM_HEXAGON:
                iw = iconlist->icon_size;
                ix = 0;
                iy = idx / 3 * iconlist->icon_total_size + (idx == 0 ? 0 : (idx % 3 == 0 ? 0 : iconlist->icon_total_size / 2));
                dy = iy - iconlist->ofs_y;
                ds = abs_s(dy);

                if (idx % 3 != 0) {
                    if (idx % 3 == 1) {
                        ix = - muls_shift16(iconlist->icon_total_size, COS(300));
                    } else {
                        ix = muls_shift16(iconlist->icon_total_size , COS(300));
                    }
                }

                focus_flag = ds < min_ds || (ds == min_ds && idx % 3 == 1);

                //printf("idx[%d], ixy[%d,%d]\n", idx, ix, iy);
                break;

            default:
                halt(HALT_GUI_COMPO_ICONLIST_STYLE);
                return;
            }
#if BOX_GUI_ROTATE_DISP
            rotate_widget_set_location(widget, ix, iy, iw, iw);
#else
            widget_set_location(widget, ix, iy, iw, iw);
#endif // BOX_GUI_ROTATE_DISP
            //获取中心图标及其相应的坐标偏移
            if (focus_flag) {
                iconlist->focus_icon = widget;
                iconlist->focus_icon_idx = idx;
                iconlist->focus_icon_x = ix;
                iconlist->focus_icon_y = iy;
                min_ds = ds;
            }

            //更新时钟尺寸
            if (iconlist->page_time != NULL && idx == iconlist->idx_time) {
                //默认iconlist->idx_time为0，正中间
#if BOX_GUI_ROTATE_DISP
                rotate_widget_page_scale_to(iconlist->page_time, iw, iw);
                rotate_widget_set_pos(iconlist->page_time, ix, iy);
#else
                widget_page_scale_to(iconlist->page_time, iw, iw);
                widget_set_pos(iconlist->page_time, ix, iy);
#endif // BOX_GUI_ROTATE_DISP
                if (iw <= 0) {
                    widget_set_visible(iconlist->page_time, false);
                } else {
                    widget_set_visible(iconlist->page_time, true);
                }
            }
            idx++;
        }
        //遍历Widget
        widget = widget_get_next(widget);
    }
//    TRACE("focus[%d]\n", iconlist->focus_icon_idx);
}

/**
 * @brief 按坐标选择图标
 * @param[in] iconlist : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @return 返回图标索引
 **/
int compo_iconlist_select(compo_iconlist_t *iconlist, s32 x, s32 y)
{
    int idx = 0;
    widget_t *widget = widget_get_next(iconlist->page);
    while (widget != NULL) {
        if (widget_get_parent(widget) == iconlist->page) {
#if BOX_GUI_ROTATE_DISP
            rect_t rect = rotate_widget_get_absolute(widget);
#else
            rect_t rect = widget_get_absolute(widget);
#endif // BOX_GUI_ROTATE_DISP
            if (iconlist->style == ICONLIST_STYLE_GRID || iconlist->style == ICONLIST_STYLE_CUM_GRID) {
                if (rect.wid >= iconlist->icon_zoomout_size - 1 && abs_s(x - rect.x) * 2 <= rect.wid && abs_s(y - rect.y) * 2 <= rect.hei) {
                    iconlist->sel_icon = widget;
                    iconlist->sel_idx = idx;
                    return idx;
                }
            } else {
                if (rect.wid >= MIN_VALID_ICON_SIZE && abs_s(x - rect.x) * 2 <= rect.wid && abs_s(y - rect.y) * 2 <= rect.hei) {
                    iconlist->sel_icon = widget;
                    iconlist->sel_idx = idx;
                    return idx;
                }
            }
            idx++;
        }
        //遍历Widget
        widget = widget_get_next(widget);
    }
    iconlist->sel_icon = NULL;
    iconlist->sel_idx = -1;
    return -1;
}

/**
 * @brief 按索引选择图标
 * @param[in] iconlist : 图标集指针
 * @param[in] idx : 编号索引
 * @return 返回图标指针
 **/
widget_icon_t *compo_iconlist_select_byidx(compo_iconlist_t *iconlist, int idx)
{
    int i = 0;
    widget_t *widget = widget_get_next(iconlist->page);
    while (widget != NULL) {
        if (widget_get_parent(widget) == iconlist->page) {
            if (i == idx) {
                iconlist->sel_icon = widget;
                iconlist->sel_idx = idx;
                return iconlist->sel_icon;
            }
            i++;
        }
        //遍历Widget
        widget = widget_get_next(widget);
    }
    iconlist->sel_icon = NULL;
    iconlist->sel_idx = -1;
    return NULL;
}

/**
 * @brief 获取选择图标的区域
 * @param[in] iconlist : 图标集指针
 * @return 返回图标坐标和大小
 **/
rect_t compo_iconlist_get_sel_rect(compo_iconlist_t *iconlist)
{
    rect_t rect = {0};
    if (iconlist->sel_icon == NULL) {
        return rect;
    }
#if BOX_GUI_ROTATE_DISP
    rect = rotate_widget_get_location(iconlist->sel_icon);
#else
    rect = widget_get_location(iconlist->sel_icon);
#endif // BOX_GUI_ROTATE_DISP
    rect.x -= iconlist->ofs_x;
    rect.y -= iconlist->ofs_y;
    return rect;
}

/**
 * @brief 设置图标集指针的起始角度
 * @param[in] iconlist : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_iconlist_set_start_angle(compo_iconlist_t *iconlist, s16 angle)
{
    iconlist->start_angle = angle;
}

