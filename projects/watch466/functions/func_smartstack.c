#if 1
#include "include.h"
#include "func.h"

#define TRACE_EN    0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

enum{   //组件ID
    COMPO_ID_BTN_DEL = 1,
    COMPO_ID_PIC_NONE_BG,
    COMPO_ID_CARD_UP_BK,
    COMPO_ID_CARD_UP_DISP,
    COMPO_ID_CARD_MID,
    COMPO_ID_CARD_DOWN_BK,
    COMPO_ID_CARD_DOWN_DISP,
};

enum {  //卡片位置
    CARD_POS_UP,
    CARD_POS_MID,
    CARD_POS_DOWN,
    CARD_POS_OVER,
};

enum {  //操作卡片
    CARD_DRAW_REFRESH,
    CARD_DRAW_MOVE_X,
    CARD_DRAW_MOVE_Y,
};

#define CARD_RECT_COUNT_MAX         1       //卡片内支持的rect数量
#define CARD_ICON_COUNT_MAX         4       //卡片内支持的icon数量
#define CARD_TEXT_COUNT_MAX         4       //卡片内支持的text数量
#define AUTO_TICK_EXPIRE            10      //自动滚动单位时间
#define AUTO_MOVE_Y_EXPIRE          10      //自动滚动单位距离
#define MID_CARD_FUNC_INIT_INDEX    2       //中间卡片加载的堆栈记录索引(从1开始)
#define AUTO_RET_ONESELF_FLAG       1       //滚动结束后是否回正
#define AUTO_ROLL_MOMENT            5       //滚动惯性动量
#define GRAY_LV(x)                  make_color(x, x, x) //生成灰度颜色值（0:黑 255:白）
#define UP_GRAY_LV                  0x24
#define MID_GRAY_LV                 0x36
#define DOWN_GRAY_LV                0x48


//功能结构体
typedef struct f_smartstack_t_ {
    s32 last_move_y;
    s32 last_move_x;
    s32 auto_move_y;
    u32 last_tick;
    bool flag_drag;
    int8_t mid_index;
    uint8_t last_move_x_card_id;
    int8_t del_index;
} f_smartstack_t;

//卡片初始位置
const rect_t card_bg[4] =
{
    {.x = GUI_SCREEN_CENTER_X, .y = 124,       .wid = 176, .hei = 212},
    {.x = GUI_SCREEN_CENTER_X, .y = 217,       .wid = 218, .hei = 262},
    {.x = GUI_SCREEN_CENTER_X, .y = 396,       .wid = 256, .hei = 308},
    {.x = GUI_SCREEN_CENTER_X, .y = 396 + 179, .wid = 256, .hei = 308},
};

//标题位置(icon+文字标题,以中间卡片初始位置为准)
const rect_t card_icon =
{
    .x = 90 - GUI_SCREEN_CENTER_X, .y = 127 - 217, .wid = 50, .hei = 50,
};

//五张卡片的初始位置
const u8 card_item[] = {CARD_POS_UP, CARD_POS_UP, CARD_POS_MID, CARD_POS_DOWN, CARD_POS_OVER};



//活动数据详情页
static void func_smartstack_sub_activity(compo_cardbox_t *cardbox, u8 icon_offset, u8 text_offset)
{
    u32 str_id[] = {STR_STEPS, STR_CALORIE, STR_DISTANCE};

    for (u8 i = text_offset; i < sizeof(cardbox->text) / sizeof(cardbox->text[0]); i++) {
        if ((i - text_offset) < (sizeof(str_id) / sizeof(str_id[0]))) {
            char buf[16] = {0};
            snprintf(buf, sizeof(buf), "%s:99999", i18n[str_id[i - text_offset]]);
            compo_cardbox_text_set(cardbox, i, buf);
            compo_cardbox_text_set_autosize(cardbox, i, true);
            compo_cardbox_text_set_location(cardbox, i, 0, card_icon.y + card_icon.hei * i, 0, 0);
            compo_cardbox_text_set_visible(cardbox, i, true);
        }
    }
}

