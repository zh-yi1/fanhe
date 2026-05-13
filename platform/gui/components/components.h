#ifndef _COMPONENTS_H
#define _COMPONENTS_H

enum COMPO_TYPE {
    COMPO_TYPE_NONE,
    COMPO_TYPE_FORM,                    //窗体
    COMPO_TYPE_PICTUREBOX,              //图像框
    COMPO_TYPE_TEXTBOX,                 //文本框
    COMPO_TYPE_LABEL,                   //标签
    COMPO_TYPE_SHAPE,                   //形状
    COMPO_TYPE_BUTTON,                  //铵钮
    COMPO_TYPE_NUMBER,                  //数字
    COMPO_TYPE_DATETIME,                //时间
    COMPO_TYPE_QRCODEBOX,               //二维码或条形码
    COMPO_TYPE_MSGBOX,                  //对话框
    COMPO_TYPE_LISTBOX,                 //列表框
    COMPO_TYPE_ROWBOX,                  //列表框（横向）
    COMPO_TYPE_STACKLIST,               //堆叠列表
    COMPO_TYPE_ICONLIST,                //图标集
    COMPO_TYPE_ROTARY,                  //转盘
    COMPO_TYPE_FOOTBALL,                //足球菜单
    COMPO_TYPE_CUBE,                    //立方体菜单
    COMPO_TYPE_GRIDBOX,                 //栅格
    COMPO_TYPE_DISKLIST,                //圆盘列表
    COMPO_TYPE_KALEIDOSCOPE,            //万花筒
    COMPO_TYPE_ANIMATION,               //动图
    COMPO_TYPE_VIDEO,                   //视频
    COMPO_TYPE_GIF,                   //视频
    COMPO_TYPE_JPG,                     //JPG图片显示
    COMPO_TYPE_CAMERA,                  //摄像头显示组件
    COMPO_TYPE_RINGS,                   //环形
    COMPO_TYPE_ARC,                     //圆弧
    COMPO_TYPE_CHARTBOX,                //图表框
    COMPO_TYPE_CARDBOX,                 //卡片
    COMPO_TYPE_BUTTERFLY,               //蝴蝶
    COMPO_TYPE_SCROLLBAR,               //滚动条
    COMPO_TYPE_NUMLIST,                 //数字列表
    COMPO_TYPE_TXTLIST,                 //文本列表
    COMPO_TYPE_WINDMILL,                //风车
    COMPO_TYPE_FINSH,                   //鱼
};

//绑定数据
enum COMPO_BOND_DATA {
    COMPO_BOND_NONE,
    COMPO_BOND_YEAD,                    //年
    COMPO_BOND_MONTH,                   //月
    COMPO_BOND_DAY,                     //日
    COMPO_BOND_WEEKDAY,                 //周
    COMPO_BOND_HOUR,                    //时
    COMPO_BOND_MINUTE,                  //分
    COMPO_BOND_SECOND,                  //秒
    COMPO_BOND_HOUR_H,                  //时(十位)
    COMPO_BOND_HOUR_L,                  //时(个位)
    COMPO_BOND_MINUTE_H,                //分(十位)
    COMPO_BOND_MINUTE_L,                //分(个位)

    COMPO_BOND_DATE,                    //日期(月/日)

    COMPO_BOND_VIDEO = 15,              //视频
    COMPO_BOND_IMAGE_STATIC = 16,       //静态图片
    COMPO_BOND_IMAGE_CLOCK_PREVIEW,     //表盘预览图
    COMPO_BOND_IMAGE_CLICK,             //点击图片
    COMPO_BOND_ANIMATION_STATIC,        //静态动图
    COMPO_BOND_ANIMATION_AREA_CLICK,    //区域点击动图
    COMPO_BOND_ANIMATION_CLICK,         //点击动图
    COMPO_BOND_IMAGE_WEATHER,           //天气
    COMPO_BOND_BLE_STA,                 //BLE连接状态
    COMPO_BOND_BT_STA,                  //BT连接状态
    COMPO_BOND_BTBLE_STA,               //BTBLE连接状态
    COMPO_BOND_DISTANCE_PROGRESS,       //距离进度
    COMPO_BOND_KCAL_PROGRESS,           //卡路里进度
    COMPO_BOND_STEPS_PROGRESS,          //步数进度
    COMPO_BOND_VBAT_PROGRESS,           //电量进度
    COMPO_BOND_APM,                     //上下午
    COMPO_BOND_TMIE_MAP_BG_M = 37,      //精准时图分盘
    COMPO_BOND_TMIE_MAP_BG_S,           //精准时图秒盘
    COMPO_BOND_TIME_MAP_M,              //精准时图分带
    COMPO_BOND_TIME_MAP_S,              //精准时图秒带
    COMPO_BOND_TIME_MAP_M_MID,          //精准时图分带(中)
    COMPO_BOND_TIME_MAP_S_MID,          //精准时图秒带(中)
    COMPO_BOND_TIME_MAP_LEFT_UP,        //精准时图
    COMPO_BOND_TIME_MAP_RIGHT_UP,       //精准时图
    COMPO_BOND_TIME_MAP_LEFT_DOWN,      //精准时图
    COMPO_BOND_TIME_MAP_RIGHT_DOWN,     //精准时图

