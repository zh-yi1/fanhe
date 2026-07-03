#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


typedef struct f_compo_select_sub_t_ {
    compo_listbox_item_t *p_list_data;
    compo_listbox_move_cb_t mcb;
} f_compo_select_sub_t;


enum {
    COMPO_ID_LISTBOX = 1,
};

const compo_listbox_item_t tbl_list_data[] = {
    // {STR_SPORTS,                 UI_BUF_ICON_SPORT_BIN,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_SPORT_ON},     //运动
    // {STR_SLEEP,                  UI_BUF_ICON_SLEEP_BIN,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_SLEEP_ON},     //睡眠
    // {STR_ACTIVITY_RECORD,        UI_BUF_ICON_ACTIVITY_BIN,       .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_ACTIVITY_ON},  //活动记录
    // {STR_HEART_RATE,             UI_BUF_ICON_HEART_RATE_BIN,     .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_HEART_ON},     //心率
    // {STR_BLOOD_PRESSURE,         UI_BUF_ICON_BLOOD_PRESSURE_BIN, .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_HRV_ON},       //血压
    // {STR_BLOOD_OXYGEN,           UI_BUF_ICON_BLOOD_OXYGEN_BIN,   .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_SPO2_ON},      //血氧
    // {STR_MESSAGE,                UI_BUF_ICON_MESSAGE_BIN,        .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_SMS_ON},       //消息
    // {STR_PHONE,                  UI_BUF_ICON_CALL_BIN,           .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_BT_CALL_ON},   //电话
    // {STR_MUSIC,                  UI_BUF_ICON_MUSIC_BIN,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_MUSIC_ON},     //音乐
    // {STR_WEATHER,                UI_BUF_ICON_WEATHER_BIN,        .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_WEATHER_ON},   //天气
    // {STR_BREATHE,                UI_BUF_ICON_BREATHE_BIN,        .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_BREATHE_ON},   //呼吸
    // {STR_CALCULATOR,             UI_BUF_ICON_CALCULATOR_BIN,     .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_CALCUL_ON},    //计算器
    // {STR_ALARM_CLOCK,            UI_BUF_ICON_ALARM_CLOCK_BIN,    .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_ALARM_ON},     //闹钟
    // {STR_TIMER,                  UI_BUF_ICON_TIMER_BIN,          .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_TIMER_ON},     //定时器
    // {STR_STOP_WATCH,             UI_BUF_ICON_STOPWATCH_BIN,      .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_STODWATCH_ON}, //秒表
    // {STR_GAME,                   UI_BUF_ICON_GAME_BIN,           .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_GAME_ON},      //游戏
    // {STR_SETTING,                UI_BUF_ICON_SETTING_BIN,        .item_mode = COMPO_LISTBOX_ITEM_MODE_SWITCH, .vidx = SYS_CTL_FUNC_SETTINGS_ON},  //设置
};

const u8 SYS_CTL_ON_TO_FUNC_STA_TABLE[] = {
    SYS_CTL_FUNC_ACTIVITY_ON,       FUNC_ACTIVITY,
    SYS_CTL_FUNC_HEART_ON,          FUNC_HEARTRATE,
    SYS_CTL_FUNC_SLEEP_ON,          FUNC_SLEEP,
    SYS_CTL_FUNC_SPO2_ON,           FUNC_BLOOD_OXYGEN,
    SYS_CTL_FUNC_MUSIC_ON,          FUNC_BT,
    SYS_CTL_FUNC_SPORT_ON,          FUNC_SPORT,
    SYS_CTL_FUNC_HRV_ON,            FUNC_BLOOD_PRESSURE,
    SYS_CTL_FUNC_SMS_ON,            FUNC_MESSAGE,
    SYS_CTL_FUNC_BT_CALL_ON,        FUNC_CALL,
    SYS_CTL_FUNC_WEATHER_ON,        FUNC_WEATHER,
    SYS_CTL_FUNC_BREATHE_ON,        FUNC_BREATHE,
    SYS_CTL_FUNC_CALCUL_ON,         FUNC_CALCULATOR,
    SYS_CTL_FUNC_ALARM_ON,          FUNC_ALARM_CLOCK,
    SYS_CTL_FUNC_TIMER_ON,          FUNC_TIMER,
    SYS_CTL_FUNC_STODWATCH_ON,      FUNC_STOPWATCH,
    SYS_CTL_FUNC_GAME_ON,           FUNC_GAME,
    SYS_CTL_FUNC_SETTINGS_ON,       FUNC_SETTING,
};

/* UI_BUF_ICON_*_BIN 未定义，tbl_list_data 为占位空数组；实际条目数固定为 17 */
#define LIST_ITEM_CNT_MAX                   17

/* list_data_sort + 配套操作函数：功能暂不可用，#if 0 避免空数组越界警告 */
#if 0

