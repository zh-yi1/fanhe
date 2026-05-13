#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define MAX_WORD_CNT                    32                          //标题最多32个字符


#if (GUI_SELECT == GUI_OLED_466_ICNA3310B)
#define GUI_PAGE_TITLE_WIDTH            (GUI_SCREEN_WIDTH * 2 / 3)
#define GUI_PAGE_TIME_WIDTH             (GUI_SCREEN_WIDTH / 3)
#else
#define GUI_PAGE_TITLE_WIDTH            (GUI_SCREEN_WIDTH * 2 / 5)
#define GUI_PAGE_TIME_WIDTH             (GUI_SCREEN_WIDTH / 3)
#endif
/**
 * @brief 创建窗体
          窗体为其他组件的容器
 * @param[in] flag_top : 是否放在界面的顶层
                         false在底层
                         true在顶层
 * @return 返回窗体指针
 **/
compo_form_t *compo_form_create(bool flag_top)
{
    compo_form_t *frm = compo_pool_create(flag_top);
    widget_page_t *page = widget_pool_create(flag_top);

    widget_icon_t *icon = widget_icon_create(page, 0);
#if BOX_GUI_ROTATE_DISP
    ro_widget_set_pos(icon, GUI_GET_SCREEN_CENTER_X, GUI_GET_SCREEN_CENTER_Y, GUI_GET_SCREEN_WIDTH, GUI_GET_SCREEN_HEIGHT);
#else
	widget_set_pos(icon, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_set_size(icon, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
#endif

    widget_page_t *page_body = widget_page_create(page);
    widget_text_t *time = widget_text_create(page, 5);
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_location(page, GUI_GET_SCREEN_CENTER_X, GUI_GET_SCREEN_CENTER_Y, GUI_GET_SCREEN_WIDTH, GUI_GET_SCREEN_HEIGHT);
    rotate_widget_set_location(page_body, GUI_GET_SCREEN_CENTER_X, GUI_GET_SCREEN_CENTER_Y, GUI_GET_SCREEN_WIDTH, GUI_GET_SCREEN_HEIGHT);
#else

    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    widget_set_location(page_body, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
#endif

    widget_set_align_center(time, false);
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_location(time, GUI_SCREEN_WIDTH - GUI_PAGE_TIME_WIDTH, 50, GUI_PAGE_TIME_WIDTH, FORM_TITLE_HEIGHT);
#else
    widget_set_location(time, GUI_SCREEN_WIDTH - GUI_PAGE_TIME_WIDTH, GUI_PAGE_HEAD_HEIGHT - FORM_TITLE_HEIGHT, GUI_PAGE_TIME_WIDTH, FORM_TITLE_HEIGHT);
#endif // BOX_GUI_ROTATE_DISP
    widget_text_set_font(time, UI_BUF_FONT_FORM_TIME);
    widget_set_visible(time, false);

    frm->type = COMPO_TYPE_FORM;
    frm->page = page;
    frm->icon = icon;

    frm->time = time;

    frm->page_body = page;
    compo_textbox_t *title = compo_textbox_create(frm, MAX_WORD_CNT);
    frm->page_body = page_body;
    compo_textbox_set_align_center(title, false);
#if BOX_GUI_ROTATE_DISP
    compo_textbox_set_location(title, FORM_TITLE_LEFT, 140, GUI_PAGE_TITLE_WIDTH, 120);
#else
    compo_textbox_set_location(title, FORM_TITLE_LEFT, GUI_PAGE_HEAD_HEIGHT - FORM_TITLE_HEIGHT, GUI_PAGE_TITLE_WIDTH, FORM_TITLE_HEIGHT);
#endif // BOX_GUI_ROTATE_DISP
    compo_textbox_set_multiline(title, false);
    compo_textbox_set_autosize(title, false);
    compo_textbox_set_align_center(title, false);
    compo_textbox_set_autoroll_mode(title, TEXT_AUTOROLL_MODE_NULL);
    compo_textbox_set_visible(title, true);
    frm->title = title;

    widget_icon_t *title_icon = widget_icon_create(page, 0);
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(title_icon, FORM_TITLE_LEFT + 10, GUI_PAGE_HEAD_HEIGHT - FORM_TITLE_HEIGHT / 2 - 2);
#else
    widget_set_pos(title_icon, FORM_TITLE_LEFT + 10, GUI_PAGE_HEAD_HEIGHT - FORM_TITLE_HEIGHT / 2 - 2);
#endif // BOX_GUI_ROTATE_DISP
    widget_set_size(title_icon, FORM_TITLE_HEIGHT, FORM_TITLE_HEIGHT);
    widget_set_visible(title_icon, false);
    frm->title_icon = title_icon;

#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
    //语音悬浮球
    frm->page_body = page;
    compo_animation_t *animation = compo_animation_create(frm, UI_BUF_ASR_VOICE_BIN);
    frm->page_body = page_body;
    compo_animation_set_pos(animation, GUI_SCREEN_CENTER_X, (GUI_SCREEN_HEIGHT - (GUI_SCREEN_HEIGHT >> 2)));
    compo_animation_set_radix(animation, 5);
    compo_animation_set_interval(animation, 10);
    widget_set_top(animation->page, true);
#if (ASR_SELECT == ASR_WS_AIR || ASR_SELECT == ASR_WS)
    compo_animation_set_visible(animation, bsp_asr_voice_wake_sta_get());
#else
    compo_animation_set_visible(animation, false);
#endif
    frm->anim = animation;
#endif

    return frm;
}

/**
 * @brief 销毁窗体
 * @param[in] frm : 窗体指针
 **/
void compo_form_destroy(compo_form_t *frm)
{
    if (frm == NULL) {
        return;
    }
    widget_pool_clear(frm->page);
    compo_pool_clear(frm);
}

/**
 * @brief 设置窗体坐标及大小
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] frm : 窗体指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 窗体宽度
 * @param[in] height : 窗体高度
 **/
void compo_form_set_location(compo_form_t *frm, s16 x, s16 y, s16 width, s16 height)
{
    if (frm == NULL || frm->page == NULL) {
        halt(HALT_GUI_COMPO_FORM_PTR);
    }
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_location(frm->page, x, y, width, height);
#else
    widget_set_location(frm->page, x, y, width, height);
#endif // BOX_GUI_ROTATE_DISP
}


/**
 * @brief 设置窗体坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] frm : 窗体指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_form_set_pos(compo_form_t *frm, s16 x, s16 y)
{
    if (frm == NULL || frm->page == NULL) {
        halt(HALT_GUI_COMPO_FORM_PTR);
    }
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(frm->page, x, y);
#else
    widget_set_pos(frm->page, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 窗体缩放
          以中心点缩放
 * @param[in] frm : 窗体指针
 * @param[in] width : 缩放后的窗体宽度
 * @param[in] height : 缩放后的窗体高度
 **/
void compo_form_scale_to(compo_form_t *frm, s16 width, s16 height)
{
    if (frm == NULL || frm->page == NULL) {
        halt(HALT_GUI_COMPO_FORM_PTR);
    }
#if BOX_GUI_ROTATE_DISP
    rotate_widget_page_scale_to(frm->page, width, height);
#else
    widget_page_scale_to(frm->page, width, height);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 窗体设置Alpha
 * @param[in] frm : 窗体指针
 * @param[in] alpha : 透明度
 **/
void compo_form_set_alpha(compo_form_t *frm, u8 alpha)
{
    if (frm == NULL || frm->page == NULL) {
        halt(HALT_GUI_COMPO_FORM_PTR);
    }
    widget_set_alpha(frm->page, alpha);
}

/**
 * @brief 更新窗体信息
 * @param[in] frm : 窗体指针
 **/
static void compo_form_page_update(compo_form_t *frm)
{
    if (frm->mode == 0) {
        compo_textbox_set_visible(frm->title, false);
        widget_set_location(frm->page_body, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
        widget_page_set_client(frm->page_body, 0, 0);
    } else {
        compo_textbox_set_visible(frm->title, (frm->mode & COMPO_FORM_MODE_SHOW_TITLE) != 0);
        widget_set_visible(frm->title_icon, (frm->mode & COMPO_FORM_MODE_SHOW_ICON) != 0);
        if (frm->mode & COMPO_FORM_MODE_SHOW_TIME) {
            widget_set_visible(frm->time, true);
            compo_cb.rtc_update = true;
        }
#if BOX_GUI_ROTATE_DISP
        if (func_cb.menu_style == 14 && func_cb.sta == FUNC_MENU) {  // MENU_STYLE_CUM_FOURGRID 修正菜单风格出现问题
            rotate_widget_set_location(frm->page_body, GUI_GET_SCREEN_CENTER_X, GUI_GET_SCREEN_CENTER_Y, GUI_GET_SCREEN_WIDTH, GUI_GET_SCREEN_HEIGHT);
        } else {
            rotate_widget_set_location(frm->page_body, GUI_GET_SCREEN_CENTER_X, GUI_PAGE_BODY_CENTER_Y, GUI_SCREEN_WIDTH, GUI_PAGE_BODY_HEIGHT);
        }

#else
        widget_set_location(frm->page_body, GUI_SCREEN_CENTER_X, GUI_PAGE_BODY_CENTER_Y, GUI_SCREEN_WIDTH, GUI_PAGE_BODY_HEIGHT);
        widget_page_set_client(frm->page_body, 0, -GUI_PAGE_HEAD_HEIGHT);
#endif // BOX_GUI_ROTATE_DISP
    }
}

/**
 * @brief 设置窗体标题栏
          通常和compo_form_set_title（窗体模式）一起调用
 * @param[in] frm : 窗体指针
 * @param[in] title : 标题文本
 **/
void compo_form_set_title(compo_form_t *frm, const char *title)
{
    if (title != NULL) {
        frm->mode |= COMPO_FORM_MODE_SHOW_TITLE;
        compo_textbox_set(frm->title, title);

        compo_textbox_t *compo_textbox = frm->title;

        area_t text_area = widget_text_get_area(compo_textbox->txt);
#if BOX_GUI_ROTATE_DISP
        rect_t textbox_rect = rotate_widget_get_location(compo_textbox->txt);
#else
        rect_t textbox_rect = widget_get_location(compo_textbox->txt);
#endif // BOX_GUI_ROTATE_DISP
        if (text_area.wid > textbox_rect.wid) {
            compo_textbox->roll_cb.mode = TEXT_AUTOROLL_MODE_SROLL_CIRC;
            compo_textbox->roll_cb.direction = -1;
        }

        compo_form_page_update(frm);
    }
}

/**
 * @brief 设置窗体模式
          通常和compo_form_set_title（窗体标题栏）一起调用
 * @param[in] frm : 窗体指针
 * @param[in] mode : COMPO_FORM_MODE_SHOW_TITLE  BIT(0) 标题栏显示文字
                     COMPO_FORM_MODE_SHOW_TIME   BIT(1) 标题栏显示时间
 **/
void compo_form_set_mode(compo_form_t *frm, int mode)
{
    frm->mode = mode;
    compo_form_page_update(frm);
}

/**
 * @brief 设置窗体标题居中
 * @param[in] frm : 窗体指针
 * @param[in] align_center : 是否居中
 **/
void compo_form_set_title_center(compo_form_t *frm, bool align_center)
{
    compo_textbox_set_align_center(frm->title, align_center);
    compo_textbox_set_location(frm->title, GUI_SCREEN_CENTER_X, GUI_PAGE_HEAD_HEIGHT - (FORM_TITLE_HEIGHT >> 1), GUI_PAGE_TITLE_WIDTH, FORM_TITLE_HEIGHT);
}

/**
 * @brief 窗体中添加图片
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图片资源的地址
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_form_add_image(compo_form_t *frm, u32 res_addr, s16 x, s16 y)
{
    //背景图不需要复杂功能，使用widget_icon更省Buffer
    widget_icon_t *icon = widget_icon_create(frm->page_body, res_addr);
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(icon, x, y);
#else
    widget_set_pos(icon, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 窗体中设置背景
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图片资源的地址
 **/
void compo_form_set_bg(compo_form_t *frm, u32 res_addr)
{
    widget_icon_set(frm->icon, res_addr);
}

/**
 * @brief title的图标
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图片资源的地址
 **/
void compo_form_set_title_icon(compo_form_t *frm, u32 res_addr)
{
    if (widget_text_get_area(frm->title->txt).wid) {
#if BOX_GUI_ROTATE_DISP
        widget_set_pos(frm->title_icon, FORM_TITLE_LEFT + widget_text_get_area(frm->title->txt).wid +
                       rotate_gui_image_get_size(res_addr).wid / 2 + 5, GUI_PAGE_HEAD_HEIGHT - FORM_TITLE_HEIGHT / 2 - 2);
#else
        widget_set_pos(frm->title_icon, FORM_TITLE_LEFT + widget_text_get_area(frm->title->txt).wid +
                       gui_image_get_size(res_addr).wid / 2 + 5, GUI_PAGE_HEAD_HEIGHT - FORM_TITLE_HEIGHT / 2 - 2);
#endif // BOX_GUI_ROTATE_DISP
        widget_set_size(frm->title_icon, FORM_TITLE_HEIGHT, FORM_TITLE_HEIGHT);
    }

    widget_icon_set(frm->title_icon, res_addr);
    compo_form_page_update(frm);
}

/**
 * @brief 设置窗体frm左右镜像翻转
 * @param[in] page:页面
 * @param[in] flip:是否左右镜像翻转
 * @return 无
 **/
void compo_form_set_flip(compo_form_t *frm, bool flip)
{
    widget_set_flip(frm->page, flip);
}

/**
 * @brief 设置窗体frm使用frm_src的3D轴
 * @param[in] frm : 窗体指针
 * @param[in] frm_src : 3D轴源窗体指针
 **/
void compo_form3d_set_axis(compo_form_t *frm, compo_form_t *frm_src)
{
    frm->axis = frm_src->axis;
    widget_page3d_set_axis(frm->page, frm->axis);
}

/**
 * @brief 设置窗体frm的旋转球半径
 * @param[in] page:页面
 * @param[in] r:球半径
 * @return 无
 **/
void compo_form3d_set_r(compo_form_t *frm, s16 r)
{
    widget_page3d_set_r(frm->page, r);
}

/**
 * @brief 设置窗体frm的极角
 * @param[in] page:页面
 * @param[in] angle:极角
 * @return 无
 **/
void compo_form3d_set_polar(compo_form_t *frm, s16 angle)
{
    widget_page3d_set_polar(frm->page, angle);
}

/**
 * @brief 设置窗体frm的方位角
 * @param[in] page:页面
 * @param[in] angle:方位角
 * @return 无
 **/
void compo_form3d_set_azimuth(compo_form_t *frm, s16 angle)
{
    widget_page3d_set_azimuth(frm->page, angle);
}

/**
 * @brief 设置窗体frm的自旋角
 * @param[in] page:页面
 * @param[in] angle:自旋角
 * @return 无
 **/
void compo_form3d_set_rotation(compo_form_t *frm, s16 angle)
{
    widget_page3d_set_rotation(frm->page, angle);
}

/**
 * @brief 设置窗体frm的旋转中心点
 * @param[in] page:页面
 * @param[in] x:相对于x
 * @param[in] y:相对于y
 * @return 无
 **/
void compo_form3d_set_rotation_center(compo_form_t *frm, s16 x, s16 y)
{
    widget_page3d_set_rotation_center(frm->page, x, y);
}

/**
 * @brief 设置窗体3D轴距
 * @param[in] frm : 窗体指针
 * @param[in] distance : 轴距(影响透视效果)
 **/
void compo_form3d_set_axis_distance(compo_form_t *frm, s16 distance)
{
    if (frm->axis == NULL) {
        frm->axis = widget_axis3d_create(frm->page);
        widget_page3d_set_axis(frm->page, frm->axis);
    }
    widget_axis3d_set_distance(frm->axis, distance);
}

/**
 * @brief 设置窗体3D轴俯视角
 * @param[in] frm : 窗体指针
 * @param[in] angle : 俯视角(透视角度)
 **/
void compo_form3d_set_axis_overlook(compo_form_t *frm, s16 angle)
{
    if (frm->axis == NULL) {
        frm->axis = widget_axis3d_create(frm->page);
        widget_page3d_set_axis(frm->page, frm->axis);
    }
    widget_axis3d_set_overlook(frm->axis, angle);
}

/**
 * @brief 设置窗体3D轴的旋转球心坐标
 * @param[in] frm : 窗体指针
 * @param[in] x, y, z:球心坐标
 **/
void compo_form3d_set_axis_pos(compo_form_t *frm, s16 x, s16 y, s16 z)
{
    if (frm->axis == NULL) {
        frm->axis = widget_axis3d_create(frm->page);
        widget_page3d_set_axis(frm->page, frm->axis);
    }
    widget_axis3d_set_pos(frm->axis, x, y, z);
}

/**
 * @brief 设置窗体3D轴的球坐标
 * @param[in] frm : 窗体指针
 * @param[in] sph : 轴的球坐标
 **/
void compo_form3d_set_axis_sph(compo_form_t *frm, sph_t sph)
{
    if (frm->axis == NULL) {
        frm->axis = widget_axis3d_create(frm->page);
        widget_page3d_set_axis(frm->page, frm->axis);
    }
    widget_axis3d_set_sph(frm->axis, sph);
}

/**
 * @brief 设置窗体3D轴的极角
 * @param[in] frm : 窗体指针
 * @param[in] angle : 轴的极角
 **/
void compo_form3d_set_axis_polar(compo_form_t *frm, s16 angle)
{
    if (frm->axis == NULL) {
        frm->axis = widget_axis3d_create(frm->page);
        widget_page3d_set_axis(frm->page, frm->axis);
    }
    widget_axis3d_set_polar(frm->axis, angle);
}

/**
 * @brief 设置窗体3D轴的方位角
 * @param[in] frm : 窗体指针
 * @param[in] angle : 轴的方位角
 **/
void compo_form3d_set_axis_azimuth(compo_form_t *frm, s16 angle)
{
    if (frm->axis == NULL) {
        frm->axis = widget_axis3d_create(frm->page);
        widget_page3d_set_axis(frm->page, frm->axis);
    }
    widget_axis3d_set_azimuth(frm->axis, angle);
}

/**
 * @brief 设置窗体3D轴的自旋角
 * @param[in] frm : 窗体指针
 * @param[in] angle : 轴的自旋角
 **/
void compo_form3d_set_axis_rotation(compo_form_t *frm, s16 angle)
{
    if (frm->axis == NULL) {
        frm->axis = widget_axis3d_create(frm->page);
        widget_page3d_set_axis(frm->page, frm->axis);
    }
    widget_axis3d_set_rotation(frm->axis, angle);
}

/**
 * @brief 窗体根据前后自动隐藏
 * @param[in] frm : 窗体指针
 **/
void compo_form3d_auto_visible(compo_form_t *frm)
{
    if (frm->axis == NULL) {
        frm->axis = widget_axis3d_create(frm->page);
        widget_page3d_set_axis(frm->page, frm->axis);
    }
    widget_set_visible(frm->page, widget_page3d_is_front(frm->page));
}

/**
 * @brief 语音悬浮球
 * @param[in] sta: true:显示；flase：不显示
 */
#if (ASR_SELECT && ASR_VOICE_BALL_ANIM)
void compo_form_set_ai_voice_anim(u8 sta)
{
    if (func_cb.sta == FUNC_CLOCK) {
        f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
        if (f_clk->sub_frm && f_clk->sta == FUNC_CLOCK_SUB_DROPDOWN) {
            if (f_clk->sub_frm->anim) {
                compo_animation_set_visible(f_clk->sub_frm->anim, sta);
                if (func_cb.frm_main) {
                    if (func_cb.frm_main->anim) {
                        compo_animation_set_visible(func_cb.frm_main->anim, sta);
                    }
                }
            }
        }
    }

    if (func_cb.frm_main) {
        if (func_cb.frm_main->anim) {
            compo_animation_set_visible(func_cb.frm_main->anim, sta);
        }
    }
}
#endif