    COMPO_BOND_KCAL = 64,               //卡路里
    COMPO_BOND_STEP,                    //步数
    COMPO_BOND_HEARTRATE,               //心率
    COMPO_BOND_BLOOD_OXYGEN,            //血氧
    COMPO_BOND_BLOOD_SUGER,             //血糖
    COMPO_BOND_WEATHER,                 //天气
    COMPO_BOND_ATMOMS,                  //气压
    COMPO_BOND_TEMPERATURE,             //温度
    COMPO_BOND_TEMPERATURE_UNIT,        //温度单位
    COMPO_BOND_ALTITUDE,                //海拔
    COMPO_BOND_DISTANCE_UNIT,           //距离单位
    COMPO_BOND_REVERSE2,                //保留
    COMPO_BOND_BATTERY,                 //电池
};

#define COMPO_STRUCT_COMMON                                         \
    u8 type;                            /*组件类型*/                \
    u8 bond_data;                       /*绑定数据*/                \
    u16 id;                             /*组件ID*/

typedef struct component_t_ {
    COMPO_STRUCT_COMMON;
    u8 content[0];                      //组件详细内容
} component_t;

typedef struct compo_cb_t_ {
    u8 *buf0;                           //组件Buffer
    u32 buf0_size;
    u32 buf0_pos;
    u8 *buf1;
    u32 buf1_size;
    u32 buf1_pos;
    u8 top_num;

    //组件时间控制
    u32 rtc_cnt;
    u32 rtc_cnt2;
    tm_t tm;                            //时间tm结构体
    u16 mtime;                          //毫秒
    bool rtc_update;
} compo_cb_t;

extern compo_cb_t compo_cb;

typedef struct compo_form_t_ compo_form_t;

void compos_init(void);
void *compo_pool_create(bool flag_top);
void compo_pool_clear(compo_form_t *compo);
compo_form_t *compo_pool_get_top(void);
compo_form_t *compo_pool_get_bottom(void);
void compo_pool_set_top(compo_form_t *frm);
void *compo_create(compo_form_t *parent, u8 type);
void compo_bonddata(void *compo, u8 bond_data);         //设置组件绑定数据
void compo_setid(void *compo, u16 id);                  //设置组件ID
void *compo_get_head(void);
component_t *compo_get_next(component_t *compo);
void bsp_uitool_create(compo_form_t *frm, u32 base_addr, u16 compo_num);

#include "component_func.h"
#include "compo_form.h"
#include "compo_picturebox.h"
#include "compo_textbox.h"
#include "compo_label.h"
#include "compo_shape.h"
#include "compo_button.h"
#include "compo_number.h"
#include "compo_datetime.h"
#include "compo_qrcodebox.h"
#include "compo_listbox.h"
#include "compo_rowbox.h"
#include "compo_stacklist.h"
#include "compo_iconlist.h"
#include "compo_rotary.h"
#include "compo_football.h"
#include "compo_cube.h"
#include "compo_disklist.h"
#include "compo_kaleidoscope.h"
#include "compo_animation.h"
#include "compo_rings.h"
#include "compo_arc.h"
#include "compo_chartbox.h"
#include "compo_cardbox.h"
#include "compo_move_ctr.h"
#include "compo_butterfly.h"
#include "compo_scrollbar.h"
#include "compo_windmill.h"
#include "compo_fish.h"
#include "compo_video.h"
#include "compo_jpg.h"
#include "compo_camera.h"
#include "compo_gif.h"
#endif
