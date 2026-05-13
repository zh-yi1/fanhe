#ifndef _COMPO_FORM_H
#define _COMPO_FORM_H

#define COMPO_FORM_MODE_SHOW_TITLE              BIT(0)          //标题栏显示文字
#define COMPO_FORM_MODE_SHOW_TIME               BIT(1)          //标题栏显示时间
#define COMPO_FORM_MODE_SHOW_ICON               BIT(2)          //标题栏显示自定义状态图标

#include "compo_animation.h"

#if (GUI_SELECT == GUI_OLED_466_ICNA3310B)
#define GUI_PAGE_HEAD_HEIGHT            (GUI_SCREEN_HEIGHT / 6)
#else
#define GUI_PAGE_HEAD_HEIGHT            (GUI_SCREEN_HEIGHT / 8)
#endif

#if BOX_GUI_ROTATE_DISP
#define COMPO_FORM_MODE_SHOW_BAT                BIT(3)          //标题栏显示电量

#define TITLE_BAT_L_ICON                        BIT(0)          //标题栏左耳机图标序号
#define TITLE_BAT_R_ICON                        BIT(1)          //标题栏左耳机图标序号
#define TITLE_BAT_BOX_ICON                      BIT(2)          //标题栏充电仓图标序号
#define TITLE_BAT_L                             BIT(3)          //标题栏左耳机电量序号
#define TITLE_BAT_R                             BIT(4)          //标题栏右耳机电量序号
#define TITLE_BAT_BOX                           BIT(5)          //标题栏充电仓电量序号

#define GUI_PAGE_BODY_HEIGHT            (GUI_GET_SCREEN_HEIGHT - GUI_PAGE_HEAD_HEIGHT)
#define GUI_PAGE_BODY_CENTER_Y          (GUI_PAGE_HEAD_HEIGHT + GUI_PAGE_BODY_HEIGHT / 2)

#else
#define GUI_PAGE_BODY_HEIGHT            (GUI_SCREEN_HEIGHT - GUI_PAGE_HEAD_HEIGHT)
#define GUI_PAGE_BODY_CENTER_Y          (GUI_PAGE_HEAD_HEIGHT + GUI_PAGE_BODY_HEIGHT / 2)
#endif // BOX_GUI_ROTATE_DISP

typedef struct compo_textbox_t_ {
    COMPO_STRUCT_COMMON;
    widget_text_t *txt;
    compo_roll_cb_t roll_cb;
    bool multiline;             //多行
} compo_textbox_t;


typedef struct compo_form_t_ {
    COMPO_STRUCT_COMMON;
    widget_page_t *page;
    widget_page_t *page_body;
    widget_icon_t *icon;
    widget_icon_t *title_icon;
#if BOX_GUI_ROTATE_DISP
    void *bat[6];					//电量
#endif // BOX_GUI_ROTATE_DISP
    compo_textbox_t *title;
    widget_text_t *time;
    int mode;

    widget_axis3d_t *axis;

#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
    compo_animation_t *anim;
#endif
} compo_form_t;

/**
 * @brief 创建窗体
          窗体为其他组件的容器
 * @param[in] flag_top : 是否放在界面的顶层
                         false在底层
                         true在顶层
 * @return 返回窗体指针
 **/
compo_form_t *compo_form_create(bool flag_top);

/**
 * @brief 销毁窗体
 * @param[in] frm : 窗体指针
 **/
void compo_form_destroy(compo_form_t *frm);

/**
 * @brief 设置窗体坐标及大小
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] frm : 窗体指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 窗体宽度
 * @param[in] height : 窗体高度
 **/
void compo_form_set_location(compo_form_t *frm, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 设置窗体坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] frm : 窗体指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_form_set_pos(compo_form_t *frm, s16 x, s16 y);

/**
 * @brief 窗体缩放
          以中心点缩放
 * @param[in] frm : 窗体指针
 * @param[in] width : 缩放后的窗体宽度
 * @param[in] height : 缩放后的窗体高度
 **/
void compo_form_scale_to(compo_form_t *frm, s16 width, s16 height);

/**
 * @brief 窗体设置Alpha
 * @param[in] frm : 窗体指针
 * @param[in] alpha : 透明度
 **/
void compo_form_set_alpha(compo_form_t *frm, u8 alpha);

/**
 * @brief 设置窗体标题栏
          通常和compo_form_set_title（窗体模式）一起调用
 * @param[in] frm : 窗体指针
 * @param[in] title : 标题文本
 **/
void compo_form_set_title(compo_form_t *frm, const char *title);

/**
 * @brief 设置窗体模式
          通常和compo_form_set_title（窗体标题栏）一起调用
 * @param[in] frm : 窗体指针
 * @param[in] mode : COMPO_FORM_MODE_SHOW_TITLE  BIT(0) 标题栏显示文字
                     COMPO_FORM_MODE_SHOW_TIME   BIT(1) 标题栏显示时间
 **/
void compo_form_set_mode(compo_form_t *frm, int mode);

