#ifndef _COMPONENT_FUNC_H
#define _COMPONENT_FUNC_H

#define ID_NULL             0

enum {
    COMPO_ROLL_STA_IDLE,
    COMPO_ROLL_STA_ROLL,
    COMPO_ROLL_STA_STOP,
};

//滚动控制结构体
typedef struct compo_roll_cb_t_ {
    u32 tick;
    s16 offset;
    u8 sta;
    s8 direction;
    u8 mode;
    bool autoroll;
    bool is_drag;
} compo_roll_cb_t;

/**
 * @brief 根据ID获取组件对象
 * @param[in] id : 组件的ID
 * @return 返回组件的指针
 **/
void *compo_getobj_byid(u16 id);

/**
 * @brief 根据ID获取组件对象
 * @param[in] id : 组件的ID
 * @return 返回组件的指针
 **/
void *compo_getobj_bytype(u8 type);

/**
 * @brief 更新组件时间等信息
 **/
void compo_update(void);

/**
 * @brief 设置组件时间
 * @param[in] tm : 时间tm结构体
 * @param[in] mtime : 毫秒
 **/
void compo_set_update(tm_t tm, u16 mtime);

/**
 * @brief 获取按键ID
 **/
int compo_get_button_id(void);

#endif
