#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define TABLE_SIZE 181
static const int cos_table[TABLE_SIZE] = {
    10000, 9998, 9994, 9986, 9976, 9962, 9945, 9925, 9903, 9877, 9848, 9816, 9781, 9743, 9702, 9659, 9612, 9563,
    9511, 9455, 9397, 9336, 9272, 9205, 9135, 9063, 8988, 8910, 8829, 8746, 8660, 8571, 8480, 8386, 8290, 8191,
    8090, 7986, 7880, 7771, 7660, 7547, 7431, 7313, 7193, 7071, 6947, 6820, 6691, 6561, 6428, 6293, 6157, 6018,
    5878, 5736, 5592, 5446, 5299, 5150, 5000, 4848, 4695, 4540, 4384, 4226, 4067, 3907, 3746, 3584, 3420, 3256,
    3090, 2924, 2756, 2588, 2419, 2250, 2079, 1908, 1736, 1564, 1391, 1218, 1045, 871, 697, 523, 349, 175, 0,
    -175, -349, -523, -697, -871, -1045, -1218, -1391, -1564, -1736, -1908, -2079, -2250, -2419, -2588, -2756,
    -2924, -3090, -3256, -3420, -3584, -3746, -3907, -4067, -4226, -4384, -4540, -4695, -4848, -5000, -5150,
    -5299, -5446, -5592, -5736, -5878, -6018, -6157, -6293, -6428, -6561, -6691, -6820, -6947, -7071, -7193,
    -7313, -7431, -7547, -7660, -7771, -7880, -7986, -8090, -8191, -8290, -8386, -8480, -8571, -8660, -8746,
    -8829, -8910, -8988, -9063, -9135, -9205, -9272, -9336, -9397, -9455, -9511, -9563, -9612, -9659, -9702,
    -9743, -9781, -9816, -9848, -9877, -9903, -9925, -9945, -9962, -9976, -9986, -9994, -9998, -10000
};

/**
 * @brief 二分法查找最接近的余弦值
 * 注：放大系数：10000
 * @param[in] value : 余弦值 * 放大系数
 * @return 返回对应的弧度 精度为1° 0~180°
 **/
