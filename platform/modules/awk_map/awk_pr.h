#ifndef _AWK_PR_H_
#define _AWK_PR_H_

#include "awk_map.h"

extern void awk_map_interface_init(void);
extern void awk_temp_mm_init(void);
extern void awk_temp_mm_uninit(void);
extern void awk_fs_init(void);
extern void awk_fs_uninit(void);
extern void awk_map_tile_flush(void);
extern void awk_aos_network_get_process(void);

/**
 *  GUI 当前是否在推屏显示中
 *  单buff机制，避免推屏的同时地图在渲染导致撕裂
 */
bool gui_is_in_drawing(void);

/**
 *  屏幕坐标转换成地图渲染图片坐标
 *  Y轴
 */
s32 awk_tp_vx2sx(s32 xo);


/**
 *  屏幕坐标转换成地图渲染图片坐标
 *  X轴
 */
s32 awk_tp_vy2sy(s32 yo);

/**
 *  GUI推屏调用，判断当前是否可以推屏显示
 *  单buff机制，避免推屏的同时地图在渲染导致撕裂
 */
bool tft_te_refresh_is_enable(void);


/**
 *  地图初始化
 *  
 */
void awk_map_init(void);


/**
 *  地图设置经纬度坐标
 *  
 */
void awk_map_set_coord2d(double lon, double lat);

/**
 *  地图放大缩小
 *  zoom_in_out :true 放大； false 缩小
 */
void awk_map_set_zoom(bool zoom_in_out);

/**
 *  地图渲染显示图片
 *  放在main线程，定时调用
 */
void awk_map_flush(void);
void awk_map_http_flush(void);

/**
 *  地图反初始化
 */
void awk_map_uninit(void);

/**
 *  地图组件初始化
 */
void awk_map_tile_init(compo_form_t *frm);

/**
 *  地图组件反初始化
 */
void awk_map_tile_uninit(void);

/**
 *  获取地图ID
 */
int awk_get_map_id(void);
#endif
