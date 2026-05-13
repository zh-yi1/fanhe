#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif
#if BOX_GUI_ROTATE_DISP
#define ROWBOX_ICON_GAP                     10                      //调整图标间距
#define ROWBOX_ARC_R                        GUI_GET_SCREEN_WIDTH    //圆弧样式半径
#endif // BOX_GUI_ROTATE_DISP
//移动相关控制
#define FOCUS_AUTO_TICK_EXPIRE              10                  //松手后自动对齐焦点单位时间(ms)
#define FOCUS_AUTO_STEP                     5                   //松手后自动对齐焦点单位时间步进
#define FOCUS_AUTO_STEP_DIV                 16

#define DRAG_AUTO_SPEED                     80                  //拖动松手后的速度

/**
 * @brief 创建一个列表（横向）控件
 * @param[in] frm : 窗体指针
 * @return 返回列表（横向）指针
 **/
compo_rowbox_t *compo_rowbox_create(compo_form_t *frm)
{
    compo_rowbox_t *rowbox = compo_create(frm, COMPO_TYPE_ROWBOX);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    rowbox->page = page;

    //先创建背景框，确保在所有图片的底部
    for (int i=0; i<IMG_MAX; i++) {
        rowbox->rect[i] = widget_rect_create(page);
        //widget_rect_set_color(rowbox->rect[i], make_color(0xff, 0x00, 0x00));
        widget_rect_set_color(rowbox->rect[i], make_color(0x48, 0x48, 0x48));//default color
    }

    for (int i=0; i<IMG_MAX; i++) {
        widget_image_t *img = widget_image_create(page, 0);
        rowbox->img[i] = img;
    }

    return rowbox;
}

//根据表盘的基地址获取对应的预览图
static u32 compo_rowbox_preview_style(u32 base_addr)
{
    uitool_header_t uitool_header;
    os_spiflash_read(&uitool_header, base_addr, UITOOL_HEADER);
	for(u16 i=0; i<uitool_header.num; i++) {
        uitool_res_t uitool_res = {0};
        os_spiflash_read(&uitool_res, base_addr + UITOOL_HEADER + i * UITOOL_RES_HEADER, UITOOL_RES_HEADER);
        u32 res_addr = base_addr + uitool_res.res_addr;
        //预览图
        if (uitool_res.res_type == UITOOL_TYPE_IMAGE && uitool_res.bond_type == COMPO_BOND_IMAGE_CLOCK_PREVIEW) {
            return res_addr;
        }
    }
    return 0;
}

/**
 * @brief 设置列表（横向）控件内容
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] margin_en : 是否使能边框
 * @param[in] margin_width : 边框的宽度
 * @param[in] margin_radius : 边框的半径
 * @param[in] margin_color : 边框的颜色
 * @return 0成功，其他失败
 **/
int compo_rowbox_set_margin(compo_rowbox_t *rowbox, u8 margin_en, u8 margin_width, u16 margin_radius, u16 margin_color)
{
    if ((!rowbox) || (!rowbox->page)) {
        return -1;
    }
    rowbox->margin_en = margin_en;
    rowbox->margin_radius = margin_radius;
    rowbox->margin_color = margin_color;
    rowbox->margin_width = margin_width;

    for (int i=0; i<IMG_MAX; i++) {
        widget_rect_set_color(rowbox->rect[i], margin_color);
    }

    return 0;
}



/**
 * @brief 设置列表（横向）控件内容
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] *res_tbl : 存放图像资源的地址
 * @param[in] img_cnt : 图像数量
 **/
