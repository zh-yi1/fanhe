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

//根据vidx信息获取list_data
static const compo_listbox_item_t *get_tbl_list_data_by_vidx(u16 vidx)
{
    for (u8 i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (vidx == tbl_list_data[i].vidx) {
            return (compo_listbox_item_t *)&tbl_list_data[i];
        }
    }

    return NULL;
}

//根据排序表更新list_data
static void func_compo_list_data_update(void)
{
    f_compo_select_sub_t *f_compo_select_sub = (f_compo_select_sub_t *)func_cb.f_cb;
    compo_listbox_item_t *p_list_data = f_compo_select_sub->p_list_data;
    int i;

    for (i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (list_data_sort[i] != p_list_data[i].vidx) {
            memcpy(&p_list_data[i], get_tbl_list_data_by_vidx(list_data_sort[i]), sizeof(compo_listbox_item_t));
        }
    }
}

//获取已添加数量
static u8 list_data_sort_get_add_cnt(void)
{
    int i;
    u8 cnt = 0;

    for (i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (bsp_sys_get_ctlbit(list_data_sort[i])) {
            cnt ++;
        } else {
            break;
        }
    }

    return cnt;
}

//添加
static void list_data_sort_add(u8 vidx)
{
    u8 list_data_sort_tmp[LIST_ITEM_CNT_MAX] = {0};
    u8 i = 0, index = 0;
    u8 add_cnt = list_data_sort_get_add_cnt();

    if (add_cnt) {
        memcpy(list_data_sort_tmp, list_data_sort, add_cnt * sizeof(list_data_sort_tmp[0]));
    }
    list_data_sort_tmp[add_cnt] = vidx;
    index = add_cnt + 1;
    for (i = add_cnt; i < LIST_ITEM_CNT_MAX; i++) {
        if (vidx != list_data_sort[i]) {
            list_data_sort_tmp[index ++] = list_data_sort[i];
        }
    }

    memcpy(list_data_sort, list_data_sort_tmp, sizeof(list_data_sort));
    func_compo_list_data_update();
}

//删除
static void list_data_sort_del(u8 vidx)
{
    u8 list_data_sort_tmp[LIST_ITEM_CNT_MAX] = {0};
    u8 i = 0, index = LIST_ITEM_CNT_MAX - 1;

    list_data_sort_tmp[index --] = vidx;
    for (i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (vidx != list_data_sort[LIST_ITEM_CNT_MAX - 1 - i]) {
            list_data_sort_tmp[index --] = list_data_sort[LIST_ITEM_CNT_MAX - 1 - i];
        }
    }

    memcpy(list_data_sort, list_data_sort_tmp, sizeof(list_data_sort));
    func_compo_list_data_update();
}

//向上
static void list_data_sort_up(u8 vidx)
{
    u8 pos = 0;
    u8 add_cnt = list_data_sort_get_add_cnt();

    if (add_cnt < 2) {
        return ;
    }

    for (u8 i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (list_data_sort[i] == vidx) {
            if (i == 0) {
                pos = add_cnt - 1;
            } else {
                pos = i - 1;
            }
            u8 vidx_tmp = list_data_sort[pos];
            list_data_sort[pos] = vidx;
            list_data_sort[i] = vidx_tmp;
            break;
        }
    }

    func_compo_list_data_update();
}


//创建组件选择窗体
compo_form_t *func_compo_select_sub_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_COMPO_SELECT]);

    //新建菜单列表
    compo_listbox_t *listbox = compo_listbox_create(frm, COMPO_LISTBOX_STYLE_TITLE_NORMAL);
    compo_setid(listbox, COMPO_ID_LISTBOX);
    compo_listbox_set_item_width(listbox, 412);
    compo_listbox_set_item_height(listbox, 134);

    if (LIST_ITEM_CNT_MAX > (MAX_FUNC_SORT_CNT - 2) || LIST_ITEM_CNT_MAX != sizeof(SYS_CTL_ON_TO_FUNC_STA_TABLE) / 2) {
        printf("%s:%d,err\n", __func__, __LINE__);
        halt(HALT_GUI_COMPO_LISTBOX_CREATE);
    }

    //控制位未加载时进行初始化
    bool bit_init = true;
    for (uint32_t i = SYS_CTL_FUNC_SPORT_ON; i <= SYS_CTL_FUNC_SETTINGS_ON; i++) {
        if (bsp_sys_get_ctlbit(i)) {
            bit_init = false;
            break;
        }
    }
    if (bit_init) {
        for (u8 i = 0; i < 5; i++) {
            bsp_sys_set_ctlbit(list_data_sort[i], true);
        }
    }
    compo_listbox_set_bithook(listbox, bsp_sys_get_ctlbit);

    return frm;
}

//组件选择功能事件处理
static void func_compo_select_sub_process(void)
{
    compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    compo_listbox_move(listbox);
    func_process();
}

