#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define DISKLIST_INFINITE_ROTATE            1                               //是否无限旋转
#if (GUI_GET_SCREEN_WIDTH > 2)
#define DISKLIST_CIRCLE_R                   (GUI_GET_SCREEN_WIDTH / 2)      //圆弧半径
#else
#define DISKLIST_CIRCLE_R                   1                               //圆弧半径（避免编译警告）
#endif

#define DISKLIST_ITEM_DEG_DIFF              360                             //两图标对于圆心相差度数*10，36度，半圆5等分
#define MAX_WORD_CNT                        32                              //每条列表项最多32个字符


/**
 * @brief 创建一个圆盘列表控件
 * @param[in] frm : 窗体指针
 * @return 返回圆盘列表指针
 **/
compo_disklist_t *compo_disklist_create(compo_form_t *frm)
{
    int i;
    compo_disklist_t *disklist = compo_create(frm, COMPO_TYPE_DISKLIST);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
#if BOX_GUI_ROTATE_DISP
    rotate_widget_page_set_client(page, 0, GUI_GET_SCREEN_CENTER_Y);  //圆心
#else
    widget_page_set_client(page, 0, GUI_SCREEN_CENTER_Y);  //圆心
#endif // BOX_GUI_ROTATE_DISP
    disklist->page = page;
    for (i=0; i<DISKLIST_ITEM_CNT; i++) {
        widget_icon_t *item_icon = widget_icon_create(page, 0);
        widget_page_t *txt_page = widget_page_create(page);
        widget_text_t *item_text = widget_text_create(txt_page, MAX_WORD_CNT);
        widget_set_align_center(txt_page, false);
        widget_set_align_center(item_text, false);
        disklist->item_icon[i] = item_icon;
        disklist->item_page_text[i] = txt_page;
        disklist->item_text[i] = item_text;
    }
    return disklist;
}

/**
 * @brief 设置圆盘列表控件内容
 * @param[in] disklist : 圆盘列表指针
 * @param[in] item : 存放列表信息，图片、文字等
 * @param[in] item_cnt : 存放列表信息的数量
 **/
void compo_disklist_set(compo_disklist_t *disklist, compo_disklist_item_t const *item, int item_cnt)
{
    if (disklist == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SET);
    }
    disklist->item = item;
    disklist->item_cnt = item_cnt;
    if (item_cnt <= 0 || item_cnt > DISKLIST_MAX_ITEM_CNT) {
        halt(HALT_GUI_COMPO_LISTBOX_CREATE);
    }
    disklist->cycle_deg = disklist->item_cnt * DISKLIST_ITEM_DEG_DIFF;
    disklist->max_deg = disklist->cycle_deg - DISKLIST_ITEM_DEG_DIFF;
    disklist->icon_org_size = gui_image_get_size(item[0].res_addr).wid;
    compo_disklist_set_iconsize(disklist, disklist->icon_org_size);
}

/**
 * @brief 设置圆盘列表控件图标大小
 * @param[in] disklist : 圆盘列表指针
 * @param[in] icon_size : 图标大小
 **/
void compo_disklist_set_iconsize(compo_disklist_t *disklist, u16 icon_size)
{
    if (disklist == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SET);
    }
    disklist->icon_size = icon_size;
}

/**
 * @brief 根据列表编号获取10倍角度
 * @param[in] disklist : 圆盘列表指针
 * @param[in] idx : 菜单编号
 * @return 所在角度*10
 **/
int compo_disklist_get_deg_byidx(compo_disklist_t *disklist, int idx)
{
//    TRACE("%s[%d]\n", __func__, idx);
    if (disklist == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_GET_Y);
    }
    return DISKLIST_ITEM_DEG_DIFF * idx;
}

/**
 * @brief 按旋转角度设置列表框焦点
 * @param[in] disklist : 圆盘列表指针
 * @param[in] deg : 角度*10
 **/
void compo_disklist_set_focus(compo_disklist_t *disklist, int deg)
{
    TRACE("%s[%d]-->", __func__, deg);
    if (disklist == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SET_FOCUS);
    }
#if DISKLIST_INFINITE_ROTATE
    disklist->ofs_deg = deg;
