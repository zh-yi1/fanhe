#ifndef _COMPO_BUTTERFLY_H
#define _COMPO_BUTTERFLY_H

//蝴蝶移动控制命令
enum COMPO_BUTTERFLY_MOVE_CMD {
    COMPO_BUTTERFLY_MOVE_CMD_NONE,
    COMPO_BUTTERFLY_MOVE_CMD_DRAG,                 //开始拖动
    COMPO_BUTTERFLY_MOVE_CMD_LEFT,                //向左飞
    COMPO_BUTTERFLY_MOVE_CMD_LOOP,                //来回飞
    COMPO_BUTTERFLY_MOVE_CMD_RIGHT,               //向右飞
};

//蝴蝶当前状态
enum COMPO_BUTTERFLY_STA {
    COMPO_BUTTERFLY_STA_IDLE,                        //空闲状态
    COMPO_BUTTERFLY_STA_DARG,                        //拖动中
    COMPO_BUTTERFLY_STA_MOVE,                        //移动中
};

//蝴蝶项定义
typedef struct compo_butterfly_res_t_ {
    u32 res_body;                       //蝴蝶身体
    u32 res_wing_up;                    //蝴蝶右上翅
    u32 res_wing_down;                  //蝴蝶右下翅
} compo_butterfly_res_t;

//蝴蝶移动控制
typedef struct compo_butterfly_move_cb_t_ {
    u32 tick;
    s32 focus_a;                        //当前自转角
    s32 focus_i;                        //当前倾斜角
    s32 moveto_a;                       //设定自动移到的转盘角度
    s16 moveto_z;                       //设定Z坐标
    s16 focus_z;                        //当前自转角
    bool flag_drag;                     //开始拖动
    bool flag_move_auto;                //自动移到坐标
    u8 flag_move_z_auto;                //自动移到坐标
    u8 sta;
    u8 animation_cnt;
} compo_butterfly_move_cb_t;

typedef struct compo_butterfly_t_ {
    COMPO_STRUCT_COMMON;
    compo_butterfly_move_cb_t move_cb;   //移动和拖动处理
    widget_page_t *page;

    sph_t sph;                          //蝴蝶的球坐标
    s16 wing_up_angle;                  //翅膀角度
    s16 wing_down_angle;
    s16 rotation;
    s16 z;
    s16 overlook;
    bool wing_dir;                      //翅膀扇动方向
    bool flag_need_update;              //是否需要更新

    widget_axis3d_t *axis;              //蝴蝶轴
    widget_image3d_t *img_body;
    widget_image3d_t *img_wing_ul;
    widget_image3d_t *img_wing_ur;
    widget_image3d_t *img_wing_dl;
    widget_image3d_t *img_wing_dr;
    widget_icon_t *img_light_ul;
    widget_icon_t *img_light_ur;
    widget_icon_t *img_light_dl;
    widget_icon_t *img_light_dr;
} compo_butterfly_t;

/**
 * @brief 创建一个蝴蝶组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图像资源地址
 * @return 返回图像指针
 **/
compo_butterfly_t *compo_butterfly_create(compo_form_t *frm, compo_butterfly_res_t const *res);

/**
 * @brief 更新蝴蝶Widget
 * @param[in] ball : 蝴蝶指针
 **/
void compo_butterfly_update(compo_butterfly_t *ball);

/**
 * @brief 设置蝴蝶的旋转角度
 * @param[in] ball : 蝴蝶指针
 * @param[in] angle : 旋转角度
 **/
s32 compo_butterfly_set_rotation(compo_butterfly_t *ball, s32 angle);

/**
 * @brief 扇动蝴蝶的翅膀
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 翅膀角度
 **/
void compo_butterfly_flap(compo_butterfly_t *btfly, s16 angle);

/**
 * @brief 蝴蝶拖动与移动初始化
 * @param[in] ball : 蝴蝶指针
 * @return 返回COMPO_BUTTERFLY_STA状态枚举
 **/
u8 compo_butterfly_get_sta(compo_butterfly_t *ball);

/**
 * @brief 蝴蝶拖动与移动处理
 * @param[in] ball : 蝴蝶指针
 **/
void compo_butterfly_move(compo_butterfly_t *ball);

/**
 * @brief 蝴蝶拖动与移动控制
 * @param[in] ball : 蝴蝶指针
 * @param[in] cmd : 控制命令
 **/
void compo_butterfly_move_control(compo_butterfly_t *ball, int cmd);

/**
 * @brief 设置蝴蝶的旋转角度
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 旋转角度
 **/
s32 compo_butterfly_get_rotation(compo_butterfly_t *btfly);

/**
 * @brief 设置蝴蝶的坐标及大小
          注意：蝴蝶设置的坐标是以默认的中心点作为参考
 * @param[in] butterfly : 蝴蝶指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 文本框宽度
 * @param[in] height : 文本框高度
 **/
void compo_butterfly_set_location(compo_butterfly_t *btfly, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 设置蝴蝶的坐标
          注意：蝴蝶设置的坐标是以默认的中心点作为参考
 * @param[in] label : 蝴蝶指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_butterfly_set_pos(compo_butterfly_t *btfly, s16 x, s16 y);

/**
 * @brief 获取蝴蝶的球Z坐标
 * @param[in] btfly : 蝴蝶指针
 * @param[in] angle : 旋转角度
 **/
s16 compo_butterfly_get_z(compo_butterfly_t *btfly);

/**
 * @brief 设置是否蝴蝶的置顶，主要用于上下拉界面
 * @param[in] btfly : 蝴蝶指针
 * @param[in] top_enable : 是否允许置顶
 **/
bool compo_butterfly_set_top_enable(compo_butterfly_t *btfly, bool top_enable);
#endif