/**
 * @brief 设置窗体标题居中
 * @param[in] frm : 窗体指针
 * @param[in] align_center : 是否居中
 **/
void compo_form_set_title_center(compo_form_t *frm, bool align_center);

/**
 * @brief 窗体中添加图片
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图片资源的地址
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_form_add_image(compo_form_t *frm, u32 res_addr, s16 x, s16 y);

/**
 * @brief 窗体中设置背景
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图片资源的地址
 **/
void compo_form_set_bg(compo_form_t *frm, u32 res_addr);

/**
 * @brief title的图标
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图片资源的地址
 **/
void compo_form_set_title_icon(compo_form_t *frm, u32 res_addr);

#if BOX_GUI_ROTATE_DISP
/**
 * @brief 刷新title元素
 * @param[in] flag : 元素类型 BIT(0):左耳电量充电图标; BIT(1):右耳电量充电图标; BIT(2):充电仓充电图标;
                            BIT(3):左耳电量; BIT(4):右耳电量; BIT(5):充电仓电量;
 **/
void compo_form_update_title_bat(u8 flag);

/**
 * @brief 刷新title蓝牙连接图标
 **/
void compo_form_update_title_connect_icon(void);
#endif // BOX_GUI_ROTATE_DISP

/**
 * @brief 设置窗体frm左右镜像翻转
 * @param[in] page:页面
 * @param[in] flip:是否左右镜像翻转
 * @return 无
 **/
void compo_form_set_flip(compo_form_t *frm, bool flip);

/**
 * @brief 设置窗体frm使用frm_src的3D轴
 * @param[in] frm : 窗体指针
 * @param[in] frm_src : 3D轴源窗体指针
 **/
void compo_form3d_set_axis(compo_form_t *frm, compo_form_t *frm_src);

/**
 * @brief 设置窗体frm的旋转球半径
 * @param[in] page:页面
 * @param[in] r:球半径
 * @return 无
 **/
void compo_form3d_set_r(compo_form_t *frm, s16 r);

/**
 * @brief 设置窗体frm的极角
 * @param[in] page:页面
 * @param[in] angle:极角
 * @return 无
 **/
void compo_form3d_set_polar(compo_form_t *frm, s16 angle);

/**
 * @brief 设置窗体frm的方位角
 * @param[in] page:页面
 * @param[in] angle:方位角
 * @return 无
 **/
void compo_form3d_set_azimuth(compo_form_t *frm, s16 angle);

/**
 * @brief 设置窗体frm的自旋角
 * @param[in] page:页面
 * @param[in] angle:自旋角
 * @return 无
 **/
void compo_form3d_set_rotation(compo_form_t *frm, s16 angle);

/**
 * @brief 设置窗体frm的旋转中心点
 * @param[in] page:页面
 * @param[in] x:相对于x
 * @param[in] y:相对于y
 * @return 无
 **/
void compo_form3d_set_rotation_center(compo_form_t *frm, s16 x, s16 y);

/**
 * @brief 设置窗体3D轴距
 * @param[in] frm : 窗体指针
 * @param[in] distance : 轴距(影响透视效果)
 **/
void compo_form3d_set_axis_distance(compo_form_t *frm, s16 distance);

/**
 * @brief 设置窗体3D轴俯视角
 * @param[in] frm : 窗体指针
 * @param[in] angle : 俯视角(透视角度)
 **/
void compo_form3d_set_axis_overlook(compo_form_t *frm, s16 angle);

/**
 * @brief 设置窗体3D轴的旋转球心坐标
 * @param[in] frm : 窗体指针
 * @param[in] x, y, z:球心坐标
 **/
void compo_form3d_set_axis_pos(compo_form_t *frm, s16 x, s16 y, s16 z);

/**
 * @brief 设置窗体3D轴的球坐标
 * @param[in] frm : 窗体指针
 * @param[in] sph : 轴的球坐标
 **/
void compo_form3d_set_axis_sph(compo_form_t *frm, sph_t sph);

/**
 * @brief 设置窗体3D轴的极角
 * @param[in] frm : 窗体指针
 * @param[in] angle : 轴的极角
 **/
void compo_form3d_set_axis_polar(compo_form_t *frm, s16 angle);

/**
 * @brief 设置窗体3D轴的方位角
 * @param[in] frm : 窗体指针
 * @param[in] angle : 轴的方位角
 **/
void compo_form3d_set_axis_azimuth(compo_form_t *frm, s16 angle);

/**
 * @brief 设置窗体3D轴的自旋角
 * @param[in] frm : 窗体指针
 * @param[in] angle : 轴的自旋角
 **/
void compo_form3d_set_axis_rotation(compo_form_t *frm, s16 angle);

/**
 * @brief 窗体根据前后自动隐藏
 * @param[in] frm : 窗体指针
 **/
void compo_form3d_auto_visible(compo_form_t *frm);

/**
 * @brief 语音悬浮球
 * @param[in] sta: true:显示；flase：不显示
 */
void compo_form_set_ai_voice_anim(u8 sta);
#endif
