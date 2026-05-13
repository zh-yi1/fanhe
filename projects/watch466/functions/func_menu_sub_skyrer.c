#include "include.h"
#include "func.h"
#include "func_menu.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MENU_SKYRER_CNT                             ((int)(sizeof(tbl_menu_skyrer) / sizeof(tbl_menu_skyrer[0])))                           //图标总数
#define SKYRER_LOOP_ICON_NUM                        12                                                                                      //环形个数
#define SKYRER_OTHER_ICON_NUM                       4                                                                                       //方形个数
#define SKYRER_EDGE_SPACE                           2                                                                                       //边缘距离
#define SKYRER_ICON_SIZE_ORG                        (gui_image_get_size(tbl_menu_skyrer[0].res_addr).wid)                                   //原始图标大小
#define SKYRER_ICON_SIZE_MAX                        ((SKYRER_ICON_SIZE_ORG >> 4) * 13)                                                      //图标最大尺寸
#define SKYRER_ICON_SIZE_MIN                        ((SKYRER_ICON_SIZE_MAX >> 4) * 13)                                                      //图标最小尺寸
#define SKYRER_CENX                                 GUI_SCREEN_CENTER_X                                                                     //中心点
#define SKYRER_CENY                                 GUI_SCREEN_CENTER_Y                                                                     //中心点
#define SKYRER_SHORT_SIDE                           ((GUI_SCREEN_HEIGHT >= GUI_SCREEN_WIDTH) ? GUI_SCREEN_WIDTH : GUI_SCREEN_HEIGHT)        //取短边
#define SKYRER_ROTATE_RADIUS                        ((SKYRER_SHORT_SIDE >> 1) - (SKYRER_ICON_SIZE_MAX >> 1) - SKYRER_EDGE_SPACE)            //旋转半径
#define SKYRER_ANGLE_DIV                            300                                                                                     //间隔角度
#define SKYRER_ANGLE_INITIAL                        900                                                                                     //初始角度
#define SKYRER_ANIMATION_DIFF                       (SKYRER_ANGLE_DIV / 6)                                                                  //入场动画差量角度必须为 SKYRER_ANGLE_DIV 的约数
#define SKYRER_ANIMATION_KICK                       12                                                                                      //入场动画刷新间隔 越大越慢
#define SKYRER_MOVE_AUTO_KICK                       16                                                                                      //自动移动刷新间隔 越大越慢

enum {
    COMPO_ID_SKYRER = 1,
};

typedef struct f_menu_skyrer_t_ {
    compo_rings_t *rings;
    bool flag_drag;                 //开始拖动
    bool flag_move_auto;            //自动移到坐标
    bool flag_move_inertia;         //惯性运动
    u8   spawn_id;                  //出生点
    u8   spawn_idx;                 //目标编号
    uint32_t ticks;                 //滑动时间
    int32_t  diff_angle;            //角度差量
    int32_t  align_angle;           //对齐角度
    int32_t  accelera;              //惯性加速度
    int8_t   rotate_dir;            //旋转方向 0:无方向;1:顺时针;-1:逆时针
    s32 sx;
    s32 sy;
    s32 x;
    s32 y;
} f_menu_skyrer_t;

typedef struct f_menu_skyrer_party_t {
    s8 sgnx;
    s8 sgny;
    u8 idx;
}f_menu_skyrer_party;

typedef struct f_menu_skyrer_back_t {
    u8 idx;
}f_menu_skyrer_back;

f_menu_skyrer_back skyrer_back_idx[SKYRER_LOOP_ICON_NUM];

