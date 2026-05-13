#ifndef _COMPO_ROTARY_H
#define _COMPO_ROTARY_H

#define ROTARY_ITEM_CNT                     7                   //转盘个数
#define ROTARY_ITEM_ANGLE                   ((3600 + ROTARY_ITEM_CNT - 1) / ROTARY_ITEM_CNT)

//转盘移动控制命令
enum COMPO_ROTARY_MOVE_CMD {
    COMPO_ROTARY_MOVE_CMD_NONE,
    COMPO_ROTARY_MOVE_CMD_ENTERING,             //入场动画
    COMPO_ROTARY_MOVE_CMD_EXITING,              //出场动画
    COMPO_ROTARY_MOVE_CMD_DRAG,                 //开始拖动
    COMPO_ROTARY_MOVE_CMD_FORWARD,              //向前滚动
    COMPO_ROTARY_MOVE_CMD_BACKWARD,             //向后滚动
};

//转盘当前状态
enum COMPO_ROTARY_STA {
    COMPO_ROTARY_STA_IDLE,                      //空闲状态
    COMPO_ROTARY_STA_ENTERING,                  //入场中
    COMPO_ROTARY_STA_EXITING,                   //出场中
    COMPO_ROTARY_STA_DARG,                      //拖动中
    COMPO_ROTARY_STA_MOVE,                      //移动中
    COMPO_ROTARY_STA_EXIT,                      //已出场
};

//转盘项定义
typedef struct compo_rotary_item_t_ {
    u32 res_addr;               //图标
    u32 str_idx;                //文字
} compo_rotary_item_t;

//列表移动控制
typedef struct compo_rotary_move_cb_t_ {
    u32 tick;
    s32 focus_a;
    int moveto_idx;                     //设定自动移到的目标菜单
    s32 moveto_a;                       //设定自动移到的转盘角度
    bool flag_drag;                     //开始拖动
    bool flag_move_auto;                //自动移到坐标
    u8 sta;
    u8 animation_cnt;
} compo_rotary_move_cb_t;

typedef struct compo_rotary_t_ {
    COMPO_STRUCT_COMMON;
    compo_rotary_move_cb_t move_cb;     //移动和拖动处理
    widget_page_t *page;
    widget_axis3d_t *axis;              //轴

    s32 total_angle;                    //转盘项目总角度
    s32 angle;                          //转盘旋转角度
    area_t img_area;                    //转盘图片大小
    u8 item_cnt;                        //转盘图片数量
    u16 radius;

    compo_rotary_item_t const *item;
    widget_image_t *item_img[ROTARY_ITEM_CNT];
    widget_text_t *item_title;
} compo_rotary_t;

/**
 * @brief 创建一个转盘组件
 * @param[in] frm : 窗体指针
 * @param[in] item : 存放转盘相关的信息，图标等
 * @param[in] item_cnt : 存放转盘中信息的数量
 * @return 返回转盘指针
 **/
compo_rotary_t *compo_rotary_create(compo_form_t *frm, compo_rotary_item_t const *item, int item_cnt, u16 radius);

/**
 * @brief 更新转盘Widget
 * @param[in] rotary : 转盘指针
 **/
void compo_rotary_update(compo_rotary_t *rotary);

/**
 * @brief 设置转盘的旋转角度
 * @param[in] rotary : 转盘指针
 * @param[in] angle : 旋转角度
 **/
s32 compo_rotary_set_rotation(compo_rotary_t *rotary, s32 angle);

/**
 * @brief 设置转盘的转盘半径
 * @param[in] rotary : 转盘指针
 * @param[in] radius : 转盘半径
 **/
void compo_rotary_set_radius(compo_rotary_t *rotary, u16 radius);

/**
 * @brief 根据idx设置转盘的旋转角度
 * @param[in] rotary : 转盘指针
 * @param[in] idx : 编号
 **/
s32 compo_rotary_set_rotation_byidx(compo_rotary_t *rotary, int idx);

/**
 * @brief 获取当前idx
 * @param[in] rotary : 转盘指针
 * @return 返回当前idx
 **/
int compo_rotary_get_idx(compo_rotary_t *rotary);

/**
 * @brief 转盘拖动与移动初始化
 * @param[in] rotary : 转盘指针
 * @return 返回COMPO_ROTARY_STA状态枚举
 **/
u8 compo_rotary_get_sta(compo_rotary_t *rotary);

/**
 * @brief 转盘拖动与移动处理
 * @param[in] rotary : 列表指针
 **/
void compo_rotary_move(compo_rotary_t *rotary);

/**
 * @brief 转盘拖动与移动控制
 * @param[in] rotary : 转盘指针
 * @param[in] cmd : 控制命令
 **/
void compo_rotary_move_control(compo_rotary_t *rotary, int cmd);

#endif