# else
    int deg_min = 0;
    int deg_max = disklist->max_deg;
    disklist->ofs_deg = deg < deg_min ? deg_min : (deg > deg_max ? deg_max : deg);
#endif // INFINITE_ROTATE
    if (disklist->focus_rem_en) {
        disklist->ofs_deg %= disklist->cycle_deg;
    }
    TRACE("[%d]\n", disklist->ofs_deg);
}


/**
 * @brief 将选择的图标设为焦点
 * @param[in] disklist : 圆盘列表指针
 * @param[in] idx : 图标编号
 **/
void compo_disklist_set_focus_byidx(compo_disklist_t *disklist, int idx)
{
    int y = compo_disklist_get_deg_byidx(disklist, idx);
    compo_disklist_set_focus(disklist, y);
}

/**
 * @brief 更新列表框Widget
 * @param[in] disklist : 圆盘列表指针
 **/
void compo_disklist_update(compo_disklist_t *disklist)
{
//TRACE("%s\n", __func__);
#define PRINT(...)  //do{if(cidx>=18||cidx<=3){TRACE(__VA_ARGS__);}} while(0)  //边界切换数值检测
    if (disklist == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_UPDATE);
    }
    int i;
#if BOX_GUI_ROTATE_DISP
    int font_height = GUI_GET_SYS_FONT_HEI();
#else
    int font_height = widget_text_get_max_height();