//天圆地方图标列表及顺序
static const compo_rings_item_t tbl_menu_skyrer[] = {
    {UI_BUF_ICON_CLOCK_BG_BIN,                  FUNC_CLOCK},            //时钟
    {UI_BUF_ICON_COMPASS_BIN,                   FUNC_COMPASS},
    {UI_BUF_ICON_HEART_RATE_BIN,                FUNC_HEARTRATE},
    {UI_BUF_ICON_ACTIVITY_BIN,                  FUNC_ACTIVITY},
    {UI_BUF_ICON_BLOOD_OXYGEN_BIN,              FUNC_BLOOD_OXYGEN},
    {UI_BUF_ICON_BLOODSUGAR_BIN,                FUNC_BLOODSUGAR},
    {UI_BUF_ICON_BREATHE_BIN,                   FUNC_BREATHE},
    {UI_BUF_ICON_MUSIC_BIN,                     FUNC_BT},
    {UI_BUF_ICON_CALL_BIN,                      FUNC_CALL},

    {UI_BUF_ICON_MENU_BIN,                      FUNC_STYLE},
    {UI_BUF_ICON_FLASHLIGHT_BIN,                FUNC_FLASHLIGHT},
    {UI_BUF_ICON_FINDPHONE_BIN,                 FUNC_FINDPHONE},
    {UI_BUF_ICON_GAME_BIN,                      FUNC_GAME},
    {UI_BUF_ICON_ALTITUDE_BIN,                  FUNC_ALTITUDE},
    {UI_BUF_ICON_ALARM_CLOCK_BIN,               FUNC_ALARM_CLOCK},
    {UI_BUF_ICON_RESTORE_FACTORY_BIN,           FUNC_RSTFY},
    {UI_BUF_ICON_LANGUAGE_BIN,                  FUNC_LANGUAGE},

    {UI_BUF_ICON_LIGHT_BIN,                     FUNC_LIGHT},
    {UI_BUF_ICON_MAP_BIN,                       FUNC_MAP},
    {UI_BUF_ICON_MESSAGE_BIN,                   FUNC_MESSAGE},
    {UI_BUF_ICON_CALCULATOR_BIN,                FUNC_CALCULATOR},
    {UI_BUF_ICON_OFF_BIN,                       FUNC_OFF},
    {UI_BUF_ICON_RESTART_BIN,                   FUNC_RESTART},
    {UI_BUF_ICON_CALENDAR_BIN,                  FUNC_CALENDAER},
    {UI_BUF_ICON_SCAN_BIN,                      FUNC_SCAN},

    {UI_BUF_ICON_SETTING_BIN,                   FUNC_SETTING},
    {UI_BUF_ICON_SLEEP_BIN,                     FUNC_SLEEP},
    {UI_BUF_ICON_VOLUME_BIN,                    FUNC_VOLUME},
    {UI_BUF_ICON_SPORT_BIN,                     FUNC_SPORT},
    {UI_BUF_ICON_STOPWATCH_BIN,                 FUNC_STOPWATCH},
    {UI_BUF_ICON_CAMERA_BIN,                    FUNC_CAMERA},
    {UI_BUF_ICON_TIMER_BIN,                     FUNC_TIMER},
    {UI_BUF_ICON_ADDRESS_BOOK_BIN,              FUNC_ADDRESS_BOOK},
};
//SIZE保留区域
static const int16_t tbl_keep_angle[] = {
    SKYRER_ANGLE_DIV * 2,  SKYRER_ANGLE_DIV * 4, SKYRER_ANGLE_DIV * 8, SKYRER_ANGLE_DIV * 10,
};
//方形图标折射角度
static const int16_t tbl_party_angle[] = {
    SKYRER_ANGLE_DIV * 1,  SKYRER_ANGLE_DIV * 5, SKYRER_ANGLE_DIV * 7, SKYRER_ANGLE_DIV * 11,
};
//方形图标信息
static const f_menu_skyrer_party tal_party[] = {
    {1, -1, 1}, {1, 1, 5}, {-1, 1, 7}, {-1, -1, 11},
};

static uint16_t func_menu_sub_skyrer_icon_size_update(compo_rings_t *rings, int32_t angle);
static void func_menu_sub_skyrer_icon_party_update(compo_rings_t *rings);
static void func_menu_sub_skyrer_update(compo_rings_t *rings, int32_t diff_angle, bool swp);
static void func_menu_sub_skyrer_animation_set(compo_rings_t *rings);
static void func_menu_sub_skyrer_animation_update(void);
static void func_menu_sub_skyrer_switch_backup(compo_rings_t *rings, u8 idx);
static void func_menu_sub_skyrer_set_focus(compo_rings_t *rings);

u8 func_menu_sub_skyrer_get_first_idx(void)
{
    return skyrer_back_idx[0].idx;
}

