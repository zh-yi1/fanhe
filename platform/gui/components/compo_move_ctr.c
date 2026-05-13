#include "include.h"

#define TRACE_EN    0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


#if (GUI_SELECT == GUI_OLED_466_ICNA3310B)
#define PAGE_TITLE_HEIGHT               (GUI_SCREEN_HEIGHT / 6) //系统标题栏高度
#else
#define PAGE_TITLE_HEIGHT               (GUI_SCREEN_HEIGHT / 8) //系统标题栏高度
#endif

#define MOVE_AUTO_TICK_EXPIRE           10  //自动滚动帧间隔，单位ms
#define MOVE_ANIMATION_TIME_DEFAULT     200 //自动滚动默认动画时间，设置时间小于帧间隔时使用
#define QUICK_SLIDE_EXPIRE              150 //滑动时间（ms）小于某值时判断为速滑
#define QUICK_SLIDE_OFFS_MIN            10 //滑动偏移大于某值时判断为速滑

/**
 * @brief 长页面滚动控制块初始化
 * @param[in] ptm : 滚动控制块
 * @param[in] page : 长页面对象
 * @param[in] info : 长页面信息
 * @return 无
 **/
void compo_page_move_init(page_tp_move_t *ptm, widget_page_t *page, page_move_info_t *info)
{
    if (NULL == ptm || NULL == page || !widget_is_page(page)) {
        return ;
    }

    memset(ptm, 0, sizeof(*ptm));
    memcpy(&ptm->info, info, sizeof(ptm->info));
#if BOX_GUI_ROTATE_DISP
    rect_t page_rect = rotate_widget_get_location(page);
#else
    rect_t page_rect = widget_get_location(page);
#endif // BOX_GUI_ROTATE_DISP
    u16 page_height = ptm->info.page_size * ptm->info.page_count + (ptm->info.page_size * ptm->info.down_spring_perc / 100);
    if (page_height > page_rect.hei) {
        ptm->down_spring_offset = page_rect.hei - page_height;    //底部回弹边沿
    } else  {
        ptm->down_spring_offset = 0;
    }
    ptm->down_border_offset = ptm->down_spring_offset - (ptm->info.page_size * ptm->info.down_over_perc / 100); //底部拖动边沿
    ptm->up_border_offset = ptm->info.page_size * ptm->info.up_over_perc / 100; //顶部拖动边沿
    if (ptm->info.anim_time_ms < MOVE_AUTO_TICK_EXPIRE) {    //防止被除数为0
        ptm->info.anim_time_ms = MOVE_ANIMATION_TIME_DEFAULT;
    }
    if (ptm->info.page_size == 0) {
#if BOX_GUI_ROTATE_DISP
        ptm->info.page_size = ptm->info.dir ? GUI_GET_SCREEN_WIDTH : GUI_GET_SCREEN_HEIGHT;
#else
        ptm->info.page_size = ptm->info.dir ? GUI_SCREEN_WIDTH : GUI_SCREEN_HEIGHT;
#endif // BOX_GUI_ROTATE_DISP
    }
    if (ptm->info.page_count == 0) {
        ptm->info.page_count = 1;
    }
    ptm->page = page;

    return ;
}

/**
 * @brief 长页面滚动触摸标志设置
 * @param[in] ptm : 滚动控制块
 * @return 无
 **/
void compo_page_move_touch_handler(page_tp_move_t *ptm)
{
    if (NULL == ptm) {
        return ;
    }
    ptm->drag_flag = true;
    ptm->last_move = 0;
    ptm->auto_move_offset = 0;
    ptm->offs_sqrt_step = 0;
    ptm->tick_quick_slide = tick_get();
}

/**
 * @brief 长页面位置刷新
 * @param[in] ptm : 滚动控制块
 * @return 无
 **/
static void app_page_move_refresh(page_tp_move_t *ptm)
{
    s16 client_pos = ptm->info.title_used ? -PAGE_TITLE_HEIGHT : 0;
    client_pos += ptm->move_offset;
#if BOX_GUI_ROTATE_DISP
    if (ptm->info.dir) {
        widget_page_set_client(ptm->page, 0, client_pos);
    } else {
        widget_page_set_client(ptm->page, -client_pos, 0);
    }
#else
    if (ptm->info.dir) {
        widget_page_set_client(ptm->page, client_pos, 0);
    } else {
        widget_page_set_client(ptm->page, 0, client_pos);
    }
#endif
}

/**
 * @brief 长页面滚动处理过程
 * @param[in] ptm : 滚动控制块
 * @return 无
 **/
