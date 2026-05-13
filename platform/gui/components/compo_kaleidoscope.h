#ifndef _COMPO_KALEIDOSCOPE_H
#define _COMPO_KALEIDOSCOPE_H

#define KALE_FUNC_LAYER                          4                                                       //任务层数 一层 KALE_CIR_ICON_NUM 个任务
#define KALE_CIR_ICON_NUM                        8                                                       //一圈图标个数
#define KALE_CIR_ANGLE_DEV_OI                    450                                                     //图标角度偏差
#define KALE_CIR_ANGLE_DEV                       225                                                     //内外圈角度偏差
#define KALE_CIR_ANGLE_COM                       (KALE_CIR_ANGLE_DEV_OI - KALE_CIR_ANGLE_DEV)            //互补角

enum KALE_CIR_IDX{
    KALE_CIR_0,
    KALE_CIR_1,
    KALE_CIR_MAX,
};

enum KALE_CTL {
    KALE_CTL_NULL,
    KALE_CTL_SWITCH,
    KALE_CTL_ROTATE,
    KALE_CTL_SWAP,
};

enum KALE_SWAP {
    KALE_SWAP_NULL,
    KALE_SWAP_LAST,
    KALE_SWAP_NEXT,
};

enum KALE_FUNC_LAY {
    KALE_FUNC_LAY_0,
    KALE_FUNC_LAY_1,
    KALE_FUNC_LAY_2,
    KALE_FUNC_LAY_MAX,
};

typedef struct  compo_kale_axis_t_ {
    int16_t x;
    int16_t y;
}compo_kale_axis_t;

typedef struct  compo_kale_func_t_ {
    widget_icon_t *widget;
    u8 func_sta;
}compo_kale_func_t;

typedef struct  compo_kale_icon_t_ {
    compo_kale_func_t fsub;
    int16_t angle;
}compo_kale_icon_t;

typedef struct  compo_kale_cir_t_ {
    compo_kale_icon_t icon[KALE_CIR_ICON_NUM];
    union {
      struct {
        u32 r       : 10;
        u32 size    : 10;
        u32 lay     : 2;
      };
    };
}compo_kale_cir_t;

typedef struct  compo_kale_org_func_t_ {
    compo_kale_func_t fsub[KALE_CIR_ICON_NUM];
}compo_kale_org_func_t;

typedef struct  compo_kale_ctl_t_ {
    union {
        struct {
            u32 adm         : 1;            //入场动画
            u32 sta         : 3;            //状态
            u32 swap        : 3;            //切换
            u32 oidx        : 2;            //当前外圈编号
            u32 iidx        : 2;            //当前内圈编号
            u32 org_or      : 10;           //初始外圈半径
            u32 org_ir      : 10;           //初始内圈半径
            u32 osize       : 10;           //外圈图标尺寸
            u32 isize       : 10;           //内圈图标尺寸
        };
    };
}compo_kale_ctl_t;

typedef struct  compo_kaleidoscope_t_ {
    COMPO_STRUCT_COMMON;
    widget_page_t *page;
    widget_icon_t *sel_icon;                            //选择的ICON
    s16 sel_idx;                                        //选择的idx
    u8 total_cnt;                                       //图标个数
    u8 focus_icon_idx;                                  //焦点的图标编号
    compo_kale_axis_t cen;                              //中心点
    compo_kale_ctl_t ctl;                               //切换控制
    compo_kale_cir_t dcir[KALE_CIR_MAX];                //显示图标集合
    compo_kale_org_func_t ofunc[KALE_FUNC_LAYER];       //任务图标集合

	u8 idx_time;				//加入时分秒针元素
	widget_page_t *page_time;
	widget_icon_t *time_bg;
	widget_image_t *hour;
	widget_image_t *min;
	widget_image_t *sec;
	s16 start_angle;
}compo_kaleidoscope_t;

typedef struct menu_kale_item_t_ {
    u32 res_addr;
    u8 func_sta;
} menu_kale_item_t;

/**
 * @brief 变量范围限制
 * 注：超出最小最大范围 返回最小或最大值
 * @param[in] val : 变量
 * @param[in] min : 最小值
 * @param[in] max : 最大值
 * @return 当前值或最大最小值
 **/
int limit_range(int val, int min, int max);

/**
 * @brief 变量环形递增
 * @param[in] val : 变量地址
 * @param[in] min : 最小值
 * @param[in] max : 最大值
 * @return 递增值
 **/
void cycle_plus(u8 *val, u8 min, u8 max);

/**
 * @brief 变量环形递减
 * @param[in] val : 变量地址
 * @param[in] min : 最小值
 * @param[in] max : 最大值
 * @return 递减值
 **/
void cycle_sub(u8 *val, u8 min, u8 max);

/**
 * @brief 创建一个万花筒组件
 * @param[in] frm : 窗体指针
 * @return 返回万花筒图标集合指针
 **/
compo_kaleidoscope_t *compo_kaleidoscope_create(compo_form_t *frm);

