#ifndef _COMPO_LISTBOX_H
#define _COMPO_LISTBOX_H

#define LISTBOX_MAX_ITEM_CNT            40

#define LISTBOX_ITEM_CNT                10       //每页列表，中心上下各保留3个进行滚动
#define LISTBOX_TEXT_LEN                30      //自定义文字的长度

enum COMPO_LISTBOX_STYLE {
    COMPO_LISTBOX_STYLE_MENU_NORMAL,            //普通菜单
    COMPO_LISTBOX_STYLE_MENU_CIRCLE,            //圆屏弧形菜单
    COMPO_LISTBOX_STYLE_TITLE,                  //带Title的列表，头部不做缩放
    COMPO_LISTBOX_STYLE_TITLE_NORMAL,           //带Title的普通列表，头部底部不做缩放
    COMPO_LISTBOX_STYLE_SELECT,                 //选择模式
    COMPO_LISTBOX_STYLE_LANGUAGE,               //语言选择模式, 列表文本不使用i18n的Radio模式
    COMPO_LISTBOX_STYLE_MENU_FOURGRID,          //四宫格列表菜单
};

enum COMPO_LISTBOX_ITEM_MODE {
    COMPO_LISTBOX_ITEM_MODE_NORMAL,             //普通菜单
    COMPO_LISTBOX_ITEM_MODE_SWITCH,             //选择开关
};

//列表框移动控制命令
enum COMPO_LISTBOX_MOVE_CMD {
    COMPO_LISTBOX_MOVE_CMD_DRAG,                //开始拖动
    COMPO_LISTBOX_MOVE_CMD_FORWARD,             //向前滚动
    COMPO_LISTBOX_MOVE_CMD_BACKWARD,            //向后滚动
};

//列表框当前状态
enum COMPO_LISTBOX_STA {
    COMPO_LISTBOX_STA_IDLE,                     //空闲状态
    COMPO_LISTBOX_STA_DARG,                     //拖动中
    COMPO_LISTBOX_STA_MOVE,                     //移动中
};

//列表框时钟
enum COMPO_LISTBOX_TIME_TYPE {
    COMPO_LISTBOX_TIME_TYPE_HOUR,
    COMPO_LISTBOX_TIME_TYPE_MIN,
    COMPO_LISTBOX_TIME_TYPE_SEC,
};

//菜单项定义
typedef struct compo_listbox_item_t_ {
    u32 str_idx;                                //文字
    u32 res_addr;                               //图标
    u8 item_mode;                               //菜单模式
    union {                                     //菜单参数
        u8 func_sta;                            //普通菜单模式
        u8 menu_style;                          //列表用于菜单风格选择
        u16 vidx;                               //选择开关Switch绑定的系统位变量vidx
    };
} compo_listbox_item_t;

//菜单自定义项定义，列表中内容需要动态更新的在這里创建
typedef struct compo_listbox_custom_item_t_ {
    char str_txt[LISTBOX_TEXT_LEN];          	//自定义文字

} compo_listbox_custom_item_t;

//列表移动控制
typedef struct compo_listbox_move_cb_t_ {
    u32 tick;
    s32 focus_y;
    s32 focus_dy;
    s32 focus_ystep;
    int moveto_idx;                 //设定自动移到的目标菜单
    s32 moveto_y;                   //设定自动移到的y坐标
    s32 first_y;                    //首图标坐标
    s32 last_y;                     //尾图标坐标
    bool flag_drag;                 //开始拖动
    bool flag_move_auto;            //自动移到坐标
} compo_listbox_move_cb_t;