//创建主菜单窗体，创建窗体中不要使用功能结构体 func_cb.f_cb
compo_form_t *func_menu_sub_skyrer_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(false);       //菜单一般创建在底层

    //新建菜单
    compo_rings_t *rings = compo_rings_create(frm);
    compo_rings_ele_t ele = {
        .x              = SKYRER_CENX,
        .y              = SKYRER_CENY,
        .r              = SKYRER_ROTATE_RADIUS,
        .div            = -SKYRER_ANGLE_DIV,            //逆时针
        .angle          = SKYRER_ANGLE_INITIAL,
        .loop_icon_num  = SKYRER_LOOP_ICON_NUM,
        .other_icon_num = SKYRER_OTHER_ICON_NUM,
        .item_num       = MENU_SKYRER_CNT,
        .item           = tbl_menu_skyrer,
    };
    compo_rings_set(rings, &ele);
    //环形图标
    for (int i = 0; i < SKYRER_LOOP_ICON_NUM; i++) {
        compo_rings_icon_t *icon = compo_rings_icon_add(rings, tbl_menu_skyrer[i].res_addr, i, true);
        if (icon) {
            int32_t angle = compo_rings_loop_icon_find_angle(rings, i);
            uint16_t size = func_menu_sub_skyrer_icon_size_update(rings, angle);
            compo_rings_set_size(rings, size, i);
        }
    }
    //方形图标
    for (int i = 0; i < SKYRER_OTHER_ICON_NUM; i++) {
        compo_rings_icon_add(rings, tbl_menu_skyrer[i].res_addr, MENU_SKYRER_CNT + tal_party[i].idx, false);
    }

    //时钟指针
    compo_rings_add_time(rings, COMPO_RINGS_TIME_TYPE_HOUR, UI_BUF_ICON_CLOCK_H_BIN, 1, 2, SKYRER_ICON_SIZE_ORG);
    compo_rings_add_time(rings, COMPO_RINGS_TIME_TYPE_MIN, UI_BUF_ICON_CLOCK_M_BIN, 1, 2, SKYRER_ICON_SIZE_ORG);
    compo_rings_add_time(rings, COMPO_RINGS_TIME_TYPE_SEC, UI_BUF_ICON_CLOCK_S_BIN, 9, 2, SKYRER_ICON_SIZE_ORG);
    compo_rings_set_start_angle(rings, 900);
    compo_rings_idx_time_set(rings, 0);

    compo_setid(rings, COMPO_ID_SKYRER);

    if (func_cb.flag_animation) {
        func_menu_sub_skyrer_animation_set(rings);
    } else {
        func_menu_sub_skyrer_set_focus(rings);
        func_menu_sub_skyrer_update(rings, 0, false);
    }

    return frm;
}

//点进图标进入应用
static void func_menu_sub_skyrer_icon_click(void)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    compo_rings_t *rings = f_menu->rings;

	if (func_cb.flag_animation) return;

    u8 func_sta;

    compo_rings_sel_t *sel = compo_rings_select(rings, ctp_get_sxy().x, ctp_get_sxy().y);

    if ((sel->idx % MENU_SKYRER_CNT) < 0 || (sel->idx % MENU_SKYRER_CNT) >= MENU_SKYRER_CNT) {
        return;
    }

    //根据图标索引获取应用ID
    func_sta = tbl_menu_skyrer[sel->idx % MENU_SKYRER_CNT].func_sta;

    // //切入应用
    if (func_sta > 0) {
        func_menu_sub_skyrer_switch_backup(rings, sel->idx);
        compo_form_t *frm = func_create_form(func_sta);
        func_switching(FUNC_SWITCH_ZOOM_ENTER | FUNC_SWITCH_AUTO, sel->icon);
        compo_form_destroy(frm);
        func_cb.sta = func_sta;
        func_cb.menu_idx = sel->idx;                //记住当前编号
        if (sel->idx % MENU_SKYRER_CNT == 0) {      //时钟则开启入场动画
            func_cb.flag_animation = true;
        } else {
            func_cb.flag_animation = false;
        }
    }
}

