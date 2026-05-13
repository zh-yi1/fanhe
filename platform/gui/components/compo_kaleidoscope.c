#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define KALE_GET_ANGLE(angle)                    ((angle >= 0) ? (angle % 3600) : (angle % 3600 + 3600))    //度数转换
#define KALE_GET_ICON_REL_X(r, angle)            (-muls_shift16(r, SIN(KALE_GET_ANGLE(angle))))           //根据角度获取相对X
#define KALE_GET_ICON_REL_Y(r, angle)            (-muls_shift16(r, COS(KALE_GET_ANGLE(angle))))           //根据角度获取相对Y

static const int16_t tbl_angle[KALE_CIR_ICON_NUM] = {
    3150, 0, 450, 900, 1350, 1800, 2250, 2700,
};

int limit_range(int val, int min, int max)
{
    if (val > max) {
        val = max;
    } else if (val < min) {
        val = min;
    }
    return val;
}

void cycle_plus(u8 *val, u8 min, u8 max)
{
    if(*val >= max){
        *val = min;
    }else{
        (*val)++;
    }
}

void cycle_sub(u8 *val, u8 min, u8 max)
{
    if(*val <= min){
        *val = max;
    }else{
        (*val)--;
    }
}

/**
 * @brief 创建一个万花筒组件
 * @param[in] frm : 窗体指针
 * @return 返回万花筒指针
 **/
