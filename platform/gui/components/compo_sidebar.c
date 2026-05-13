#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SIDEBAR_PAGE_HEIGHT 1082  //长图总高度
#define SIDEBAR_DRAG_OFFS   80  //拖动页面超出边界允许值
#define SIDEBAR_Y_MIN       (-SIDEBAR_PAGE_HEIGHT + GUI_SCREEN_HEIGHT) //底部pos_y值
#define SIDEBAR_Y_MAX       0  //顶部pos_y值
#define SIDEBAR_DRAG_MIN    (SIDEBAR_Y_MIN - SIDEBAR_DRAG_OFFS)  //拖动超出底部时的pos_y值
#define SIDEBAR_DRAG_MAX    (SIDEBAR_Y_MAX + SIDEBAR_DRAG_OFFS)  //拖动超出顶部时的pos_y值
#define SIDEBAR_BTN_CNT     ((int)((sizeof(table_btn_jump) / sizeof(table_btn_jump[0]))))  //按钮数量

//按钮跳转信息
static const sidebar_btn_jump_t table_btn_jump[] = {
    {SIDEBAR_BTN_ID_WEATHER,    FUNC_WEATHER,       245, 126, 130, 150},
    {SIDEBAR_BTN_ID_TIMER,      FUNC_TIMER,         160, 296, 308, 156},
    {SIDEBAR_BTN_ID_STOPWATCH,  FUNC_STOPWATCH,     160, 466, 308, 156},
    {SIDEBAR_BTN_ID_CALCULATOR, FUNC_CALCULATOR,    81,  631, 146, 146},
    {SIDEBAR_BTN_ID_CALENDAR,   FUNC_CALENDAER,     241, 631, 146, 146},
    {SIDEBAR_BTN_ID_GAME,       FUNC_GAME,          81,  791, 146, 146},
    {SIDEBAR_BTN_ID_BREATHE,    FUNC_BREATHE,       241, 791, 146, 146},
    {SIDEBAR_BTN_ID_LATEST0,    0,                  60,  979, 72,  72},
    {SIDEBAR_BTN_ID_LATEST1,    0,                  160, 979, 72,  72},
    {SIDEBAR_BTN_ID_LATEST2,    0,                  260, 979, 72,  72},
};

////////////////////////////////////////////////[todo]////////////////////////////////////////////////////
#define TIMER_HOUR          (sys_cb.timer_left_sec / 3600)  //计时器数据
#define TIMER_MIN           ((sys_cb.timer_left_sec % 3600) / 60)
#define TIMER_SEC           (sys_cb.timer_left_sec % 60)
#define STOPWATCH_HOUR      (sys_cb.stopwatch_total_msec / 1000 / 3600)  //秒表数据
#define STOPWATCH_MIN       (((sys_cb.stopwatch_total_msec / 1000) % 3600) / 60)
#define STOPWATCH_SEC       ((sys_cb.stopwatch_total_msec / 1000) % 60)

//根据序号获取最近任务序号（idx=0为最近，无任务返回0）
static u8 sidebar_get_latest_func(u8 idx)
{
    u8 func_sta = 0;
    switch (idx) {
    case 0:
        func_sta = FUNC_SLEEP;
        break;
    case 1:
        func_sta = FUNC_SPORT;
        break;
    case 2:
        func_sta = FUNC_HEARTRATE;
        break;
    default:
        func_sta = FUNC_CLOCK;
        break;
    }
    return func_sta;
}