static u8 list_data_sort[LIST_ITEM_CNT_MAX] = {
    SYS_CTL_FUNC_ACTIVITY_ON,
    SYS_CTL_FUNC_HEART_ON,
    SYS_CTL_FUNC_SLEEP_ON,
    SYS_CTL_FUNC_SPO2_ON,
    SYS_CTL_FUNC_MUSIC_ON,
    SYS_CTL_FUNC_SPORT_ON,
    SYS_CTL_FUNC_HRV_ON,
    SYS_CTL_FUNC_SMS_ON,
    SYS_CTL_FUNC_BT_CALL_ON,
    SYS_CTL_FUNC_WEATHER_ON,
    SYS_CTL_FUNC_BREATHE_ON,
    SYS_CTL_FUNC_CALCUL_ON,
    SYS_CTL_FUNC_ALARM_ON,
    SYS_CTL_FUNC_TIMER_ON,
    SYS_CTL_FUNC_STODWATCH_ON,
    SYS_CTL_FUNC_GAME_ON,
    SYS_CTL_FUNC_SETTINGS_ON,
};

static const compo_listbox_item_t *get_tbl_list_data_by_vidx(u16 vidx)
{
    for (u8 i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (vidx == tbl_list_data[i].vidx) {
            return (compo_listbox_item_t *)&tbl_list_data[i];
        }
    }
    return NULL;
}

static void func_compo_list_data_update(void)
{
    f_compo_select_sub_t *f_compo_select_sub = (f_compo_select_sub_t *)func_cb.f_cb;
    compo_listbox_item_t *p_list_data = f_compo_select_sub->p_list_data;
    for (int i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (list_data_sort[i] != p_list_data[i].vidx) {
            memcpy(&p_list_data[i], get_tbl_list_data_by_vidx(list_data_sort[i]), sizeof(compo_listbox_item_t));
        }
    }
}

static u8 list_data_sort_get_add_cnt(void)
{
    u8 cnt = 0;
    for (int i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (bsp_sys_get_ctlbit(list_data_sort[i])) { cnt++; } else { break; }
    }
    return cnt;
}

static void list_data_sort_add(u8 vidx)
{
    u8 tmp[LIST_ITEM_CNT_MAX] = {0};
    u8 add_cnt = list_data_sort_get_add_cnt();
    u8 idx = add_cnt;
    if (add_cnt) memcpy(tmp, list_data_sort, add_cnt);
    tmp[add_cnt] = vidx;
    idx = add_cnt + 1;
    for (u8 i = add_cnt; i < LIST_ITEM_CNT_MAX; i++) {
        if (vidx != list_data_sort[i]) tmp[idx++] = list_data_sort[i];
    }
    memcpy(list_data_sort, tmp, sizeof(list_data_sort));
    func_compo_list_data_update();
}

static void list_data_sort_del(u8 vidx)
{
    u8 tmp[LIST_ITEM_CNT_MAX] = {0};
    u8 idx = LIST_ITEM_CNT_MAX - 1;
    tmp[idx--] = vidx;
    for (u8 i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (vidx != list_data_sort[LIST_ITEM_CNT_MAX - 1 - i]) {
            tmp[idx--] = list_data_sort[LIST_ITEM_CNT_MAX - 1 - i];
        }
    }
    memcpy(list_data_sort, tmp, sizeof(list_data_sort));
    func_compo_list_data_update();
}

static void list_data_sort_up(u8 vidx)
{
    u8 add_cnt = list_data_sort_get_add_cnt();
    if (add_cnt < 2) return;
    for (u8 i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (list_data_sort[i] == vidx) {
            u8 pos = (i == 0) ? (add_cnt - 1) : (i - 1);
            u8 t = list_data_sort[pos];
            list_data_sort[pos] = vidx;
            list_data_sort[i] = t;
            break;
        }
    }
    func_compo_list_data_update();
}
#endif

compo_form_t *func_compo_select_sub_form_create(void)
{
    /* UI_BUF_ICON 宏未定义，功能不可用 */
    return NULL;
}

//进入组件选择功能（空实现）
void func_compo_select_sub_enter(void)
{
}

//退出组件选择功能
void func_compo_select_sub_exit(void)
{
    func_cb.last = FUNC_COMPO_SELECT_SUB;
    u8 index = 1;
    for (u8 i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (bsp_sys_get_ctlbit(SYS_CTL_FUNC_SPORT_ON + i)) {
            func_cb.tbl_sort[index++] = SYS_CTL_ON_TO_FUNC_STA_TABLE[2 * i + 1];
        }
    }
    func_cb.tbl_sort[index++] = FUNC_COMPO_SELECT;
    func_cb.sort_cnt = index;
    func_cb.flag_sort = true;
}

//组件选择功能
void func_compo_select_sub(void)
{
    printf("%s\n", __func__);
}
