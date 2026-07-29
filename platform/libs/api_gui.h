#ifndef _API_GUI_H
#define _API_GUI_H

enum {
    FONT_TYPE_NORMAL = 0,       	//完整字体，或拼接的第一个
    FONT_TYPE_HALF_FOLLOW,      	//半型连体
    FONT_TYPE_FOLLOW,           	//连体
    FONT_TYPE_FOLLOW_EN,        	//英文连体,需要保留x_ofs
    FONT_TYPE_PART,             	//偏旁
    FONT_TYPE_PART_LEFT,        	//偏旁居左
    FONT_TYPE_PART_MID,         	//偏旁居中
    FONT_TYPE_PART_RIGHT,       	//偏旁居右
    FONT_TYPE_PART_RIGHT_CROSS, 	//偏旁越过居右
    FONT_TYPE_PART_HEAD,        	//头
    FONT_TYPE_PART_HEAD_FOLLOW,
    FONT_TYPE_PART_FOOT,        	//足
    FONT_TYPE_PART_MID_UP,      	//偏旁原本位置抬高自身高度一半
    FONT_TYPE_PART_FOLLOW_CENTER,   //连体并且高度居中
    FONT_TYPE_PART_LEFT_UP,			//偏旁水平居左并且高度抬高自身高度一半
    FONT_TYPE_PART_DOWN,      	    //偏旁原本位置降低自身高度一半
};

typedef struct rect_t_ {
    s16 x;
    s16 y;
    s16 wid;
    s16 hei;
} rect_t;

typedef struct point_t_ {
    s16 x;
    s16 y;
} point_t;

typedef struct area_t_ {
    s16 wid;
    s16 hei;
} area_t;

//球坐标
typedef struct sph_t_ {
    s16 r;
    s16 polar;
    s16 azimuth;
    s16 rotation;
} sph_t;

typedef void widget_t;
typedef void widget_page_t;
typedef void widget_image_t;        //功能比较全
typedef void widget_image3d_t;
typedef void widget_axis3d_t;
typedef void widget_icon_t;         //只有简单的贴图和缩放功能，但是更省Buffer
typedef void widget_text_t;
typedef void widget_rect_t;
typedef void widget_qrcode_t;
typedef void widget_arc_t;
typedef void widget_chart_t;
typedef void widget_circle_t;
typedef void widget_line_t;
typedef void widget_bar_t;


//=== 常用颜色，非常用颜色可以用make_color函数来生成 ===
#define COLOR_WHITE             0xFFFF
#define COLOR_BLACK             0
#define COLOR_RED               0xF800
#define COLOR_GREEN             0x07E0
#define COLOR_BLUE              0x4456// 0x001F
#define COLOR_YELLOW            0xFFE0
#define COLOR_MAGENTA           0xF81F
#define COLOR_CYAN              0x07FF
#define COLOR_GRAY              0x8410
#define COLOR_DARKGRAY          0xAD55
#define COLOR_DIMGRAY           0x6B4D


//=== GUI ===
//GUI初始化参数
typedef struct gui_init_param_t_ {
    u16 screen_width;               //屏幕宽
    u16 screen_height;              //屏幕高
    u8 *element_buf;                //element与widget缓存
    u8 *widget_buf;
    u16 element_buf_size;
    u16 widget_buf_size;
    u8 *temp_buf;                   //中间计算缓存
    u32 temp_buf_size;              //中间计算缓存大小
    u8 *lines_buf;                  //推屏缓存(双份)
    u32 lines_buf_size;             //推屏缓存大小
    u16 lines_count;                //每次推屏的行数
    u16 maxsize_parbuf;             //PAR解码缓存
    u32 font_res_addr;              //字库资源地址
    u16 max_font_size;
    u8 font_wspace;                 //字的间距
    u8 font_hspace;
    u8 *temp_buf2;                  //第二计算缓存，与temp_buf不同RAM加大RAM带宽
    u32 temp_buf2_size;             //第二计算缓存大小
    u8 rsvd[8];
} gui_init_param_t;

void os_gui_init(const gui_init_param_t *param);        //GUI初始化
void os_gui_draw(void);                                 //TE信号起来后，GUI开始绘制图形
void os_gui_flush_lcd(void);                            //推LCD
void gui_process(void);                                 //主循环调用，更新Widget绘制
void gui_refresh(void);                                 //直接widget绘制，截图用
void os_gui_draw_force(void);                           //直接ele绘制，截图用
void gui_set_te_margin(u8 margin);                      //设置TE时隙
void gui_widget_refresh(void);                          //刷新widget
void os_gui_draw_w4_done(void);                         //等待当前帧刷完
area_t gui_image_get_size(u32 res_addr);                //获取图像尺寸
area_t widget_image_get_size(widget_image_t *img);		//根据控件获取图像尺寸

typedef enum WGT_CHART_TYPE_ {
    CHART_TYPE_BAR,                     //直角柱形
    CHART_TYPE_BAR_ARC,                 //圆角柱形,上下都是圆角
    CHART_TYPE_BAR_ARC_U,               //圆角柱形上圆；下直角
    CHART_TYPE_BAR_ARC_D,               //圆角柱形上直角；下圆
}WGT_CHART_TYPE;


/**
 * @brief 生成一个RGB565的颜色值
 * @param[in] r : #ff0000  红色饱和度(0 ~ 0xff)
 * @param[in] g : #00ff00  绿色饱和度(0 ~ 0xff)
 * @param[in] b : #0000ff  蓝色饱和度(0 ~ 0xff)
 * @return RGB565的颜色值
 **/