void compo_rowbox_set(compo_rowbox_t *rowbox, const u32 *res_tbl, u8 img_cnt)
{
    area_t img_area;

#if BOX_GUI_ROTATE_DISP
     if (rowbox->style == COMPO_ROWBOX_STYLE_NORMAL) {
        img_area = rotate_gui_image_get_size(res_tbl[0]);
    } else {
        img_area = rotate_gui_image_get_size(compo_rowbox_preview_style(res_tbl[0]));
    }
#else
    if (rowbox->style == COMPO_ROWBOX_STYLE_NORMAL) {
        img_area = gui_image_get_size(res_tbl[0]);
    } else {
        img_area = gui_image_get_size(compo_rowbox_preview_style(res_tbl[0]));
    }
#endif // BOX_GUI_ROTATE_DISP
#if BOX_GUI_ROTATE_DISP
    s16 row_space = ROWBOX_ICON_GAP;
#else
    s16 row_space = 15;                                         //空白间隙
#endif // BOX_GUI_ROTATE_DISP
    s16 row_width = img_area.wid;                               //图片宽度
    s16 row_center_x = row_width >> 1;                          //图片x轴中心位置
    s16 row_width_total = row_space + row_width;                //图片宽度+空白
    s16 total_width = row_width_total * img_cnt - row_space;    //图片拼接的总长度
    s16 cycle_width = row_width_total * img_cnt;                //循环时，图片拼接的总长度

    rowbox->row_space = row_space;
    rowbox->row_width = row_width;
    rowbox->row_center_x = row_center_x;
    rowbox->row_width_total = row_width_total;

    rowbox->total_width = total_width;
    rowbox->cycle_width = cycle_width;

    rowbox->res_tbl = res_tbl;
    rowbox->img_area = img_area;
    rowbox->img_cnt = img_cnt;

//    printf("row_space:%d\n", row_space);
//    printf("row_width:%d\n", row_width);
//    printf("row_center_x:%d\n", row_center_x);
//    printf("row_width_total:%d\n", row_width_total);
//    printf("total_width:%d\n", total_width);
//    printf("cycle_width:%d\n", cycle_width);
//    printf("img_cnt:%d\n", img_cnt);
}

/**
 * @brief 根据编号获取x坐标
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] idx : 菜单编号
 * @return x坐标
 **/
int compo_rowbox_getx_byidx(compo_rowbox_t *rowbox, int idx)
{
    return (rowbox->row_center_x + rowbox->row_width_total * idx);
}

/**
 * @brief 按x坐标设置列表（横向）焦点
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] x : x坐标
 **/
void compo_rowbox_set_focus(compo_rowbox_t *rowbox, s16 x)
{
    int ix;
    rect_t location;

    if (!rowbox->flag_cycle) {
        if (x < 0) {
            x = 0;
        } else if (x > rowbox->total_width) {
            x = rowbox->total_width;
        }
    }
    rowbox->ofs_x = x;
#if BOX_GUI_ROTATE_DISP
    location = rotate_widget_get_location(rowbox->page);
    ix = location.x - rowbox->ofs_x;
    rotate_widget_page_set_client(rowbox->page, ix, 0);
#else
    location = widget_get_location(rowbox->page);
    ix = location.x - rowbox->ofs_x;
    widget_page_set_client(rowbox->page, ix, 0);
#endif // BOX_GUI_ROTATE_DISP
}

/**
 * @brief 将选择的图标设为焦点
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] idx : 图标编号
 **/
void compo_rowbox_set_focus_byidx(compo_rowbox_t *rowbox, int idx)
{
    int x = compo_rowbox_getx_byidx(rowbox, idx);
    compo_rowbox_set_focus(rowbox, x);
}

/**
 * @brief 设置列表（横向）样式
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] style : 样式
 **/
void compo_rowbox_set_style(compo_rowbox_t *rowbox, u8 style)
{
    rowbox->style = style;
}

/**
 * @brief 设置获取资源地址的回调函数
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] get_addr : 回调函数
 **/
void compo_rowbox_set_addrhook(compo_rowbox_t *rowbox, u32 (*get_addr)(u32 idx))
{
    if (rowbox == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SET_BITHOOK);
    }
    rowbox->get_addr = get_addr;
}

/**
 * @brief 更新列表框Widget
 * @param[in] rowbox : 列表（横向）指针
 **/