int binarysearch(int value)
{
    int low = 0;
    int high = TABLE_SIZE - 1;
    while (low <= high) {
        int mid = adds(low, high) >> 1;
        if (cos_table[mid] == value) {
            return mid;
        } else if (cos_table[mid] < value) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    if (low > TABLE_SIZE - 1) {
        return high;
    } else if (high < 0) {
        return low;
    } else {
        return (subs(value, cos_table[high]) < subs(cos_table[low], value)) ? high : low;
    }
}

/**
 * @brief acos函数
 * 注：放大系数：10000
 * @param[in] value : 余弦值 * 放大系数
 * @return 返回对应的弧度 精度为1° 0~180°
 **/
int myacos(int value)
{
    int angle = binarysearch(value);
    return angle;
}

/**
 * @brief 弧度计算
 * 注：根据当前点和中心点确定余弦值并计算弧度值 精度为1°
 * @param[in] cx :中心点X
 * @param[in] cy :中心点Y
 * @param[in] x  :当前点X
 * @param[in] y  :当前点Y
 * @return 返回对应的弧度 精度为1° 0~180°
 **/
int myrad(int cx, int cy, int x, int y)
{
    int dx = subs(x, cx);
    int dy = subs(y, cy);
    int div = (int)(sqrt64(muls(dx, dx) + muls(dy, dy)));
    if (div == 0) {
        return 0;
    }

    int cos_value = (int)((int)muls(dx, COS_SCALE)) / div;
    return ACOS(cos_value);
}

/**
 * @brief 创建一个环形组件
 * @param[in] frm : 窗体指针
 * @return 返回环形组件指针
 **/
compo_rings_t *compo_rings_create(compo_form_t *frm)
{
    compo_rings_t *rings = compo_create(frm, COMPO_TYPE_RINGS);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    widget_page_set_client(page, 0, 0);  
    rings->page = page;

    return rings;
}

/**
 * @brief 初始一个环形组件
 * @param[in] rings : 环形组件指针
 * @param[in] ele   : 初始元素指针
 * @return 无
 **/
void compo_rings_set(compo_rings_t *rings, compo_rings_ele_t *ele)
{
    if (rings == NULL && (ele->loop_icon_num + ele->other_icon_num) > RINGS_MAX_ICON_CNT) {
        halt(HALT_GUI_COMPO_LISTBOX_SET);
        return;
    }

    rings->ele.x                = ele->x;
    rings->ele.y                = ele->y;
    rings->ele.r                = ele->r;
    rings->ele.div              = ele->div;
    rings->ele.angle            = ele->angle;
    rings->ele.loop_icon_num    = ele->loop_icon_num;
    rings->ele.other_icon_num   = ele->other_icon_num;
    rings->ele.item_num         = ele->item_num;
    rings->ele.item             = ele->item;
}

/**
 * @brief 往环形图标集上面添加图标
 * @param[in] rings     : 环形组件指针
 * @param[in] res_addr  : 图标资源地址
 * @param[in] idx       : 图标任务编号
 * @param[in] loop      : 是否为环形图标
 * @return 图标信息指针
 **/
compo_rings_icon_t *compo_rings_icon_add(compo_rings_t *rings, u32 res_addr, int idx, bool loop)
{
    if (rings == NULL) {
        halt(HALT_GUI_COMPO_ICONLIST_ADD);
    }

    compo_rings_icon_t *icon = NULL;

    if (loop) {     //加载环形上的图标
        if (rings->loop_icon_cnt < rings->ele.loop_icon_num) {
            widget_icon_t *widget = widget_icon_create(rings->page, res_addr);
            rect_t location = widget_get_location(widget);
            icon = &rings->icon[rings->loop_icon_cnt];
            icon->widget   = widget;
            icon->size     = location.wid;
            icon->idx      = idx;
            icon->angle    = rings->ele.angle + rings->loop_icon_cnt * rings->ele.div;
            icon->angle    = RINGS_GET_ANGLE(icon->angle);
            rings->loop_icon_cnt++;
            widget_set_visible(widget, false);
        }
    } else {        //加载非环形上的图标
        if (rings->other_icon_cnt < rings->ele.other_icon_num) {
            widget_icon_t *widget = widget_icon_create(rings->page, res_addr);
            rect_t location = widget_get_location(widget);
            icon = &rings->icon[rings->ele.loop_icon_num + rings->other_icon_cnt];
            icon->widget   = widget;
            icon->size     = location.wid;
            icon->idx      = idx;
            rings->other_icon_cnt++;
            icon->angle    = -rings->other_icon_cnt;
            widget_set_visible(widget, false);
        }
    }

    return icon;
}

/**
 * @brief 返回当前点相对于中心点坐标正Y轴的角度
 * @param[in] rings     : 环形组件指针
 * @param[in] touch     : 触摸信息指针
 * @param[in] cx        : 中心点X
 * @param[in] cy        : 中心点Y
 * @param[in] x         : 当前点X
 * @param[in] y         : 当前点Y
 * @return 相对角度
 **/
int32_t compo_rings_touch_angle(compo_rings_t *rings, compo_rings_touch_t *touch, s16 cx, s16 cy, s16 x, s16 y)
{
    //触摸点相对坐标
    int dx = x - cx;
    int dy = -(y - cy);
    touch->angle = -1;
    //触摸点的象限
    if (dx > 0 && dy > 0) {
        touch->quad = QUADRANT_1;
    } else if (dx < 0 && dy > 0) {
        touch->quad = QUADRANT_2;
    } else if (dx < 0 && dy < 0) {
        touch->quad = QUADRANT_3;
    } else if (dx > 0 && dy < 0) {
        touch->quad = QUADRANT_4;
    }

    int res = myrad(cx, cy, x, y);
    if (res != 0) {
        //根据象限确定和正向Y轴的夹角 0~3600
        if (touch->quad == QUADRANT_1 || touch->quad == QUADRANT_2) {
            touch->angle = RINGS_GET_ANGLE((90 - res) * 10);
        } else if (touch->quad == QUADRANT_3 || touch->quad == QUADRANT_4) {
            touch->angle = RINGS_GET_ANGLE((90 + res) * 10);
        }
    }

    if (touch->angle == -1) {
        if ((dx == 0 && dy == 0) || (dx == 0 && dy > 0)) {
            touch->angle = 0;
        } else if (dx > 0 && dy == 0) {
            touch->angle = 900;
        } else if (dx == 0 && dy < 0) {
            touch->angle = 1800;
        } else if (dx < 0 && dy == 0) {
            touch->angle = 2700;
        }
    }

    return touch->angle;
}

/**
 * @brief 设置滑动前的触摸信息
 * @param[in] rings     : 环形组件指针
 * @param[in] cx        : 中心点X
 * @param[in] cy        : 中心点Y
 * @param[in] x         : 当前点X
 * @param[in] y         : 当前点Y
 * @return 无
 **/
void compo_rings_touch_angle_pre(compo_rings_t *rings, s16 cx, s16 cy, s16 x, s16 y)
{
    compo_rings_touch_angle(rings, &rings->pre, cx, cy, x, y);
}

/**
 * @brief 获取滑动差量角度
 * @param[in] rings     : 环形组件指针
 * @param[in] cx        : 中心点X
 * @param[in] cy        : 中心点Y
 * @param[in] x         : 当前点X
 * @param[in] y         : 当前点Y
 * @return 差量角度
 *          正值：正向顺时针;
 *          负值：逆向逆时针;
 **/
int32_t compo_rings_touch_angle_diff(compo_rings_t *rings, s16 cx, s16 cy, s16 x, s16 y)
{
    compo_rings_touch_angle(rings, &rings->cur, cx, cy, x, y);
    int32_t diff_angle = 0;
    if (rings->cur.angle == rings->pre.angle) return diff_angle;

    if (rings->pre.quad == QUADRANT_2 && rings->cur.quad == QUADRANT_1) {           //临界区域 顺时针
        diff_angle = rings->cur.angle + 3600 - rings->pre.angle;
    } else if (rings->pre.quad == QUADRANT_1 && rings->cur.quad == QUADRANT_2) {    //临界区域 逆时针
        diff_angle = rings->cur.angle - 3600 - rings->pre.angle;
    } else {
        diff_angle = rings->cur.angle - rings->pre.angle;
    }

    diff_angle = abs(diff_angle) > 10 ? diff_angle : 0;                             //消抖
    rings->pre.angle = rings->cur.angle;
    rings->pre.quad  = rings->cur.quad;
    return diff_angle;
}

/**
 * @brief 查找环形图标集上的图标角度
 * @param[in] rings     : 环形组件指针
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 角度
 **/
int32_t compo_rings_loop_icon_find_angle(compo_rings_t *rings, int id)
{
    if (id > RINGS_MAX_ICON_CNT) return -1;
    compo_rings_icon_t *icon = &rings->icon[id];
    if (icon->widget) return icon->angle;
    return -2;
}

/**
 * @brief 查找环形图标集上的图标任务编号
 * @param[in] rings     : 环形组件指针
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 任务编号
 **/
u8 compo_rings_loop_icon_find_idx(compo_rings_t *rings, u8 id)
{
    if (id > RINGS_MAX_ICON_CNT) return 0xFF;
    compo_rings_icon_t *icon = &rings->icon[id];
    if (icon->widget) return icon->idx;
    return 0xFF;
}

/**
 * @brief 查找环形图标集上的图标尺寸
 * @param[in] rings     : 环形组件指针
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 图标尺寸
 **/
u8 compo_rings_loop_icon_find_size(compo_rings_t *rings, u8 id)
{
    if (id > RINGS_MAX_ICON_CNT) return 0xFF;
    compo_rings_icon_t *icon = &rings->icon[id];
    if (icon->widget) return icon->size;
    return 0xFF;
}

/**
 * @brief 设置环形图标集上的图标偏移角度
 * @param[in] rings     : 环形组件指针
 * @param[in] diff_angle: 差量角度
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_loop_icon_set_angle_diff(compo_rings_t *rings, int32_t diff_angle, int id)
{
    if (id > RINGS_MAX_ICON_CNT) return;
    compo_rings_icon_t *icon = &rings->icon[id];
    if (icon->widget) {
        icon->angle += diff_angle;
        icon->angle = RINGS_GET_ANGLE(icon->angle);
    }
}

/**
 * @brief 设置环形图标集上的图标绝对角度
 * @param[in] rings     : 环形组件指针
 * @param[in] angle     : 绝对角度
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_set_angle(compo_rings_t *rings, int32_t angle, int id)
{
    if (id > RINGS_MAX_ICON_CNT) return;
    compo_rings_icon_t *icon = &rings->icon[id];
    if (icon->widget) icon->angle = angle;
}

/**
 * @brief 设置环形图标集上的图标尺寸
 * @param[in] rings     : 环形组件指针
 * @param[in] size      : 尺寸
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_set_size(compo_rings_t *rings, uint16_t size, int id)
{
    if (id > RINGS_MAX_ICON_CNT) return;
    compo_rings_icon_t *icon = &rings->icon[id];
    if (icon->widget) icon->size = size;
}

/**
 * @brief 设置环形图标集上的图标任务编号
 * @param[in] rings     : 环形组件指针
 * @param[in] idx       : 任务编号
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_set_uidx(compo_rings_t *rings, u8 idx, int id)
{
    if (id > RINGS_MAX_ICON_CNT) return;
    compo_rings_icon_t *icon = &rings->icon[id];
    if (icon->widget) icon->idx = idx;
}

/**
 * @brief 设置环形图标集上的图标资源地址
 * @param[in] rings     : 环形组件指针
 * @param[in] res_addr  : 资源地址
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_set_res(compo_rings_t *rings, u32 res_addr, int id)
{
    if (id > RINGS_MAX_ICON_CNT) return;
    compo_rings_icon_t *icon = &rings->icon[id];
    if (icon->widget) widget_icon_set(icon->widget, res_addr);
}

/**
 * @brief 设置环形图标集上的图标任务编号
 * @param[in] rings     : 环形组件指针
 * @param[in] uidx      : 变更图标任务编号
 * @param[in] idx       : 原始图标任务编号
 * @return 无
 **/
void compo_rings_set_uidx_from_idx(compo_rings_t *rings, int uidx, int idx)
{
    for (int i = 0; i < RINGS_MAX_ICON_CNT; i++) {
        compo_rings_icon_t *icon = &rings->icon[i];
        if (icon->idx == idx) {
            if (icon->widget) {
                icon->idx = uidx;
                break;
            }
        }
    }
}

/**
 * @brief 设置环形图标集上的图标资源地址
 * @param[in] rings     : 环形组件指针
 * @param[in] res_addr  : 资源地址
 * @param[in] idx       : 图标任务编号
 * @return 无
 **/
void compo_rings_set_res_from_idx(compo_rings_t *rings, u32 res_addr, int idx)
{
    for (int i = 0; i < RINGS_MAX_ICON_CNT; i++) {
        compo_rings_icon_t *icon = &rings->icon[i];
        if (icon->idx == idx) {
            if (icon->widget) {
                widget_icon_set(icon->widget, res_addr);
                break;
            }
        }
    }
}

/**
 * @brief 设置环形图标集上的图标位置信息
 * @param[in] rings    : 环形组件指针
 * @param[in] x        : X
 * @param[in] y        : Y
 * @param[in] width    : W
 * @param[in] height   : H
 * @param[in] idx      : 图标任务编号
 * @return 无
 **/
compo_rings_icon_t *compo_rings_set_location(compo_rings_t *rings, s16 x, s16 y, s16 width, s16 height, int idx)
{
    for (int i = 0; i < RINGS_MAX_ICON_CNT; i++) {
        compo_rings_icon_t *icon = &rings->icon[i];
        if (icon->widget) {
            if (icon->idx == idx) {
                widget_set_visible(icon->widget, true);
                widget_set_location(icon->widget, x, y, width, height);
                return icon;
            }
        }
    }
    return NULL;
}

/**
 * @brief 更新环形图标集合
 * 注：仅更新在环形上的，非环形上的图标请单独更新
 * @param[in] rings : 图标集指针
 **/
void compo_rings_update_loop(compo_rings_t *rings)
{
    if (rings == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_UPDATE);
    }
    compo_rings_page_time0_vis(rings, false);
    for (int i = 0; i < rings->ele.loop_icon_num; i++) {
        compo_rings_icon_t *icon = &rings->icon[i];
        if (icon->widget) {
            int ix = rings->ele.x + RINGS_GET_ICON_REL_X(rings->ele.r, icon->angle);
            int iy = rings->ele.y + RINGS_GET_ICON_REL_Y(rings->ele.r, icon->angle);
            widget_set_visible(icon->widget, true);
            widget_set_location(icon->widget, ix, iy, icon->size, icon->size);
            compo_rings_page_time0_bg(rings, icon->widget, icon->idx);
            compo_rings_page_time0_update(rings, ix, iy, icon->size, icon->idx);
        }
    }
}