//切换到时钟
static void func_menu_sub_skyrer_switch_to_clock(void)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    compo_rings_t *rings = f_menu->rings;
    widget_icon_t *icon = compo_rings_select_byidx(rings, 0);
    u8 func_sta = FUNC_CLOCK;
    compo_form_t *frm = func_create_form(func_sta);
    func_switching(FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO, icon);
    compo_form_destroy(frm);
    func_cb.sta = func_sta;
    func_cb.menu_idx = 0;
    func_cb.menu_idx_angle = -1;
    func_cb.flag_animation = true;
}

//备份环形图标
static void func_menu_sub_skyrer_switch_backup(compo_rings_t *rings, u8 idx)
{
    for (int i = 0; i < SKYRER_LOOP_ICON_NUM; i++) {
        u8 idx0 = compo_rings_loop_icon_find_idx(rings, i);
        skyrer_back_idx[i].idx = idx0;
        if (idx0 == idx % MENU_SKYRER_CNT) {
            func_cb.menu_idx_angle = compo_rings_loop_icon_find_angle(rings, i);
        }
    }
}

//设置图标集合焦点
static void func_menu_sub_skyrer_set_focus(compo_rings_t *rings)
{
    u8 foucs_angle_idx = 0;
    for (int i = 0; i < SKYRER_LOOP_ICON_NUM; i++) {
        if (skyrer_back_idx[i].idx == 0) {
            foucs_angle_idx++;
        }
    }
    if (foucs_angle_idx > 1) return;
    for (int i = 0; i < SKYRER_LOOP_ICON_NUM; i++) {
        compo_rings_set_uidx(rings, skyrer_back_idx[i].idx, i);                              //设置IDX
        compo_rings_set_res(rings, tbl_menu_skyrer[skyrer_back_idx[i].idx].res_addr, i);     //设置图片
        if (skyrer_back_idx[i].idx == func_cb.menu_idx % MENU_SKYRER_CNT) {
            foucs_angle_idx = i;
        }
    }

    //小于就加
    for (int i = foucs_angle_idx; i >= 0; i--) {
        int32_t angle = func_cb.menu_idx_angle + (foucs_angle_idx - i) * SKYRER_ANGLE_DIV;
        angle = RINGS_GET_ANGLE(angle);
        compo_rings_set_angle(rings, angle, i);
    }

    //大于就减
    for (int i = foucs_angle_idx; i < SKYRER_LOOP_ICON_NUM; i++) {
        int32_t angle = func_cb.menu_idx_angle - (i - foucs_angle_idx) * SKYRER_ANGLE_DIV;
        angle = RINGS_GET_ANGLE(angle);
        compo_rings_set_angle(rings, angle, i);
    }
}

//入场动画配置
static void func_menu_sub_skyrer_animation_set(compo_rings_t *rings)
{
    for (int i = 0; i < SKYRER_LOOP_ICON_NUM; i++) {
        compo_rings_set_angle(rings, SKYRER_ANGLE_INITIAL, i);
        uint16_t size = func_menu_sub_skyrer_icon_size_update(rings, SKYRER_ANGLE_INITIAL);
        compo_rings_set_size(rings, size, i);
    }
}

