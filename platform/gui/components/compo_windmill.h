#ifndef __COMPO_WINDMILL_H__
#define __COMPO_WINDMILL_H__


#define WINDMILL_ITEM_CNT   50


//立方体菜单移动控制命令
enum COMPO_WINDMILL_MOVE_CMD {
    COMPO_WINDMILL_MOVE_CMD_NONE,
    COMPO_WINDMILL_MOVE_CMD_DRAG,                       //开始拖动
    COMPO_WINDMILL_MOVE_CMD_FORWARD,                    //向前滚动
    COMPO_WINDMILL_MOVE_CMD_BACKWARD,                   //向后滚动
    COMPO_WINDMILL_MOVE_CMD_AUTO,                       //向后滚动
};

//立方体菜单当前状态
enum COMPO_WINDMILL_STA {
    COMPO_WINDMILL_STA_IDLE,                        //空闲状态
    COMPO_WINDMILL_STA_DARG,                        //拖动中
    COMPO_WINDMILL_STA_MOVE,                        //移动中
};

typedef struct compo_windmill_res_t_ {
    u32 res_addr;               //图标
} compo_windmill_item_t;

typedef struct compo_windmill_move_cb_t_ {
    u32 tick;
    sph_t focus_sph;                    //当前球坐标
    s32 start_a;                        //开始角度
    s32 moveto_a;                       //设定自动移到的角度
    s16 roll_azimuth;
    bool flag_drag;                     //开始拖动
    bool flag_move_auto;                //自动移到坐标
} compo_windmill_move_cb_t;

typedef struct compo_windmill_t_ {
    COMPO_STRUCT_COMMON;
    compo_windmill_move_cb_t move_cb;       //移动和拖动处理
    widget_page_t* page;

    sph_t sph;                              //风车的球坐标

    bool flag_need_update;                  //是否需要更新

    widget_axis3d_t *axis;                  //风车轴

    u8 item_cnt;

    compo_windmill_item_t const *item;
    widget_image3d_t *item_img[WINDMILL_ITEM_CNT];

    s32 wind_grade;      //风强度与方向
    int mode;

} compo_windmill_t;


compo_windmill_t* compo_windmill_create(compo_form_t* frm, compo_windmill_item_t const *item, u16 item_cnt);
void compo_windmill_update(compo_windmill_t* windmill);
s32 compo_windmill_set_rotation(compo_windmill_t *windmill, s32 angle);

void compo_windmill_roll(compo_windmill_t* windmill, s16 roll_polar);
void compo_windmill_roll_from(compo_windmill_t *windmill, s16 roll_polar, s16 roll_azimuth);
s32 compo_windmill_set_polar(compo_windmill_t *windmill, s32 angle);
u8 compo_windmill_get_sta(compo_windmill_t *windmill);
int compo_windmill_get_idx(compo_windmill_t *windmill, s16 x, s16 y);
void compo_windmill_move(compo_windmill_t *windmill);
void compo_windmill_move_control(compo_windmill_t *windmill, int cmd);

void compo_windmill_set_pos(compo_windmill_t *windmill, s16 x, s16 y);

#endif // __COMPO_WINDMILL_H__