//心率数据详情页
static void func_smartstack_sub_heart(compo_cardbox_t *cardbox, u8 icon_offset, u8 text_offset)
{
    //hr text
    char buf[16] = {0};
    snprintf(buf, sizeof(buf), "-- 次/分");
    compo_cardbox_text_set(cardbox, text_offset, buf);
    compo_cardbox_text_set_autosize(cardbox, text_offset, true);
    compo_cardbox_text_set_location(cardbox, text_offset, card_icon.x + card_icon.wid / 2, card_icon.y + card_icon.hei * text_offset, 0, 0);
    compo_cardbox_text_set_visible(cardbox, text_offset, true);
}

//睡眠数据详情页
static void func_smartstack_sub_sleep(compo_cardbox_t *cardbox, u8 icon_offset, u8 text_offset)
{
    u8 i;
//    u32 pic[] = {UI_BUF_SLEEP_LIGHT_SLEEP_BIN, UI_BUF_SLEEP_LIGHT_SLEEP_BIN};

    //icon
    for (i = 0; i < 2; i++) {
//        compo_cardbox_icon_set(cardbox, i + icon_offset, pic[i]);
        compo_cardbox_icon_set_location(cardbox, i + icon_offset, card_icon.x, card_icon.y + card_icon.hei * (i + icon_offset - 1), card_icon.wid, card_icon.hei);
        compo_cardbox_icon_set_visible(cardbox, i + icon_offset, true);

        char buf[16] = {0};
        snprintf(buf, sizeof(buf), "7%s30%s", i18n[STR_HOUR], i18n[STR_MIN]);
        compo_cardbox_text_set(cardbox, i + text_offset, buf);
        compo_cardbox_text_set_autosize(cardbox, i + text_offset, true);
        area_t area = compo_cardbox_text_get_area(cardbox, 0);
        compo_cardbox_text_set_location(cardbox, i + text_offset, card_icon.x + card_icon.wid + area.wid / 2, card_icon.y + card_icon.hei * (i + text_offset), 0, 0);
        compo_cardbox_text_set_visible(cardbox, i + text_offset, true);
    }
}

//血氧数据详情页
static void func_smartstack_sub_spo2(compo_cardbox_t *cardbox, u8 icon_offset, u8 text_offset)
{
    char buf[16] = {0};

    snprintf(buf, sizeof(buf), "%s", i18n[STR_MEASURING]);
    compo_cardbox_text_set(cardbox, text_offset, buf);
    compo_cardbox_text_set_autosize(cardbox, text_offset, true);
    compo_cardbox_text_set_location(cardbox, text_offset, card_icon.x + card_icon.wid, card_icon.y + card_icon.hei * text_offset, 0, 0);
    compo_cardbox_text_set_visible(cardbox, text_offset, true);
}

//音乐详情页
static void func_smartstack_sub_music(compo_cardbox_t *cardbox, u8 icon_offset, u8 text_offset)
{
    if (icon_offset < CARD_ICON_COUNT_MAX) {
  //      compo_cardbox_icon_set(cardbox, icon_offset, UI_BUF_MUSIC_PREV_BIN);
        widget_set_alpha(cardbox->icon[icon_offset], UI_BTN_CLICK_EFFECT_ALPHA1);
        compo_cardbox_icon_set_location(cardbox, icon_offset, card_icon.x, card_icon.y + card_icon.hei, card_icon.wid, card_icon.hei);
        compo_cardbox_icon_set_visible(cardbox, icon_offset, true);
    }
    icon_offset ++;
    if (icon_offset < CARD_ICON_COUNT_MAX) {
  //      compo_cardbox_icon_set(cardbox, icon_offset, UI_BUF_MUSIC_PAUSE_BIN);
        compo_cardbox_icon_set_location(cardbox, icon_offset, 0, card_icon.y + card_icon.hei, card_icon.wid + 10, card_icon.hei + 10);
        compo_cardbox_icon_set_visible(cardbox, icon_offset, true);
    }
    icon_offset ++;
    if (icon_offset < CARD_ICON_COUNT_MAX) {
    //    compo_cardbox_icon_set(cardbox, icon_offset, UI_BUF_MUSIC_NEXT_BIN);
        widget_set_alpha(cardbox->icon[icon_offset], UI_BTN_CLICK_EFFECT_ALPHA1);
        compo_cardbox_icon_set_location(cardbox, icon_offset, abs_s(card_icon.x), card_icon.y + card_icon.hei, card_icon.wid, card_icon.hei);
        compo_cardbox_icon_set_visible(cardbox, icon_offset, true);
    }
    icon_offset ++;
}




