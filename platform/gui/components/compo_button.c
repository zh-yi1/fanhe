#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

/**
 * @brief 创建一个按钮
 * @param[in] frm : 窗体指针
 * @return 返回按钮指针
 **/
compo_button_t *compo_button_create(compo_form_t *frm)
{
    compo_button_t *btn = compo_create(frm, COMPO_TYPE_BUTTON);
    widget_page_t *page = widget_page_create(frm->page_body);
    btn->widget = page;
#if BOX_GUI_ROTATE_DISP
    btn->active = true;
    btn->ble_needed = false;
#endif // BOX_GUI_ROTATE_DISP
    return btn;
}

/**
 * @brief 根据图像创建一个按钮
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图像资源的地址
 * @return 返回按钮指针
 **/
compo_button_t *compo_button_create_by_image(compo_form_t *frm, u32 res_addr)
{
    compo_button_t *btn = compo_create(frm, COMPO_TYPE_BUTTON);
#if BOX_GUI_ROTATE_DISP
    widget_image_t *img = widget_image_create(frm->page_body, res_addr);
    btn->widget = img;

    btn->active = true;
    btn->ble_needed = false;
#else
    widget_icon_t *img = widget_icon_create(frm->page_body, res_addr);
    btn->widget = img;
#endif // BOX_GUI_ROTATE_DISP

    return btn;
}

/**
 * @brief 设置按钮坐标及大小
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] btn : 按钮指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 按钮宽度
 * @param[in] height : 按钮高度
 **/
void compo_button_set_location(compo_button_t *btn, s16 x, s16 y, s16 width, s16 height)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_location(btn->widget, x, y, width, height);
#else
    widget_set_location(btn->widget, x, y, width, height);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置按钮坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] btn : 按钮指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_button_set_pos(compo_button_t *btn, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(btn->widget, x, y);
#else
    widget_set_pos(btn->widget, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置按钮大小
 * @param[in] btn : 按钮指针
 * @param[in] width : 宽度
 * @param[in] height : 高度
 **/
void compo_button_set_size(compo_button_t *btn, s16 width, s16 height)
{
    rotate_widget_set_size(btn->widget, width, height);
}

/**
 * @brief 设置按钮触摸区域
          注意：默认使用widgetd的location，赋值后优先使用触摸区域
 * @param[in] btn : 按钮指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 触摸宽度
 * @param[in] height : 触摸高度
 **/
void compo_button_set_touch_rect(compo_button_t *btn, s16 x, s16 y, s16 width, s16 height)
{
    btn->rect.x = x;
    btn->rect.y = y;
    btn->rect.wid = width;
    btn->rect.hei = height;
}
/**
 * @brief 设置按钮透明度
 * @param[in] btn : 按钮指针
 * @param[in] alpha : 透明度
 **/
void compo_button_set_alpha(compo_button_t *btn, u8 alpha)
{
    widget_set_alpha(btn->widget, alpha);
}

/**
 * @brief 设置按键是否可见
 * @param[in] btn : 按钮指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_button_set_visible(compo_button_t *btn, bool visible)
{
    widget_set_visible(btn->widget, visible);
}

/**
 * @brief 更换按钮资源地址
 * @param[in] btn : 按钮指针
 * @param[in] addr : 图像资源的地址
 **/
void compo_button_set_bgimg(compo_button_t *btn, u32 addr)
{
#if BOX_GUI_ROTATE_DISP
    if (!widget_is_page(btn->widget)) {
        widget_image_set(btn->widget, addr);
    }
#else
    widget_icon_set(btn->widget, addr);
#endif // BOX_GUI_ROTATE_DISP
}


/**
 * @brief 设置按键激活状态（设为失效时自动降低透明度）
 * @param[in] btn : 按钮指针
 * @param[in] active : true  激活
                        false 失效
 **/
void compo_button_set_active(compo_button_t *btn, bool active)
{
    btn->active = active;
    if (active) {
        widget_set_alpha(btn->widget, 255);
    } else {
        widget_set_alpha(btn->widget, COMPO_BUTTON_INACTIVE_ALPHA);
    }
}

/**
 * @brief 设置按钮是否依赖于ble连接
 * @param[in] btn : 按钮指针
 * @param[in] needed : true  依赖于ble连接
                        false 不依赖于ble连接
 **/
void compo_button_set_ble_needed(compo_button_t *btn, bool needed)
{
    btn->ble_needed = needed;
}

/**
 * @brief 设置按钮图片旋转角度
 * @param[in] btn : 按钮指针
 * @param[in] angle : 旋转角度（0-3600）
 **/
void compo_button_set_rotation(compo_button_t *btn, s16 angle)
{
    if (!widget_is_page(btn->widget)) {
        widget_image_set_rotation(btn->widget, angle);
    }
}
