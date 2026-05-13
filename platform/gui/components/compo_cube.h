#ifndef _COMPO_CUBE_H
#define _COMPO_CUBE_H

#define CUBE_ITEM_CNT               6               //立方体菜单项列表数量，必须是6项
#define CUBE_ITEM_ELE_CNT           30               //立方体菜单项列表数量，必须是6项

//立方体菜单移动控制命令
enum COMPO_CUBE_MOVE_CMD {
    COMPO_CUBE_MOVE_CMD_NONE,
    COMPO_CUBE_MOVE_CMD_DRAG,                       //开始拖动
    COMPO_CUBE_MOVE_CMD_FORWARD,                    //向前滚动
    COMPO_CUBE_MOVE_CMD_BACKWARD,                   //向后滚动
    COMPO_CUBE_MOVE_CMD_AUTO,                       //自动滚动
};

//立方体菜单当前状态
enum COMPO_CUBE_STA {
    COMPO_CUBE_STA_IDLE,                        //空闲状态
    COMPO_CUBE_STA_DARG,                        //拖动中
    COMPO_CUBE_STA_MOVE,                        //移动中
};

enum CUBE_TYPE {
    COMPO_CUBE_TYPE_NONE,                        //无效
    COMPO_CUBE_TYPE_NORMAL,                      //正常立方体
    COMPO_CUBE_TYPE_POWER,                       //能量球
    COMPO_CUBE_TYPE_MAX,                         //最大值
};

//立方体菜单项定义
typedef struct compo_cube_item_t_ {
    u32 res_addr;               //图标
    u8 func_sta;                //功能ID
} compo_cube_item_t;

//立方体菜单移动控制
typedef struct compo_cube_move_cb_t_ {
    u32 tick;
    sph_t focus_sph;                    //当前球坐标
    s32 start_a;                        //开始角度
    s32 moveto_a;                       //设定自动移到的角度
    s16 roll_azimuth;
    bool flag_drag;                     //开始拖动
    bool flag_move_auto;                //自动移到坐标
    u8 sta;
    u8 animation_cnt;
    u8 cmd;                             //控制命令
    bool flag_intertia;                 //惯性标志 0:惯性移动完成; 1:惯性移动开始
} compo_cube_move_cb_t;


typedef struct compo_cube_ele_t_ {
    u32 res;                            //元素图片资源
    widget_image3d_t *img3d;            //2.5d图片控件
    rect_t location;                    //元素初始位置
    s16 radius;                         //元素的半径
    u8 sta;
} compo_cube_ele_t;

typedef struct compo_cube_t_ {
    COMPO_STRUCT_COMMON;
    compo_cube_move_cb_t move_cb;   //移动和拖动处理
    widget_page_t *page;

    sph_t sph;                          //球坐标
    area_t img_area;                    //立方体上图片大小

    bool flag_need_update;              //是否需要更新

    widget_axis3d_t *axis;              //轴
    compo_cube_item_t const *item;
    s16 radius;
    widget_image3d_t *item_img[CUBE_ITEM_CNT];
    compo_cube_ele_t ele[CUBE_ITEM_ELE_CNT];

    uint8_t cube_type;                       //立方体组件类型，可选COMPO_CUBE_TYPE_NORMAL(正常的立方体)，COMPO_CUBE_TYPE_POWER(能量球)
} compo_cube_t;

/**
 * @brief 创建一个立方体菜单组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图像资源地址
 * @return 返回图像指针
 **/
compo_cube_t *compo_cube_create(compo_form_t *frm, s16 radius, compo_cube_item_t const *item, int item_cnt);

/**
 * @brief 更新立方体菜单Widget
 * @param[in] cube : 立方体菜单指针
 **/
void compo_cube_update(compo_cube_t *cube);

/**
 * @brief 设置立方体菜单的自旋角
 * @param[in] cube : 立方体菜单指针
 * @param[in] angle : 自旋角
 **/
s32 compo_cube_set_rotation(compo_cube_t *cube, s32 angle);

/**
 * @brief 设置立方体菜单的极角
 * @param[in] cube : 立方体菜单指针
 * @param[in] angle : 极角
 **/
s32 compo_cube_set_polar(compo_cube_t *cube, s32 angle);

/**
 * @brief 立方体菜单拖动与移动初始化
 * @param[in] cube : 立方体菜单指针
 * @return 返回COMPO_FOOTBALL_STA状态枚举
 **/
u8 compo_cube_get_sta(compo_cube_t *cube);

/**
 * @brief 获取立方体菜单项编号
 * @param[in] cube : 立方体菜单指针
 * @param[in] x:屏幕坐标x
 * @param[in] y:屏幕坐标y
 * @return 返回菜单编号
 **/
int compo_cube_get_idx(compo_cube_t *cube, s16 x, s16 y);

/**
 * @brief 立方体菜单拖动与移动处理
 * @param[in] cube : 立方体菜单指针
 **/
void compo_cube_move(compo_cube_t *cube);

/**
 * @brief 立方体菜单拖动与移动控制
 * @param[in] cube : 立方体菜单指针
 * @param[in] cmd : 控制命令
 **/
void compo_cube_move_control(compo_cube_t *cube, int cmd);

void compo_cube_set_pos(compo_cube_t *cube, s16 x, s16 y);

/**
 * 设置compo_cube的类型
 * @param cube 指向compo_cube结构体的指针，用于指定哪个立方体的类型需要被设置
 * @param type 要设置的立方体类型，使用uint8_t表示
 * 此函数用于更新compo_cube结构体中的类型信息如果传入的指针为NULL，将调用halt函数
 * 停止程序运行，以避免潜在的程序崩溃或未定义行为
 */
void compo_cube_set_type(compo_cube_t *cube, uint8_t type);

/**
 * @brief 立方体菜单添加元素
 * @param[in] cube : 立方体菜单指针
 * @param[in] cmd : 控制命令
 **/
void compo_cube_add_element(compo_cube_t *cube, s16 radius, u32 ele_res, int ele_cnt);

#endif