compo_kaleidoscope_t *compo_kaleidoscope_create(compo_form_t *frm)
{
    // printf("compo_kaleidoscope_t size->%d\n", sizeof(compo_kaleidoscope_t));

    compo_kaleidoscope_t *kale = compo_create(frm, COMPO_TYPE_KALEIDOSCOPE);
    widget_page_t *page = widget_page_create(frm->page_body);
    widget_set_location(page, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    widget_page_set_client(page, 0, 0);  //圆心
    kale->page = page;

    return kale;
}

/**
 * @brief 万花筒图标角度初始化
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_angle_init(compo_kaleidoscope_t *kale)
{
    compo_kale_icon_t *p = NULL;
    for (u8 k = 0; k < KALE_CIR_MAX; k++) {
        for (u8 i = 0; i < KALE_CIR_ICON_NUM; i++) {
            p = &kale->dcir[k].icon[i];
            p->angle = tbl_angle[i] + k * KALE_CIR_ANGLE_DEV;
        }
    }
}

/**
 * @brief 初始化万花筒内外圈层
 * 注：根据屏幕大小自适应内外圈层半径
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] dc_width : 屏幕宽度
 * @param[in] dc_height : 屏幕高度
 * @param[in] osize : 外圈图标尺寸
 * @param[in] isize : 内圈图标尺寸
 * @param[in] edge_space : 离屏幕边缘距离
 * @param[in] icon_space : 内外圈图标间距离
 **/
void compo_kaleidoscope_init(compo_kaleidoscope_t *kale, s16 dc_width, s16 dc_height, u16 osize, u16 isize, s16 edge_space, s16 icon_space)
{
    u16 short_side , sw, sh = 0;
    sw = dc_width;
    sh = dc_height;
    kale->cen.x = sw >> 1;
    kale->cen.y = sh >> 1;
    short_side = (sw > sh) ? sh : sw;
    kale->ctl.osize = osize;
    kale->ctl.isize = isize;
    kale->ctl.org_or = (short_side >> 1) - edge_space - (kale->ctl.osize >> 1);
    kale->ctl.org_ir = kale->ctl.org_or - (kale->ctl.osize >> 1) - icon_space - (kale->ctl.isize >> 1);
    kale->ctl.oidx = KALE_CIR_0;
    kale->ctl.iidx = KALE_CIR_1;

    kale->dcir[kale->ctl.oidx].size = kale->ctl.osize;
    kale->dcir[kale->ctl.oidx].r    = kale->ctl.org_or;
    kale->dcir[kale->ctl.oidx].lay  = KALE_FUNC_LAY_MAX;
    kale->dcir[kale->ctl.iidx].size = kale->ctl.isize;
    kale->dcir[kale->ctl.iidx].r    = kale->ctl.org_ir;
    kale->dcir[kale->ctl.iidx].lay  = KALE_FUNC_LAY_0;

    compo_kaleidoscope_angle_init(kale);
}

/**
 * @brief 往万花筒图标集合里面添加图标
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] item : 图标集指针
 * @param[in] item_cnt : 图标集总数
 **/
void compo_kaleidoscope_icon_add(compo_kaleidoscope_t *kale, menu_kale_item_t const *item, int item_cnt)
{
	if (item_cnt <= 0) {
		halt(HALT_GUI_COMPO_ICONLIST_ADD);
	}
    if (item_cnt > (KALE_FUNC_LAYER * KALE_CIR_ICON_NUM)) {
        item_cnt = KALE_FUNC_LAYER * KALE_CIR_ICON_NUM;
    }

    kale->dcir[kale->ctl.oidx].lay  = (item_cnt - 1) / KALE_CIR_ICON_NUM;
    kale->dcir[kale->ctl.iidx].lay  = KALE_FUNC_LAY_0;

    for (int i = 0; i < item_cnt; i++) {
        u8 idx_cir  = i / KALE_CIR_ICON_NUM;
        u8 idx_icon = i % KALE_CIR_ICON_NUM;
        widget_icon_t *icon = widget_icon_create(kale->page, item[i].res_addr);
		if (i == 0) {
			kale->time_bg = icon;    //默认第一个为时钟
		}

        kale->ofunc[idx_cir].fsub[idx_icon].widget   = icon;
        kale->ofunc[idx_cir].fsub[idx_icon].func_sta = item[i].func_sta;

        compo_kale_func_t *fp = &kale->ofunc[idx_cir].fsub[idx_icon];

        if (idx_cir == kale->dcir[kale->ctl.oidx].lay) {
            compo_kale_icon_t *dp = &kale->dcir[kale->ctl.oidx].icon[idx_icon];
            dp->fsub.widget       = fp->widget;
            dp->fsub.func_sta     = fp->func_sta;
        }

        if (idx_cir == kale->dcir[kale->ctl.iidx].lay) {
            compo_kale_icon_t *dp = &kale->dcir[kale->ctl.iidx].icon[idx_icon];
            dp->fsub.widget      = fp->widget;
            dp->fsub.func_sta    = fp->func_sta;
        }
        kale->total_cnt++;
    }
}

/**
 * @brief 根据变化半径，万花筒内外圈半径更新
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] dr : 变化半径
 **/
void compo_kaleidoscope_update_r(compo_kaleidoscope_t *kale, s16 dr)
{
    kale->dcir[kale->ctl.oidx].r -= dr;
    kale->dcir[kale->ctl.iidx].r += dr;
    kale->dcir[kale->ctl.oidx].r = limit_range(kale->dcir[kale->ctl.oidx].r, kale->ctl.org_ir, kale->ctl.org_or);
    kale->dcir[kale->ctl.iidx].r = limit_range(kale->dcir[kale->ctl.iidx].r, kale->ctl.org_ir, kale->ctl.org_or);
    if (kale->dcir[kale->ctl.oidx].r == kale->ctl.org_ir || kale->dcir[kale->ctl.oidx].r == kale->ctl.org_or) {
        kale->ctl.sta = KALE_CTL_ROTATE;
    }
}

/**
 * @brief 根据内外圈半径，获取当前圈层的图标尺寸
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] r : 内外圈半径
 * @return 返回当前圈层图标尺寸
 **/
u16 compo_kaleidoscope_size_from_r(compo_kaleidoscope_t *kale, u16 r)
{
    return ((kale->ctl.osize - kale->ctl.isize) * (r - kale->ctl.org_ir) / (kale->ctl.org_or - kale->ctl.org_ir) + kale->ctl.isize);
}

/**
 * @brief 万花筒图标尺寸更新
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_update_size(compo_kaleidoscope_t *kale)
{
    kale->dcir[kale->ctl.oidx].size = compo_kaleidoscope_size_from_r(kale, kale->dcir[kale->ctl.oidx].r);
    kale->dcir[kale->ctl.iidx].size = compo_kaleidoscope_size_from_r(kale, kale->dcir[kale->ctl.iidx].r);
}

/**
 * @brief 根据变化角度，万花筒旋转角度更新
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] dr : 变化半径
 **/
void compo_kaleidoscope_update_angle(compo_kaleidoscope_t *kale, int16_t dangle)
{
    int16_t angle = dangle;
    angle = (angle < 0 || angle > KALE_CIR_ANGLE_COM) ? 1 : angle;
    compo_kale_icon_t *p = NULL;
    for (u8 k = 0; k < KALE_CIR_MAX; k++) {
        for (u8 i = 0; i < KALE_CIR_ICON_NUM; i++) {
            p = &kale->dcir[k].icon[i];
            p->angle = (kale->ctl.oidx == k) ? (p->angle - angle) : (p->angle + angle);
            if (kale->ctl.oidx == k) {
                if (kale->ctl.oidx == KALE_CIR_0) {
                    p->angle = (p->angle <= (tbl_angle[i] - KALE_CIR_ANGLE_COM)) ? (tbl_angle[i] - KALE_CIR_ANGLE_COM) : p->angle;
                } else if (kale->ctl.oidx == KALE_CIR_1) {
                    p->angle = (p->angle <= (tbl_angle[i] + KALE_CIR_ANGLE_DEV)) ? (tbl_angle[i] + KALE_CIR_ANGLE_DEV) : p->angle;
                }
            } else {
                if (kale->ctl.oidx == KALE_CIR_0) {
                    p->angle = (p->angle >= (tbl_angle[i] + KALE_CIR_ANGLE_DEV_OI)) ? (tbl_angle[i] + KALE_CIR_ANGLE_DEV_OI) : p->angle;
                } else if (kale->ctl.oidx == KALE_CIR_1) {
                    p->angle = (p->angle >= (tbl_angle[i])) ? (tbl_angle[i]) : p->angle;
                }
            }
        }
    }

    p = &kale->dcir[kale->ctl.oidx].icon[0];
    if (p->angle == (tbl_angle[0] - KALE_CIR_ANGLE_COM) || p->angle == tbl_angle[0] + KALE_CIR_ANGLE_DEV) {
        kale->ctl.oidx = (kale->ctl.oidx == KALE_CIR_0) ? KALE_CIR_1 : KALE_CIR_0;
        kale->ctl.iidx = (kale->ctl.iidx == KALE_CIR_0) ? KALE_CIR_1 : KALE_CIR_0;
        kale->ctl.sta = KALE_CTL_SWAP;
    }
}

/**
 * @brief 万花筒上下层任务切换设置
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] msg : 消息
 **/
void compo_kaleidoscope_ctl_swap_set(compo_kaleidoscope_t *kale, size_msg_t msg)
{
    if (kale->ctl.sta) return;

    switch (msg) {
        case MSG_CTP_SHORT_UP:
        case MSG_CTP_SHORT_RIGHT:
        case MSG_QDEC_FORWARD:
            kale->ctl.swap = KALE_SWAP_LAST;
            break;

        case MSG_CTP_SHORT_DOWN:
        case MSG_CTP_SHORT_LEFT:
        case MSG_QDEC_BACKWARD:
            kale->ctl.swap = KALE_SWAP_NEXT;
            break;

        default:
            kale->ctl.swap = KALE_SWAP_NULL;
            break;
    }
}

/**
 * @brief 万花筒上下层任务信息更替
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] swap : 上下切换
 * @return  true  切换成功
 *          false 切换失败
 **/
bool compo_kaleidoscope_ctl_swap_pro(compo_kaleidoscope_t *kale, u8 swap)
{
    bool sf = false;
    if (kale->ctl.sta) return sf;

    u8 lay;
    compo_kale_func_t *fp, *dp = NULL;
    if (swap == KALE_SWAP_LAST) {                            //上一级
        lay = kale->dcir[kale->ctl.oidx].lay;
        cycle_sub((u8 *)&lay, KALE_FUNC_LAY_0, ((kale->total_cnt - 1) / KALE_CIR_ICON_NUM));
        kale->dcir[kale->ctl.iidx].lay = lay;
        for (u8 i = 0; i < KALE_CIR_ICON_NUM; i++) {
            fp = &kale->ofunc[kale->dcir[kale->ctl.iidx].lay].fsub[i];
            dp = &kale->dcir[kale->ctl.iidx].icon[i].fsub;
            dp->widget = fp->widget;
            dp->func_sta = fp->func_sta;
        }
        sf = true;
    } else if (kale->ctl.swap == KALE_SWAP_NEXT) {         //下一级
        lay = kale->dcir[kale->ctl.iidx].lay;
        cycle_plus((u8 *)&lay, KALE_FUNC_LAY_0, ((kale->total_cnt - 1) / KALE_CIR_ICON_NUM));
        kale->dcir[kale->ctl.oidx].lay = lay;
        for (u8 i = 0; i < KALE_CIR_ICON_NUM; i++) {
            fp = &kale->ofunc[kale->dcir[kale->ctl.oidx].lay].fsub[i];
            dp = &kale->dcir[kale->ctl.oidx].icon[i].fsub;
            dp->widget = fp->widget;
            dp->func_sta = fp->func_sta;
        }
        sf = true;
    }
    return sf;
}

/**
 * @brief 万花筒上下层任务切换处理
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] sta : 切换状态
 **/
void compo_kaleidoscope_ctl_switch_pro(compo_kaleidoscope_t *kale, u8 sta)
{
    if (kale->ctl.sta) return;

    if (compo_kaleidoscope_ctl_swap_pro(kale, kale->ctl.swap)) {
        compo_kale_update(kale);
        kale->ctl.sta = sta;
    }
}

/**
 * @brief 万花筒上下层任务切换处KICK
 * @param[in] kale : 万花筒图标集合指针
 * @param[in] msg : 消息
 **/
void compo_kaleidoscope_ctl_switch_kick(compo_kaleidoscope_t *kale, size_msg_t msg)
{
    if (kale->ctl.sta) return;

    compo_kaleidoscope_ctl_swap_set(kale, msg);
    compo_kaleidoscope_ctl_switch_pro(kale, KALE_CTL_SWITCH);
}

/**
 * @brief 万花筒入场动画KICK
 * 注：从表盘页面进入万花筒，使能入场动画
 * 注：从子任务返回不具备入场动画，并返回子任务所在的层级
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_adm_animation_kick(compo_kaleidoscope_t *kale)
{
    if (kale->ctl.sta) return;
    if (kale->focus_icon_idx) return;
    kale->ctl.adm = true;
    kale->ctl.sta = KALE_CTL_SWITCH;
}

/**
 * @brief 万花筒入场动画
 * 注：从表盘页面进入万花筒，使能入场动画
 * 注：从子任务返回不具备入场动画，并返回子任务所在的层级
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_adm_animation_pro(compo_kaleidoscope_t *kale)
{
    if (!kale->ctl.adm) return;

    compo_kale_func_t *fp, *dp = NULL;
    kale->dcir[kale->ctl.iidx].lay = kale->dcir[kale->ctl.oidx].lay + 1;

    if (kale->dcir[kale->ctl.iidx].lay > ((kale->total_cnt - 1) / KALE_CIR_ICON_NUM)) {
        printf("%s ctl.iidx lay over %d\n", __func__, ((kale->total_cnt - 1) / KALE_CIR_ICON_NUM));
        kale->dcir[kale->ctl.iidx].lay = ((kale->total_cnt - 1) / KALE_CIR_ICON_NUM);
    }

    for (u8 i = 0; i < KALE_CIR_ICON_NUM; i++) {
        fp = &kale->ofunc[kale->dcir[kale->ctl.iidx].lay].fsub[i];
        dp = &kale->dcir[kale->ctl.iidx].icon[i].fsub;
        dp->widget = fp->widget;
        dp->func_sta = fp->func_sta;
    }
    compo_kale_update(kale);
    kale->ctl.adm = false;
}

/**
 * @brief 万花筒图标可视化设置
 * 注：显示内外圈层所在任务层图标
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_set_vis(compo_kaleidoscope_t *kale)
{
    u8 dolay = kale->dcir[kale->ctl.oidx].lay;
    u8 dilay = kale->dcir[kale->ctl.iidx].lay;

	if (kale->page_time != NULL) {
		widget_set_visible(kale->page_time, false);
	}
    for (int i = 0; i < kale->total_cnt; i++) {
        u8 idx_cir  = i / KALE_CIR_ICON_NUM;
        u8 idx_icon = i % KALE_CIR_ICON_NUM;
        compo_kale_func_t *fp = &kale->ofunc[idx_cir].fsub[idx_icon];
        if (fp->widget) {
            widget_set_visible(fp->widget, false);
            if (idx_cir == dolay || idx_cir == dilay) {
                widget_set_visible(fp->widget, true);
				
				if(kale->page_time != NULL && fp->widget == kale->time_bg) {
					widget_set_visible(kale->page_time, true);
				}
            }
        }

    }
}

/**
 * @brief 万花筒切换动画
 * @param[in] kale : 万花筒图标集合指针
 **/
void compo_kaleidoscope_switch_animation(compo_kaleidoscope_t *kale, u32 expire, s16 dr, int16_t dangle)
{
    if (!kale->ctl.sta) return;
    static u32 ticks = 0;
    if (kale->ctl.sta == KALE_CTL_SWITCH) {
        if (tick_check_expire(ticks, expire)) {
            ticks = tick_get();
            compo_kaleidoscope_update_r(kale, dr);
            compo_kaleidoscope_update_size(kale);
            compo_kale_update(kale);
        }
    } else if (kale->ctl.sta == KALE_CTL_ROTATE) {
        if (tick_check_expire(ticks, expire)) {
            ticks = tick_get();
            compo_kaleidoscope_update_angle(kale, dangle);
            compo_kale_update(kale);
        }
    } else if (kale->ctl.sta == KALE_CTL_SWAP) {
        compo_kaleidoscope_adm_animation_pro(kale);
        kale->ctl.sta = KALE_CTL_NULL;
    }
}

/**
 * @brief 更新图标集合Widget
 * @param[in] kale : 图标集指针
 **/
void compo_kale_update(compo_kaleidoscope_t *kale)
{
    if (kale == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_UPDATE);
    }

    compo_kaleidoscope_set_vis(kale);

    //刷新显示层
    compo_kale_icon_t *p = NULL;
    for (u8 k = 0; k < KALE_CIR_MAX; k++) {
        for (u8 i = 0; i < KALE_CIR_ICON_NUM; i++) {
            p = &kale->dcir[k].icon[i];
            if (p->fsub.widget) {
                int16_t ix = kale->cen.x + KALE_GET_ICON_REL_X(kale->dcir[k].r, p->angle);
                int16_t iy = kale->cen.y + KALE_GET_ICON_REL_Y(kale->dcir[k].r, p->angle);
                widget_set_location(p->fsub.widget, ix, iy, kale->dcir[k].size, kale->dcir[k].size);
			
				//更新时钟尺寸
			    if (kale->page_time != NULL && p->fsub.widget == kale->time_bg) {
					widget_page_scale_to(kale->page_time, kale->dcir[k].size, kale->dcir[k].size);
			        widget_set_pos(kale->page_time, ix, iy);
				}
			
            }
        }
    }

}