/**
 * @brief 按坐标选择图标
 * @param[in] rings : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @return 返回图标索引
 **/
compo_rings_sel_t *compo_rings_select(compo_rings_t *rings, int x, int y)
{
    if (rings == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SELECT);
    }

    for (int i = 0; i < RINGS_MAX_ICON_CNT; i++) {
        compo_rings_icon_t *icon = &rings->icon[i];
        if (icon->widget) {
            rect_t rect = widget_get_absolute(icon->widget);
            if (abs_s(x - rect.x) * 2 <= rect.wid && abs_s(y - rect.y) * 2 <= rect.hei) {
                rings->sel.icon  = icon->widget;
                rings->sel.idx   = icon->idx;
                return &rings->sel;
            }
        }
    }

    rings->sel.icon  = NULL;
    rings->sel.idx   = -1;
    return &rings->sel;
}

/**
 * @brief 按索引选择图标
 * @param[in] rings : 图标集指针
 * @param[in] idx : 编号索引
 * @return 返回图标指针
 **/
widget_icon_t *compo_rings_select_byidx(compo_rings_t *rings, int idx)
{
    for (int i = 0; i < RINGS_MAX_ICON_CNT; i++) {
        compo_rings_icon_t *icon = &rings->icon[i];
        if (icon->widget) {
            if (icon->idx == idx) {
                rings->sel.icon = icon->widget;
                rings->sel.idx  = idx;
                return rings->sel.icon;
            }
        }
    }
    rings->sel.icon = NULL;
    rings->sel.idx = -1;
    return NULL;
}

