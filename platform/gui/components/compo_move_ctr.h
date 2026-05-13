#ifndef __COMPO_MOVE_CTR_H
#define __COMPO_MOVE_CTR_H

#define MOVE_AUTO_TICK_EXPIRE 10  //自动滚动间隔，单位ms 不少于10ms
//长页面信息
typedef struct {
    bool        title_used;         //是否使用了标题栏
    uint16_t    page_size;          //分页大小
    uint8_t     page_count;         //页面数量
    uint8_t     dir;                //0:纵向  1:横向
    uint8_t     jump_perc;          //超过每页大小比例则滚动下一页，否则回弹(0表示不自动滚动)
    uint16_t    quick_jump_perc;    //快速触摸滑动自动滚动距离为每页大小比例(0表示不使用该功能)
    uint16_t    up_over_perc;       //顶部可超过每页大小比例，超过回弹
    uint16_t    down_over_perc;     //底部可超过每页大小比例，超过回弹
    uint16_t    down_spring_perc;   //底部回弹后留白为每页大小比例
    uint16_t    anim_time_ms;       //滚动时间(0表示使用默认值)
} page_move_info_t;

//控制块过程变量
typedef struct {
    widget_page_t*      page;
    page_move_info_t    info;
    uint32_t            tick;
    uint32_t            tick_quick_slide;
    bool                drag_flag;
    int16_t             last_move;
    int16_t             move_offset;
    int16_t             auto_move_offset;
    int16_t             up_border_offset;
    int16_t             down_border_offset;
    int16_t             down_spring_offset; //长页面底部offs，顶部默认为0
    int16_t             offs_sqrt;
    int16_t             offs_sqrt_step;
} page_tp_move_t;


/**
 * @brief 长页面滚动控制块初始化
 * @param[in] ptm : 滚动控制块
 * @param[in] page : 长页面对象
 * @param[in] info : 长页面信息
 * @return 无
 **/
void compo_page_move_init(page_tp_move_t *ptm, widget_page_t *page, page_move_info_t *info);

/**
 * @brief 长页面滚动触摸标志设置
 * @param[in] ptm : 滚动控制块
 * @return 无
 **/
void compo_page_move_touch_handler(page_tp_move_t *ptm);

/**
 * @brief 长页面滚动处理过程
 * @param[in] ptm : 滚动控制块
 * @return 无
 **/
void compo_page_move_process(page_tp_move_t *ptm);

/**
 * @brief 获取长页面当前offset
 * @param[in] ptm : 滚动控制块
 * @return 无
 **/
int16_t compo_page_move_get_offset(page_tp_move_t *ptm);

/**
 * @brief 长页面自动滚动偏移值计算
 * @param[in] ptm : 滚动控制块
 * @param[in] offset_dest : 目标位置
 * @return 无
 **/
void compo_page_move_to(page_tp_move_t *ptm, int16_t offset_dest);

/**
 * @brief 长页面按偏移值自动滚动
 * @param[in] ptm : 滚动控制块
 * @param[in] offset : 滚动偏移（负数为向上滚动）
 * @return 无
 **/
void compo_page_move_set(page_tp_move_t *ptm, int16_t offset);

/**
 * @brief 长页面按分页数自动滚动
 * @param[in] ptm : 滚动控制块
 * @param[in] pages : 滚动页数（负数为向下滚动）
 * @return 无
 **/
void compo_page_move_set_by_pages(page_tp_move_t *ptm, int8_t pages);

#endif