typedef struct compo_listbox_t_ {
    COMPO_STRUCT_COMMON;
    compo_listbox_move_cb_t *mcb;   //移动控制
    widget_page_t *page;
    widget_page_t *sel_page;        //选择的item page
    widget_icon_t *sel_icon;        //选择的ICON
    widget_icon_t *sel_icon2;       //状态的ICON
    int sel_idx;                    //选择的idx
    u32 style;                      //列表框样式

    compo_listbox_item_t const *item;
    compo_listbox_custom_item_t *custom_item;
    int item_cnt;
    int total_height;
    u16 cycle_height;               //滑动一圈回到原点经过的距离

    area_t icon_area;
    u8 (*get_bit)(uint n);          //获取位变量接口

    u32 res_sta_icon1;              //状态图标资源
    u32 res_sta_icon2;

    u16 line_height;
    u16 line_space;
    u16 line_height_total;          //line_height + line_space
    u16 line_center_y;

    u16 item_width;
    u16 item_height;                //无图标和背景资源时，可以手动设置item高度
    s16 ofs_y;

    s16 focus_icon_idx;              //焦点的图标编号
    bool flag_sel_icon2;            //是否选中状态图标
    bool flag_text_center;          //是否居中文本
    bool flag_text_modify;          //是否修改文本内容
    bool flag_cycle;                //是否循环滑动

    u8 flag_area;                   //是否点击特定区域
    int sidx;                       //起始编号
    bool flash_read_flag;
    bool txt_roll_need_rst;         //文本滚动重置标志
    widget_page_t *item_page[LISTBOX_ITEM_CNT];
    widget_icon_t *item_bgimg[LISTBOX_ITEM_CNT];
    widget_icon_t *item_icon[LISTBOX_ITEM_CNT];
    widget_text_t *item_text[LISTBOX_ITEM_CNT];
    widget_icon_t *item_icon2[LISTBOX_ITEM_CNT];
    widget_rect_t *item_rect[LISTBOX_ITEM_CNT];
    compo_roll_cb_t roll_cb[LISTBOX_ITEM_CNT];
    s16 item_idx[LISTBOX_ITEM_CNT];

    u8 idx_time;				//加入时分秒针元素
	widget_page_t *page_time;
	widget_icon_t *time_bg;
	widget_image_t *hour;
	widget_image_t *min;
	widget_image_t *sec;
	s16 start_angle;

} compo_listbox_t;

/**
 * @brief 创建一个列表控件
 * @param[in] frm : 窗体指针
 * @param[in] style : COMPO_LISTBOX_STYLE_MENU_NORMAL, 普通菜单
                      COMPO_LISTBOX_STYLE_MENU_CIRCLE, 圆屏弧形菜单
 * @return 返回列表指针
 **/
compo_listbox_t *compo_listbox_create(compo_form_t *frm, u32 style);

/**
 * @brief 设置列表控件内容
 * @param[in] listbox : 列表指针
 * @param[in] item : 存放列表信息，图片、文字等
 * @param[in] item_cnt : 存放列表信息的数量
 **/
void compo_listbox_set(compo_listbox_t *listbox, compo_listbox_item_t const *item, int item_cnt);

/**
 * @brief 设置列表项矩形背景的大小和圆角半径
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] listbox : 列表指针
 * @param[in] width : 矩形宽度
 * @param[in] height : 矩形高度
 * @param[in] r : 半径
 **/
void compo_listbox_set_rect_size(compo_listbox_t *listbox, s16 width, s16 height, u16 r);

/**
 * @brief 设置列表项矩形背景的颜色
 * @param[in] listbox : 列表指针
 * @param[in] color : 颜色
 **/
void compo_listbox_set_rect_color(compo_listbox_t *listbox, u16 color);

/**
 * @brief 设置列表项矩形背景是否可见
 * @param[in] listbox : 列表指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_listbox_set_rect_visible(compo_listbox_t *listbox, bool visible);

/**
 * @brief 设置列表项宽度（需要放在其他属性设置之前，如bgimg设置之前）
 * @param[in] listbox : 列表指针
 * @param[in] wid : 列表项宽度
 **/
void compo_listbox_set_item_width(compo_listbox_t *listbox, u16 wid);


/**
 * @brief 设置列表框背景
 * @param[in] listbox : 列表指针
 * @param[in] res_addr : 图像资源地址
 **/
void compo_listbox_set_bgimg(compo_listbox_t *listbox, u32 res_addr);

/**
 * @brief 设置列表框中心焦点背景（高亮效果）
          注意需要在设置背景后调用
 * @param[in] listbox : 列表指针
 * @param[in] res_addr : 图像资源地址
 **/