//根据序号获取最近任务图标资源地址
static u32 sidebar_get_latest_icon(u8 idx)
{
    u32 addr = 0;
    switch (idx) {
    case 0:
        addr = UI_BUF_ICON_SLEEP_BIN;
        break;
    case 1:
        addr = UI_BUF_ICON_SPORT_BIN;
        break;
    case 2:
        addr = UI_BUF_ICON_HEART_RATE_BIN;
        break;
    }
    return addr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 创建图像控件
 * @param[in] page : 页面指针
 * @param[in] res_addr : 资源地址
 * @param[in] x, y : 图像中心所在位置
 * @param[in] wid, hei : 图像宽高（有0保持原大小）
 * @param[in] save_ptr : 存储图像控件指针地址（空指针不存）
 **/
static void compo_sidebar_img_create(widget_page_t *page, u32 res_addr, s16 x, s16 y, s16 wid, s16 hei, widget_image_t **save_ptr)
{
    widget_image_t *img = widget_image_create(page, res_addr);
    widget_set_pos(img, x, y);
    if (wid && hei) {
        widget_set_size(img, wid, hei);
    }
    if (save_ptr) {
        *save_ptr = img;
    }
}

/**
 * @brief 裁剪图像（保留原size）
 * @param[in] img : 图像控件
 * @param[in] index : 裁剪序号
 * @param[in] total_cnt : 分割总数
 **/
static void compo_sidebar_img_cut(widget_image_t *img, u8 index, u8 total_cnt)
{
	if (total_cnt) {
	    rect_t rect = widget_get_location(img);
        area_t area = widget_image_get_size(img);
        widget_image_cut(img, 0, index * (area.hei / total_cnt), area.wid, area.hei / total_cnt);
        widget_set_size(img, rect.wid, rect.hei);
	}
}

/**
 * @brief 创建单个数字图像控件
 * @param[in] page : 页面指针
 * @param[in] res_addr : 长图资源地址
 * @param[in] wid, hei : 单个数字宽高
 * @param[in] x, y : 图像中心所在位置
 * @param[in] save_ptr : 存储图像控件指针地址（空指针不存）
 **/
static void compo_sidebar_num_img_create(widget_page_t *page, u32 res_addr, s16 wid, s16 hei, s16 x, s16 y, widget_image_t **save_ptr)
{
    widget_image_t *img = widget_image_create(page, res_addr);
    widget_image_cut(img, 0, 0, wid, hei);
    widget_set_size(img, wid, hei);
    widget_set_pos(img, x, y);
    if (save_ptr) {
        *save_ptr = img;
    }
}

/**
 * @brief 创建文本控件
 * @param[in] page : 页面指针
 * @param[in] content : 文本内容指针
 * @param[in] word_cnt : 显示最多字符个数
 * @param[in] x, y : 文本中心所在位置
 * @param[in] save_ptr : 存储文本控件指针地址（空指针不存）
 **/
static void compo_sidebar_txt_create(widget_page_t *page, const char* content, u16 word_cnt, s16 x, s16 y, widget_text_t **save_ptr)
{
    widget_text_t *text = widget_text_create(page, word_cnt);
    widget_set_pos(text, x, y);
    widget_text_set(text, content);
    widget_text_set_autosize(text, true);
    if (save_ptr) {
        *save_ptr = text;
    }
}

/**
 * @brief 设置连续存储的数字图像
 * @param[in] num_img : 数字图像指针数组首地址
 * @param[in] num_count : 数字个数
 * @param[in] value : 数值（十进制）
 * @param[in] zfill : 高位是否补零
 **/
static void compo_sidebar_num_set(widget_image_t **num_img, s8 num_count, u32 value, bool zfill)
{
    u8 value_bit = 0;
    rect_t rect = widget_get_location(num_img[0]);
    while (--num_count >= 0) {
        widget_set_visible(num_img[num_count], true);
        if (value) {
            value_bit = value % 10;
            value /= 10;
            widget_image_cut(num_img[num_count], 0, value_bit * rect.hei, rect.wid, rect.hei);
        } else if (zfill){
            widget_image_cut(num_img[num_count], 0, 0, rect.wid, rect.hei);
        } else {
            widget_set_visible(num_img[num_count], false);
        }
    }
}

/**
 * @brief 创建一个右滑组件
 * @param[in] frm : 窗体指针
 * @return 返回右滑组件指针
 **/
compo_sidebar_t *compo_sidebar_create(compo_form_t *frm)
{
    compo_sidebar_t *sidebar = compo_create(frm, COMPO_TYPE_SIDEBAR);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    widget_page_set_client(page, 0, 0);
    sidebar->page = page;
    sidebar->ofs_y_min = SIDEBAR_Y_MIN;
    sidebar->ofs_y_max = SIDEBAR_Y_MAX;

    //创建背景图&图标
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_BG_308_156_BIN, 160, 126, 0, 0, NULL);  //时间天气方框
    compo_sidebar_img_create(page, UI_BUF_WEATHER_WEATHER_LIST_BIN, 245, 111, 96, 88, &sidebar->icon_weather);  //天气
    compo_sidebar_img_create(page, UI_BUF_COMMON_COLON_NUM_30_46_BIN, 95, 93, 0, 0, NULL);  //时间冒号
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_SLAS_BIN, 94, 145, 0, 0, NULL);  //日期斜杠
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_DEGREE_BIN, 263, 175, 0, 0, NULL);  //摄氏度
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_BG_308_156_BIN, 160, 296, 0, 0, NULL);  //计时器方框
    compo_sidebar_img_create(page, UI_BUF_ICON_TIMER_BIN, 54, 270, 64, 64, NULL);  //计时器
    compo_sidebar_img_create(page, UI_BUF_COMMON_COLON_NUM_16_24_BIN, 160, 330, 0, 0, NULL);  //计时器冒号
    compo_sidebar_img_create(page, UI_BUF_COMMON_COLON_NUM_16_24_BIN, 222, 330, 0, 0, NULL);
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_BG_308_156_BIN, 160, 466, 0, 0, NULL);  //秒表方框
    compo_sidebar_img_create(page, UI_BUF_ICON_STOPWATCH_BIN, 54, 440, 64, 64, NULL);  //秒表
    compo_sidebar_img_create(page, UI_BUF_COMMON_COLON_NUM_16_24_BIN, 160, 500, 0, 0, NULL);  //秒表冒号
    compo_sidebar_img_create(page, UI_BUF_COMMON_COLON_NUM_16_24_BIN, 222, 500, 0, 0, NULL);
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_BG_146_146_BIN, 81, 631, 0, 0, NULL);  //计算器方框
    compo_sidebar_img_create(page, UI_BUF_ICON_CALCULATOR_BIN, 60, 612, 72, 72, NULL);  //计算器
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_BG_146_146_BIN, 241, 631, 0, 0, NULL);  //日历方框
    compo_sidebar_img_create(page, UI_BUF_ICON_CALENDAR_BIN, 220, 612, 72, 72, NULL);  //日历
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_BG_146_146_BIN, 81, 791, 0, 0, NULL);  //游戏方框
    compo_sidebar_img_create(page, UI_BUF_ICON_GAME_BIN, 60, 772, 72, 72, NULL);  //游戏
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_BG_146_146_BIN, 241, 791, 0, 0, NULL);  //呼吸方框
    compo_sidebar_img_create(page, UI_BUF_ICON_BREATHE_BIN, 220, 772, 72, 72, NULL);  //呼吸
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_BG_308_156_BIN, 160, 956, 0, 0, NULL);  //最近应用方框
    compo_sidebar_img_create(page, UI_BUF_SIDEBAR_TOP_308_50_BIN, 160, 903, 0, 0, NULL);  //最近应用方框头
    compo_sidebar_img_create(page, sidebar_get_latest_icon(0), 60, 979, 72, 72, &sidebar->icon_latest[0]);  //最近应用，只在创建时设置图片
    compo_sidebar_img_create(page, sidebar_get_latest_icon(1), 160, 979, 72, 72, &sidebar->icon_latest[1]);
    compo_sidebar_img_create(page, sidebar_get_latest_icon(2), 260, 979, 72, 72, &sidebar->icon_latest[2]);
    //创建数字
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_30_46_BIN, 30, 46, 37, 93, &sidebar->num_time[0]);  //时间hour1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_30_46_BIN, 30, 46, 67, 93, &sidebar->num_time[1]);  //时间hour0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_30_46_BIN, 30, 46, 123, 93, &sidebar->num_time[2]);  //时间min1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_30_46_BIN, 30, 46, 153, 93, &sidebar->num_time[3]);  //时间min0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_16_24_BIN, 16, 24, 60, 145, &sidebar->num_date[0]);  //月1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_16_24_BIN, 16, 24, 78, 145, &sidebar->num_date[1]);  //月0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_16_24_BIN, 16, 24, 110, 145, &sidebar->num_date[2]);  //日1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_16_24_BIN, 16, 24, 128, 145, &sidebar->num_date[3]);  //日0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_16_24_BIN, 16, 24, 219, 175, &sidebar->temperature[0]);  //温度1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_16_24_BIN, 16, 24, 237, 175, &sidebar->temperature[1]);  //温度0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 116, 330, &sidebar->num_timer[0]);  //计时器hour1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 142, 330, &sidebar->num_timer[1]);  //计时器hour0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 178, 330, &sidebar->num_timer[2]);  //计时器min1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 204, 330, &sidebar->num_timer[3]);  //计时器min0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 240, 330, &sidebar->num_timer[4]);  //计时器sec1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 266, 330, &sidebar->num_timer[5]);  //计时器sec0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 116, 500, &sidebar->num_stopwatch[0]);  //秒表hour1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 142, 500, &sidebar->num_stopwatch[1]);  //秒表hour0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 178, 500, &sidebar->num_stopwatch[2]);  //秒表min1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 204, 500, &sidebar->num_stopwatch[3]);  //秒表min0
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 240, 500, &sidebar->num_stopwatch[4]);  //秒表sec1
    compo_sidebar_num_img_create(page, UI_BUF_COMMON_NUM_24_38_BIN, 24, 38, 266, 500, &sidebar->num_stopwatch[5]);  //秒表sec0
    //创建文本
    compo_sidebar_txt_create(page, NULL, 8, 92, 181, &sidebar->txt_weekday);  //星期
    compo_sidebar_txt_create(page, i18n[STR_TIMER], 8, 140, 270, NULL);  //计时器
    compo_sidebar_txt_create(page, i18n[STR_STOP_WATCH], 8, 129, 440, NULL);  //秒表
    compo_sidebar_txt_create(page, i18n[STR_CALCULATOR], 8, 98, 673, NULL);  //计算器
    compo_sidebar_txt_create(page, i18n[STR_CALENDAR], 8, 273, 673, NULL);  //日历
    compo_sidebar_txt_create(page, i18n[STR_GAME], 8, 113, 833, NULL);  //游戏
    compo_sidebar_txt_create(page, i18n[STR_BREATHE], 8, 273, 833, NULL);  //呼吸
    compo_sidebar_txt_create(page, i18n[STR_LATEST_APP], 8, 160, 903, NULL);  //最近应用

    sidebar->m_time_min = -1;
    sidebar->m_timer_sec = -1;
    sidebar->m_stopwatch_sec = -1;

    return sidebar;
}

