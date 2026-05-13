
#ifndef _COMPO_RINGS_H
#define _COMPO_RINGS_H

#define RINGS_MAX_ICON_CNT                        16                                                         //环形组件最多挂载图标个数
#define RINGS_GET_ANGLE(angle)                    ((angle >= 0) ? (angle % 3600) : (angle % 3600 + 3600))    //度数转换
#define RINGS_GET_ICON_REL_X(r, angle)            (muls_shift16(r, SIN(RINGS_GET_ANGLE(angle))))             //根据角度获取相对X 
#define RINGS_GET_ICON_REL_Y(r, angle)            (-muls_shift16(r, COS(RINGS_GET_ANGLE(angle))))            //根据角度获取相对Y

enum COMPO_RINGS_QUADRANT {
    QUADRANT_NULL,
    QUADRANT_1,
    QUADRANT_2,
    QUADRANT_3,
    QUADRANT_4,
};

enum COMPO_RINGS_TIME_TYPE {
    COMPO_RINGS_TIME_TYPE_HOUR,
    COMPO_RINGS_TIME_TYPE_MIN,
    COMPO_RINGS_TIME_TYPE_SEC,
};

typedef struct compo_rings_item_t_ {
    u32 res_addr;                   //图标
    u8  func_sta;                   //任务序号
} compo_rings_item_t;

typedef struct compo_rings_ele_t_ {
    int16_t  x;                                         //环形中心点X坐标
    int16_t  y;                                         //环形中心点Y坐标
    uint16_t r;                                         //环形半径
    int16_t  div;                                       //图标角度间隔
    int32_t  angle;                                     //起始角度
    uint8_t  loop_icon_num;                             //环形图标数     loop_icon_num + other_icon_num <= RINGS_MAX_ICON_CNT
    uint8_t  other_icon_num;                            //其他图标数     loop_icon_num + other_icon_num <= RINGS_MAX_ICON_CNT
    uint8_t  item_num;                                  //资源总数
    compo_rings_item_t const *item;                     //资源地址及任务序号
}compo_rings_ele_t;

typedef struct compo_rings_icon_t_ {
    widget_icon_t *widget;
    uint16_t size;
    int32_t angle;
    u8 idx;
}compo_rings_icon_t;

typedef struct compo_rings_touch_t_ {
    int32_t angle;
    u8 quad;
}compo_rings_touch_t;

typedef struct compo_rings_sel_t_ {
    widget_icon_t *icon;                                //选择的ICON
    s16 idx;                                            //选择的IDX
}compo_rings_sel_t;

typedef struct compo_rings_time_t_ {
    u8 idx_time;                //加入时分秒针元素
    widget_page_t *page_time;
    widget_icon_t *time_bg;
    widget_image_t *hour;
    widget_image_t *min;
    widget_image_t *sec;
    s16 start_angle;
}compo_rings_time_t;

typedef struct compo_rings_t_ {
    COMPO_STRUCT_COMMON;
    widget_page_t *page;
    u8 loop_icon_cnt;                                   //环形图标个数
    u8 other_icon_cnt;                                  //其他图标个数
    compo_rings_sel_t sel;
    compo_rings_ele_t ele;
    compo_rings_icon_t icon[RINGS_MAX_ICON_CNT];
    compo_rings_touch_t pre;
    compo_rings_touch_t cur;
    compo_rings_time_t  time0;
    compo_rings_time_t  time1;
}compo_rings_t;

#define COS_SCALE     10000
#define ACOS(value)   myacos(value)

/**
 * @brief 二分法查找最接近的余弦值
 * 注：放大系数：10000
 * @param[in] value : 余弦值 * 放大系数
 * @return 返回对应的弧度 精度为1° 0~180°
 **/
int binarysearch(int value);

/**
 * @brief acos函数
 * 注：放大系数：10000
 * @param[in] value : 余弦值 * 放大系数
 * @return 返回对应的弧度 精度为1° 0~180°
 **/
int myacos(int value);