#endif // BOX_GUI_ROTATE_DISP
    int cidx, sidx;
    s32 ofs = disklist->ofs_deg;
    if (ofs >= 0) {
        cidx = (ofs + DISKLIST_ITEM_DEG_DIFF / 2) / DISKLIST_ITEM_DEG_DIFF;
    } else {
        cidx = (ofs - DISKLIST_ITEM_DEG_DIFF / 2) / DISKLIST_ITEM_DEG_DIFF;
    }
    disklist->focus_icon_idx = cidx;
    while (cidx < 0 || cidx >= disklist->item_cnt) {
        cidx = cidx + (cidx < 0 ? disklist->item_cnt : (cidx >= disklist->item_cnt ? -disklist->item_cnt : 0));
    }
    sidx = cidx - DISKLIST_ITEM_CNT / 2;
    PRINT("focus[%d]-->[%d] \n", disklist->focus_icon_idx, cidx);
    for (i=0; i<DISKLIST_ITEM_CNT; i++) {  // 图标处理
        int idx = sidx + i;
#if DISKLIST_INFINITE_ROTATE
        disklist->item_idx[i] = idx + (idx < 0 ? disklist->item_cnt : (idx >= disklist->item_cnt ? -disklist->item_cnt : 0));
#else
        if (idx < 0 || idx >= disklist->item_cnt) {
            widget_set_visible(disklist->item_icon[i], false);
            widget_set_visible(disklist->item_page_text[i], false);
            disklist->item_idx[i] = -1;
            continue;
        } else {
            disklist->item_idx[i] = idx;
            widget_set_visible(disklist->item_icon[i], true);
            widget_set_visible(disklist->item_page_text[i], true);
        }
#endif // DISKLIST_INFINITE_ROTATE
        PRINT("icon[%d]idx[%d]", disklist->item_idx[i], idx);
        compo_disklist_item_t const *item = &disklist->item[disklist->item_idx[i]];
        widget_icon_set(disklist->item_icon[i], item->res_addr);
        widget_text_set(disklist->item_text[i], i18n[item->str_idx]);

        int ix, iy, deg10;
        ofs %= disklist->cycle_deg;  //图标0所在角度
        if (cidx==0 && abs(ofs)>disklist->max_deg) {  //转了一圈后focus在0和-1间变化
            if (ofs > 0) {
                ofs = ofs % disklist->max_deg + 3600 - DISKLIST_ITEM_DEG_DIFF;
            } else {
                ofs = ofs % disklist->max_deg + DISKLIST_ITEM_DEG_DIFF;
            }
            PRINT("ofs[%d]", ofs);
        }
        if (idx != disklist->item_idx[i]) {  //补头尾
            if (ofs > -DISKLIST_ITEM_DEG_DIFF / 2) {
                deg10 = (cidx - idx) * DISKLIST_ITEM_DEG_DIFF + (ofs - cidx * DISKLIST_ITEM_DEG_DIFF);
            } else {
                deg10 = (cidx - idx) * DISKLIST_ITEM_DEG_DIFF + (ofs + (disklist->item_cnt - cidx) * DISKLIST_ITEM_DEG_DIFF);
            }
            PRINT("!deg[%d]", deg10);
        } else {  //正常画
            if (ofs > -DISKLIST_ITEM_DEG_DIFF / 2) {
                deg10 = ofs - idx * DISKLIST_ITEM_DEG_DIFF;
                PRINT("+deg[%d]", deg10);
            } else {
                deg10 = ofs + (disklist->item_cnt - idx) * DISKLIST_ITEM_DEG_DIFF;
                PRINT("-deg[%d]", deg10);
            }
        }
        ix = muls_shift16(DISKLIST_CIRCLE_R, COS(deg10));
        iy = -muls_shift16(DISKLIST_CIRCLE_R, SIN(deg10));
#if BOX_GUI_ROTATE_DISP
        rotate_widget_set_location(disklist->item_icon[i], ix, iy, disklist->icon_size, disklist->icon_size);
#else
        widget_set_location(disklist->item_icon[i], ix, iy, disklist->icon_size, disklist->icon_size);
#endif // BOX_GUI_ROTATE_DISP
        PRINT("\n");
//        TRACE("deg[%d] xy[%d %d]\n", deg10, ix, iy);

        if (disklist->page_time != NULL && disklist->idx_time == disklist->item_idx[i]) {
            disklist->time_bg = disklist->item_icon[i];
#if BOX_GUI_ROTATE_DISP
            rotate_widget_page_scale_to(disklist->page_time, disklist->icon_size, disklist->icon_size);
            rotate_widget_set_pos(disklist->page_time, ix, iy);
#else
            widget_page_scale_to(disklist->page_time, disklist->icon_size, disklist->icon_size);
            widget_set_pos(disklist->page_time, ix, iy);
#endif // BOX_GUI_ROTATE_DISP
        }

    }
    for (i=0; i<DISKLIST_ITEM_CNT; i++) {  //文本处理
        if (disklist->item_idx[i] >= 0) {
            int ptx, pty, pth, tx, ty, temp;
            rect_t rect_cur, rect_cmp;
#if BOX_GUI_ROTATE_DISP
            rect_cur = rotate_widget_get_location(disklist->item_icon[i]);
#else
            rect_cur = widget_get_location(disklist->item_icon[i]);
#endif // BOX_GUI_ROTATE_DISP
            rect_cur.wid = disklist->icon_org_size;
            rect_cur.hei = disklist->icon_org_size;
            ptx = rect_cur.x + rect_cur.wid / 2;
            pty = rect_cur.y - rect_cur.hei / 2;
            pth = rect_cur.hei;
            tx = rect_cur.wid / 8;
            ty = (pth - font_height) / 2;
            if (i < DISKLIST_ITEM_CNT / 2) {
#if BOX_GUI_ROTATE_DISP
                rect_cmp = rotate_widget_get_location(disklist->item_icon[i + 1]);
#else
                rect_cmp = widget_get_location(disklist->item_icon[i + 1]);
#endif // BOX_GUI_ROTATE_DISP
                temp = pty + ty;
                ty -= (temp * temp) * (rect_cur.wid / 5) / (DISKLIST_CIRCLE_R * DISKLIST_CIRCLE_R);  //离中心越远向外移动越多
                temp = rect_cmp.y - rect_cmp.hei / 2;
                pth = pty < temp ? ((pty + pth) < temp ? pth : (temp - pty)) : 0;
                ty = (pty + ty + font_height + 5) > temp ? (temp - font_height - 5 - pty) : ty;
                temp = pty + ty;
                tx += (temp * temp) * (rect_cur.wid / 4) / (DISKLIST_CIRCLE_R * DISKLIST_CIRCLE_R);
            } else if (i > DISKLIST_ITEM_CNT / 2) {
#if BOX_GUI_ROTATE_DISP
                rect_cmp = rotate_widget_get_location(disklist->item_icon[i - 1]);
#else
                rect_cmp = widget_get_location(disklist->item_icon[i - 1]);
#endif // BOX_GUI_ROTATE_DISP
                temp = pty + ty + font_height;
                ty += (temp * temp) * (rect_cur.wid / 5) / (DISKLIST_CIRCLE_R * DISKLIST_CIRCLE_R);
                temp = (rect_cmp.y + rect_cmp.hei / 2) - pty;
                pty += temp > 0 ? temp : 0;
                ty -= temp > 0 ? temp : 0;
                temp = rect_cmp.y + rect_cmp.hei / 2;
                ty = (pty + ty - 5) < temp ? (temp + 5 - pty) : ty;
                temp = (rect_cur.y + rect_cur.hei / 2) - pty;
                pth = temp > 0 ? temp : 0;
                temp = pty + ty + font_height;
                tx += (temp * temp) * (rect_cur.wid / 4) / (DISKLIST_CIRCLE_R * DISKLIST_CIRCLE_R);
            } else if (i == DISKLIST_ITEM_CNT / 2) {
                temp = pty + ty + (rect_cur.y > 0 ? font_height : 0);
                if (rect_cur.y > 0) {
                    ty += (temp * temp) * (rect_cur.wid / 5) / (DISKLIST_CIRCLE_R * DISKLIST_CIRCLE_R);
                } else {
                    ty -= (temp * temp) * (rect_cur.wid / 5) / (DISKLIST_CIRCLE_R * DISKLIST_CIRCLE_R);
                }
                tx += (temp * temp) * (rect_cur.wid / 4) / (DISKLIST_CIRCLE_R * DISKLIST_CIRCLE_R);
            }
//            TRACE("ptx[%d] pty[%d] pth[%d] tx[%d] ty[%d]\n", ptx, pty, pth, tx, ty);
#if BOX_GUI_ROTATE_DISP
            rotate_widget_set_location(disklist->item_page_text[i], ptx, pty + pth, GUI_GET_SCREEN_WIDTH - ptx, pth);
            ro_widget_set_location(disklist->item_text[i], tx, ty + font_height, GUI_GET_SCREEN_WIDTH - ptx - tx, font_height, GUI_GET_SCREEN_WIDTH - ptx, pth);
#else
            widget_set_location(disklist->item_page_text[i], ptx, pty, GUI_SCREEN_WIDTH, pth);
            widget_set_location(disklist->item_text[i], tx, ty, GUI_SCREEN_WIDTH, font_height);
#endif
        }
        }