u16 make_color(u8 r, u8 g, u8 b);

//============= Widget Common ==============//

/**
 * @brief 创建控件池(最多允许创建两个)，并创建首个widget_page
 * @param[in] flag_top : 是否在顶层
 * @return page
 **/
widget_page_t *widget_pool_create(bool flag_top);

/**
 * @brief 清除一个控件池
 * @param[in] widget : 首个widget_page
 * @return 无
 **/
void widget_pool_clear(widget_page_t *widget);

/**
 * @brief 将Widget置顶
 * @param[in] top_num : 编号
 * @return 无
 **/
void widget_pool_set_top(u8 top_num);

/**
 * @brief 获取第一个Widget，配合widget_get_next来遍历所有Widget
 * @param[in] 无
 * @return 无
 **/
void *widget_get_head(void);

/**
 * @brief 根据当前widget获取下一个widget
 * @param[in] widget:当前的widget
 * @return next widget
 **/
widget_t *widget_get_next(const widget_t *widget);

/**
 * @brief 获取Widget的父Page
 * @param[in] widget:当前的widget
 * @return 当前widget的widget_page
 **/
widget_page_t *widget_get_parent(const widget_t *widget);

/**
 * @brief 设置Widget的Alpha值
 * @param[in] widget:需要设置的控件
 * @param[in] alpha:透明度(0 ~ 0xff)
 * @return 当前widget的widget_page
 **/
void widget_set_alpha(widget_t *widget, u8 alpha);

/**
 * @brief 获取Widget的Alpha值
 * @param[in] widget:控件指针
 * @return alpha:透明度(0 ~ 0xff)
 **/
u8 widget_get_alpha(widget_t *widget);

/**
 * @brief 设置Widget是否按中心点来设置坐标
 * @param[in] widget:需要设置的控件
 * @param[in] align_center:true:按中心对齐,false：按左上角对齐，default:true
 * @return 无
 **/
void widget_set_align_center(widget_t *widget, bool align_center);

/**
 * @brief 设置Widget是否按中心点来设置坐标
 * @param[in] widget:需要设置的控件
 * @param[in] align_center:true:按中心对齐,false：按左上角对齐，default:true
 * @return 无
 **/
bool widget_get_align_center(widget_t *widget);

/**
 * @brief 设置Widget是否置顶
 * @param[in] widget:需要设置的控件
 * @param[in] flag_top:是否置顶
 * @return 无
 **/
void widget_set_top(widget_t *widget, bool flag_top);

/**
 * @brief 设置Widget坐标
 * @param[in] widget:需要设置的控件
 * @param[in] x:以屏幕左上角为原点，控件的中心点或左上角的x坐标
 * @param[in] y:以屏幕左上角为原点，控件的中心点或左上角的y坐标
 * @return 无
 **/
void widget_set_pos(widget_t *widget, s16 x, s16 y);

/**
 * @brief 设置Widget大小
 * @param[in] widget:需要设置的控件
 * @param[in] width:宽度
 * @param[in] height:高度
 * @return 无
 **/
void widget_set_size(widget_t *widget, s16 width, s16 height);

/**
 * @brief 设置Widget坐标及大小
 * @param[in] widget:需要设置的控件
 * @param[in] x:以屏幕左上角为原点，控件的中心点或左上角的x坐标
 * @param[in] y:以屏幕左上角为原点，控件的中心点或左上角的y坐标
 * @param[in] width:宽度
 * @param[in] height:高度
 * @return 无
 **/
