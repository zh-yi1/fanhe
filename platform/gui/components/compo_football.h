#ifndef _COMPO_FOOTBALL_H
#define _COMPO_FOOTBALL_H

#define FOOTBALL_ITEM_CNT               20          //足球菜单项列表数量，必须是20项

//足球菜单移动控制命令
enum COMPO_FOOTBALL_MOVE_CMD {
    COMPO_FOOTBALL_MOVE_CMD_NONE,
    COMPO_FOOTBALL_MOVE_CMD_ENTER,
    COMPO_FOOTBALL_MOVE_CMD_DRAG,                   //开始拖动
    COMPO_FOOTBALL_MOVE_CMD_FORWARD,                //向前滚动
    COMPO_FOOTBALL_MOVE_CMD_BACKWARD,               //向后滚动
};

//足球菜单当前状态
enum COMPO_FOOTBALL_STA {
    COMPO_FOOTBALL_STA_IDLE,                        //空闲状态
    COMPO_FOOTBALL_STA_DARG,                        //拖动中
    COMPO_FOOTBALL_STA_MOVE,                        //移动中
};


//足球菜单项定义
typedef struct compo_football_item_t_ {
    u32 res_addr;               //图标
    u8 func_sta;                //功能ID
} compo_football_item_t;

//足球菜单移动控制
typedef struct compo_football_move_cb_t_ {
    u32 tick;
    sph_t focus_sph;                    //当前球坐标
    s32 start_a;                        //开始角度
    s32 moveto_a;                       //设定自动移到的角度
    s16 roll_azimuth;
    bool flag_drag;                     //开始拖动
    bool flag_move_auto;                //自动移到坐标
    u8 sta;
    u8 animation_cnt;
} compo_football_move_cb_t;

typedef struct compo_football_t_ {
    COMPO_STRUCT_COMMON;
    compo_football_move_cb_t move_cb;   //移动和拖动处理
    widget_page_t *page;

    sph_t sph;                          //球坐标
    area_t img_area;                    //足球上图片大小

    bool flag_need_update;              //是否需要更新

    widget_axis3d_t *axis;              //轴
    compo_football_item_t const *item;
    widget_image3d_t *item_img[FOOTBALL_ITEM_CNT];
} compo_football_t;

/**
 * @brief 创建一个足球菜单组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图像资源地址
 * @return 返回图像指针
 **/
compo_football_t *compo_football_create(compo_form_t *frm, s16 radius, compo_football_item_t const *item, int item_cnt);

/**
 * @brief 更新足球菜单Widget
 * @param[in] ball : 足球菜单指针
 **/
void compo_football_update(compo_football_t *ball);

/**
 * @brief 设置足球菜单的自旋角
 * @param[in] ball : 足球菜单指针
 * @param[in] angle : 自旋角
 **/
s32 compo_football_set_rotation(compo_football_t *ball, s32 angle);

/**
 * @brief 设置足球菜单的极角
 * @param[in] ball : 足球菜单指针
 * @param[in] angle : 极角
 **/
s32 compo_football_set_polar(compo_football_t *ball, s32 angle);

/**
 * @brief 足球菜单拖动与移动初始化
 * @param[in] ball : 足球菜单指针
 * @return 返回COMPO_FOOTBALL_STA状态枚举
 **/
u8 compo_football_get_sta(compo_football_t *ball);

/**
 * @brief 获取足球菜单项编号
 * @param[in] ball : 足球菜单指针
 * @param[in] x:屏幕坐标x
 * @param[in] y:屏幕坐标y
 * @return 返回菜单编号
 **/
int compo_football_get_idx(compo_football_t *ball, s16 x, s16 y);

/**
 * @brief 足球菜单拖动与移动处理
 * @param[in] ball : 足球菜单指针
 **/
void compo_football_move(compo_football_t *ball);

/**
 * @brief 足球菜单拖动与移动控制
 * @param[in] ball : 足球菜单指针
 * @param[in] cmd : 控制命令
 **/
void compo_football_move_control(compo_football_t *ball, int cmd);

#endif
