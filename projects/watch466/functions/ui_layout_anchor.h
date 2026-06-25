#ifndef _UI_LAYOUT_ANCHOR_H
#define _UI_LAYOUT_ANCHOR_H

/*
 * Heat / Mode 页共享：设计稿右上角锚点 (320×240)
 * compo_picturebox_set_pos 使用中心点，layout 中 tr → center 转换
 */
#define HEAT_LAYOUT_REF_W                 320
#define HEAT_LAYOUT_REF_H                 240
#define HEAT_TIMER_TR_Y                   ((s16)((s32)60 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define HEAT_TIMER_TR_H10_X               ((s16)((s32)100 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define HEAT_TIMER_TR_H1_X                ((s16)((s32)141 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define HEAT_TIMER_TR_COLON_X             ((s16)((s32)172 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define HEAT_TIMER_TR_M10_X               ((s16)((s32)225 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define HEAT_TIMER_TR_M1_X                ((s16)((s32)283 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define HEAT_TEMP_TR_Y                    ((s16)((s32)158 * GUI_SCREEN_HEIGHT / HEAT_LAYOUT_REF_H))
#define HEAT_TEMP_TR_H_X                  ((s16)((s32)89 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define HEAT_TEMP_TR_T10_X                ((s16)((s32)139 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define HEAT_TEMP_TR_T1_X                 ((s16)((s32)185 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))
#define HEAT_TEMP_TR_UNIT_X               ((s16)((s32)247 * GUI_SCREEN_WIDTH / HEAT_LAYOUT_REF_W))

#endif