//入场动画刷新
static void func_menu_sub_skyrer_animation_update(void)
{
    if (!func_cb.flag_animation) return;
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    compo_rings_t *rings = f_menu->rings;
    
    if (tick_check_expire(f_menu->ticks, SKYRER_ANIMATION_KICK)) {      //动画刷新
        f_menu->ticks = tick_get();
        int32_t end_angle = compo_rings_loop_icon_find_angle(rings, SKYRER_LOOP_ICON_NUM-1);
        
        if (end_angle == (SKYRER_ANGLE_INITIAL + SKYRER_ANGLE_DIV)) {   //动画结束
            func_menu_sub_skyrer_icon_party_update(rings);
            for (int i = 0; i < SKYRER_OTHER_ICON_NUM; i++) {
                int idx1 = compo_rings_loop_icon_find_idx(rings, SKYRER_LOOP_ICON_NUM + i);
                s16 x = SKYRER_CENX + tal_party[i].sgnx * ((SKYRER_ICON_SIZE_MAX >> 1) + 5);
                s16 y = SKYRER_CENY + tal_party[i].sgny * ((SKYRER_ICON_SIZE_MAX >> 1) + 5);
                compo_rings_set_location(rings, x, y, SKYRER_ICON_SIZE_MAX, SKYRER_ICON_SIZE_MAX, idx1);
            }
            func_cb.flag_animation = false;
        } else {                                                        //动画开始
            compo_rings_loop_icon_set_angle_diff(rings, SKYRER_ANIMATION_DIFF, 0);         //差量角度必须为 SKYRER_ANGLE_DIV 的约数
            int32_t angle0 = compo_rings_loop_icon_find_angle(rings, 0);
            uint16_t size0 = func_menu_sub_skyrer_icon_size_update(rings, angle0);
            compo_rings_set_size(rings, size0, 0);

            for (int i = 1; i < SKYRER_LOOP_ICON_NUM; i++) {
                int32_t pre_angle = compo_rings_loop_icon_find_angle(rings, i-1);
                int32_t cur_angle = compo_rings_loop_icon_find_angle(rings, i);
                if (abs(pre_angle - cur_angle) > SKYRER_ANGLE_DIV) {
                    compo_rings_loop_icon_set_angle_diff(rings, SKYRER_ANIMATION_DIFF, i); //差量角度必须为 SKYRER_ANGLE_DIV 的约数
                }
                int32_t angle = compo_rings_loop_icon_find_angle(rings, i);
                uint16_t size = func_menu_sub_skyrer_icon_size_update(rings, angle);
                compo_rings_set_size(rings, size, i);
            }
            compo_rings_update_loop(rings);
        }
    }
}

//设置出生点
static void func_menu_sub_skyrer_icon_spawn_idx(void)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    compo_rings_t *rings = f_menu->rings;
    u8 flag_swawn = 0;
    for (int i = 0; i < SKYRER_LOOP_ICON_NUM; i++) {
        int32_t angle = compo_rings_loop_icon_find_angle(rings, i);
        if (f_menu->rotate_dir == 1) {              //顺时针
            if (angle >= SKYRER_ANGLE_INITIAL && angle < (SKYRER_ANGLE_INITIAL + SKYRER_ANGLE_DIV)) {
                f_menu->spawn_id = i;               //标记
                flag_swawn |= BIT(1);
            }
            if (angle >= (SKYRER_ANGLE_INITIAL + SKYRER_ANGLE_DIV) && angle < (SKYRER_ANGLE_INITIAL + SKYRER_ANGLE_DIV * 2)) {
                u8 idx0 = compo_rings_loop_icon_find_idx(rings, i);
                cycle_plus(&idx0, 0, (MENU_SKYRER_CNT - 1));
                f_menu->spawn_idx = idx0;           //下一个
                flag_swawn |= BIT(2);
            }
        } else if (f_menu->rotate_dir == -1) {      //逆时针
            if (angle > SKYRER_ANGLE_INITIAL && angle <= (SKYRER_ANGLE_INITIAL + SKYRER_ANGLE_DIV)) {
                f_menu->spawn_id = i;               //标记
                flag_swawn |= BIT(1);
            }
            if (angle <= SKYRER_ANGLE_INITIAL && angle > (SKYRER_ANGLE_INITIAL - SKYRER_ANGLE_DIV)) {
                u8 idx0 = compo_rings_loop_icon_find_idx(rings, i);
                cycle_sub(&idx0, 0, (MENU_SKYRER_CNT - 1));
                f_menu->spawn_idx = idx0;           //上一个
                flag_swawn |= BIT(2);
            }
        }

        if (flag_swawn == (BIT(1) | BIT(2))) break;
    }
}

//更新出生点
static void func_menu_sub_skyrer_swawn_update(void)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    compo_rings_t *rings = f_menu->rings;

    int32_t angle = compo_rings_loop_icon_find_angle(rings, f_menu->spawn_id);
    if (f_menu->rotate_dir == 1) {
        if (angle >= (SKYRER_ANGLE_INITIAL + SKYRER_ANGLE_DIV)) {
            u8 idx0 = compo_rings_loop_icon_find_idx(rings, f_menu->spawn_id);
            compo_rings_set_res_from_idx(rings, tbl_menu_skyrer[f_menu->spawn_idx].res_addr, idx0);
            compo_rings_set_uidx_from_idx(rings, f_menu->spawn_idx, idx0);
        }
    } else if (f_menu->rotate_dir == -1) {
        if (angle <= SKYRER_ANGLE_INITIAL) {
            u8 idx0 = compo_rings_loop_icon_find_idx(rings, f_menu->spawn_id);
            compo_rings_set_res_from_idx(rings, tbl_menu_skyrer[f_menu->spawn_idx].res_addr, idx0);
            compo_rings_set_uidx_from_idx(rings, f_menu->spawn_idx, idx0);
        }
    }
}