//刷新卡片内容(以中间卡片为初始坐标和大小，移动过程中page做整体缩放)
static void func_smartstack_card_content_refresh(compo_cardbox_t *cardbox, u8 pos, u8 func_sta)
{
    u8 i;
    compo_cardbox_set_visible(cardbox, true);

    //矩形框颜色
    if (CARD_POS_UP == pos)  {
        compo_cardbox_rect_set_color(cardbox, 0, GRAY_LV(UP_GRAY_LV));
    } else if (CARD_POS_MID == pos) {
        compo_cardbox_rect_set_color(cardbox, 0, GRAY_LV(MID_GRAY_LV));
    } else {
        compo_cardbox_rect_set_color(cardbox, 0, GRAY_LV(DOWN_GRAY_LV));
    }

    //设置标题icon
    compo_cardbox_icon_set(cardbox, 1, func_get_icon_addr(func_sta));
    compo_cardbox_icon_set_location(cardbox, 1, card_icon.x, card_icon.y, card_icon.wid, card_icon.hei);
    //设置标题
    compo_cardbox_text_set(cardbox, 0, i18n[func_get_str_idx(func_sta)]);
    compo_cardbox_text_set_autosize(cardbox, 0, true);
    area_t area = compo_cardbox_text_get_area(cardbox, 0);
    compo_cardbox_text_set_location(cardbox, 0, card_icon.x + card_icon.wid / 2 + area.wid / 2 + 10, card_icon.y + 5, 0, 0);
    //设置card
    compo_cardbox_set_location(cardbox, card_bg[pos].x, card_bg[pos].y, card_bg[pos].wid, card_bg[pos].hei);

    //刷新内容
    u8 icon_offset = 2;
    u8 text_offset = 1;
    for (i = icon_offset; i < CARD_ICON_COUNT_MAX; i++) {
        compo_cardbox_icon_set_visible(cardbox, i, false);
    }
    for (i = text_offset; i < CARD_TEXT_COUNT_MAX; i++) {
        compo_cardbox_text_set_visible(cardbox, i, false);
    }
    switch (func_sta) {
        case FUNC_CLOCK:
            break;
        case FUNC_ACTIVITY:
            func_smartstack_sub_activity(cardbox, icon_offset, text_offset);
            break;

        case FUNC_HEARTRATE:
            func_smartstack_sub_heart(cardbox, icon_offset, text_offset);
            break;

        case FUNC_SLEEP:
            func_smartstack_sub_sleep(cardbox, icon_offset, text_offset);
            break;

        case FUNC_BLOOD_OXYGEN:
            func_smartstack_sub_spo2(cardbox, icon_offset, text_offset);
            break;

        case FUNC_BT:
            func_smartstack_sub_music(cardbox, icon_offset, text_offset);
            break;

        case FUNC_CALCULATOR:
            break;

        case FUNC_GAME:
        break;
    }
}

//创建堆栈界面
compo_form_t * func_smartstack_form_create(void)
{
    compo_cardbox_t *cardbox;
    u8 total_cnt = latest_task_count();
    int8_t func_start = total_cnt - MID_CARD_FUNC_INIT_INDEX - 2;

    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //无堆栈记录
    // compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_SMARTSTACK_NULL_BIN);
    // compo_setid(pic, COMPO_ID_PIC_NONE_BG);
    // compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    // if (total_cnt) {
    //     compo_picturebox_set_visible(pic, false);
    // }

    // //创建卡片和设置背景框
    // for (u8 i = 0; i < sizeof(card_item) / sizeof(card_item[0]); i++) {
    //     cardbox = compo_cardbox_create(frm, CARD_RECT_COUNT_MAX, CARD_ICON_COUNT_MAX, CARD_TEXT_COUNT_MAX, card_bg[CARD_POS_MID].wid, card_bg[CARD_POS_MID].hei);
    //     compo_setid(cardbox, COMPO_ID_CARD_UP_BK + i);
    //     compo_cardbox_rect_set_location(cardbox, 0, 0, 0, card_bg[CARD_POS_MID].wid, card_bg[CARD_POS_MID].hei, 30);
    //     compo_cardbox_set_location(cardbox, card_bg[card_item[i]].x, card_bg[card_item[i]].y, card_bg[card_item[i]].wid, card_bg[card_item[i]].hei);
    //     if ((func_start + i) > 0 && (func_start + i) < total_cnt) {
    //         func_smartstack_card_content_refresh(cardbox, card_item[i], latest_task_get(func_start + i));
    //     } else {
    //         compo_cardbox_set_visible(cardbox, false);
    //     }
    //     if (!total_cnt) {
    //         compo_cardbox_set_visible(cardbox, false);
    //     }
    // }

    // //删除按钮
    // compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_SMARTSTACK_DELET_BIN);
    // compo_setid(btn, COMPO_ID_BTN_DEL);
    // compo_button_set_visible(btn, false);

    return frm;
}