#if BOX_GUI_ROTATE_DISP
        if (sidx != disklist->sidx || disklist->txt_roll_need_rst) {    //滚动控制器
        u8 i = 0;
        u8 i_end = 0;
        int sidx_pre = disklist->sidx;
        disklist->sidx = sidx;

        if (!disklist->txt_roll_need_rst) {
            if  (sidx * sidx_pre < 0) {  //边界处理
                if (sidx < 0) {
                    sidx += disklist->item_cnt;
                } else {
                    sidx_pre += disklist->item_cnt;
                }
            }
            if (sidx > sidx_pre) {                      //向上滑动
                int k = sidx - sidx_pre;                //向上滑动的列表行数
                for (i=0; i<DISKLIST_ITEM_CNT-k; i++) { //将下一行列表项对应的滚动控制器 赋值给 上一行列表对应的滚动控制器
                    disklist->roll_cb[i] = disklist->roll_cb[i + k];
                }
                i_end = DISKLIST_ITEM_CNT;
            } else if (sidx < sidx_pre) {                           //向下滑动
                int k = sidx_pre - sidx;                            //向下滑动的列表行数
                for (i_end=DISKLIST_ITEM_CNT; i_end>k; i_end--) {   //将上一行列表项对应的滚动控制器 赋值给 下一行列表对应的滚动控制器
                    disklist->roll_cb[i_end - 1] = disklist->roll_cb[i_end - 1 - k];
                }
            }
        } else {
            i_end = DISKLIST_ITEM_CNT;
            disklist->txt_roll_need_rst = false;
        }

        for (; i<i_end; i++) {  //对移动超出列表项个数一半的滚动控制器重新赋值
            memset(&disklist->roll_cb[i], 0, sizeof(compo_roll_cb_t));
            disklist->roll_cb[i].tick = tick_get();
            widget_text_set_autoroll_mode(disklist->item_text[i], TEXT_AUTOROLL_MODE_NULL);
        }

        for (i=0; i<DISKLIST_ITEM_CNT; i++) {   //判断列表中的项是否可以滚动
            widget_text_t *txt = disklist->item_text[i];
            if (widget_get_visble(txt)) {
                area_t text_area = widget_text_get_area(txt);
                rect_t textbox_rect = rotate_widget_get_location(txt);
                if (text_area.wid > textbox_rect.wid) {
                    disklist->roll_cb[i].mode = TEXT_AUTOROLL_MODE_SROLL_CIRC;
                    disklist->roll_cb[i].direction = -1;
//                    disklist->roll_cb[i].tick = tick_get();
                    if (widget_get_align_center(txt)) {
                        disklist->roll_cb[i].offset = (text_area.wid - textbox_rect.wid) / 2;
                    }
                    widget_text_set_autoroll_mode(disklist->item_text[i], TEXT_AUTOROLL_MODE_SROLL_CIRC);
                } else if (disklist->roll_cb[i].direction == 0) {
                    widget_text_set_autoroll_mode(disklist->item_text[i], TEXT_AUTOROLL_MODE_NULL);
                } else {
                    widget_text_set_autoroll_mode(disklist->item_text[i], TEXT_AUTOROLL_MODE_SROLL_CIRC);
                }
            }
            widget_text_set_client(disklist->item_text[i], disklist->roll_cb[i].offset, 0);
        }
    }