void widget_set_location(widget_t *widget, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 获取Widget坐标及大小
 * @param[in] widget:控件指针
 * @return 控件的位置和大小
 **/
rect_t widget_get_location(const widget_t *widget);

/**
 * @brief 获取Widget绝对坐标及大小
 * @param[in] widget:需要设置的控件
 * @return 控件的绝对位置和大小
 **/
rect_t widget_get_absolute(const widget_t *widget);

/**
 * @brief 设置Widget的显示状态
 * @param[in] widget:需要设置的控件
 * @param[in] visible:true:可见  false:隐藏
 * @return 无
 **/
void widget_set_visible(widget_t *widget, bool visible);

/**
 * @brief 获取widget显示状态
 * @param[in] widget:控件指针
 * @return 显示状态 true:可见  false:隐藏
 **/
bool widget_get_visble(widget_t *widget);

/**
 * @brief 设置Widget是否进行水平翻转
 * @param[in] widget:需要设置的控件
 * @param[in] flip:true:进行水平翻转  false:保持原样
 * @return 无
 **/
void widget_set_flip(widget_t *widget, bool flip);

/**
 * @brief 获取widget水平翻转状态
 * @param[in] widget:控件指针
 * @return 水平翻转状态 true:进行水平翻转  false:保持原样
 **/
bool widget_get_flip(widget_t *widget);
//============= Widget Page ==============//


/**
 * @brief 创建一个Page型widget
 * @param[in] parent:父控件
 * @return page widget
 **/
widget_page_t *widget_page_create(widget_page_t *parent);

/**
 * @brief 判断widget是否为一个Page型
 * @param[in] widget:需要判断的控件
 * @return true or false
 **/
bool widget_is_page(const void *widget);

/**
 * @brief 设置Page client相对Page Window左上的坐标。0,0则对齐到左上角
 * @param[in] widget:需要设置的控件
 * @param[in] x:相对于左上角x坐标
 * @param[in] y:相对于左上角y坐标
 * @return 无
 **/
void widget_page_set_client(widget_page_t *widget, s16 x, s16 y);

/**
 * @brief 设置Page window缩放到的大小。0,0则不进行缩放
 * @param[in] wid:缩放的宽度
 * @param[in] hei:缩放的高度
 * @return 无
 **/
void widget_page_scale_to(widget_page_t *widget, s16 wid, s16 hei);

/**
 * @brief 更新Page信息
 * @param[in] 无
 * @return 无
 **/
void widget_page_update(void);

/**
 * @brief 设置页面3D轴
 * @param[in] page: 页面
 * @param[in] axis: 轴
 * @return 无
 **/
void widget_page3d_set_axis(widget_page_t *page, widget_axis3d_t *axis);

/**
 * @brief 设置Page的旋转球半径
 * @param[in] page:页面
 * @param[in] r:球半径
 * @return 无
 **/
void widget_page3d_set_r(widget_page_t *page, s16 r);

/**
 * @brief 设置Page的极角
 * @param[in] page:页面
 * @param[in] angle:极角
 * @return 无
 **/
void widget_page3d_set_polar(widget_page_t *page, s16 angle);

/**
 * @brief 设置3D图片的方位角
 * @param[in] page:页面
 * @param[in] angle:方位角
 * @return 无
 **/
void widget_page3d_set_azimuth(widget_page_t *page, s16 angle);


/**
 * @brief 设置Page的自旋角
 * @param[in] page:页面
 * @param[in] angle:自旋角
 * @return 无
 **/
void widget_page3d_set_rotation(widget_page_t *page, s16 angle);

/**
 * @brief 设置Page的旋转中心点
 * @param[in] page:页面
 * @param[in] x:相对于x
 * @param[in] y:相对于y
 * @return 无
 **/
void widget_page3d_set_rotation_center(widget_page_t *page, s16 x, s16 y);

//
/**
 * @brief 获取方位角
 * @param[in] page:页面
 * @return 方位角
 **/
s16 widget_page3d_get_azimuth(widget_page_t *page);

/**
 * @brief 获取方位角
 * @param[in] page:页面
 * @return 极角
 **/
s16 widget_page3d_get_polar(widget_page_t *page);

/**
 * @brief 获取自转角
 * @param[in] page:页面
 * @return 方位角
 **/
s16 widget_page3d_get_rotation(widget_page_t *page);

/**
 * @brief 判断正反面
 * @param[in] page:页面
 * @return 正面返回true, 背面返回false
 **/
bool widget_page3d_is_front(widget_page_t *page);

//============= Widget Image ==============//

enum {
    ROT_MODE_NORMAL,        //普通旋转模式
    ROT_MODE_X,             //只旋转X (平行四边形, 2.5D效果)
};

/**
 * @brief 创建图片widget,支持缩放、裁剪和旋转
 * @param[in] parent:父控件
 * @param[in] res_addr:图片地址，在ui.h中获取
 * @return widget_image_t
 **/
widget_image_t *widget_image_create(widget_page_t *parent, u32 res_addr);

/**
 * @brief 设置图片
 * @param[in] img:图片控件
 * @param[in] res_addr:图片地址，在ui.h中获取
 * @return 无
 **/
void widget_image_set(widget_image_t *img, u32 res_addr);

/**
 * @brief 从RAM中设置图标的图片
 * @param[in] icon:图标控件
 * @param[in] buf:图片在RAM中的缓存地址，需要提前手动加载
 * @return 无
 **/
void widget_image_set_ram(widget_image_t *icon, void *buf);

/**
 * @brief 设置图片的旋转角度，图片旋转时，需要经过转换工具勾上 旋转压缩 的选项
 * @param[in] img_ptr:图片控件
 * @param[in] angle:旋转角度 0 ~ 3600
 * @return 无
 **/
void widget_image_set_rotation(void *img_ptr, s16 angle);

/**
 * @brief 设置图片的旋转中心点
 * @param[in] img_ptr:图片控件
 * @param[in] x:相对于x
 * @param[in] y:相对于y
 * @return 无
 **/
void widget_image_set_rotation_center(void *img_ptr, s16 x, s16 y);

/**
 * @brief 裁剪图片区域
 * @param[in] img:图片控件
 * @param[in] x:相对于图片左上角x坐标
 * @param[in] y:相对于图片左上角y坐标
 * @param[in] wid:截取宽度
 * @param[in] hei:截取高度
 * @return 无
 **/
void widget_image_cut(widget_image_t *img, s16 x, s16 y, s16 wid, s16 hei);

//设置图像前景色
AT(.text.widget)
void widget_image_set_color(widget_image_t *img, u16 color);

//============= Widget Image 3D ==============//
/**
 * @brief 创建3D图片widget，RES必须支持旋转功能
 * @param[in] parent:父Widget
 * @param[in] res_addr:图片地址，在ui.h中获取
 * @return widget_image3d_t
 **/
widget_image3d_t *widget_image3d_create(widget_page_t *parent, u32 res_addr);

/**
 * @brief 设置3D图片轴
 * @param[in] img: 3D图片Widget
 * @param[in] axis: 轴
 * @return 无
 **/
void widget_image3d_set_axis(widget_image3d_t *img, widget_axis3d_t *axis);

/**
 * @brief 设置3D图片，RES必须支持旋转功能
 * @param[in] img:3D图片Widget
 * @param[in] res_addr:图片地址，在ui.h中获取
 * @return 无
 **/
void widget_image3d_set(widget_image3d_t *img, u32 res_addr);

/**
 * @brief 从RAM中设置3D图片
 * @param[in] img:3D图片Widget
 * @param[in] buf:图片在RAM中的缓存地址，需要提前手动加载
 * @return 无
 **/
void widget_image3d_set_ram(widget_image3d_t *img, void *buf);

/**
 * @brief 设置3D图片的旋转球半径
 * @param[in] img:3D图片Widget
 * @param[in] r:球半径
 * @return 无
 **/
void widget_image3d_set_r(widget_image3d_t *img, s16 r);

/**
 * @brief 设置3D图片的极角
 * @param[in] img:3D图片Widget
 * @param[in] angle:极角
 * @return 无
 **/
void widget_image3d_set_polar(widget_image3d_t *img, s16 angle);

/**
 * @brief 设置3D图片的方位角
 * @param[in] img:3D图片Widget
 * @param[in] angle:方位角
 * @return 无
 **/
void widget_image3d_set_azimuth(widget_image3d_t *img, s16 angle);


/**
 * @brief 设置3D图片的自旋角
 * @param[in] img:3D图片Widget
 * @param[in] angle:自旋角
 * @return 无
 **/
void widget_image3d_set_rotation(widget_image3d_t *img, s16 angle);

/**
 * @brief 设置3D图片的旋转中心点
 * @param[in] img:3D图片Widget
 * @param[in] x:相对于x
 * @param[in] y:相对于y
 * @return 无
 **/
void widget_image3d_set_rotation_center(widget_image3d_t *img, s16 x, s16 y);

/**
 * @brief 判断正反面
 * @param[in] img:3D图片Widget
 * @return 正面返回true, 背面返回false
 **/
bool widget_image3d_is_front(widget_image3d_t *img);

/**
 * @brief 判断3D图片是否包含屏幕上的坐标
 * @param[in] img:3D图片Widget
 * @param[in] x:屏幕坐标x
 * @param[in] y:屏幕坐标y
 * @return 是否包含
 **/
bool widget_image3d_contains(widget_image3d_t *img, s16 x, s16 y);

//============= Widget Axis 3D ==============//
/**
 * @brief 创建轴，轴的三维默认为900, 900, 900
 * @param[in] parent:父Widget
 * @return widget_axis3d_t
 **/
widget_axis3d_t *widget_axis3d_create(widget_page_t *parent);

/**
 * @brief 设置轴的视距，越小透视效果越明显
 * @param[in] axis:轴Widget
 * @param[in] distance:视距
 * @return 无
 **/
void widget_axis3d_set_distance(widget_axis3d_t *axis, s16 distance);

/**
 * @brief 设置轴的俯视角
 * @param[in] axis:轴Widget
 * @param[in] angle:俯视角
 * @return 无
 **/
void widget_axis3d_set_overlook(widget_axis3d_t *axis, s16 angle);

/**
 * @brief 设置轴的旋转球心坐标
 * @param[in] axis:轴Widget
 * @param[in] x, y, z:球心坐标
 * @return 无
 **/
void widget_axis3d_set_pos(widget_axis3d_t *axis, s16 x, s16 y, s16 z);

/**
 * @brief 设置轴的球坐标
 * @param[in] img:轴Widget
 * @param[in] sph:球坐标
 * @return 无
 **/
void widget_axis3d_set_sph(widget_axis3d_t *axis, sph_t sph);

/**
 * @brief 设置轴的极角
 * @param[in] axis:轴Widget
 * @param[in] angle:极角
 * @return 无
 **/
void widget_axis3d_set_polar(widget_axis3d_t *axis, s16 angle);

/**
 * @brief 设置轴的方位角
 * @param[in] axis:轴Widget
 * @param[in] angle:方位角
 * @return 无
 **/
void widget_axis3d_set_azimuth(widget_axis3d_t *axis, s16 angle);

/**
 * @brief 设置轴的自旋角
 * @param[in] axis:轴Widget
 * @param[in] angle:自旋角
 * @return 无
 **/
void widget_axis3d_set_rotation(widget_axis3d_t *axis, s16 angle);

/**
 * @brief 轴滚动
 * @param[in] axis:轴Widget
 * @param[in] roll_polar:极角
 * @param[in] roll_azimuth:方位角
 * @return 滚动后的球坐标
 **/
sph_t widget_axis3d_roll(widget_axis3d_t *axis, s16 roll_polar, s16 roll_azimuth);

/**
 * @brief 轴滚动
 * @param[in] axis:轴Widget
 * @param[in] sph:起始球坐标
 * @param[in] roll_polar:极角
 * @param[in] roll_azimuth:方位角
 * @return 滚动后的球坐标
 **/
sph_t widget_axis3d_roll_from(widget_axis3d_t *axis, sph_t sph, s16 roll_polar, s16 roll_azimuth);

//============= Widget Icon ==============//

/**
 * @brief 创建图标控件(和wiget_image的区别：只有简单的贴图和缩放功能，但是更省Buffer)
 * @param[in] parent:父控件
 * @param[in] res_addr:图片地址，在ui.h中获取
 * @return 图标控件
 **/
widget_icon_t *widget_icon_create(widget_page_t *parent, u32 res_addr);

/**
 * @brief 判断是否是一个图标控件
 * @param[in] icon:任意widget
 * @return true or false
 **/
bool widget_is_icon(widget_icon_t *icon);

/**
 * @brief 设置图标的图片
 * @param[in] icon:图标控件
 * @param[in] res_addr:图片地址，在ui.h中获取
 * @return 无
 **/
void widget_icon_set(widget_icon_t *icon, u32 res_addr);

/**
 * @brief 从RAM中设置图标的图片
 * @param[in] icon:图标控件
 * @param[in] buf:图片在RAM中的缓存地址，需要提前手动加载
 * @return 无
 **/
void widget_icon_set_ram(widget_icon_t *icon, void *buf);


//============= Widget Text ==============//

/**
 * @brief 创建一个文本控件.
 * @param[in] parent:父控件
 * @param[in] max_word_cnt:最多多少个字(不分全半角)
 * @return 文本控件
 **/
widget_text_t *widget_text_create(widget_page_t *parent, u16 max_word_cnt);

/**
 * @brief 设置文本字库, 同时清空文本内容(需要重新设置文本内容)
 * @param[in] txt:文本控件
 * @param[in] font_res_addr:字库地址, 0则表示用回系统字体
 * @return 无
 **/
void widget_text_set_font(widget_text_t *txt, u32 font_res_addr);

/**
 * @brief 清空文本内容.
 * @param[in] txt:文本控件
 * @return 无
 **/
void widget_text_clear(widget_text_t *txt);

/**
 * @brief 设置文本内容.
 * @param[in] txt:文本控件
 * @param[in] text:字符串文本内容
 * @return 无
 **/
void widget_text_set(widget_text_t *txt, const char *text);

/**
 * @brief 设置文本内容，带部首位置属性.
 * @param[in] txt:文本控件
 * @param[in] ubuf:unicode buf 字符编码
 * @param[in] tbuf:type buf 位置属性buf，和ubuf对应，枚举本h文件头部
 * @return 无
 **/
void widget_text_set_with_type(widget_text_t *txt, u16 *ubuf, u8 *tbuf);

/**
 * @brief 设置文本颜色.
 * @param[in] txt:文本控件
 * @param[in] color:rgb565颜色值
 * @return 无
 **/
void widget_text_set_color(widget_text_t *txt, u16 color);

/**
 * @brief 设置文本框文字间的行间距
 * @param[in] textbox : 文本指针
 * @param[in] hspace : 行间距
 **/
void widget_text_set_hspace(widget_text_t *txt, s8 hspace);

/**
 * @brief 设置文本框文字是否单词整体换行
 * @param[in] textbox : 文本指针
 * @param[in] wholewrap : 是否换行
 **/
void widget_text_set_wholewrap(widget_text_t *txt, bool wholewrap);

/**
 * @brief 设置文本框文字是否等宽行间距
 * @param[in] textbox : 文本指针
 * @param[in] equal_hspace : 是否等宽行间距
 **/
void widget_text_set_equal_hspace(widget_text_t *txt, bool equal_hspace);

/**
 * @brief 设置是否支持自适应大小
 * @param[in] txt:文本控件
 * @param[in] autosize:true or false
 * @return 无
 **/
void widget_text_set_autosize(widget_text_t *txt, bool autosize);

/**
 * @brief 设置文本是否自动换行。
 * @param[in] txt:文本控件
 * @param[in] wordwrap:true or false
 * @return 无
 **/
void widget_text_set_wordwrap(widget_text_t *txt, bool wordwrap);

/**
 * @brief 设置文本排版方向
 * @param[in] txt:文本控件
 * @param[in] direction:true 倒序(阿拉伯/波斯/希伯来文从右到左)排版, false 正序(CJK等从左到右)排版
 * @return 无
 **/
void widget_text_set_direction(widget_text_t *txt, bool direction);

/**
 * @brief 获取文本排版方向
 * @param[in] txt:文本控件
 * @param[in] direction:true 倒序(阿拉伯/波斯/希伯来文从右到左)排版, false 正序(CJK等从左到右)排版
 * @return 无
 **/
bool widget_text_get_direction(widget_text_t *txt);

/**
 * @brief 设置超过文本框大小的内容是否使用省略号代替
 * @param[in] txt:文本控件
 * @param[in] ellipsis:true or false
 * @return 无
 **/
void widget_text_set_ellipsis(widget_text_t *txt, bool ellipsis);

/**
 * @brief 获取超过文本框大小的内容是否使用省略号代替
 * @param[in] txt:文本控件
 * @return ellipsis:true or false
 **/
bool widget_text_get_ellipsis(widget_text_t *txt);

/**
 * @brief 设置Client相对左上的坐标。0,0则对齐到左上角(用于文字滚动)
 * @param[in] txt:文本控件
 * @param[in] rel_x:以文本布局方向为参考系，内容相对于控件左上角x坐标
 * @param[in] rel_y:以文本布局方向为参考系，内容相对于控件左上角y坐标
 * @return 无
 **/
void widget_text_set_client(widget_text_t *txt, s16 rel_x, s16 rel_y);

/**
 * @brief 获取Client相对左上的坐标
 * @param[in] txt:文本控件
 * @param[in] *rel_x:获取x坐标
 * @param[in] *rel_y:获取y坐标
 * @return 无
 **/
void widget_text_get_client(widget_text_t *txt, s16 *rel_x, s16 *rel_y);

/**
 * @brief 以文本布局方向为参考系，获取字体区域
 * @param[in] txt:文本控件
 * @return 文本内容的宽度和高度
 **/
area_t widget_text_get_area(widget_text_t *txt);

/**
 * @brief 以文本布局方向为参考系，获取文本框区域
 * @param[in] txt:文本控件
 * @return 文本框的宽度和高度
 **/
area_t widget_text_get_box_area_rel(widget_text_t *txt);

/**
 * @brief 获取文字行数
 * @param[in] txt:文本控件
 * @return ：行数
 **/
u8 widget_text_get_line_cnt(widget_text_t *txt);

/**
 * @brief 获取文字行数
 * @param[in] txt:文本控件
 * @return ：像素点数
 **/
u16 widget_text_get_autoroll_circ_pixel(widget_text_t *txt);

/**
 * @brief 获取系统字高(典型值，非最大值)
 * @param[in] 无
 * @return 系统字库字体字高，无中文时返回0，需要用max height函数获取高度
 **/
u8 widget_text_get_height(void);

/**
 * @brief 获取系统最大字高
 * @param[in] 无
 * @return 系统最大字高
 **/
u8 widget_text_get_max_height(void);

/**
 * @brief 设置排版方向
 * @param[in] right_align:排版方向是否右对齐
 * @return 无
 **/
void widget_text_set_right_align(widget_text_t *txt, bool right_align);

/**
 * @brief 设置滚动模式
 * @param[in] txt : 文本指针
 * @param[in] mode : 滚动模式，0:不滚动, 1:环形滚动 2、左右、普通滚动
 **/
void widget_text_set_autoroll_mode(widget_text_t *txt, u8 autoroll_mode);

/**
 * @brief 设置环形滚动间隔
 * @param[in] txt : 文本指针
 * @param[in] space_pixel : 环形滚动间隔，以英文空格为单位，默认是50
 **/
void widget_text_set_autoroll_circ_space(widget_text_t *txt, u16 space_pixel);

/**
 * @brief 获取排版方向
 * @param[in] 无
 * @return 排版方向1:右对齐 0:左对齐
 **/
bool widget_text_get_right_align(widget_text_t *txt);

/**
 * @brief 获取布局方向
 * @param[in] 无
 * @return 布局方向0:横向布局 1:竖向布局
 **/
int widget_text_get_layout(widget_text_t *txt);

//============= Widget Rect ==============//

/**
 * @brief 创建一个矩形控件
 * @param[in] parent：父控件
 * @return 矩形控件
 **/
widget_rect_t *widget_rect_create(widget_page_t *parent);

/**
 * @brief 设置矩形的填充颜色
 * @param[in] rect:矩形控件
 * @param[in] color:填充颜色
 * @return 无
 **/
void widget_rect_set_color(widget_rect_t *rect, u16 color);


/**
 * @brief 设置矩形的圆角半径
 * @param[in] rect:矩形控件
 * @param[in] r:圆角半径
                当半大于高或宽的一半时，底层自动把半径设置为其中最小的；(高或宽的一半)
 * @return 无
 **/
void widget_rect_set_radius(widget_rect_t *rect, u16 r);


//============= Widget QRCode ==============//

enum {
    QRCODE_TYPE_2D,                 //二维码
    QRCODE_TYPE_HORIZONTAL,         //横向条形码
    QRCODE_TYPE_VERTICAL,           //竖向条形码
};

/**
 * @brief 创建Widget QRCode
 * @param[in] parent：父控件
 * @param[in] qr_type：类型
 * @param[in] max_ch_cnt:需要支持的最大字符数
 * @return 矩形控件
 **/
widget_qrcode_t *widget_qrcode_create(widget_page_t *parent, u8 qr_type, u16 max_ch_cnt);

/**
 * @brief 设置QRCode
 * @param[in] qrcode：码控件
 * @param[in] str：内容信息
 * @return 无
 **/
void widget_qrcode_set(widget_qrcode_t *qrcode, const char *str);


/**
 * @brief 设置二维码QRCode
 * @param[in] qrcode：码控件
 * @param[in] code: 数据，可以是字符串，或是数字
 * @param[in] len: 数据长度(字节)
 * @return 无
 **/
void widget_qrcode_2d_set(widget_qrcode_t *qrcode, const char *code, int len);


/**
 * @brief 设置单位宽度, 同时会自动计算更新QRCode的大小
 * @param[in] qrcode：码控件
 * @param[in] bit_wid：像素宽度
 * @return 无
 **/
void widget_qrcode_set_bitwid(widget_qrcode_t *qrcode, u8 bit_wid);

/**
 * @brief 设置显示宽度, 同时会自动计算更新QRCode的大小
 * @param[in] qrcode：码控件
 * @param[in] qr_wid：码宽度
 * @return 无
 **/
void widget_qrcode_set_bitwid_by_qrwid(widget_qrcode_t *qrcode, u16 qr_wid);

/**
 * @brief 设置最大显示宽度, 同时会自动计算更新QRCode的大小
 * @param[in] qrcode：码控件
 * @param[in] max_wid：最大显示宽度
 * @return 无
 **/
void widget_qrcode_set_maxwid(widget_qrcode_t *qrcode, u16 max_wid);

/**
 * @brief 获取生成码的最小宽度(显示是以这个为基数放大N倍)
 * @param[in] qrcode：码控件
 * @return 宽度
 */
int widget_qrcode_get_bit_cnt(widget_qrcode_t *qrcode);

/**
 * @brief 设置显示宽度, 同时会自动计算更新QRCode的大小
 * @param[in] qrcode：码控件
 * @param[in] level：码纠错等级
 * @return 无
 **/
void widget_qrcode_set_level(widget_qrcode_t *qrcode, u8 level);

/**
 * @brief 设置二维码的编码格式
 * @param[in] qrcode：码控件
 * @param[in] type: 0 code128A, 1 code 128C(不支持字符串，优点比较小)
 * @return 无
 **/
void widget_barcode_set_type(widget_qrcode_t *qrcode, u8 type);

//============= Widget ARC ==============//

/**
 * 创建圆弧类型的widget
 * parent:父亲
   注意: widget_set_size 或 widget_set_location设置宽度；来设置圆弧的大小；所以没有在单独创建其他函数
*/
widget_arc_t *widget_arc_create(widget_page_t *parent);

/**
 * 设置圆弧角度
 * arc:arc widgeit
 * start_angle:起始角度，单位是0.1度(参考位置在右水平线上，可以任意角度，软件自动3600对齐，如下图)
 * end_angle  :结束角度，单位是0.1度(参考位置在右水平线上，可以任意角度，软件自动3600对齐，如下图)
   起始和结束角度没有限制谁大谁小
                  /|\ 270.0 度
                   |
                   |
                   |
                   |
180.0度             |
-------------------|---------------->  0 度
                   |
                   |
                   |
                   |
                   |
                   | 90.0 度
*/
void widget_arc_set_angles(widget_arc_t *arc, u16 start_angle, u16 end_angle);

/**
 * 设置圆弧颜色
 * arc:arc widgeit
 * color_intra:设置角度内的颜色
 * color_outre:设置角度外的颜色
   比如设置的是30度到90度，30到90度圆弧内颜色是color_intra；其他圆弧是color_outre
*/
void widget_arc_set_color(widget_arc_t *arc, u16 color_intra, u16 color_outre);

/**
 * 设置圆弧透明度
 * arc:arc widgeit
 * alpha_intra:设置角度内的透明度
 * alpha_outre:设置角度外的透明度
   比如设置的是30度到90度，30到90度圆弧内透明度是alpha_intra；其他圆弧是alpha_outre
*/
void widget_arc_set_alpha(widget_arc_t *arc, u8 alpha_intra, u8 alpha_outre);

/**
 * 设置圆弧透宽度
 * arc:arc widgeit
 * arc_width:圆弧宽度(设置两个圆中间的那段圆弧宽度，R1-R2)
   不是半径哦，是内外半径差这个东东
*/
void widget_arc_set_width(widget_arc_t *arc, u16 arc_width);

/**
 * 设置圆弧截面是否圆弧化
 * arc:arc widgeit
 * start_onoff: 开始角度位置 1:圆弧化 0:直线化
   end_onoff  : 结束角度位置 1:圆弧化 0:直线化
   圆弧化后目前缩放对齐有点偏差
*/
void widget_arc_set_edge_circle(widget_arc_t *arc, bool start_onoff, bool end_onoff);

/**
 * 创建实心圆类型的widget
 * parent:父亲
   注意: widget_set_size 或 widget_set_location设置宽度；来设置圆的大小；所以没有在单独创建其他函数
*/
widget_circle_t *widget_circle_create(widget_page_t *parent);

/**
 * 设置实心圆颜色
 * circle:circle widgeit
 * color_intra:颜色
 * alpha_intra:透明度
   比如设置的是30度到90度，30到90度圆弧内颜色是color_intra；其他圆弧是color_outre
*/
void widget_circle_set_acolor(widget_circle_t *circle, u16 color_intra, u8 alpha_intra);

//============= Widget chart ==============//
/**
 * 创建图表类型的widget
 * parent:父亲
 * type:选择图表类型，直角还是圆角
 * max_num:需要最大的柱形数量
   max_num设置一个最大值，实际使用数量用后面widget_chart_set_real_num函数来设置
*/
widget_chart_t *widget_chart_create(widget_page_t *parent, WGT_CHART_TYPE type, u8 max_num);


/**
 * 设置图表的等效范围
 * chart:图表
 * range_x：X轴的范围
 * range_y: Y轴的范围
   图表的高和宽是widget_set_size 或 widget_set_location设置；
   range_x和range_y是设置多少等份；举例widget_set_size设置的宽度是200像素，range_x设置100，表示将200分成100等份；
   这样做的原因是参考LVGL，方便客户代码通用性；
*/
void widget_chart_set_range(widget_chart_t *chart, u16 range_x, u16 range_y);

/**
 * 设置图表的等效范围
 * chart:图表
 * real_num:设置实际使用的图表柱状个数，出现这个函数的原因是方便客户设置需要显示的数量；
   需要小于create时候设置的max_num；
*/
bool widget_chart_set_real_num(widget_chart_t *chart, u8 real_num);

/**
 * 设置图表的显示方向
 * chart:图表
 * dir：

 0垂直方向 ：
/|\ y轴正方向
 |    **      **
 |    **      **
 |    **      **
 |    **      **
 |    **      **
 |    **
 |    **
 |    **
0|    **
----------------------------------------------> x正方向


1水平方向
/|\ y轴正方向
 |
 |************
 |************
 |
 |
 |    ******
 |    ******
 |
0|
----------------------------------------------> x正方向
*/
bool widget_chart_set_dir(widget_chart_t *chart, u8 dir);


/**
 * 设置图表数据
 * chart:图表
 * id     : 柱形图序列号，0开始，0 到 real_num-1(包含real_num-1)
   x_start: 0到range_x范围
   x_end  : 0到range_x范围
   y_start: 0到range_y范围
   y_end  : 0到range_y范围
   color  : RGB565

   说明：x_start和x_end及y_start和y_end意思设置这个柱状图x和y显示区域

   举个例子，range_x设置为100，range_y设置为100；调用如下函数后：
   widget_chart_set_value(chart, 0, 10, 20, 0, 100, COLOR_RED);
   widget_chart_set_value(chart, 1, 30, 40, 50, 100, COLOR_RED);
   大致如下显示

/|\ y轴正方向
 |    **      **
 |    **      **
 |    **      **
 |    **      **
 |    **      **
 |    **
 |    **
 |    **
0|    **
----------------------------------------------> x正方向
*/
bool widget_chart_set_value(widget_chart_t *chart, u8 id, u16 x_start, u16 x_end, u16 y_start, u16 y_end, u16 color);

/**
 * 创建线条类型的widget
 * parent:父亲
 * item_max:设置最大多少个点
            每个点和点会用线连接
*/
widget_line_t *widget_line_create(widget_page_t *parent, u16 item_max);

/**
 * 一次给线条widget设置item_num个点
 * line:线条 widget
 * item_buff: point类型的点buff
 * item_num : buff里有多少个点
*/
bool widget_line_set_item(widget_line_t *line, point_t *item_buff, u16 item_num);

/**
 * 一次给线条widget 添加1个点
 * line:线条 widget
 * item_point: point类型的点
   这个函数一次添加一个点，和已有的点链接一起
*/
bool widget_line_add_item(widget_line_t *line, point_t item_point);

/**
 * 一次给线条widget 添加1个点x,y坐标
 * line:线条 widget
 * x: x位置
 * y: y位置
   这个函数一次添加一个点，和已有的点链接一起
*/
bool widget_line_add_item_xy(widget_line_t *line, s16 x, s16 y);

/**
 * 修改第N点数据
 * line:线条 widget
 * item_id: 第几个点数据
 * item_point: point类型的点
*/
bool widget_line_modify_item(widget_line_t *line, u16 item_id, point_t item_point);

/**
 * 设置线条颜色
 * line:线条 widget
 * color: 颜色
*/
bool widget_line_set_color(widget_line_t *line, u16 color);

/**
 * 设置线条宽度
 * line:线条 widget
 * width: 宽度
*/
bool widget_line_set_width(widget_line_t *line, u16 width);

/**
 * 线条是开始和结束是否圆角
 * line:线条 widget
 * onof: 1是，0否
        默认为0
*/
bool widget_line_set_circle(widget_line_t *line, bool onof);

/**
 * 获取line widget上已经有的点个数
 * line:线条 widget
 * return: 返回点个数
*/
u16 widget_line_get_item_num(widget_line_t *line);

/**
 * 获取指定id的point点结构体指针
 * line:线条 widget
 * return: 返回point结构体指针，非法返回NULL
 */
point_t * widget_line_get_point(widget_line_t *line, u16 item_id);

/**
 * 是否虚线化
 * line:线条 widget
 * is_dashes ：1虚线；0实线
 * dashes_pixel：虚实线之间的间距
 */
void widget_line_set_dashes(widget_line_t *line, bool is_dashes, u16 dashes_pixel);

///**
// * 创建贝塞尔曲线类型线条(暂时只有2阶运算能力)
// * parent:父亲
// * pixel_size: 一般指x轴的大小；比如可以设置GUI_SCREEN_WIDTH
// **/
//widget_bezier_t *widget_bezier_create(widget_page_t *parent, u16 pixel_size);

///**
// * 一次给线条bezier设置item_num个点
//   (2阶计算，需要大于3个点)
// * bezier:线条 widget
// * item_buff: point类型的点buff
// * item_num : buff里有多少个点
// * return: false失败，true成功
//*/
//bool widget_bezier_set_item(widget_bezier_t *bezier, point_t *item_buff, u16 item_num);

/**
 * 创建bar类型的widget(进度条)
 * parent:父亲
*/
widget_bar_t *widget_bar_create(widget_page_t *parent);

/**
 * 设置bar的取值范围
 * bar: bar控件
   min：范围最小值
   max：范围最大值
*/
void widget_bar_set_range(widget_bar_t *bar, int32_t min, int32_t max);

/**
 * 设置bar的设置值
 * bar: bar控件
   value：取值
          bar会按这个值和上面设置的范围显示
*/
void widget_bar_set_value(widget_bar_t *bar, int32_t value);

/**
 * 设置bar的设置颜色
 * bar: bar控件
   color1：进度条颜色（理解为设置值的占比颜色）
   color2：底部颜色（理解为底部颜色）
*/
void widget_bar_set_color(widget_bar_t *bar, u16 color1, u16 color2);

/**
 * 设置bar是水平或垂直
 * bar: bar widget
   vh：0 垂直方向
       1 水平方向

 * 当设置为垂直方向后，上部分按value设置的比例显示color1颜色；剩下下面部分用color2显示；
 * 当设置为水平方向后，左边分按value设置的比例显示color1颜色；剩下右边部分用color2显示；
*/
void widget_bar_set_vh(widget_bar_t *bar, u16 vh);

/**
 * @brief 设置矩形的圆角半径
 * @param[in] bar:bar控件
 * @param[in] r:圆角半径
                当半大于高或宽的一半时，底层自动把半径设置为其中最小的；(高或宽的一半)
 * @return 无
 **/
void widget_bar_set_rabardius(widget_bar_t *bar, u16 r);

/**
 * @brief 设置哪个角需要圆角化
 * @param[in] bar:bar控件
 * @param[in] corner:BIT(0):左上; BIT(1):右上; BIT(2):左下; BIT(3):右下;
 * @return 无
 **/
void widget_bar_set_corner(widget_rect_t *bar, u8 corner);

/**
 * @brief 获取元素buf剩余容量
 * @return 剩余字节数
 **/
u32 element_remain_cap_get(void);

void gui_set_screenshot(u8 *addr);
bool gui_get_screenshot(void);

#endif // _API_GUI_H