//单击按钮
static void func_compo_select_sub_click(void)
{
    compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    f_compo_select_sub_t *f_compo_select_sub = (f_compo_select_sub_t *)func_cb.f_cb;

    bool del = false;
    point_t cur_point = ctp_get_sxy();
    if (cur_point.x < 180) {
        return ;
    }
    if (cur_point.x >= 250) {
        del = true;
    }

    int idx = compo_listbox_select(listbox, cur_point);
    if (idx < 0) {
        return ;
    }
    u8 vidx = f_compo_select_sub->p_list_data[idx].vidx;

    bool bit_sta = bsp_sys_get_ctlbit(vidx);
    if (!bit_sta) {
        list_data_sort_add(vidx);
        bsp_sys_reverse_ctlbit(vidx);
    } else {
        if (del) {
            list_data_sort_del(vidx);
            bsp_sys_reverse_ctlbit(vidx);
        } else {
            list_data_sort_up(vidx);
        }
    }

    compo_listbox_update_with_text_scroll_rst(listbox);
}

//组件选择功能消息处理
static void func_compo_select_sub_message(size_msg_t msg)
{
    compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);
    if (compo_listbox_message(listbox, msg)) {
        return;                                         //处理列表框信息
    }

    switch (msg) {
    case MSG_CTP_CLICK:
        func_compo_select_sub_click();
        break;

    case MSG_CTP_SHORT_UP:
        break;

    case MSG_CTP_SHORT_DOWN:
        break;

    case MSG_CTP_LONG:
        break;

    default:
        func_message(msg);
        break;
    }
}

//进入组件选择功能
void func_compo_select_sub_enter(void)
{
    // func_cb.f_cb = func_zalloc(sizeof(f_compo_select_sub_t));
    // func_cb.frm_main = func_compo_select_sub_form_create();

    // f_compo_select_sub_t *f_compo_select_sub = (f_compo_select_sub_t *)func_cb.f_cb;
    // f_compo_select_sub->p_list_data = func_zalloc(LIST_ITEM_CNT_MAX * sizeof(compo_listbox_item_t));
    // if (NULL == f_compo_select_sub->p_list_data) {
    //     printf("%s:%d,err\n", __func__, __LINE__);
    //     halt(HALT_GUI_COMPO_LISTBOX_CREATE);
    // }
    // func_compo_list_data_update();

    // compo_listbox_t *listbox = compo_getobj_byid(COMPO_ID_LISTBOX);

    // compo_listbox_set(listbox, f_compo_select_sub->p_list_data, LIST_ITEM_CNT_MAX);
    // compo_listbox_set_item_text(listbox, 150, 45, 160, 50, false);
    // u8 menu_idx = func_cb.menu_idx;
    // if (menu_idx < 1) {
    //     menu_idx = 1;
    // }
    // compo_listbox_set_focus(listbox, 140);
    // compo_listbox_set_sta_icon(listbox, UI_BUF_COMPO_SELECT_ADD_DEL_BIN, UI_BUF_COMPO_SELECT_ADD_BIN);
    // compo_listbox_update(listbox);

    // compo_listbox_move(listbox);
    // listbox->mcb = &f_compo_select_sub->mcb;
    // compo_listbox_move_init_modify(listbox, listbox->ofs_y, compo_listbox_gety_byidx(listbox, LIST_ITEM_CNT_MAX - 2));
}

//退出组件选择功能
void func_compo_select_sub_exit(void)
{
    func_cb.last = FUNC_COMPO_SELECT_SUB;

    f_compo_select_sub_t *f_compo_select_sub = (f_compo_select_sub_t *)func_cb.f_cb;
    func_free(f_compo_select_sub->p_list_data);

    u8 index = 1;
    for (u8 i = 0; i < LIST_ITEM_CNT_MAX; i++) {
        if (bsp_sys_get_ctlbit(list_data_sort[i])) {
            for (u8 j = 0; j < sizeof(SYS_CTL_ON_TO_FUNC_STA_TABLE) / 2; j++) {
                if (SYS_CTL_ON_TO_FUNC_STA_TABLE[2 * j] == list_data_sort[i]) {
                    func_cb.tbl_sort[index ++] = SYS_CTL_ON_TO_FUNC_STA_TABLE[2 * j + 1];
                    break;
                }
            }
        }
    }
    func_cb.tbl_sort[index ++] = FUNC_COMPO_SELECT;
    func_cb.sort_cnt = index;
    func_cb.flag_sort = true;
}

//组件选择功能
void func_compo_select_sub(void)
{
    printf("%s\n", __func__);
    func_compo_select_sub_enter();
    while (func_cb.sta == FUNC_COMPO_SELECT_SUB) {
        func_compo_select_sub_process();
        func_compo_select_sub_message(msg_dequeue());
    }
    func_compo_select_sub_exit();
}