//卡片大小和位置为线性关系
static rect_t func_smartstack_get_size_by_y(s16 y, u8 *color_level)
{
    rect_t rect = {0};
    *color_level = UP_GRAY_LV;

    rect.y = y;
    rect.x = GUI_SCREEN_CENTER_X;
    if (y <= card_bg[CARD_POS_UP].y) {
        rect = card_bg[CARD_POS_UP];
    } else if (y <= card_bg[CARD_POS_MID].y) {
        rect.wid = card_bg[CARD_POS_UP].wid + ((y - card_bg[CARD_POS_UP].y) / 2);       //此处2为增量差值倍数，下同
        if (rect.wid > card_bg[CARD_POS_MID].wid) {
            rect.wid = card_bg[CARD_POS_MID].wid;
        }
        rect.hei = card_bg[CARD_POS_UP].hei + ((y - card_bg[CARD_POS_UP].y) / 2) + 4;   //此处4为增量差值倍数无法整除情况下的增益，下同
        if (rect.hei > card_bg[CARD_POS_MID].hei) {
            rect.hei = card_bg[CARD_POS_MID].hei;
        }
        *color_level = UP_GRAY_LV + (y - card_bg[CARD_POS_UP].y) / 5;
        if (*color_level > MID_GRAY_LV) {
            *color_level = MID_GRAY_LV;
        }
    } else if (y <= card_bg[CARD_POS_DOWN].y) {
        rect.wid = card_bg[CARD_POS_MID].wid + ((y - card_bg[CARD_POS_MID].y) / 4);
        if (rect.wid > card_bg[CARD_POS_DOWN].wid) {
            rect.wid = card_bg[CARD_POS_DOWN].wid;
        }
        rect.hei = card_bg[CARD_POS_MID].hei + ((y - card_bg[CARD_POS_MID].y) / 2) + 2;
        if (rect.hei > card_bg[CARD_POS_DOWN].hei) {
            rect.hei = card_bg[CARD_POS_DOWN].hei;
        }
        *color_level = MID_GRAY_LV + (y - card_bg[CARD_POS_MID].y) / 9;
        if (*color_level > DOWN_GRAY_LV) {
            *color_level = DOWN_GRAY_LV;
        }
    } else {
        rect.wid = card_bg[CARD_POS_DOWN].wid + ((y - card_bg[CARD_POS_DOWN].y) / 4);
        if (rect.wid > GUI_SCREEN_WIDTH) {
            rect.wid = GUI_SCREEN_WIDTH;
        }
        rect.hei = card_bg[CARD_POS_DOWN].hei + ((y - card_bg[CARD_POS_DOWN].y) / 2);
        if (rect.hei > GUI_SCREEN_HEIGHT) {
            rect.hei = GUI_SCREEN_HEIGHT;
        }
        *color_level = DOWN_GRAY_LV + (y - card_bg[CARD_POS_DOWN].y) / 9;
    }

    return rect;
}

//上下移动处理
static void func_smartstack_move_y_handle(compo_cardbox_t *cardbox, s32 dy)
{
    rect_t rect = compo_cardbox_get_location(cardbox);
    u8 color_level;

    rect.y += dy;
    if (rect.y < card_bg[CARD_POS_UP].y) {
        rect.y = card_bg[CARD_POS_UP].y;
    }
    rect = func_smartstack_get_size_by_y(rect.y, &color_level);
    compo_cardbox_rect_set_color(cardbox, 0, GRAY_LV(color_level));
    compo_cardbox_set_location(cardbox, rect.x, rect.y, rect.wid, rect.hei);
}