/**
 * @brief 刷新右滑组件Widget（时间、温度、图标等）
 * @param[in] sidebar : 右滑组件指针
 **/
void compo_sidebar_update(compo_sidebar_t *sidebar)
{
    u32 time_dec;

    if (sidebar == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_UPDATE);
    }
    //时间，天气图标，温度
    if (sidebar->m_time_min != compo_cb.tm.min) { //一分钟更新一次
        time_dec = compo_cb.tm.hour * 100 + compo_cb.tm.min;
        compo_sidebar_num_set(sidebar->num_time, 4, time_dec, true);
        compo_sidebar_img_cut(sidebar->icon_weather, sys_cb.weather_idx, WEATHER_CNT);
        compo_sidebar_num_set(sidebar->temperature, 2, sys_cb.temperature[1], true);
        //日期，星期
        if (sidebar->m_time_min <= 0) { //一小时更新一次
            time_dec = compo_cb.tm.mon * 100 + compo_cb.tm.day;
            compo_sidebar_num_set(sidebar->num_date, 4, time_dec, true);
            widget_text_set(sidebar->txt_weekday, i18n[STR_SUNDAY + compo_cb.tm.weekday]);
        }
        sidebar->m_time_min = compo_cb.tm.min;
    }
    //计时器
    if (sidebar->m_timer_sec != TIMER_SEC) {
        time_dec = TIMER_HOUR * 10000 + TIMER_MIN * 100 + TIMER_SEC;
        compo_sidebar_num_set(sidebar->num_timer, 6, time_dec, true);
        sidebar->m_timer_sec = TIMER_SEC;
    }
    //秒表
    if (sidebar->m_stopwatch_sec != STOPWATCH_SEC) {
        time_dec = STOPWATCH_HOUR * 10000 + STOPWATCH_MIN * 100 + STOPWATCH_SEC;
        compo_sidebar_num_set(sidebar->num_stopwatch, 6, time_dec, true);
        sidebar->m_stopwatch_sec = STOPWATCH_SEC;
    }
}

