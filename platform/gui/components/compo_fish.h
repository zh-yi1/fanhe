#ifndef __COMPO_FINSH_H__
#define __COMPO_FINSH_H__

#define FINSH_BODY_FIXED_CNT    5       //鱼切图的固定数量, 其他数量的鱼切图姿态不支持, 不可修改

enum {
    COMPO_FINSH_MCB_NONE,
    COMPO_FINSH_MCB_CLICK,
};

enum {
    COMPO_FINSH_CLICK_NONE,
    COMPO_FINSH_CLICK_HEAD,
    COMPO_FINSH_CLICK_TAIL,
};

enum {
    COMPO_FINSH_MCB_SWIM_NONE,
    COMPO_FINSH_MCB_SWIM_NORMAL,
    COMPO_FINSH_MCB_SWIM_FAST,
    COMPO_FINSH_MCB_SWIM_FAST_TIME,
    COMPO_FINSH_MCB_SWIM_TURN,
};

typedef struct fish_body_location_t_ {
    point_t pos;
    s16 rotate;
} fish_location_t;

typedef struct fish_body_t_ {
    fish_location_t location;
    area_t raw_res_area;
    u8 len;
    widget_image_t* img;
} fish_body_t;

typedef struct sin_path_t_ {        //y = A sin(K*x)
    float AMP;
    float K;
    bool is_init;
} sin_path_t;

typedef struct fish_move_cb_t_ {
    sin_path_t swim_path;           //游动路径
    fish_location_t body_location[FINSH_BODY_FIXED_CNT];

    s16 rotate;
    s16 rotate_last;
    s16 delta_rotate;
    s16 frist_rotate;

    point_t rotate_center;
    point_t rotate_center_last;

    s16 turn_omiga;
    u8 turn_sta;
    bool turning;

    u32 ticks;
    u32 ticks_cnt;

    u8 flag_click;
    u8 swim_sta;

    point_t itera;

    s16 head_location;

    bool flag_need_render;

    bool flag_rotate_rand;

} fish_move_cb_t;

typedef struct compo_fish_t_ {
    COMPO_STRUCT_COMMON;

    widget_page_t* page;
    fish_body_t body[FINSH_BODY_FIXED_CNT];    //组成鱼身体的各个部分信息
    fish_body_t shadow[FINSH_BODY_FIXED_CNT];

    fish_move_cb_t mcb;

    point_t offset;

    bool swim_state;                            //鱼当前处于状态

} compo_fish_t;

void compo_fish_mcb_process(compo_fish_t* fish);
void compo_fish_bend_update(compo_fish_t* fish, bool lr);
void compo_fish_set_swim_delta_x2path(compo_fish_t* fish, s16 delta);
void compo_fish_mcb_message(compo_fish_t* fish, point_t pt, u8 sta);
void compo_fish_render_update(compo_fish_t* fish);
void compo_fish_mcb_location_update(compo_fish_t* fish);
void compo_fish_set_path(compo_fish_t* fish, float amp, float zita);
compo_fish_t* compo_fish_create(compo_form_t* frm, const u32 *res_body, const u32 *res_shadow, u8 res_cnt);
void compo_fish_set_visable(compo_fish_t* fish, bool visable);
bool compo_fish_get_visable(compo_fish_t* fish);
void compo_fish_set_frist_rotate(compo_fish_t* fish, s16 rotate);
void compo_fish_set_frist_rotate_rand(compo_fish_t* fish, bool yn);

#endif // __COMPO_FINSH_H__