/**
 * @brief 向集合中添加时分秒针
          cx cy为旋转中心点, x y为目标坐标
 * @param[in] rings : 图标集指针
 * @param[in] type : COMPO_RINGS_TIME_TYPE_HOUR, 时
                     COMPO_RINGS_TIME_TYPE_MIN,  分
                     COMPO_RINGS_TIME_TYPE_SEC,  秒
 * @param[in] res_addr : 图像资源地址
 * @param[in] cx : x轴坐标
 * @param[in] cy : y轴坐标
 **/
void compo_rings_add_time(compo_rings_t *rings, u8 type, u32 res_addr, s16 cx, s16 cy, s16 iw)
{
    if (rings->time0.page_time == NULL) {
        rings->time0.page_time = widget_page_create(rings->page);
        widget_set_location(rings->time0.page_time, 0, 0, iw, iw);
        widget_page_set_client(rings->time0.page_time, iw >> 1, iw >> 1);
    }
    if (rings->time1.page_time == NULL) {
        rings->time1.page_time = widget_page_create(rings->page);
        widget_set_location(rings->time1.page_time, 0, 0, iw, iw);
        widget_page_set_client(rings->time1.page_time, iw >> 1, iw >> 1);
    }
    widget_image_t *img0 = widget_image_create(rings->time0.page_time, res_addr);
 #if BOX_GUI_ROTATE_DISP
    rotate_widget_image_set_rotation_center(img0, cx, cy);
#else
    widget_image_set_rotation_center(img0, cx, cy);
#endif // BOX_GUI_ROTATE_DISP
    widget_image_t *img1 = widget_image_create(rings->time1.page_time, res_addr);
#if BOX_GUI_ROTATE_DISP
    rotate_widget_image_set_rotation_center(img1, cx, cy);
#else
    widget_image_set_rotation_center(img1, cx, cy);
#endif // BOX_GUI_ROTATE_DISP
    switch (type) {
    case COMPO_RINGS_TIME_TYPE_HOUR:
        rings->time0.hour = img0;
        rings->time1.hour = img1;
        break;

    case COMPO_RINGS_TIME_TYPE_MIN:
        rings->time0.min = img0;
        rings->time1.min = img1;
        break;

    case COMPO_RINGS_TIME_TYPE_SEC:
        rings->time0.sec = img0;
        rings->time1.sec = img1;
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_TIME_TYPE);
        return;
    }
    compo_cb.rtc_update = true;                 //创建时钟组件，需要同步更新时间
}