/**
 * @brief 按Y坐标设置右滑组件偏移位置
 * @param[in] sidebar : 右滑组件指针
 * @param[in] y : Y坐标
 **/
void compo_sidebar_set_offs(compo_sidebar_t *sidebar, s16 y)
{
    if (sidebar == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SET_FOCUS);
    }

    y = MIN(SIDEBAR_DRAG_MAX, MAX(SIDEBAR_DRAG_MIN, y));
    sidebar->ofs_y = y;
    widget_page_set_client(sidebar->page, 0, sidebar->ofs_y);
}

/**
 * @brief 将坐标对齐到最临近的点
 * @param[in] sidebar : 右滑组件指针
 * @param[in] y : Y坐标
 * @return 对齐y坐标
 **/
s16 compo_sidebar_align_y(compo_sidebar_t *sidebar, s16 y)
{
    //暂时不做对齐，只做两端限制
    y = MIN(SIDEBAR_DRAG_MAX, MAX(SIDEBAR_DRAG_MIN, y));
    return y;
}

/**
 * @brief 按坐标选择跳转任务序号
 * @param[in] sidebar : 右滑组件指针
 * @param[in] pt : 坐标
 * @return 返回要跳转的任务序号（无匹配返回0）
 **/
u8 compo_sidebar_select(compo_sidebar_t *sidebar)
{
    int i = 0;
    u8 func_sta = 0;
    sidebar_btn_jump_t btn_cur;
    point_t pt = ctp_get_sxy();
    for(; i<SIDEBAR_BTN_CNT; i++) {
        btn_cur = table_btn_jump[i];
        if (abs_s(pt.x - btn_cur.x) * 2 <= btn_cur.wid && abs_s(pt.y - (btn_cur.y + sidebar->ofs_y)) * 2 <= btn_cur.hei) {
            if (btn_cur.id < SIDEBAR_BTN_ID_LATEST0) {
                func_sta = btn_cur.func_sta;
            } else {
                func_sta = sidebar_get_latest_func(btn_cur.id - SIDEBAR_BTN_ID_LATEST0);
            }
            TRACE("CLICK_ID:%d [%d]\n", btn_cur.id, func_sta);
            break;
        }
    }
    return func_sta;
}