//左右移动处理
static void func_smartstack_move_x_handle(s32 move)
{
    u8 i;
    compo_cardbox_t *cardbox = NULL;
    f_smartstack_t *f_smartstack = (f_smartstack_t *)func_cb.f_cb;

    if (f_smartstack->last_move_x_card_id) {
        cardbox = compo_getobj_byid(f_smartstack->last_move_x_card_id);
    } else {
        u8 max_cnt = sizeof(card_item) / sizeof(card_item[0]);
        for (i = 0; i < max_cnt; i++) {
            cardbox = compo_getobj_byid(COMPO_ID_CARD_UP_BK + max_cnt - i - 1);  //由于存在重叠，优先判断上面
            if (!compo_cardbox_get_visible(cardbox)) {
                continue ;
            }
            rect_t rect = compo_cardbox_get_absolute(cardbox);
            point_t point = ctp_get_sxy();
            if (abs_s(point.y - rect.y) * 2 <= rect.hei) {
                f_smartstack->last_move_x_card_id = COMPO_ID_CARD_UP_BK + max_cnt - i - 1;
                f_smartstack->del_index = f_smartstack->mid_index + (2 - i);
                break;
            } else {
                cardbox = NULL;
            }
        }
    }

    if (cardbox && compo_cardbox_get_visible(cardbox)) {
        rect_t rect = compo_cardbox_get_location(cardbox);
        s16 last_x = rect.x;
        u8 color_level;
        rect = func_smartstack_get_size_by_y(rect.y, &color_level);
        last_x += move;
        rect.x = last_x;
        if (rect.x > GUI_SCREEN_CENTER_X) {
            rect.x = GUI_SCREEN_CENTER_X;
        }
        if (rect.x < (card_bg[CARD_POS_UP].x - card_bg[CARD_POS_UP].wid / 2)) {
            rect.x = card_bg[CARD_POS_UP].x - card_bg[CARD_POS_UP].wid / 2;
        }
        compo_cardbox_set_location(cardbox, rect.x, rect.y, rect.wid, rect.hei);

        //del btn
        compo_button_t *btn = compo_getobj_byid(COMPO_ID_BTN_DEL);
        if (rect.x >= GUI_SCREEN_CENTER_X) {
            compo_button_set_visible(btn, false);
        } else {
            compo_button_set_visible(btn, true);
            compo_button_set_pos(btn, rect.x + rect.wid / 2, rect.y - rect.hei / 2);
        }
    }
}

//刷新所有卡片
static void func_smartstack_card_refresh(u8 draw, s32 move)
{
    f_smartstack_t *f_smartstack = (f_smartstack_t *)func_cb.f_cb;
    int8_t start_index = f_smartstack->mid_index - 2;
    int8_t end_index = f_smartstack->mid_index + 2;
    compo_cardbox_t *cardbox_mid = compo_getobj_byid(COMPO_ID_CARD_MID);
    rect_t mid_rect = compo_cardbox_get_location(cardbox_mid);
    bool refresh_flag = false;
    u8 total_cnt = latest_task_count();

    if (CARD_DRAW_MOVE_X == draw) {
        func_smartstack_move_x_handle(move);
        return ;
    }

    if (CARD_DRAW_MOVE_Y == draw) {
        if (f_smartstack->mid_index >= (total_cnt - 1) && move < 0) {
            return ;
        }
        if (f_smartstack->mid_index == 0 && move > 0) {
            return ;
        }
    }

    for (int8_t i = start_index; i <= end_index; i++) {
        compo_cardbox_t *cardbox = compo_getobj_byid(COMPO_ID_CARD_UP_BK + i - start_index);
        if (NULL == cardbox) {
            continue ;
        }
        if (i < 0) {
            compo_cardbox_set_visible(cardbox, false);
            continue ;
        } else if (i >= total_cnt) {
            compo_cardbox_set_visible(cardbox, false);
            break;
        } else {
            switch (draw) {
                case CARD_DRAW_REFRESH: {
                    func_smartstack_card_content_refresh(cardbox, card_item[i - start_index], latest_task_get(i));
                }break;

                case CARD_DRAW_MOVE_Y: {
                    if (i == start_index) {
                        continue ;
                    } else if (i < f_smartstack->mid_index) {
                        if (mid_rect.y < card_bg[CARD_POS_MID].y) {
                            continue ;
                        }
                    } else {
                        rect_t cur_rect = compo_cardbox_get_location(cardbox);
                        if (cur_rect.y < card_bg[CARD_POS_DOWN].y) {
                            move *= 2;
                        }
                        if (!refresh_flag && i == f_smartstack->mid_index) {
                            if (mid_rect.y > card_bg[CARD_POS_UP].y && (mid_rect.y + move) <= card_bg[CARD_POS_UP].y) {
                                f_smartstack->mid_index ++;
                                refresh_flag = true;
                            } else if (mid_rect.y < card_bg[CARD_POS_DOWN].y && (mid_rect.y + move) >= card_bg[CARD_POS_DOWN].y) {
                                f_smartstack->mid_index --;
                                refresh_flag = true;
                            }
                        }
                    }
                    func_smartstack_move_y_handle(cardbox, move);
                }break;

                default:
                    break;
            }
        }
    }

    if (refresh_flag) {
        func_smartstack_card_refresh(CARD_DRAW_REFRESH, 0);
        return ;
    }
}