//方形图标刷新
static void func_menu_sub_skyrer_icon_party_update(compo_rings_t *rings)
{
    for (int i = 0; i < SKYRER_LOOP_ICON_NUM; i++) {
        int32_t angle = compo_rings_loop_icon_find_angle(rings, i);
        u8      idx0  = compo_rings_loop_icon_find_idx(rings, i);
        for (int k = 0; k < SKYRER_OTHER_ICON_NUM; k++) {
            if (abs(angle - tbl_party_angle[k]) <= 50) {
                u8 idx1 = compo_rings_loop_icon_find_idx(rings, SKYRER_LOOP_ICON_NUM + k);
                compo_rings_set_res_from_idx(rings, tbl_menu_skyrer[idx0].res_addr, idx1);
                compo_rings_set_uidx_from_idx(rings, idx0 + MENU_SKYRER_CNT, idx1);
            }
        }
    }
}

//环形图标size更新
static uint16_t func_menu_sub_skyrer_icon_size_update(compo_rings_t *rings, int32_t angle)
{
    uint16_t size = SKYRER_ICON_SIZE_MIN;
    if ((angle >= tbl_keep_angle[0] && angle <= tbl_keep_angle[1]) || (angle >= tbl_keep_angle[2] && angle <= tbl_keep_angle[3])) return size;
    int32_t l0 = muls(SKYRER_ANGLE_DIV, rings->ele.r);
    int32_t l1 = muls((angle % (SKYRER_ANGLE_DIV * 2)), rings->ele.r);

    if (l1 <= l0) {
        size = muls((SKYRER_ICON_SIZE_MAX - SKYRER_ICON_SIZE_MIN), l1) / l0 + SKYRER_ICON_SIZE_MIN;
    } else if (l1 < 2 * l0) {
        size = muls((SKYRER_ICON_SIZE_MAX - SKYRER_ICON_SIZE_MIN), (2 * l0 - l1)) / l0 + SKYRER_ICON_SIZE_MIN;
    }

    return size;
}

//天圆地方菜单刷新
static void func_menu_sub_skyrer_update(compo_rings_t *rings, int32_t diff_angle, bool swp)
{
    if (swp) {                                                  //设置出生点
        func_menu_sub_skyrer_icon_spawn_idx();
    }

    for (int i = 0; i < SKYRER_LOOP_ICON_NUM; i++) {            //更新差量角度 环形图标size
        compo_rings_loop_icon_set_angle_diff(rings, diff_angle, i);
        int32_t angle = compo_rings_loop_icon_find_angle(rings, i);
        uint16_t size = func_menu_sub_skyrer_icon_size_update(rings, angle);
        compo_rings_set_size(rings, size, i);
    }

    if (swp) {
        func_menu_sub_skyrer_swawn_update();                    //刷新出生点
    }

    compo_rings_page_time1_vis(rings, false);
    func_menu_sub_skyrer_icon_party_update(rings);              //设置方形图标
    for (int i = 0; i < SKYRER_OTHER_ICON_NUM; i++) {           //刷新方形图标
        int idx1 = compo_rings_loop_icon_find_idx(rings, SKYRER_LOOP_ICON_NUM + i);
        s16 x = SKYRER_CENX + tal_party[i].sgnx * ((SKYRER_ICON_SIZE_MAX >> 1) + 5);
        s16 y = SKYRER_CENY + tal_party[i].sgny * ((SKYRER_ICON_SIZE_MAX >> 1) + 5);
        compo_rings_icon_t *icon = compo_rings_set_location(rings, x, y, SKYRER_ICON_SIZE_MAX, SKYRER_ICON_SIZE_MAX, idx1);
        compo_rings_page_time1_bg(rings, icon->widget, idx1 % MENU_SKYRER_CNT);
        compo_rings_page_time1_update(rings, x, y, SKYRER_ICON_SIZE_MAX, idx1 % MENU_SKYRER_CNT);
    }
    compo_rings_update_loop(rings);                             //刷新环形图标
}