/**
 * @brief 设置集合时间对应的图标编号
 * @param[in] rings : 图标集指针
 * @param[in] idx : 图标编号
 **/
void compo_rings_idx_time_set(compo_rings_t *rings, u8 idx)
{
    if (rings == NULL) {
        halt(HALT_GUI_COMPO_ICONLIST_TIME_IDX);
    }
    rings->time0.idx_time = idx;
    rings->time1.idx_time = idx;
}

/**
 * @brief 设置图标集指针的起始角度
 * @param[in] rings : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_rings_set_start_angle(compo_rings_t *rings, s16 angle)
{
    rings->time0.start_angle = angle;
    rings->time1.start_angle = angle;
}

/**
 * @brief 设置图标集指针0的可视
 * @param[in] rings : 图标集指针
 * @param[in] visible : true 可视
 *                      false 不可视
 **/
void compo_rings_page_time0_vis(compo_rings_t *rings, bool visible)
{
    if (rings->time0.page_time != NULL) {
        widget_set_visible(rings->time0.page_time, visible);
    }
}

/**
 * @brief 设置图标集指针1的可视
 * @param[in] rings : 图标集指针
 * @param[in] visible : true 可视
 *                      false 不可视
 **/
void compo_rings_page_time1_vis(compo_rings_t *rings, bool visible)
{
    if (rings->time1.page_time != NULL) {
        widget_set_visible(rings->time1.page_time, visible);
    }
}