//移动
static void func_smartstack_move_process(void)
{
    f_smartstack_t *f_smartstack = (f_smartstack_t *)func_cb.f_cb;

    if (f_smartstack->flag_drag) {
        s32 dx, dy;
        bool move_y = false;
        f_smartstack->flag_drag = ctp_get_dxy(&dx, &dy);
        //拖拽判断
        if (f_smartstack->flag_drag) {
            //左右上下滑动的判断
            if (f_smartstack->last_move_y) {
                move_y = true;
            } else if (f_smartstack->last_move_x) {
                move_y = false;
            } else {
                if (abs_s(dx) < abs_s(dy)) {
                    move_y = true;
                }
                if (dx > 0) {
                    move_y = true;
                }
            }
            if (f_smartstack->last_move_x_card_id) {
                move_y = false;
            }
            //上下滑动处理
            if (move_y && dy != f_smartstack->last_move_y) {
                func_smartstack_card_refresh(CARD_DRAW_MOVE_Y, dy - f_smartstack->last_move_y);
                f_smartstack->auto_move_y = dy - f_smartstack->last_move_y;
                f_smartstack->last_move_y = dy;
            }
            //左右滑动处理
            if (!move_y && dx != f_smartstack->last_move_x) {
                func_smartstack_card_refresh(CARD_DRAW_MOVE_X, dx - f_smartstack->last_move_x);
                f_smartstack->last_move_x = dx;
            }
        } else { //release
            f_smartstack->flag_drag = false;
            f_smartstack->last_move_y = 0;
            f_smartstack->last_move_x = 0;
            if (f_smartstack->last_move_x_card_id) {
                compo_cardbox_t *cardbox = compo_getobj_byid(f_smartstack->last_move_x_card_id);
                rect_t rect = compo_cardbox_get_location(cardbox);
                if (rect.x == GUI_SCREEN_CENTER_X) {
                    f_smartstack->last_move_x_card_id = 0;
                }
            }
            f_smartstack->auto_move_y *= AUTO_ROLL_MOMENT;
        }
    } else if (f_smartstack->auto_move_y) {
        if (tick_check_expire(f_smartstack->last_tick, AUTO_TICK_EXPIRE)) {
            s32 dy = 0;
            f_smartstack->last_tick = tick_get();
            if (f_smartstack->auto_move_y > 0) {
                dy = f_smartstack->auto_move_y > AUTO_MOVE_Y_EXPIRE ? AUTO_MOVE_Y_EXPIRE : f_smartstack->auto_move_y;
            } else {
                dy = f_smartstack->auto_move_y < -AUTO_MOVE_Y_EXPIRE / 2 ? -AUTO_MOVE_Y_EXPIRE / 2 : f_smartstack->auto_move_y;
            }
            f_smartstack->auto_move_y -= dy;
            func_smartstack_card_refresh(CARD_DRAW_MOVE_Y, dy);
            //滚动结束进行回正
#if AUTO_RET_ONESELF_FLAG
            if (!f_smartstack->auto_move_y) {
                func_smartstack_card_refresh(CARD_DRAW_REFRESH, 0);
            }
#endif
        }
    }
}

//智能堆栈主要事件流程处理
static void func_smartstack_process(void)
{
    func_smartstack_move_process();
    func_process();
}