#endif // BOX_GUI_ROTATE_DISP


}

/**
 * @brief 更新列表框Widget，同时重置所有文本滚动（item中str突变时需要调用，防止C208蓝屏）
 * @param[in] disklist : 圆盘列表指针
 **/
void compo_disklist_update_with_text_scroll_rst(compo_disklist_t *disklist)
{
    disklist->txt_roll_need_rst = true;
    compo_disklist_update(disklist);
}
/**
 * @brief 按坐标选择菜单项
 * @param[in] disklist : 圆盘列表指针
 * @param[in] x : x坐标
 * @param[in] y : y坐标
 * @return 返回菜单项索引
 **/
int compo_disklist_select(compo_disklist_t *disklist, int x, int y)
{
    if (disklist == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SELECT);
    }
    int i;
    for (i=0; i<DISKLIST_ITEM_CNT; i++) {
        if (disklist->item_idx[i] < 0) {
            continue;
        }
//        widget_page_t *page = disklist->item_page_text[i];
#if BOX_GUI_ROTATE_DISP
        rect_t rect = rotate_widget_get_absolute(disklist->item_icon[i]);
        rect_t rect_cur = rotate_widget_get_absolute(disklist->item_page_text[i]);
#else
        rect_t rect = widget_get_absolute(disklist->item_icon[i]);
        rect_t rect_cur = widget_get_absolute(disklist->item_page_text[i]);
#endif // BOX_GUI_ROTATE_DISP
#if BOX_GUI_ROTATE_DISP
        if ((x > rect_cur.x && x < (rect_cur.x + rect_cur.wid) && y < rect_cur.y && y > (rect_cur.y - rect_cur.hei)) ||
#else
        if ((x > rect_cur.x && x < (rect_cur.x + rect_cur.wid) && y > rect_cur.y && y < (rect_cur.y + rect_cur.hei)) ||
#endif
            (abs_s(x - rect.x) * 2 <= rect.wid && abs_s(y - rect.y) * 2 <= rect.hei)) {  //点击区域：图标+文本page
            disklist->sel_icon = disklist->item_icon[i];
            disklist->sel_idx = disklist->item_idx[i];
            TRACE("%s-->xy[%d,%d] idx[%d]\n", __func__, x, y, disklist->sel_idx);
            return disklist->sel_idx;
        }
    }
    disklist->sel_icon = NULL;
    disklist->sel_idx = -1;
    return -1;
}