void compo_rowbox_update(compo_rowbox_t *rowbox)
{
    int i;
    int cidx, sidx;
    cidx = rowbox->ofs_x / rowbox->row_width_total;
    cidx -= rowbox->ofs_x < 0 ? 1 : 0;  //逆向（循环滑动）
    rowbox->focus_icon_idx = cidx;

    sidx = cidx - 3;
    for (i=0; i<IMG_MAX; i++) {
        int idx = sidx + i;
        int idx_tmp;

        if (!rowbox->flag_cycle) {      //normal
            if (idx < 0 || idx >= rowbox->img_cnt) {
                widget_set_visible(rowbox->img[i], false);
                widget_set_visible(rowbox->rect[i], false);
                rowbox->img_idx[i] = -1;
                continue;
            } else {
                rowbox->img_idx[i] = idx;
                widget_set_visible(rowbox->img[i], true);
                widget_set_visible(rowbox->rect[i], true);
            }
        } else {                        //cycle
            idx_tmp = idx + (idx < 0 ? rowbox->img_cnt : (idx >= rowbox->img_cnt ? -rowbox->img_cnt : 0));
            while (idx_tmp < 0 || idx_tmp >= rowbox->img_cnt) {
                idx_tmp = idx_tmp + (idx_tmp < 0 ? rowbox->img_cnt : (idx_tmp >= rowbox->img_cnt ? -rowbox->img_cnt : 0));
            }
            rowbox->img_idx[i] = idx_tmp;
        }

        if (rowbox->style == COMPO_ROWBOX_STYLE_NORMAL) {
            widget_image_set(rowbox->img[i], rowbox->res_tbl[rowbox->img_idx[i]]);
        } else {    //表盘的图片需要解析一下地址，才能获取预览图
            widget_image_set(rowbox->img[i], compo_rowbox_preview_style(rowbox->res_tbl[rowbox->img_idx[i]]));
        }

        int lnx = rowbox->row_center_x + idx * rowbox->row_width_total;
#if BOX_GUI_ROTATE_DISP
        int lny = GUI_GET_SCREEN_CENTER_Y + rowbox->ofs_y;
#else
        int lny = GUI_SCREEN_CENTER_Y;
#endif // BOX_GUI_ROTATE_DISP
        int dx = lnx - rowbox->ofs_x;                  //离中心距离
        int udx = abs_s(dx);
        int img_wid = rowbox->img_area.wid;
        int img_hei = rowbox->img_area.hei;
#if BOX_GUI_ROTATE_DISP
        int udx_th = GUI_GET_SCREEN_WIDTH * 2 / 25; //值越大，图标越大
        img_hei = img_hei - img_hei * (udx - udx_th) / GUI_GET_SCREEN_WIDTH;
 #else
        int udx_th = 25;

        img_hei = img_hei - img_hei * (udx - udx_th) / 320;
#endif // BOX_GUI_ROTATE_DISP
        img_wid = img_hei * img_wid / rowbox->img_area.hei;

        widget_set_size(rowbox->img[i], img_wid, img_hei);
        widget_set_alpha(rowbox->img[i], img_wid >= rowbox->img_area.wid ? 255 : (img_wid * 255 / rowbox->img_area.wid));
        widget_set_pos(rowbox->img[i], lnx, lny);
#if BOX_GUI_ROTATE_DISP
        if (rowbox->style == COMPO_ROWBOX_STYLE_ARC) {
            if (udx < ROWBOX_ARC_R) {
                lny += ROWBOX_ARC_R - sqrt64(ROWBOX_ARC_R * ROWBOX_ARC_R - udx * udx);
            }
        }

        rotate_widget_set_location(rowbox->img[i], lnx, lny, img_wid, img_hei);
        widget_set_alpha(rowbox->img[i], img_wid >= rowbox->img_area.wid ? 255 : (img_wid * 255 / rowbox->img_area.wid));
#else
        if (rowbox->margin_en) {
            //widget_set_alpha(rowbox->img[i], 255);
            if (dx == 0)
            {
                //widget_set_size(rowbox->rect[i], img_wid + 10, img_hei + 10);
                //widget_set_pos(rowbox->rect[i], lnx, lny);
                widget_set_location(rowbox->rect[i], lnx, lny, img_wid + rowbox->margin_width, img_hei + rowbox->margin_width);
                widget_rect_set_radius(rowbox->rect[i], rowbox->margin_radius);
                widget_set_visible(rowbox->rect[i], true);
            } else {
                widget_set_visible(rowbox->rect[i], false);
            }
        }
#endif // BOX_GUI_ROTATE_DISP
    }
}