//单击
static void func_smartstack_click_handler(void)
{
    u8 i;
    compo_cardbox_t *cardbox;
    f_smartstack_t *f_smartstack = (f_smartstack_t *)func_cb.f_cb;
    u8 max_cnt = sizeof(card_item) / sizeof(card_item[0]);
    u8 total_cnt = latest_task_count();

    //del btn
    if (f_smartstack->last_move_x_card_id) {
        if(COMPO_ID_BTN_DEL == compo_get_button_id()) {
            if (f_smartstack->del_index < total_cnt) {
                compo_button_t *btn = compo_getobj_byid(COMPO_ID_BTN_DEL);
                compo_button_set_visible(btn, false);
                latest_task_del(f_smartstack->del_index);
                if (latest_task_count()) {
                    f_smartstack->mid_index --;
                    if (f_smartstack->mid_index < 0) {
                        f_smartstack->mid_index = 0;
                    }
                    func_smartstack_card_refresh(CARD_DRAW_REFRESH, 0);
                } else {
                    for (i = 0; i < sizeof(card_item) / sizeof(card_item[0]); i++) {
                        cardbox = compo_getobj_byid(COMPO_ID_CARD_UP_BK + i);
                        compo_cardbox_set_visible(cardbox, false);
                    }
                    compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_PIC_NONE_BG);
                    compo_picturebox_set_visible(pic, true);
                }
                f_smartstack->del_index = 0xff;
                f_smartstack->last_move_x_card_id = 0;
            }
        }
        return ;
    }

    //card btn
    for (i = 0; i < max_cnt; i++) {
        cardbox = compo_getobj_byid(COMPO_ID_CARD_UP_BK + max_cnt - i - 1);
        if (!compo_cardbox_get_visible(cardbox)) {
            continue ;
        }
        if (compo_cardbox_btn_is(cardbox, ctp_get_sxy())) {
            int8_t index = f_smartstack->mid_index + (2 - i);
            if (index < total_cnt) {
                func_switch_to(latest_task_get(index), FUNC_SWITCH_DIRECT | FUNC_SWITCH_AUTO);
            }
            break;
        }
    }
}

//功能消息处理
static void func_smartstack_message(size_msg_t msg)
{
    f_smartstack_t *f_smartstack = (f_smartstack_t *)func_cb.f_cb;

    switch (msg) {
        case MSG_CTP_TOUCH:
            f_smartstack->flag_drag = true;
            break;

        case MSG_CTP_CLICK:
            func_smartstack_click_handler();
            break;

        case MSG_CTP_SHORT_UP:
        case MSG_CTP_SHORT_DOWN:
            break;

        case MSG_CTP_SHORT_LEFT:
        case MSG_CTP_LONG:
            break;

        case MSG_CTP_SHORT_RIGHT:
            if (f_smartstack->last_move_x_card_id) {
                break;
            }
            func_backing_to();	//右滑缓慢退出任务
            break;

        case MSG_QDEC_FORWARD:
            f_smartstack->auto_move_y += (card_bg[CARD_POS_UP].y - card_bg[CARD_POS_MID].y);
            f_smartstack->last_tick = tick_get();
            break;

        case MSG_QDEC_BACKWARD:
            f_smartstack->auto_move_y += (card_bg[CARD_POS_MID].y - card_bg[CARD_POS_UP].y);
            f_smartstack->last_tick = tick_get();
            break;

        case KL_BACK:
            break;

        default:
            func_message(msg);
            break;
    }
}

//智能堆栈进入处理
void func_smartstack_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_smartstack_t));
    func_cb.frm_main = func_smartstack_form_create();

    f_smartstack_t *f_smartstack = (f_smartstack_t *)func_cb.f_cb;
    u8 total_cnt = latest_task_count();
    if (total_cnt == 1) {
        f_smartstack->mid_index = 0;
    } else {
        f_smartstack->mid_index = total_cnt - MID_CARD_FUNC_INIT_INDEX;
    }
    func_smartstack_card_refresh(CARD_DRAW_REFRESH, 0);
}

//智能堆栈退出处理
void func_smartstack_exit(void)
{
    func_cb.last = FUNC_SMARTSTACK;
}

//智能堆栈
void func_smartstack(void)
{
    printf("%s\n", __func__);
    func_smartstack_enter();
    while (func_cb.sta == FUNC_SMARTSTACK) {
        func_smartstack_process();
        func_smartstack_message(msg_dequeue());
    }
    func_smartstack_exit();
}

#endif