void compo_listbox_set_focus_bgimg(compo_listbox_t *listbox, u32 focus_res_addr);

/**
 * @brief 设置状态图标 ON, OFF 的图像资源地址
 * @param[in] listbox : 列表指针
 * @param[in] res_addr1 : 图像资源地址
 * @param[in] res_addr2 : 图像资源地址
 **/
void compo_listbox_set_sta_icon_res(compo_listbox_t *listbox, u32 res_addr1, u32 res_addr2);

/**
 * @brief 设置状态图标 ON, OFF 的位置
 * @param[in] listbox : 列表指针
 * @param[in] x : x轴坐标, 0则表示按默认坐标
 * @param[in] y : y轴坐标, 0则表示按默认坐标
 **/
void compo_listbox_set_sta_icon_pos(compo_listbox_t *listbox, s16 x, s16 y);

/**
 * @brief 设置状态图标 ON, OFF 是否置顶
 * @param[in] listbox : 列表指针
 * @param[in] flag_top: 是否置顶
 * @return 无
 **/
void compo_listbox_set_sta_icon_top(compo_listbox_t *listbox, bool flag_top);

/**
 * @brief 设置状态图标 ON, OFF
 * @param[in] listbox : 列表指针
 * @param[in] res_addr1 : 图像资源地址
 * @param[in] res_addr2 : 图像资源地址
 **/
void compo_listbox_set_sta_icon(compo_listbox_t *listbox, u32 res_addr1, u32 res_addr2);

/**
 * @brief 设置获取位变量回调函数
 * @param[in] listbox : 列表指针
 * @param[in] get_bit : 获取位变量回调函数
 **/
void compo_listbox_set_bithook(compo_listbox_t *listbox, u8 (*get_bit)(uint n));

/**
 * @brief 根据列表编号获取Y坐标
 * @param[in] listbox : 列表指针
 * @param[in] idx : 菜单编号
 * @return y坐标
 **/
int compo_listbox_gety_byidx(compo_listbox_t *listbox, int idx);

/**
 * @brief 按Y坐标设置列表框焦点
 * @param[in] listbox : 列表指针
 * @param[in] y : Y坐标
 **/
void compo_listbox_set_focus(compo_listbox_t *listbox, s16 y);

/**
 * @brief 将选择的图标设为焦点
 * @param[in] listbox : 列表指针
 * @param[in] idx : 图标编号
 **/
void compo_listbox_set_focus_byidx(compo_listbox_t *listbox, int idx);

/**
 * @brief 更新列表框Widget
 * @param[in] listbox : 列表指针
 **/
void compo_listbox_update(compo_listbox_t *listbox);

/**
 * @brief 更新列表框Widget，同时重置所有文本滚动（item中str突变时需要调用，防止C208蓝屏）
 * @param[in] listbox : 列表指针
 **/
void compo_listbox_update_with_text_scroll_rst(compo_listbox_t *listbox);

/**
 * @brief 按坐标选择菜单项
 * @param[in] listbox : 列表指针
 * @param[in] pt : 坐标
 * @return 返回菜单项索引
 **/
int compo_listbox_select(compo_listbox_t *listbox, point_t pt);

/**
 * @brief 按坐标选择菜单项
 * @param[in] listbox : 列表指针
 * @return 返回是否选择状态图标
 **/
bool compo_listbox_is_sel_sta(compo_listbox_t *listbox);

/**
 * @brief 按索引选择图标
 * @param[in] listbox : 列表指针
 * @param[in] idx : 编号
 * @return 返回图标指针
 **/
widget_icon_t *compo_listbox_select_byidx(compo_listbox_t *listbox, int idx);

/**
 * @brief 获取选择图标的区域
 * @param[in] listbox : 列表指针
 * @return 返回图标坐标和大小
 **/
rect_t compo_listbox_get_sel_rect(compo_listbox_t *listbox);

/**
 * @brief 列表框拖动与移动初始化
 * @param[in] listbox : 列表指针
 * @return 返回COMPO_LISTBOX_STA状态枚举
 **/