/**
 * @brief 按坐标选择图标
 * @param[in] kale : 图标集指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 * @return 返回图标索引
 **/
int compo_kale_select(compo_kaleidoscope_t *kale, int x, int y)
{
    if (kale == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SELECT);
    }

    compo_kale_icon_t *p = NULL;
    bool select = false;
    int idx;
    for (u8 k = 0; k < KALE_CIR_MAX; k++) {
        for (u8 i = 0; i < KALE_CIR_ICON_NUM; i++) {
            p = &kale->dcir[k].icon[i];
            if (p->fsub.widget) {
#if BOX_GUI_ROTATE_DISP
                rect_t rect = rotate_widget_get_absolute(p->fsub.widget);
#else
                rect_t rect = widget_get_absolute(p->fsub.widget);
#endif // BOX_GUI_ROTATE_DISP
                select = rect.wid > kale->ctl.isize && abs_s(x - rect.x) * 2 <= rect.wid && abs_s(y - rect.y) * 2 <= rect.hei;
                idx = kale->dcir[k].lay * KALE_CIR_ICON_NUM + i;
                if (select) {
                    kale->sel_icon = p->fsub.widget;
                    kale->sel_idx = idx;
                    return kale->sel_idx;
                }
            }
        }
    }
    kale->sel_icon = NULL;
    kale->sel_idx = -1;
    return -1;
}