/**
 * @brief 设置图标集指针0的背景
 * @param[in] rings : 图标集指针
 * @param[in] widget : 背景图标控件指针
 * @param[in] idx ： 图标任务编号
 **/
void compo_rings_page_time0_bg(compo_rings_t *rings, widget_t *widget, u8 idx)
{
    if (widget != NULL && idx == rings->time0.idx_time) {
        rings->time0.time_bg = widget;
    }
}

/**
 * @brief 设置图标集指针1的背景
 * @param[in] rings : 图标集指针
 * @param[in] widget : 背景图标控件指针
 * @param[in] idx ： 图标任务编号
 **/
void compo_rings_page_time1_bg(compo_rings_t *rings, widget_t *widget, u8 idx)
{
    if (widget != NULL && idx == rings->time1.idx_time) {
        rings->time1.time_bg = widget;
    }
}

/**
 * @brief 更新集合指针0
                ix iy为目标坐标
 * @param[in] rings : 图标集指针
 * @param[in] ix : x轴坐标
 * @param[in] iy : y轴坐标
 * @param[in] iw : 尺寸
 * @param[in] idx ： 图标任务编号
 **/
void compo_rings_page_time0_update(compo_rings_t *rings, s16 ix, s16 iy, s16 iw, u8 idx)
{
    if (rings->time0.page_time != NULL && idx == rings->time0.idx_time) {
        compo_rings_page_time0_vis(rings, true);
        widget_page_scale_to(rings->time0.page_time, iw, iw);
        widget_set_pos(rings->time0.page_time, ix, iy);
    }
}

/**
 * @brief 更新集合指针1
                ix iy为目标坐标
 * @param[in] rings : 图标集指针
 * @param[in] ix : x轴坐标
 * @param[in] iy : y轴坐标
 * @param[in] iw : 尺寸
 * @param[in] idx ： 图标任务编号
 **/
void compo_rings_page_time1_update(compo_rings_t *rings, s16 ix, s16 iy, s16 iw, u8 idx)
{
    if (rings->time1.page_time != NULL && idx == rings->time1.idx_time) {
        compo_rings_page_time1_vis(rings, true);
        widget_page_scale_to(rings->time1.page_time, iw, iw);
        widget_set_pos(rings->time1.page_time, ix, iy);
    }
}
