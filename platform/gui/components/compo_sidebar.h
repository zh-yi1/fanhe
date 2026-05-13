#ifndef _COMPO_SIDEBAR_H
#define _COMPO_SIDEBAR_H

//按钮
enum{
    SIDEBAR_BTN_ID_WEATHER = 1,
    SIDEBAR_BTN_ID_TIMER,
    SIDEBAR_BTN_ID_STOPWATCH,
    SIDEBAR_BTN_ID_CALCULATOR,
    SIDEBAR_BTN_ID_CALENDAR,
    SIDEBAR_BTN_ID_GAME,
    SIDEBAR_BTN_ID_BREATHE,
    SIDEBAR_BTN_ID_LATEST0,
    SIDEBAR_BTN_ID_LATEST1,
    SIDEBAR_BTN_ID_LATEST2,
};

//按钮跳转信息结构体
typedef struct sidebar_btn_jump_t_ {
    u8 id;
    u8 func_sta;
    s16 x;
    s16 y;
    s16 wid;
    s16 hei;
} sidebar_btn_jump_t;

typedef struct compo_sidebar_t_ {
    COMPO_STRUCT_COMMON;
    widget_page_t *page;
    s16 ofs_y;
    s16 ofs_y_min;
    s16 ofs_y_max;
    widget_image_t *num_time[4];
    widget_image_t *num_date[4];
    widget_image_t *icon_weather;
    widget_image_t *temperature[2];
    widget_image_t *num_timer[6];
    widget_image_t *num_stopwatch[6];
    widget_text_t *txt_weekday;
    widget_image_t *icon_latest[3];
    s8 m_time_min;  //时间记录，判断是否更新控件
    s8 m_timer_sec;
    s8 m_stopwatch_sec;
} compo_sidebar_t;

/**
 * @brief 创建一个右滑组件
 * @param[in] frm : 窗体指针
 * @return 返回右滑组件指针
 **/
compo_sidebar_t *compo_sidebar_create(compo_form_t *frm);

/**
 * @brief 按Y坐标设置右滑组件偏移位置
 * @param[in] sidebar : 右滑组件指针
 * @param[in] y : Y坐标
 **/
void compo_sidebar_set_offs(compo_sidebar_t *sidebar, s16 y);

/**
 * @brief 将坐标对齐到最临近的点
 * @param[in] sidebar : 图标集指针
 * @param[in] y : Y坐标
 * @return 对齐y坐标
 **/
s16 compo_sidebar_align_y(compo_sidebar_t *sidebar, s16 y);

/**
 * @brief 刷新右滑组件Widget（时间、温度、图标等）
 * @param[in] sidebar : 右滑组件指针
 **/
void compo_sidebar_update(compo_sidebar_t *sidebar);

/**
 * @brief 按坐标选择跳转任务序号
 * @param[in] sidebar : 右滑组件指针
 * @return 返回要跳转的任务序号
 **/
u8 compo_sidebar_select(compo_sidebar_t *sideba);

#endif
