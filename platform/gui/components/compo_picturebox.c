#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

extern void blur_draw(u8 *map_buf, u8 *obuf, u32 radius);

/**
 * @brief 创建一个图像框组件
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 图像资源地址
 * @return 返回图像指针
 **/
compo_picturebox_t *compo_picturebox_create(compo_form_t *frm, u32 res_addr)
{
    compo_picturebox_t *picbox = compo_create(frm, COMPO_TYPE_PICTUREBOX);
    void *img = widget_image_create(frm->page_body, res_addr);
    picbox->img = img;
    picbox->radix = 1;
    return picbox;
}

/**
 * @brief 设置图像
          用于改变图像组件中的图片
 * @param[in] picbox : 图像指针
 * @param[in] res_addr : 图像资源地址
 **/
void compo_picturebox_set(compo_picturebox_t *picbox, u32 res_addr)
{
    widget_image_set(picbox->img, res_addr);
}

/**
 * @brief 设置图像
          从RAM中设置图标的图片
 * @param[in] picbox : 图像指针
 * @param[in] buf:图片在RAM中的缓存地址，需要提前手动加载
 * @return 无
 **/
void compo_picturebox_set_ram(compo_picturebox_t *picbox, void *buf)
{
    widget_image_set_ram(picbox->img, buf);
}


/**
 * @brief 剪切图像
          用于剪切图像组件中的图片
 * @param[in] picbox : 图像指针
 * @param[in] index :  截取索引：第index张图
 * @param[in] total_cnt : 总拼图数量
 **/
void compo_picturebox_cut(compo_picturebox_t *picbox, u8 index, u8 total_cnt)
{
	if (total_cnt == 0) {
		halt(HALT_GUI_COMPO);
	}
    area_t area = widget_image_get_size(picbox->img);
    widget_image_cut(picbox->img, 0, index * (area.hei / total_cnt), area.wid, area.hei / total_cnt);
	picbox->radix = total_cnt;
}

/**
 * @brief 设置图像框组件的坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] picbox : 图像指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_picturebox_set_pos(compo_picturebox_t *picbox, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_pos(picbox->img, x, y);
#else
    widget_set_pos(picbox->img, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置图像框组件的前景色, 仅支持灰度图
 * @param[in] picbox : 图像指针
 * @param[in] color : 前景色
 **/
void compo_picturebox_set_color(compo_picturebox_t *picbox, u16 color)
{
    widget_image_set_color(picbox->img, color);
}

/**
 * @brief 设置图像框组件的大小
 * @param[in] picbox : 图像指针
 * @param[in] width : 图像框宽度
 * @param[in] height : 图像框高度
 **/
void compo_picturebox_set_size(compo_picturebox_t *picbox, s16 width, s16 height)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_set_size(picbox->img, width, height);
#else
    widget_set_size(picbox->img, width, height);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置图像框的旋转角度
          图片旋转时，需要经过转换工具勾上 旋转压缩 的选项
 * @param[in] picbox : 图像指针
 * @param[in] angle : 旋转的角度
 **/
void compo_picturebox_set_rotation(compo_picturebox_t *picbox, s16 angle)
{
    widget_image_set_rotation(picbox->img, angle);
}

/**
 * @brief 设置图像框的旋转中心点
 * @param[in] picbox : 图像指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_picturebox_set_rotation_center(compo_picturebox_t *picbox, s16 x, s16 y)
{
#if BOX_GUI_ROTATE_DISP
    rotate_widget_image_set_rotation_center(picbox->img, x, y);
#else
    widget_image_set_rotation_center(picbox->img, x, y);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 设置图像框是否可见
 * @param[in] picbox : 图像指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_picturebox_set_visible(compo_picturebox_t *picbox, bool visible)
{
    widget_set_visible(picbox->img, visible);
}

/**
 * @brief 设置图像框是否可见
 * @param[in] picbox : 图像指针
 * @return visible : true  可见
                        false 不可见
 **/
bool compo_picturebox_get_visible(compo_picturebox_t *picbox)
{
    return widget_get_visble(picbox->img);
}

/**
 * @brief 获取图像框的坐标和大小
 * @param[in] picbox : 图像指针
 * @return 返回图像的坐标和大小
 **/
rect_t compo_picturebox_get_location(compo_picturebox_t *picbox)
{
#if BOX_GUI_ROTATE_DISP
    rect_t rect = rotate_widget_get_location(picbox->img);
#else
    rect_t rect = widget_get_location(picbox->img);
#endif // BOX_GUI_ROTATE_DISP
    return rect;

}

/**
 * @brief 设置图片透明度
 * @param[in] picbox : 按钮指针
 * @param[in] alpha : 透明度
 **/
void compo_picturebox_set_alpha(compo_picturebox_t *picbox, u8 alpha)
{
    widget_set_alpha(picbox->img, alpha);
}

/**
 * @brief 获取图片透明度
 * @param[in] picbox : 按钮指针
 * @return alpha : 透明度
 **/
u8 compo_picturebox_get_alpha(compo_picturebox_t *picbox)
{
    return widget_get_alpha(picbox->img);
}

#if GUI_USE_BLUR
/**
 * @brief 设置图像
          从RAM中设置图标的图片并做高斯模糊处理
 * @param[in] picbox : 图像指针
 * @param[in] map_buf:原图图片在RAM中的缓存地址，需要提前手动加载
 * @param[in] obuf:高斯模糊后的图片在RAM中的缓存地址
 * @param[in] radius:模糊半径1-127
 * @return 无
 **/
void compo_picturebox_blur_set_ram(compo_picturebox_t *picbox, void *map_buf, void *obuf, u32 radius)
{
    blur_draw(map_buf, obuf, radius);
    widget_image_set_ram(picbox->img, obuf);
}
#endif

