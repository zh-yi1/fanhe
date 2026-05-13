#ifndef _COMPO_BUTTON_H
#define _COMPO_BUTTON_H

#define COMPO_BUTTON_INACTIVE_ALPHA  130 //按钮未激活时透明度

typedef struct compo_button_t_ {
    COMPO_STRUCT_COMMON;
    widget_t *widget;
    rect_t rect;        //触摸区域，未赋值时才使用widgetd的location
    bool active;        //未激活状态不可点击
    bool ble_needed;    //ble连接后才能点击，否则跳转到提示页面
} compo_button_t;


/**
 * @brief 创建一个按钮
 * @param[in] frm : 窗体指针
 * @return 返回按钮指针
 **/
compo_button_t *compo_button_create(compo_form_t *frm);

/**
 * @brief 根据图像创建一个按钮
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图像资源的地址
 * @return 返回按钮指针
 **/
compo_button_t *compo_button_create_by_image(compo_form_t *frm, u32 res_addr);

/**
 * @brief 设置按钮坐标及大小
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] btn : 按钮指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 按钮宽度
 * @param[in] height : 按钮高度
 **/
void compo_button_set_location(compo_button_t *btn, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 设置按钮坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] btn : 按钮指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_button_set_pos(compo_button_t *btn, s16 x, s16 y);

/**
 * @brief 设置按钮大小
 * @param[in] btn : 按钮指针
 * @param[in] width : 宽度
 * @param[in] height : 高度
 **/
void compo_button_set_size(compo_button_t *btn, s16 width, s16 height);

/**
 * @brief 设置按钮触摸区域
          注意：默认使用widgetd的location，赋值后优先使用触摸区域
 * @param[in] btn : 按钮指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 触摸宽度
 * @param[in] height : 触摸高度
 **/
void compo_button_set_touch_rect(compo_button_t *btn, s16 x, s16 y, s16 width, s16 height);


/**
 * @brief 设置按钮透明度
 * @param[in] btn : 按钮指针
 * @param[in] alpha : 透明度
 **/
void compo_button_set_alpha(compo_button_t *btn, u8 alpha);

/**
 * @brief 设置按键是否可见
 * @param[in] btn : 按钮指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_button_set_visible(compo_button_t *btn, bool visible);

/**
 * @brief 设置按键是否可见
 * @param[in] btn : 按钮指针
 * @param[in] addr : 图像资源的地址
 **/
void compo_button_set_bgimg(compo_button_t *btn, u32 addr);

/**
 * @brief 设置按键激活状态（设为未激活时自动降低透明度）
 * @param[in] btn : 按钮指针
 * @param[in] active : true  激活
                        false 未激活
 **/
void compo_button_set_active(compo_button_t *btn, bool active);

/**
 * @brief 设置按钮是否依赖于ble连接
 * @param[in] btn : 按钮指针
 * @param[in] needed : true  依赖于ble连接
                        false 不依赖于ble连接
 **/
void compo_button_set_ble_needed(compo_button_t *btn, bool needed);

/**
 * @brief 设置按钮图片旋转角度
 * @param[in] btn : 按钮指针
 * @param[in] angle : 旋转角度（0-3600）
 **/
void compo_button_set_rotation(compo_button_t *btn, s16 angle);
#endif