/**
 * @brief 弧度计算
 * 注：根据当前点和中心点确定余弦值并计算弧度值 精度为1°
 * @param[in] cx :中心点X 
 * @param[in] cy :中心点Y
 * @param[in] x  :当前点X 
 * @param[in] y  :当前点Y
 * @return 返回对应的弧度 精度为1° 0~180°
 **/
int myrad(int cx, int cy, int x, int y);

/**
 * @brief 创建一个环形组件
 * @param[in] frm : 窗体指针
 * @return 返回环形组件指针
 **/
compo_rings_t *compo_rings_create(compo_form_t *frm);

/**
 * @brief 初始一个环形组件
 * @param[in] rings : 环形组件指针
 * @param[in] ele   : 初始元素指针
 * @return 无
 **/
void compo_rings_set(compo_rings_t *rings, compo_rings_ele_t *ele);

/**
 * @brief 往环形图标集上面添加图标
 * @param[in] rings     : 环形组件指针
 * @param[in] res_addr  : 图标资源地址
 * @param[in] idx       : 图标任务编号
 * @param[in] loop      : 是否为环形图标
 * @return 图标信息指针
 **/
compo_rings_icon_t *compo_rings_icon_add(compo_rings_t *rings, u32 res_addr, int idx, bool loop);

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
int32_t compo_rings_touch_angle(compo_rings_t *rings, compo_rings_touch_t *touch, s16 cx, s16 cy, s16 x, s16 y);

/**
 * @brief 设置滑动前的触摸信息
 * @param[in] rings     : 环形组件指针
 * @param[in] cx        : 中心点X
 * @param[in] cy        : 中心点Y
 * @param[in] x         : 当前点X
 * @param[in] y         : 当前点Y
 * @return 无
 **/
void compo_rings_touch_angle_pre(compo_rings_t *rings, s16 cx, s16 cy, s16 x, s16 y);

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
int32_t compo_rings_touch_angle_diff(compo_rings_t *rings, s16 cx, s16 cy, s16 x, s16 y);

/**
 * @brief 查找环形图标集上的图标角度
 * @param[in] rings     : 环形组件指针
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 角度
 **/
int32_t compo_rings_loop_icon_find_angle(compo_rings_t *rings, int id);

/**
 * @brief 查找环形图标集上的图标任务编号
 * @param[in] rings     : 环形组件指针
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 任务编号
 **/
u8 compo_rings_loop_icon_find_idx(compo_rings_t *rings, u8 id);

/**
 * @brief 查找环形图标集上的图标尺寸
 * @param[in] rings     : 环形组件指针
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 图标尺寸
 **/
u8 compo_rings_loop_icon_find_size(compo_rings_t *rings, u8 id);

/**
 * @brief 设置环形图标集上的图标偏移角度
 * @param[in] rings     : 环形组件指针
 * @param[in] diff_angle: 差量角度
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_loop_icon_set_angle_diff(compo_rings_t *rings, int32_t diff_angle, int id);

/**
 * @brief 设置环形图标集上的图标绝对角度
 * @param[in] rings     : 环形组件指针
 * @param[in] angle     : 绝对角度
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_set_angle(compo_rings_t *rings, int32_t angle, int id);

/**
 * @brief 设置环形图标集上的图标尺寸
 * @param[in] rings     : 环形组件指针
 * @param[in] size      : 尺寸
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_set_size(compo_rings_t *rings, uint16_t size, int id);

/**
 * @brief 设置环形图标集上的图标任务编号
 * @param[in] rings     : 环形组件指针
 * @param[in] idx       : 任务编号
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_set_uidx(compo_rings_t *rings, u8 idx, int id);

/**
 * @brief 设置环形图标集上的图标资源地址
 * @param[in] rings     : 环形组件指针
 * @param[in] res_addr  : 资源地址
 * @param[in] id        : compo_rings_icon_t icon[RINGS_MAX_ICON_CNT] 数组下标
 * @return 无
 **/
void compo_rings_set_res(compo_rings_t *rings, u32 res_addr, int id);

/**
 * @brief 设置环形图标集上的图标任务编号
 * @param[in] rings     : 环形组件指针
 * @param[in] uidx      : 变更图标任务编号
 * @param[in] idx       : 原始图标任务编号
 * @return 无
 **/