/**
 * @brief 按索引选择图标
 * @param[in] kale : 图标集指针
 * @param[in] idx : 编号索引
 * @return 返回图标指针
 **/
widget_icon_t *compo_kale_select_byidx(compo_kaleidoscope_t *kale, int idx)
{
    if (kale == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SELECT);
    }

    compo_kale_icon_t *p = NULL;
    int byidx;
    for (u8 k = 0; k < KALE_CIR_MAX; k++) {
        for (u8 i = 0; i < KALE_CIR_ICON_NUM; i++) {
            p = &kale->dcir[k].icon[i];
            if (p->fsub.widget) {
                byidx = kale->dcir[k].lay * KALE_CIR_ICON_NUM + i;
                if (byidx == idx) {
                    kale->sel_icon = p->fsub.widget;
                    kale->sel_idx = idx;
                    return kale->sel_icon;
                }
            }
        }
    }
    kale->sel_icon = NULL;
    kale->sel_idx = -1;
    return NULL;
}

/**
 * @brief 按索引设置图标集合焦点坐标 并设置当前任务圈层
 * @param[in] kale : 图标集指针
 * @param[in] idx : 编号索引
 **/
void compo_kale_set_focus_byidx(compo_kaleidoscope_t *kale, int idx)
{
    if (kale == NULL) {
        halt(HALT_GUI_COMPO_LISTBOX_SELECT);
    }

    kale->focus_icon_idx = idx;
    if (kale->focus_icon_idx == 0) {
        return;
    }

    //索引图标所处的外圈任务层及内圈为外圈的下一级任务层
    u8 dolay, dilay = 0;
    dilay = dolay = (idx / KALE_CIR_ICON_NUM);
    cycle_plus((u8 *)&dilay, KALE_FUNC_LAY_0, ((kale->total_cnt - 1) / KALE_CIR_ICON_NUM));

    //更新内外圈的圈层编号
    if ((dolay % 2) == 0) {                 //任务在外圈 圈层为 KALE_CIR_1
        kale->ctl.oidx = KALE_CIR_1;
        kale->ctl.iidx = KALE_CIR_0;
    } else {                                //任务在外圈 圈层为 KALE_CIR_0
        kale->ctl.oidx = KALE_CIR_0;
        kale->ctl.iidx = KALE_CIR_1;
    }

    //更新内外圈的半径
    kale->dcir[kale->ctl.oidx].r = kale->ctl.org_or;
    kale->dcir[kale->ctl.iidx].r = kale->ctl.org_ir;
    //更新内外圈的图标尺寸
    kale->dcir[kale->ctl.oidx].size = kale->ctl.osize;
    kale->dcir[kale->ctl.iidx].size = kale->ctl.isize;
    //更新内外圈的任务层
    kale->dcir[kale->ctl.oidx].lay = dolay;
    kale->dcir[kale->ctl.iidx].lay = dilay;

    for (int i = 0; i < kale->total_cnt; i++) {
        u8 idx_cir  = i / KALE_CIR_ICON_NUM;
        u8 idx_icon = i % KALE_CIR_ICON_NUM;
        compo_kale_func_t *fp = &kale->ofunc[idx_cir].fsub[idx_icon];
        
        //设置外圈的任务层
        if (idx_cir == kale->dcir[kale->ctl.oidx].lay) {                        //外圈
            compo_kale_icon_t *dp = &kale->dcir[kale->ctl.oidx].icon[idx_icon];
            dp->fsub.widget       = fp->widget;
            dp->fsub.func_sta     = fp->func_sta;

            if ((idx_cir % 2) == 0) {                   //任务在外圈 圈层为 KALE_CIR_1
                dp->angle += KALE_CIR_ANGLE_DEV;
            }
        }

        //设置内圈的任务层
        if (idx_cir == kale->dcir[kale->ctl.iidx].lay) {                        //内圈
            compo_kale_icon_t *dp = &kale->dcir[kale->ctl.iidx].icon[idx_icon];
            dp->fsub.widget       = fp->widget;
            dp->fsub.func_sta     = fp->func_sta;

            if ((idx_cir % 2) != 0) {                   //任务在外圈 圈层为 KALE_CIR_0
                dp->angle -= KALE_CIR_ANGLE_DEV;
            }
        }
    }
}