//主菜单功能事件处理
static void func_menu_sub_skyrer_process(void)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    compo_rings_t *rings = f_menu->rings;

    func_menu_sub_skyrer_animation_update();

    if (!func_cb.flag_animation) {
        if (f_menu->flag_drag) {
            f_menu->flag_drag = ctp_get_cur_point(&f_menu->sx, &f_menu->sy, &f_menu->x, &f_menu->y);
            if (f_menu->flag_drag) {
                f_menu->diff_angle = compo_rings_touch_angle_diff(rings, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, f_menu->x, f_menu->y);
                if (abs(f_menu->diff_angle) != 0) {
                    f_menu->diff_angle = (abs(f_menu->diff_angle) >= SKYRER_ANGLE_DIV) ? ((SKYRER_ANGLE_DIV - 1) * (f_menu->diff_angle / abs(f_menu->diff_angle))) : f_menu->diff_angle;
                }

                if (f_menu->diff_angle != 0) {
                    uint32_t ticks      = (uint32_t)(tick_get() - f_menu->ticks);
                    f_menu->accelera    = ticks > 0 ? (int32_t)((int32_t)f_menu->diff_angle / (int32_t)ticks) : f_menu->accelera;           //惯性加速度
                    f_menu->rotate_dir  = (abs(f_menu->diff_angle) > 0) ? (f_menu->diff_angle / abs(f_menu->diff_angle)) : 0;               //旋转方向
                }
                func_menu_sub_skyrer_update(rings, f_menu->diff_angle, true);
                f_menu->ticks = tick_get();
            } else {
                f_menu->flag_move_auto    = true;
                f_menu->ticks             = tick_get();
            }
        } else if (f_menu->flag_move_auto) {
            if (tick_check_expire(f_menu->ticks, SKYRER_MOVE_AUTO_KICK)) {  //自动刷新
                f_menu->ticks = tick_get();
                if (f_menu->accelera != 0) {
                    f_menu->diff_angle = f_menu->accelera >> 0;             //差量角度
                    f_menu->accelera += -f_menu->rotate_dir * 20;           //惯性加速度归0
                    if ((f_menu->rotate_dir >= 0 && f_menu->accelera <= 0) || (f_menu->rotate_dir <= 0 && f_menu->accelera >= 0)) {
                        f_menu->accelera = 0;
                    }
                    func_menu_sub_skyrer_update(rings, f_menu->diff_angle, true);
                } else if (f_menu->accelera == 0) {
                    int32_t margin_angle = compo_rings_loop_icon_find_angle(rings, 0) % SKYRER_ANGLE_DIV;

                    if (margin_angle != 0) {
                        //如果没有辨别到方向时，做最近点的方向补偿
                        f_menu->rotate_dir  = (f_menu->rotate_dir == 0) ? (((abs(margin_angle) - (SKYRER_ANGLE_DIV >> 1)) > 0) ? 1 : -1) : f_menu->rotate_dir;
                        f_menu->align_angle = (f_menu->rotate_dir > 0) ? (SKYRER_ANGLE_DIV - margin_angle) : (-margin_angle);
                        f_menu->diff_angle  = f_menu->rotate_dir * 20;

                        if (abs(f_menu->diff_angle) > abs(f_menu->align_angle)) {
                            f_menu->diff_angle = f_menu->align_angle;
                        }
                        func_menu_sub_skyrer_update(rings, f_menu->diff_angle, true);
                    } else {
                        f_menu->rotate_dir          = 0;
                        f_menu->align_angle         = 0;
                        f_menu->diff_angle          = 0;
                        f_menu->flag_move_auto      = false;
                    }
                }
            }
        }
    }

    func_process();
}