void compo_rings_set_uidx_from_idx(compo_rings_t *rings, int uidx, int idx);

/**
 * @brief 设置环形图标集上的图标资源地址
 * @param[in] rings     : 环形组件指针
 * @param[in] res_addr  : 资源地址
 * @param[in] idx       : 图标任务编号
 * @return 无
 **/
void compo_rings_set_res_from_idx(compo_rings_t *rings, u32 res_addr, int idx);

/**
 * @brief 设置环形图标集上的图标位置信息
 * @param[in] rings    : 环形组件指针
 * @param[in] x        : X
 * @param[in] y        : Y
 * @param[in] width    : W
 * @param[in] height   : H
 * @param[in] idx      : 图标任务编号
 * @return 返回当前设置的 location 的 compo_rings_icon_t 结构体
 **/
compo_rings_icon_t *compo_rings_set_location(compo_rings_t *rings, s16 x, s16 y, s16 width, s16 height, int idx);

/**
 * @brief 更新图标集合Widget
 * @param[in] rings : 图标集指针
 **/
void compo_rings_update_loop(compo_rings_t *rings);

/**
 * @brief 按坐标选择图标
 * @param[in] rings : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @return 返回图标索引
 **/
compo_rings_sel_t *compo_rings_select(compo_rings_t *rings, int x, int y);

/**
 * @brief 按索引选择图标
 * @param[in] rings : 图标集指针
 * @param[in] idx : 编号索引
 * @return 返回图标指针
 **/
widget_icon_t *compo_rings_select_byidx(compo_rings_t *rings, int idx);

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
void compo_rings_add_time(compo_rings_t *rings, u8 type, u32 res_addr, s16 cx, s16 cy, s16 iw);

/**
 * @brief 设置集合时间对应的图标编号
 * @param[in] rings : 图标集指针
 * @param[in] idx : 图标编号
 **/
void compo_rings_idx_time_set(compo_rings_t *rings, u8 idx);

/**
 * @brief 设置图标集指针的起始角度
 * @param[in] rings : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_rings_set_start_angle(compo_rings_t *rings, s16 angle);

/**
 * @brief 设置图标集指针0的可视
 * @param[in] rings : 图标集指针
 * @param[in] visible : true 可视
 *                      false 不可视
 **/
void compo_rings_page_time0_vis(compo_rings_t *rings, bool visible);

/**
 * @brief 设置图标集指针1的可视
 * @param[in] rings : 图标集指针
 * @param[in] visible : true 可视
 *                      false 不可视
 **/
void compo_rings_page_time1_vis(compo_rings_t *rings, bool visible);

/**
 * @brief 设置图标集指针0的背景
 * @param[in] rings : 图标集指针
 * @param[in] widget : 背景图标控件指针
 * @param[in] idx ： 图标任务编号
 **/
void compo_rings_page_time0_bg(compo_rings_t *rings, widget_t *widget, u8 idx);

/**
 * @brief 设置图标集指针1的背景
 * @param[in] rings : 图标集指针
 * @param[in] widget : 背景图标控件指针
 * @param[in] idx ： 图标任务编号
 **/
void compo_rings_page_time1_bg(compo_rings_t *rings, widget_t *widget, u8 idx);

/**
 * @brief 更新集合指针0
                ix iy为目标坐标
 * @param[in] rings : 图标集指针
 * @param[in] ix : x轴坐标
 * @param[in] iy : y轴坐标
 * @param[in] iw : 尺寸
 * @param[in] idx ： 图标任务编号
 **/
void compo_rings_page_time0_update(compo_rings_t *rings, s16 ix, s16 iy, s16 iw, u8 idx);

/**
 * @brief 更新集合指针1
                ix iy为目标坐标
 * @param[in] rings : 图标集指针
 * @param[in] ix : x轴坐标
 * @param[in] iy : y轴坐标
 * @param[in] iw : 尺寸
 * @param[in] idx ： 图标任务编号
 **/
void compo_rings_page_time1_update(compo_rings_t *rings, s16 ix, s16 iy, s16 iw, u8 idx);

#endif