/**
 * @brief 向集合中添加时分秒针
          cx cy为旋转中心点, x y为目标坐标
 * @param[in] kale : 图标集指针
 * @param[in] type : COMPO_ICONLIST_TIME_TYPE_HOUR, 时
                     COMPO_ICONLIST_TIME_TYPE_MIN,  分
                     COMPO_ICONLIST_TIME_TYPE_SEC,  秒
 * @param[in] res_addr : 图像资源地址
 * @param[in] cx : x轴坐标
 * @param[in] cy : y轴坐标
 **/
void compo_kale_add_time(compo_kaleidoscope_t *kale, u8 type, u32 res_addr, s16 cx, s16 cy, s16 size)
{
    if (kale->page_time == NULL) {
        kale->page_time = widget_page_create(kale->page);
        widget_set_location(kale->page_time, 0, 0, size, size);
        widget_page_set_client(kale->page_time, size / 2, size / 2);
    }
    widget_image_t *img = widget_image_create(kale->page_time, res_addr);
#if BOX_GUI_ROTATE_DISP
    rotate_widget_image_set_rotation_center(img, cx, cy);
#else
    widget_image_set_rotation_center(img, cx, cy);
#endif // BOX_GUI_ROTATE_DISP
    switch (type) {
    case COMPO_ICONLIST_TIME_TYPE_HOUR:
        kale->hour = img;
        break;

    case COMPO_ICONLIST_TIME_TYPE_MIN:
        kale->min = img;
        break;

    case COMPO_ICONLIST_TIME_TYPE_SEC:
        kale->sec = img;
        break;

    default:
        halt(HALT_GUI_COMPO_ICONLIST_TIME_TYPE);
        return;
    }
    compo_cb.rtc_update = true;                 //创建时钟组件，需要同步更新时间
}

/**
 * @brief 设置图标集指针的起始角度
 * @param[in] kale : 图标集指针
 * @param[in] angle : 原图的起始角度
 **/
void compo_kale_set_start_angle(compo_kaleidoscope_t *kale, s16 angle)
{
    kale->start_angle = angle;
}