//旋钮滚动处理
static void func_menu_sub_knob_scroll(size_msg_t msg)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;

    if (func_cb.flag_animation) return;
    if (f_menu->flag_drag) return;

    f_menu->flag_move_auto  = true;
    f_menu->rotate_dir = (msg == MSG_QDEC_FORWARD) ? 1 : -1;
    int32_t diff_angle = (msg == MSG_QDEC_FORWARD) ? ((SKYRER_ANGLE_DIV >> 2)) : -((SKYRER_ANGLE_DIV >> 2));//带旋转路径
    func_menu_sub_skyrer_update(f_menu->rings, diff_angle, true);
}

//拖动处理
static void func_menu_sub_drag_move(void)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    compo_rings_t *rings = f_menu->rings;

    if (func_cb.flag_animation) return;

    ctp_get_cur_point(&f_menu->sx, &f_menu->sy, &f_menu->x, &f_menu->y);
    compo_rings_touch_angle_pre(rings, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, f_menu->sx, f_menu->sy);
    f_menu->ticks = tick_get();
    f_menu->flag_drag = true;                           //任何方向短划，开启拖动
}

//拖动过程中，只响应部分消息
static void func_menu_sub_skyrer_drag_message(size_msg_t msg)
{
    switch (msg) {

    default:
        evt_message(msg);
        break;
    }
}

//自动移动中，只响应部分消息
static void func_menu_sub_skyrer_move_auto_message(size_msg_t msg)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    switch (msg) {
    case MSG_CTP_TOUCH:
        f_menu->flag_drag = true;                       //移动过程中，触屏停止
        f_menu->flag_move_auto = false;
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单
        func_menu_sub_knob_scroll(msg);
        break;
    case MSG_QDEC_BACKWARD:                             //向后滚动菜单
        func_menu_sub_knob_scroll(msg);
        break;

    default:
        evt_message(msg);
        break;
    }
}

//正常模式下
static void func_menu_sub_skyrer_normal_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_menu_sub_skyrer_icon_click();                //单击图标
        break;

    case MSG_CTP_SHORT_LEFT:
    case MSG_CTP_SHORT_UP:
    case MSG_CTP_SHORT_RIGHT:
    case MSG_CTP_SHORT_DOWN:
        func_menu_sub_drag_move();
        break;

    case MSG_QDEC_FORWARD:                              //向前滚动菜单
        func_menu_sub_knob_scroll(msg);
        break;
    case MSG_QDEC_BACKWARD:                             //向后滚动菜单
        func_menu_sub_knob_scroll(msg);
        break;

    case KU_DELAY_BACK:
        if (tick_check_expire(func_cb.enter_tick, TICK_IGNORE_KEY)) {
            func_menu_sub_skyrer_switch_to_clock();     //返回时钟表盘界面
        }
        break;

    default:
        func_menu_sub_message(msg);
        break;
    }
}

//主菜单功能消息处理
static void func_menu_sub_skyrer_message(size_msg_t msg)
{
    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    if (f_menu->flag_drag) {
        //拖动过程中，只响应部分消息
        func_menu_sub_skyrer_drag_message(msg);
    } else if (f_menu->flag_move_auto) {
        //自动移动中，只响应部分消息
        func_menu_sub_skyrer_move_auto_message(msg);
    } else {
        //正常模式下
        func_menu_sub_skyrer_normal_message(msg);
    }
}

//进入主菜单功能
static void func_menu_sub_skyrer_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_menu_skyrer_t));
    func_cb.frm_main = func_menu_sub_skyrer_form_create();

    f_menu_skyrer_t *f_menu = (f_menu_skyrer_t *)func_cb.f_cb;
    f_menu->rings = compo_getobj_byid(COMPO_ID_SKYRER);
    compo_rings_t *rings = f_menu->rings;

    if (rings->type != COMPO_TYPE_RINGS) {
        halt(HALT_GUI_COMPO_ICONLIST_TYPE);
    }

    if (func_cb.flag_animation) {
        f_menu->ticks = tick_get();
    }

    func_cb.enter_tick = tick_get();
}

//主菜单功能
void func_menu_sub_skyrer(void)
{
    printf("%s\n", __func__);
    func_menu_sub_skyrer_enter();
    while (func_cb.sta == FUNC_MENU && func_cb.menu_style == MENU_STYLE_SKYRER) {
        func_menu_sub_skyrer_process();
        func_menu_sub_skyrer_message(msg_dequeue());
    }

    func_menu_sub_exit();
}