/**
 * @brief 列表框拖动与移动初始化
 * @param[in] rowbox : 列表（横向）指针
 **/
void compo_rowbox_move_init(compo_rowbox_t *rowbox)
{
    compo_rowbox_move_cb_t *mcb = rowbox->mcb;
    if (mcb == NULL) {
        return;
    }
    mcb->focus_x = rowbox->ofs_x;
    mcb->first_x = compo_rowbox_getx_byidx(rowbox, 0);
    mcb->last_x = compo_rowbox_getx_byidx(rowbox, rowbox->img_cnt - 1);
}

/**
 * @brief 列表框拖动与移动处理
 * @param[in] rowbox : 列表（横向）指针
 **/
void compo_rowbox_move(compo_rowbox_t *rowbox)
{
    compo_rowbox_move_cb_t *mcb = rowbox->mcb;
    if (mcb == NULL) {
        return;
    }
    if (mcb->flag_drag) {
        s32 dx = mcb->focus_dx;
        s32 dy;
        mcb->flag_drag = ctp_get_dxy(&mcb->focus_dx, &dy);
        if (mcb->flag_drag) {
            //拖动菜单图标
            mcb->focus_xstep = mcb->focus_dx - dx;
            compo_rowbox_set_focus(rowbox, mcb->focus_x - mcb->focus_dx);
            compo_rowbox_update(rowbox);
        } else {
            //抬手后开始自动移动
            s32 last_dx = ctp_get_last_dxy().x;
            mcb->focus_x = rowbox->ofs_x;
            mcb->flag_move_auto = true;
            mcb->moveto_idx = rowbox->focus_icon_idx - (last_dx * DRAG_AUTO_SPEED / GUI_SCREEN_HEIGHT);
            if (mcb->moveto_idx > rowbox->img_cnt && !rowbox->flag_cycle) {
                mcb->moveto_idx = rowbox->img_cnt;
            } else if (mcb->moveto_idx < -1 && !rowbox->flag_cycle) {
                mcb->moveto_idx = -1;
            }
            mcb->moveto_x = compo_rowbox_getx_byidx(rowbox, mcb->moveto_idx);
            mcb->tick = tick_get();
        }
    }
    if (mcb->flag_move_auto) {
        //自动移动
        if (mcb->focus_x == mcb->moveto_x) {
            if (mcb->focus_x < mcb->first_x && !rowbox->flag_cycle) {
                mcb->moveto_x = mcb->first_x;             //超过第1个，回弹
            } else if (mcb->focus_x > mcb->last_x && !rowbox->flag_cycle) {
                mcb->moveto_x = mcb->last_x;              //超过最后1个，回弹
            } else {
                mcb->flag_move_auto = false;              //移动完成
                rowbox->ofs_x %= rowbox->cycle_width;  //循环滑动防止ofs_y溢出
                rowbox->focus_icon_idx = rowbox->ofs_x / rowbox->row_width_total - (rowbox->ofs_x < 0 ? 1 : 0);
                mcb->focus_x = rowbox->ofs_x;
            }
        } else if (tick_check_expire(mcb->tick, FOCUS_AUTO_TICK_EXPIRE)) {
            s32 dx;
            mcb->tick = tick_get();
            dx = mcb->moveto_x - mcb->focus_x;
            if (dx > 0) {
                if (dx > FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    dx = dx / FOCUS_AUTO_STEP_DIV;
                } else if (dx > FOCUS_AUTO_STEP) {
                    dx = FOCUS_AUTO_STEP;
                } else {
                    dx = 1;
                }
            } else {
                if (dx < -FOCUS_AUTO_STEP * FOCUS_AUTO_STEP_DIV) {
                    dx = dx / FOCUS_AUTO_STEP_DIV;
                } else if (dx < -FOCUS_AUTO_STEP) {
                    dx = -FOCUS_AUTO_STEP;
                } else {
                    dx = -1;
                }
            }
            mcb->focus_x += dx;
            compo_rowbox_set_focus(rowbox, mcb->focus_x);
            compo_rowbox_update(rowbox);
        }
    }
}

