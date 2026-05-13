#ifndef _ROTATE_DISP_H
#define _ROTATE_DISP_H

#if BOX_GUI_ROTATE_DISP

#define RO_DISP_GET_X(x, y, h)                  (ro_90_counterclockwise_get_x(x, y, h))     //获取x坐标旋转后的值
#define RO_DISP_GET_Y(x, y, w)                  (ro_90_counterclockwise_get_y(x, y))        //获取y坐标旋转后的值
#define RO_DISP_GET_XY(xp, yp, w, h)            (ro_90_counterclockwise_get_xy(xp, yp, h))    //获取x y坐标旋转后的值
#define RO_DISP_GET_DXY_S16(xp, yp)             (ro_90_counterclockwise_get_dxy_s16(xp, yp))  //获取x y坐标变化量旋转后的值
#define RO_DISP_GET_DXY_S32(xp, yp)             (ro_90_counterclockwise_get_dxy_s32(xp, yp))  //获取x y坐标变化量旋转后的值

/**
 * @brief 获取x坐标逆时针旋转90°的值
 * @param[in] x: 当前x坐标
 * @param[in] y: 当前y坐标
 * @param[in] page_hei:当前方向下, page的高度
 * @return 旋转后的x坐标
 **/
s16 ro_90_counterclockwise_get_x(s16 x, s16 y, s16 page_hei);

/**
 * @brief 获取y坐标逆时针旋转90°的值
 * @param[in] x: 当前x坐标
 * @param[in] y: 当前y坐标
 * @return 旋转后的y坐标
 **/
s16 ro_90_counterclockwise_get_y(s16 x, s16 y);

/**
 * @brief 获取xy坐标逆时针旋转90°的值
 * @param[in] x: 当前x坐标指针, 并获取旋转后的值
 * @param[in] y: 当前y坐标指针, 并获取旋转后的值
 **/
void ro_90_counterclockwise_get_xy(s16 *x, s16 *y, s16 page_hei);

/**
 * @brief 获取xy坐标变化量, 逆时针旋转90°的值
 * @param[in] x: 当前x坐标变化量指针, 并获取旋转后的值
 * @param[in] y: 当前y坐标变化量指针, 并获取旋转后的值
 **/
void ro_90_counterclockwise_get_dxy_s16(s16 *dx, s16 *dy);
void ro_90_counterclockwise_get_dxy_s32(s32 *dx, s32 *dy);

#else

#define RO_DISP_GET_X(x, y, h)              (x)
#define RO_DISP_GET_Y(x, y, w)              (y)
#define RO_DISP_GET_XY(xp, yp, w, h)
#define RO_DISP_GET_DXY_S16(xp, yp)
#define RO_DISP_GET_DXY_S32(xp, yp)

#endif // BOX_GUI_ROTATE_DISP

#endif  //_ROTATE_DISP_H