/**
 * @brief 按索引选择图标
 * @param[in] disklist : 圆盘列表指针
 * @param[in] idx : 编号
 * @return 返回图标指针
 **/
widget_icon_t *compo_disklist_select_byidx(compo_disklist_t *disklist, int idx)
{
//    TRACE("%s[%d]\n", __func__, idx);
    if (disklist == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SELECT);
    }
    int i;
    for (i=0; i<DISKLIST_ITEM_CNT; i++) {
        if (disklist->item_idx[i] == idx) {
            disklist->sel_icon = disklist->item_icon[i];
            disklist->sel_idx = idx;
            return disklist->sel_icon;
        }
    }
    disklist->sel_icon = NULL;
    disklist->sel_idx = -1;
    return NULL;
}

/**
 * @brief 获取选择图标的区域
 * @param[in] disklist : 圆盘列表指针
 * @return 返回图标坐标和大小
 **/
rect_t compo_disklist_get_sel_rect(compo_disklist_t *disklist)
{
    if (disklist == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_GET_SELECT);
    }
    rect_t rect = {0};
    if (disklist->sel_icon == NULL) {
        return rect;
    }
#if BOX_GUI_ROTATE_DISP
    rect = rotate_widget_get_location(disklist->sel_icon);
#else
    rect = widget_get_location(disklist->sel_icon);
#endif // BOX_GUI_ROTATE_DISP
    return rect;
}

/**
 * @brief 向集合中添加时分秒针
          cx cy为旋转中心点, x y为目标坐标
 * @param[in] disklist : 图标集指针
 * @param[in] type : COMPO_ICONLIST_TIME_TYPE_HOUR, 时
                     COMPO_ICONLIST_TIME_TYPE_MIN,  分
                     COMPO_ICONLIST_TIME_TYPE_SEC,  秒
 * @param[in] res_addr : 图像资源地址
 * @param[in] cx : x轴坐标
 * @param[in] cy : y轴坐标
 **/
void compo_disklist_add_time(compo_disklist_t *disklist, u8 type, u32 res_addr, s16 cx, s16 cy)
{
    if (disklist->page_time == NULL) {
        disklist->page_time = widget_page_create(disklist->page);
#if BOX_GUI_ROTATE_DISP
        rotate_widget_set_location(disklist->page_time, 0, 0, disklist->icon_size, disklist->icon_size);
        rotate_widget_page_set_client(disklist->page_time, disklist->icon_size / 2, disklist->icon_size / 2);
#else
        widget_set_location(disklist->page_time, 0, 0, disklist->icon_size, disklist->icon_size);
        widget_page_set_client(disklist->page_time, disklist->icon_size / 2, disklist->icon_size / 2);
#endif // BOX_GUI_ROTATE_DISP
    }
    widget_image_t *img = widget_image_create(disklist->page_time, res_addr);
#if BOX_GUI_ROTATE_DISP
    ro_widget_set_pos(img, 0, 0, disklist->icon_org_size, disklist->icon_org_size);
    rotate_widget_image_set_rotation_center(img, cx, cy);
#else
    widget_image_set_rotation_center(img, cx, cy);
#endif // BOX_GUI_ROTATE_DISP

    switch (type) {
    case COMPO_ICONLIST_TIME_TYPE_HOUR:
        disklist->hour = img;
        break;

    case COMPO_ICONLIST_TIME_TYPE_MIN:
        disklist->min = img;
        break;

    case COMPO_ICONLIST_TIME_TYPE_SEC:
        disklist->sec = img;
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_TIME_TYPE);
        return;
    }
    compo_cb.rtc_update = true;                 //创建时钟组件，需要同步更新时间
}

/**
 * @brief 设置图标集指针的起始角度
 * @param[in] disklist : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_disklist_set_start_angle(compo_disklist_t *disklist, s16 angle)
{
    disklist->start_angle = angle;
}

/**
 * @brief 设置图标集合时钟序号
 * @param[in] disklist : 图标集指针
 * @param[in] idx : 时钟序号
 **/
void compo_disklist_set_time_idx(compo_disklist_t *disklist, u8 idx)
{

    disklist->idx_time = idx;
}