/**
 * @brief 按坐标选择菜单项
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] pt : 坐标
 * @return 返回菜单项索引
 **/
int compo_rowbox_select(compo_rowbox_t *rowbox, point_t pt)
{
    if (rowbox == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SELECT);
    }
    int i;
    for (i=0; i<IMG_MAX; i++) {
        if (rowbox->img_idx[i] < 0) {
            continue;
        }
#if BOX_GUI_ROTATE_DISP
        rect_t rect = rotate_widget_get_absolute(rowbox->img[i]);
#else
        rect_t rect = widget_get_absolute(rowbox->img[i]);
#endif // BOX_GUI_ROTATE_DISP
        if (abs_s(pt.y - rect.y) * 2 <= rect.hei && abs_s(pt.x - rect.x) * 2 <= rect.wid) {
            return rowbox->img_idx[i];
        }
    }
    return -1;
}

/**
 * @brief 是否使能列表循环滑动功能
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] cycle : true 使能循环滑动
                      false 关闭循环滑动
 **/
void compo_rowbox_cycle_en(compo_rowbox_t *rowbox, bool cycle)
{
    rowbox->flag_cycle = cycle;
}

/**
 * @brief 已设置列表图标的纵向偏移
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] ofs_y : 相对于屏幕中央的纵向偏移
 **/
void compo_rowbox_set_ofs_y(compo_rowbox_t *rowbox, s16 ofs_y)
{
    rowbox->ofs_y = ofs_y;
}

/**
 * @brief 获取焦点序号
 * @param[in] rowbox : 列表（横向）指针
 * @return 返回焦点序号（如果是负数则转为正数）
 **/
s16 compo_rowbox_get_focus_idx(compo_rowbox_t *rowbox)
{
    return (((rowbox->focus_icon_idx % rowbox->img_cnt) + rowbox->img_cnt) % rowbox->img_cnt);
}

/**
 * @brief 按索引选择图标
 * @param[in] rowbox : 列表（横向）指针
 * @param[in] idx : 编号索引（正数）
 * @return 返回图标指针
 **/
widget_icon_t *compo_rowbox_select_byidx(compo_rowbox_t *rowbox, int idx)
{
    s16 idx_offs = 0;
    s16 idx_real = 0;
    s16 focus = compo_rowbox_get_focus_idx(rowbox);
    if (rowbox->img_cnt > IMG_MAX && focus < (IMG_MAX / 2) && (rowbox->img_cnt - idx) <= (IMG_MAX / 2)) {
        idx_offs = idx - (rowbox->img_cnt + focus);
    } else if (rowbox->img_cnt > IMG_MAX && (rowbox->img_cnt - focus) <= (IMG_MAX / 2) && idx < (IMG_MAX / 2)) {
        idx_offs = (idx + rowbox->img_cnt) - focus;
    } else {
        idx_offs = idx - focus;
    }
    idx_real = IMG_MAX / 2 + idx_offs;
    idx_real = MAX(0, MIN(IMG_MAX - 1, idx_real));
    return rowbox->img[idx_real];
}