/**
 * @brief 万花筒图标角度初始化
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_angle_init(compo_kaleidoscope_t *kale);

/**
 * @brief 初始化万花筒内外圈层
 * 注：根据屏幕大小自适应内外圈层半径
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] dc_width : 屏幕宽度
 * @param[in] dc_height : 屏幕高度
 * @param[in] osize : 外圈图标尺寸
 * @param[in] isize : 内圈图标尺寸
 * @param[in] edge_space : 离屏幕边缘距离
 * @param[in] icon_space : 内外圈图标间距离
 **/
void compo_kaleidoscope_init(compo_kaleidoscope_t *kale, s16 dc_width, s16 dc_height, u16 osize, u16 isize, s16 edge_space, s16 icon_space);

/**
 * @brief 往万花筒图标集合里面添加图标
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] item : 图标集指针
 * @param[in] item_cnt : 图标集总数
 **/
void compo_kaleidoscope_icon_add(compo_kaleidoscope_t *kale, menu_kale_item_t const *item, int item_cnt);

/**
 * @brief 根据变化半径，万花筒内外圈半径更新
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] dr : 变化半径
 **/
void compo_kaleidoscope_update_r(compo_kaleidoscope_t *kale, s16 dr);

/**
 * @brief 根据内外圈半径，获取当前圈层的图标尺寸
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] r : 内外圈半径
 * @return 返回当前圈层图标尺寸
 **/
u16 compo_kaleidoscope_size_from_r(compo_kaleidoscope_t *kale, u16 r);

/**
 * @brief 万花筒图标尺寸更新
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_update_size(compo_kaleidoscope_t *kale);

/**
 * @brief 根据变化角度，万花筒旋转角度更新
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] dr : 变化半径
 **/
void compo_kaleidoscope_update_angle(compo_kaleidoscope_t *kale, int16_t dangle);

/**
 * @brief 万花筒上下层任务切换设置
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] msg : 消息
 **/
void compo_kaleidoscope_ctl_swap_set(compo_kaleidoscope_t *kale, size_msg_t msg);

/**
 * @brief 万花筒上下层任务信息更替
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] swap : 上下切换
 * @return  true  切换成功
 *          false 切换失败
 **/
bool compo_kaleidoscope_ctl_swap_pro(compo_kaleidoscope_t *kale, u8 swap);

/**
 * @brief 万花筒上下层任务切换处理
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] sta : 切换状态
 **/
void compo_kaleidoscope_ctl_switch_pro(compo_kaleidoscope_t *kale, u8 sta);

/**
 * @brief 万花筒上下层任务切换处KICK
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] msg : 消息
 **/
void compo_kaleidoscope_ctl_switch_kick(compo_kaleidoscope_t *kale, size_msg_t msg);

/**
 * @brief 万花筒入场动画KICK
 * 注：从表盘页面进入万花筒，使能入场动画
 * 注：从子任务返回不具备入场动画，并返回子任务所在的层级
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_adm_animation_kick(compo_kaleidoscope_t *kale);

/**
 * @brief 万花筒入场动画
 * 注：从表盘页面进入万花筒，使能入场动画
 * 注：从子任务返回不具备入场动画，并返回子任务所在的层级
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_adm_animation_pro(compo_kaleidoscope_t *kale);

/**
 * @brief 万花筒图标可视化设置
 * 注：显示内外圈层所在任务层图标
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_set_vis(compo_kaleidoscope_t *kale);

/**
 * @brief 万花筒切换动画
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] expire : 刷新间隔
 * @param[in] dr : 相对半径
 * @param[in] dangle : 旋转角度
 **/
void compo_kaleidoscope_switch_animation(compo_kaleidoscope_t *kale, u32 expire, s16 dr, int16_t dangle);

/**
 * @brief 更新图标集合Widget
 * @param[in] kale : 图标集指针
 **/
void compo_kale_update(compo_kaleidoscope_t *kale);

/**
 * @brief 按坐标选择图标
 * @param[in] kale : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @return 返回图标索引
 **/
int compo_kale_select(compo_kaleidoscope_t *kale, int x, int y);

/**
 * @brief 按索引选择图标
 * @param[in] kale : 图标集指针
 * @param[in] idx : 编号索引
 * @return 返回图标指针
 **/
widget_icon_t *compo_kale_select_byidx(compo_kaleidoscope_t *kale, int idx);

/**
 * @brief 按索引设置图标集合焦点坐标 并设置当前任务圈层
 * @param[in] kale : 图标集指针
 * @param[in] idx : 编号索引
 **/
void compo_kale_set_focus_byidx(compo_kaleidoscope_t *kale, int idx);

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
void compo_kale_add_time(compo_kaleidoscope_t *kale, u8 type, u32 res_addr, s16 cx, s16 cy, s16 size);

/**
 * @brief 设置图标集指针的起始角度
 * @param[in] iconlist : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_kale_set_start_angle(compo_kaleidoscope_t *kale, s16 angle);

#endif