void compo_page_move_process(page_tp_move_t *ptm)
{
    if (NULL == ptm) {
        return ;
    }

    if (ptm->drag_flag) {
        s32 dx, dy;
        bool press_flag = ctp_get_dxy(&dx, &dy);
        s16 cur_move = ptm->info.dir ? dx : dy;

        if (press_flag) {
            if (cur_move != ptm->last_move) {
                ptm->move_offset += (cur_move - ptm->last_move);
                ptm->move_offset = MAX(ptm->down_border_offset, MIN(ptm->up_border_offset, ptm->move_offset));
                app_page_move_refresh(ptm);
                ptm->auto_move_offset = cur_move - ptm->last_move;
                ptm->last_move = cur_move;
            }
        } else {
            ptm->drag_flag = false;

//            TRACE(">>>tick_diff:%d offs_abs:%d\n", tick_get() - ptm->tick_quick_slide, ptm->auto_move_offset);
            s16 quick_jump_offs = 0;
            if (!tick_check_expire(ptm->tick_quick_slide, QUICK_SLIDE_EXPIRE) && abs_s(ptm->auto_move_offset) > QUICK_SLIDE_OFFS_MIN) {
                quick_jump_offs = ptm->info.page_size * ptm->info.quick_jump_perc / 100;
            }

            if (ptm->move_offset > 0) {
                ptm->auto_move_offset = -ptm->move_offset;
            } else if (ptm->move_offset < ptm->down_spring_offset) {
                ptm->auto_move_offset = ptm->down_spring_offset - ptm->move_offset;
            } else {
                s16 jump_limit = ptm->info.page_size * ptm->info.jump_perc / 100;
                s16 diff = ptm->auto_move_offset && jump_limit ? abs_s(ptm->move_offset % ptm->info.page_size) : 0;
                if (ptm->auto_move_offset > 0) {  //down
                    if (diff) {
                        diff = ptm->info.page_size - diff;
                        if (diff > jump_limit) {
                            diff = ptm->info.page_size - diff;
                        } else {
                            diff = -diff;
                        }
                    }
                    diff += quick_jump_offs;
                } else if (ptm->auto_move_offset < 0) {    //up
                    if (diff > jump_limit) {
                        diff -= ptm->info.page_size;
                    }
                    diff -= quick_jump_offs;
                }
                compo_page_move_to(ptm, ptm->move_offset + diff);
            }
            ptm->tick = tick_get();
        }
    } else if (ptm->auto_move_offset) {
        if (tick_check_expire(ptm->tick, MOVE_AUTO_TICK_EXPIRE)) {
            ptm->tick = tick_get();

            //二次函数型步进
            if (ptm->offs_sqrt_step == 0) {
                ptm->offs_sqrt = sqrt64(abs(ptm->auto_move_offset) * 100);    //平方根的十倍，提高精度，防止动画时间过短
                u16 frames = ptm->info.anim_time_ms / MOVE_AUTO_TICK_EXPIRE;
                ptm->offs_sqrt_step = (ptm->offs_sqrt + frames / 2) / frames;
                ptm->offs_sqrt_step += ptm->offs_sqrt_step ? 0 : 1; //动画时间过长，帧数过大，默认步进为1
                if (ptm->auto_move_offset < 0) {    //正负值标记方向
                    ptm->offs_sqrt = -ptm->offs_sqrt;
                } else {
                    ptm->offs_sqrt_step = -ptm->offs_sqrt_step;
                }
                TRACE(">>>auto:%d\n", ptm->auto_move_offset);
            }

            ptm->offs_sqrt += ptm->offs_sqrt_step;
            if (abs(ptm->offs_sqrt) < abs(ptm->offs_sqrt_step)) {
                ptm->offs_sqrt = 0;
            }
            s16 move_offset = ptm->auto_move_offset;
            ptm->auto_move_offset = ptm->offs_sqrt * abs(ptm->offs_sqrt) / 100;
            move_offset -= ptm->auto_move_offset;

            ptm->move_offset += move_offset;
            TRACE(">>>move:%d %d %d %d\n", ptm->move_offset, ptm->offs_sqrt, ptm->offs_sqrt_step, ptm->auto_move_offset);
            app_page_move_refresh(ptm);

            if (ptm->auto_move_offset == 0) { //超出边界，回弹
                if (ptm->move_offset > 0) {
                    ptm->auto_move_offset = -ptm->move_offset;
                } else if (ptm->move_offset < ptm->down_spring_offset) {
                    ptm->auto_move_offset = ptm->down_spring_offset - ptm->move_offset;
                }
                ptm->offs_sqrt_step = 0;
            }
        }
    }
}

/**
 * @brief 获取长页面当前offset
 * @param[in] ptm : 滚动控制块
 * @return 无
 **/
int16_t compo_page_move_get_offset(page_tp_move_t *ptm)
{
    if (ptm) {
        return ptm->move_offset;
    }
    return 0;
}

/**
 * @brief 长页面自动滚动偏移值计算
 * @param[in] ptm : 滚动控制块
 * @param[in] offset_dest : 目标位置
 * @return 无
 **/
void compo_page_move_to(page_tp_move_t *ptm, int16_t offset_dest)
{
    if (offset_dest > ptm->up_border_offset) {
        if (ptm->move_offset > 0) {
            return ;    //正在回弹
        }
        ptm->auto_move_offset = ptm->up_border_offset - ptm->move_offset;
    } else if (offset_dest < ptm->down_border_offset) {
        if (ptm->move_offset < ptm->down_spring_offset) {
            return ;
        }
        ptm->auto_move_offset = ptm->down_border_offset - ptm->move_offset;
    } else {
        ptm->auto_move_offset = offset_dest - ptm->move_offset;
    }

    ptm->offs_sqrt_step = 0;    //重新计算步进
    ptm->tick = tick_get();
}

/**
 * @brief 长页面按偏移值自动滚动
 * @param[in] ptm : 滚动控制块
 * @param[in] offset : 滚动偏移
 * @return 无
 **/
void compo_page_move_set(page_tp_move_t *ptm, int16_t offset)
{
    if (NULL == ptm) {
        return ;
    }

    compo_page_move_to(ptm, ptm->move_offset + ptm->auto_move_offset + offset);
}

/**
 * @brief 长页面按分页数自动滚动
 * @param[in] ptm : 滚动控制块
 * @param[in] pages : 滚动页数（负数为向下滚动）
 * @return 无
 **/
void compo_page_move_set_by_pages(page_tp_move_t *ptm, int8_t pages)
{
    if (NULL == ptm) {
        return ;
    }

    s16 offset_dest = ptm->move_offset + ptm->auto_move_offset - pages * ptm->info.page_size;
    s16 remain = abs_s(offset_dest % ptm->info.page_size);
    if (remain > ptm->info.page_size / 2) {
        offset_dest += remain - ptm->info.page_size;
    } else {
        offset_dest += remain;
    }
    compo_page_move_to(ptm, offset_dest);
}