u8 compo_listbox_get_sta(compo_listbox_t *listbox);

/**
 * @brief 列表框拖动与移动初始化
 * @param[in] listbox : 列表指针
 **/
void compo_listbox_move_init(compo_listbox_t *listbox);

/**
 * @brief 列表框拖动与移动初始化
          用于自定义首尾图标位置的修改
 * @param[in] listbox : 列表指针
 * @param[in] first_y : 首图标坐标
 * @param[in] last_y : 尾图标坐标
 **/
void compo_listbox_move_init_modify(compo_listbox_t *listbox, s32 first_y, s32 last_y);

/**
 * @brief 列表框拖动与移动处理
 * @param[in] listbox : 列表指针
 * @param[in] mcb : 列表移动控制结构块指针
 **/
void compo_listbox_move(compo_listbox_t *listbox);

/**
 * @brief 列表框拖动与移动控制
 * @param[in] listbox : 列表指针
 * @param[in] cmd : 控制命令
 **/
void compo_listbox_move_control(compo_listbox_t *listbox, int cmd);

/**
 * @brief 设置列表坐标及大小
          用于限制page的大小，超出page的地方不显示出来
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] listbox : 列表指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @param[in] width : 列表宽度
 * @param[in] height : 列表高度
 **/
void compo_listbox_set_location(compo_listbox_t *listbox, s16 x, s16 y, s16 width, s16 height);

/**
 * @brief 设置列表控件item的高度
 * @param[in] listbox : 列表指针
 * @param[in] item_height : item高度
 **/
void compo_listbox_set_item_height(compo_listbox_t *listbox, int item_height);

/**
 * @brief 设置列表控件的文本坐标、大小、是否居中显示
          注意：需在compo_listbox_update前一步调用，避免被compo_listbox_init_update刷新了
 * @param[in] listbox : 列表指针
 * @param[in] item : 存放列表信息，图片、文字等
 * @param[in] item_cnt : 存放列表信息的数量
 **/
void compo_listbox_set_item_text(compo_listbox_t *listbox, s16 x, s16 y, s16 width, s16 height, bool align_center);

/**
 * @brief 是否修改列表控件的文本内容
 * @param[in] listbox : 列表指针
 * @param[in] custom_item : 存放自定义文本的数组
 **/
void compo_listbox_set_text_modify(compo_listbox_t *listbox, compo_listbox_custom_item_t *custom_item);

/**
 * @brief 是否使能列表循环滑动功能
 * @param[in] listbox : 列表指针
 * @param[in] cycle : true 使能循环滑动
                      false 关闭循环滑动
 **/
void compo_listbox_cycle_en(compo_listbox_t *listbox, bool cycle);

/**
 * @brief 向集合中添加时分秒针
          cx cy为旋转中心点, x y为目标坐标
 * @param[in] listbox : 图标集指针
 * @param[in] type : COMPO_ICONLIST_TIME_TYPE_HOUR, 时
                     COMPO_ICONLIST_TIME_TYPE_MIN,  分
                     COMPO_ICONLIST_TIME_TYPE_SEC,  秒
 * @param[in] res_addr : 图像资源地址
 * @param[in] cx : x轴坐标
 * @param[in] cy : y轴坐标
 **/
void compo_listbox_add_time(compo_listbox_t *listbox, u8 type, u32 res_addr, s16 cx, s16 cy);

/**
 * @brief 设置图标集指针的起始角度
 * @param[in] listbox : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_listbox_set_start_angle(compo_listbox_t *listbox, s16 angle);

/**
 * @brief 设置图标集合时钟序号
 * @param[in] listbox : 图标集指针
 * @param[in] idx : 时钟序号
 **/
void compo_listbox_set_time_idx(compo_listbox_t *listbox, u8 idx);

/**
 * @brief 设置列表是否可见
 * @param[in] listbox : 列表指针
 * @param[in] visible : 0隐藏，1可见
 **/
void compo_listbox_set_visible(compo_listbox_t *listbox, bool visible);
#endif
