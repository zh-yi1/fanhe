#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define COMPO_TEXT_AUTOROLL_NORMAL            0							//单方向滚动后循环滚动
#define COMPO_TEXT_AUTOROLL_LR                1							//左右回弹
#define COMPO_TEXT_AUTOROLL_DIRECTION         COMPO_TEXT_AUTOROLL_NORMAL//文本滚动方向

#define COMPO_TEXT_AUTOROLL_START_TICK        1000                      //文本滚动开始延时
#define COMPO_TEXT_AUTOROLL_STOP_TICK         1500                      //文本滚动结束延时
#define COMPO_TEXT_AUTOROLL_TICK              18                        //文本滚动

void func_clock_butterfly_process(void);

/**
 * @brief 根据ID获取组件对象
 * @param[in] id : 组件的ID
 * @return 返回组件的指针
 **/
void *compo_getobj_byid(u16 id)
{
    if (id == 0) {
        return NULL;
    }
    component_t *compo = compo_get_head();
    while (compo != NULL) {
        if (compo->id == id) {
            return compo;
        }
        compo = compo_get_next(compo);          //遍历组件
    }
    return NULL;
}

/**
 * @brief 根据ID获取组件对象
 * @param[in] id : 组件的ID
 * @return 返回组件的指针
 **/
void *compo_getobj_bytype(u8 type)
{
    if (type == 0) {
        return NULL;
    }
    component_t *compo = compo_get_head();
    while (compo != NULL) {
        if (compo->type == type) {
            return compo;
        }
        compo = compo_get_next(compo);          //遍历组件
    }
    return NULL;
}

/**
 * @brief 更新组件时间到TE时间
 **/
ALIGNED(256)
void compo_update(void)
{
    u32 rtc_cnt, rtc_cnt2, rtc_pr2;
    bool rtc_update;

    GLOBAL_INT_DISABLE();
    rtc_update = compo_cb.rtc_update;
    rtc_cnt = compo_cb.rtc_cnt;
    rtc_cnt2 = compo_cb.rtc_cnt2 >> 17 << 2;
    rtc_pr2 = (compo_cb.rtc_cnt2 & 0x1FFFF) + 1;
    compo_cb.rtc_update = false;
    GLOBAL_INT_RESTORE();

    if (!rtc_update) {
        return;
    }
    compo_cb.tm = time_to_tm(rtc_cnt);
    compo_cb.mtime = rtc_cnt2 * 1000 / rtc_pr2;
    compo_set_update(compo_cb.tm, compo_cb.mtime);
}

/**
 * @brief 设置绑定数据
 * @param[in] compo : 组件
 * @param[in] tm : 时间tm结构体

 **/
void compo_set_bonddata(component_t *compo, tm_t tm)
{
    bool flag_update = true;
    int value = 0;
    char value_str[16];
    memset(value_str, '\0', 16);

    switch (compo->bond_data) {
    case COMPO_BOND_YEAD:
        value = tm.year;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_MONTH:
        value = tm.mon;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_DAY:
        value = tm.day;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_WEEKDAY:
        value = tm.weekday;
        sprintf(value_str, "%d", value);
        //strcpy(value_str, i18n[STR_SUNDAY + value]);
        break;

    case COMPO_BOND_HOUR:
        value = tm.hour;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_MINUTE:
        value = tm.min;
        sprintf(value_str, "%02d", value);
        break;

    case COMPO_BOND_HOUR_H:
        value = tm.hour / 10;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_HOUR_L:
        value = tm.hour % 10;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_MINUTE_H:
        value = tm.min / 10;
        sprintf(value_str, "%02d", value);
        break;

    case COMPO_BOND_MINUTE_L:
        value = tm.min % 10;
        sprintf(value_str, "%02d", value);
        break;

    case COMPO_BOND_SECOND:
        value = tm.sec;
        sprintf(value_str, "%02d", value);
        break;

    case COMPO_BOND_DATE:
        value = tm.mon * 100 + tm.day;
        sprintf(value_str, "%02d/%02d", tm.mon, tm.day);
        break;


    case COMPO_BOND_IMAGE_WEATHER:
        value = sys_cb.weather_idx;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_BLE_STA:
        value = !ble_is_connected();
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_BT_STA:
        value = !bt_is_connected();
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_BTBLE_STA:
        //0-3:BTBLE,BT,BLE,NULL
        value = ((!bt_is_connected()) << 1) | (!ble_is_connect());
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_DISTANCE_PROGRESS:
        value = 0;
        if (compo->type == COMPO_TYPE_PICTUREBOX && sys_cb.distance_goal) {
            value = ((compo_picturebox_t*)compo)->radix * sys_cb.distance_cur / sys_cb.distance_goal;
            value = MAX(0, MIN(((compo_picturebox_t*)compo)->radix - 1, value));
        }
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_KCAL_PROGRESS:
        value = 0;
        if (compo->type == COMPO_TYPE_PICTUREBOX && sys_cb.kcal_goal) {
            value = ((compo_picturebox_t*)compo)->radix * sys_cb.kcal_cur / sys_cb.kcal_goal;
            value = MAX(0, MIN(((compo_picturebox_t*)compo)->radix - 1, value));
        }
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_STEPS_PROGRESS:
        value = 0;
        if (compo->type == COMPO_TYPE_PICTUREBOX && sys_cb.step_goal) {
            value = ((compo_picturebox_t*)compo)->radix * sys_cb.step_cur / sys_cb.step_goal;
            value = MAX(0, MIN(((compo_picturebox_t*)compo)->radix - 1, value));
        }
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_VBAT_PROGRESS:
        value = 0;
        if (compo->type == COMPO_TYPE_PICTUREBOX) {
            value = ((compo_picturebox_t*)compo)->radix * sys_cb.vbat_percent / 100;
            value = MAX(0, MIN(((compo_picturebox_t*)compo)->radix - 1, value));
        }
        sprintf(value_str, "%d", value);
        break;


    case COMPO_BOND_KCAL:
        value = 0;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_STEP:
        value = 0;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_HEARTRATE:
        value = 0;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_BLOOD_OXYGEN:
        value = 0;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_BLOOD_SUGER:
        value = 0;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_WEATHER:
        value = sys_cb.weather_idx;
       // sprintf(value_str, "%s", i18n[STR_CLOUDY + value]);
        break;

    case COMPO_BOND_ATMOMS:
        value = 0;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_TEMPERATURE:
        value = (sys_cb.temperature[0] + sys_cb.temperature[1] + 1) / 2;  //平均温度
        sprintf(value_str, "%d~%d℃", sys_cb.temperature[0], sys_cb.temperature[1]);  //温度范围
        break;

    case COMPO_BOND_ALTITUDE:
        value = 0;
        sprintf(value_str, "%d", value);
        break;

    case COMPO_BOND_BATTERY:
        value = sys_cb.vbat_percent;
        sprintf(value_str, "%d%%", value);
        break;

    case COMPO_BOND_TEMPERATURE_UNIT:
        value = 0;              //0:℃ 1:℉
        //strcpy(value_str, i18n[STR_CELSIUS + value]);
        break;

    case COMPO_BOND_DISTANCE_UNIT:
        value = 0;              //0:km 1:mile
        //strcpy(value_str, i18n[STR_KM + value]);
        break;

    default:
        flag_update = false;
        break;
    }

    if (flag_update) {
//        printf("bond_data:%d, type:%d, value:%d\n", compo->bond_data, compo->type, value);
        if (compo->type == COMPO_TYPE_NUMBER) {
            compo_number_t *num = (compo_number_t *)compo;
            if (num->num_part) {
                if (num->num_part > 1) {
                    value /= soft_pow(10, num->num_part - 1);
                }
                value %= 10;
            }
            compo_number_set(num, value);
            compo_number_set_visible(num, true);
        } else if (compo->type == COMPO_TYPE_TEXTBOX) {
            compo_textbox_t *txt = (compo_textbox_t *)compo;
            compo_textbox_set(txt, value_str);
            compo_textbox_set_visible(txt, true);
        } else if (compo->type == COMPO_TYPE_PICTUREBOX) {
            compo_picturebox_t *pic = (compo_picturebox_t *)compo;
            compo_picturebox_cut(pic, value, pic->radix);
            compo_picturebox_set_visible(pic, true);
        }
    }
}

/**
 * @brief 设置滚动
 * @param[in] rcb : 滚动控制结构体
 * @param[in] txt : 滚动Widget Text
 * @param[in] multiline : 是否为多行竖向滚动

 **/
static void compo_set_roll(compo_roll_cb_t *rcb, widget_text_t *txt, bool multiline)
{
    if (!rcb->mode && !rcb->is_drag) {
        return;
    }

    static bool flag_drag = false;
    static s32 ddy = 0;
    s32 dx=0,dy=0;
    bool align_center = widget_get_align_center(txt);
    area_t rel_textbox_area = widget_text_get_box_area_rel(txt);
    area_t rel_text_area = widget_text_get_area(txt);
    u16 circ_pixel = widget_text_get_autoroll_circ_pixel(txt);
    int end_hei = rel_textbox_area.hei * 2 / 3;
    bool txt_directoion = widget_text_get_right_align(txt);
    s16 client_x = 0, client_y = 0;
    widget_text_get_client(txt, &client_x, &client_y);


    flag_drag = ctp_get_dxy(&dx, &dy);
    if (flag_drag) {    //按下

    } else {        //抬起
        ddy = rcb->offset;
    }

    switch (rcb->sta) {
    case COMPO_ROLL_STA_IDLE:
        if (flag_drag && rcb->is_drag && multiline) {           //多行显示可拖拽
            rcb->tick = tick_get();
            rcb->sta = COMPO_ROLL_STA_ROLL;
        } else if (tick_check_expire(rcb->tick, COMPO_TEXT_AUTOROLL_START_TICK)) {
            rcb->tick = tick_get();
            rcb->sta = COMPO_ROLL_STA_ROLL;
        }
        break;

    case COMPO_ROLL_STA_ROLL:
        if (flag_drag && rcb->is_drag && multiline) {           //多行显示可拖拽
            rcb->tick = tick_get();
            rcb->offset = ddy + dy;
            if (rcb->offset > 0) {
                rcb->offset = 0;
            } else if (rcb->offset < end_hei - rel_text_area.hei) {
                rcb->offset = end_hei - rel_text_area.hei;
            }
            widget_text_set_client(txt, client_x, rcb->offset);
        } else if (tick_check_expire(rcb->tick, COMPO_TEXT_AUTOROLL_TICK)) {
            rcb->tick = tick_get();
            if (rcb->mode) {
                rcb->offset += rcb->direction;
            }
            if (multiline) {
                if (rel_text_area.hei + rcb->offset <= end_hei) {
                    rcb->sta = COMPO_ROLL_STA_STOP;
                }
                widget_text_set_client(txt, client_x, rcb->offset);

            } else {
                int end_wid = 0;
				rel_text_area.wid = align_center ? rel_text_area.wid / 2 : rel_text_area.wid;  //文本居中时，宽度需要/2，防止滚完导致C208蓝屏
                if (txt_directoion) {
#if COMPO_TEXT_AUTOROLL_DIRECTION
                    end_wid = rel_textbox_area.wid * 1 / 3;
                    if ((rcb->offset <= -(rel_text_area.wid * 1 / 3 + end_wid)) || (rcb->offset >= end_wid)) {
                        rcb->direction = -rcb->direction;
                    }
#else
                    if (rcb->mode == TEXT_AUTOROLL_MODE_SROLL_CIRC) {
                        end_wid = 0;
                    } else if (rcb->mode == TEXT_AUTOROLL_MODE_SROLL) {
                        end_wid = rel_text_area.wid;
                        if (rel_textbox_area.wid < rel_text_area.wid / 3) {
                            end_wid += (rel_text_area.wid / 3 - rel_textbox_area.wid);
                        }
                    }

                    if (rcb->offset > end_wid) {
                        if (rcb->mode == TEXT_AUTOROLL_MODE_SROLL_CIRC) {
							rcb->offset = -(rel_text_area.wid * (1 + align_center)  + circ_pixel);
						} else if (rcb->mode == TEXT_AUTOROLL_MODE_SROLL) {
                            rcb->sta = COMPO_ROLL_STA_STOP;
						}
                    }
#endif
                } else {
#if COMPO_TEXT_AUTOROLL_DIRECTION
                    end_wid = rel_textbox_area.wid * 1 / 3;
                    if ((rcb->offset <= -(rel_text_area.wid * 1 / 3 + end_wid)) || (rcb->offset >= end_wid)) {
                        rcb->direction = -rcb->direction;
                    }
#else
                    s16 circ_end_wid = align_center ? (rel_text_area.wid * 2 + circ_pixel) : (rel_text_area.wid + circ_pixel);
                    if (rcb->mode == TEXT_AUTOROLL_MODE_SROLL_CIRC) {
                        end_wid = circ_end_wid;
                    } else if (rcb->mode == TEXT_AUTOROLL_MODE_SROLL) {
                        end_wid = rel_text_area.wid * 2 / 3;
                    }

                    if (rel_textbox_area.wid < rel_text_area.wid / 3) {
                        end_wid += (rel_text_area.wid / 3 - rel_textbox_area.wid);
                    }
                    if (rcb->offset < -end_wid) {
						if (rcb->mode == TEXT_AUTOROLL_MODE_SROLL_CIRC) {
							rcb->offset = 0;
						} else if (rcb->mode == TEXT_AUTOROLL_MODE_SROLL){
                          rcb->sta = COMPO_ROLL_STA_STOP;
						}

                    }
#endif
                }
                widget_text_set_client(txt, rcb->offset, client_y);
            }
        }
        break;

    case COMPO_ROLL_STA_STOP:
        if (flag_drag && rcb->is_drag && multiline) {               //多行显示可拖拽
            rcb->tick = tick_get();
            rcb->sta = COMPO_ROLL_STA_ROLL;
        } else if (tick_check_expire(rcb->tick, COMPO_TEXT_AUTOROLL_STOP_TICK)) {
            rcb->tick = tick_get();
            rcb->sta = COMPO_ROLL_STA_IDLE;
            if (multiline) {
                rcb->offset = 0;
                widget_text_set_client(txt, client_x, rcb->offset);
            } else {
                rcb->offset = (txt_directoion && rcb->mode) ?  -rel_text_area.wid : 0;
                if (txt_directoion && (rcb->mode == TEXT_AUTOROLL_MODE_SROLL_CIRC)) {
                    rcb->offset = -rel_text_area.wid;
                    //居中对齐且存在滚动时从最左边开始滚动
                    if ( widget_get_align_center(txt)) {
                        rcb->offset = (rel_text_area.wid - rel_textbox_area.wid) / 2;
                    }
                } else {
                    rcb->offset = 0;
                }
                widget_text_set_client(txt, rcb->offset, client_y);
            }
        }
        break;

    default:
        halt(HALT_GUI_COMPO_ROLL);
        break;
    }
}

/**
 * @brief 设置组件更新
 * @param[in] tm : 时间tm结构体
 * @param[in] mtime : 毫秒
 **/
void compo_set_update(tm_t tm, u16 mtime)
{
    component_t *compo = compo_get_head();
    s16 angle_h, angle_m, angle_s;
    char time_str[8];

    angle_h = tm.hour * 300 + tm.min * 5 + tm.sec / 12;
    angle_m = tm.min * 60 + tm.sec;
    //angle_s = tm.sec * 60 + mtime / 100 * 6 - 900;              //按100ms对齐
    angle_s = tm.sec * 60 + mtime * 3 / 100 * 2;          //按33ms(0.2度)对齐
    //angle_s = tm.sec * 60 + mtime * 6 / 100 - 900;              //按实际ms

    //printf("%d %d %d\n", angle_h, angle_m, angle_s);
    while (compo != NULL) {
        switch (compo->type) {
        case COMPO_TYPE_DATETIME:
            {
                s16 angle = 0;
                switch (compo->bond_data) {
                case COMPO_BOND_HOUR:
                    angle = angle_h;
                    break;

                case COMPO_BOND_MINUTE:
                    angle = angle_m;
                    break;

                case COMPO_BOND_SECOND:
                    angle = angle_s;
                    break;

                default:
                    break;
                }
                if (angle) {
                    compo_datetime_t *dtime = (compo_datetime_t *)compo;
                    angle -= dtime->start_angle;
                    angle = dtime->anticlockwise ? (3600 - angle) : angle;
                    widget_image_set_rotation(dtime->img, angle);
                }
            }
            break;

        case COMPO_TYPE_ICONLIST:
            {
                s16 angle = 0;
                compo_iconlist_t *iconlist = (compo_iconlist_t *)compo;
                if (iconlist->hour != NULL) {
                    angle = angle_h - iconlist->start_angle;
                    widget_image_set_rotation(iconlist->hour, angle);
                }
                if (iconlist->min != NULL) {
                    angle = angle_m - iconlist->start_angle;
                    widget_image_set_rotation(iconlist->min, angle);
                }
                if (iconlist->sec != NULL) {
                    angle = angle_s - iconlist->start_angle;
                    widget_image_set_rotation(iconlist->sec, angle);
                }
                if (iconlist->page_time != NULL) {
                    //同步时钟图标Alpha值
                    widget_set_alpha(iconlist->page_time , widget_get_alpha(iconlist->time_bg));
                }
            }
            break;

        case COMPO_TYPE_LISTBOX:
            {
                compo_listbox_t *listbox = (compo_listbox_t *)compo;
                int i;
                for (i=0; i<LISTBOX_ITEM_CNT; i++) {
                    compo_set_roll(&listbox->roll_cb[i], listbox->item_text[i], false);     //滚动
                }

                s16 angle = 0;
                if (listbox->hour != NULL) {
                    angle = angle_h - listbox->start_angle;
                    widget_image_set_rotation(listbox->hour, angle);
                }
                if (listbox->min != NULL) {
                    angle = angle_m - listbox->start_angle;
                    widget_image_set_rotation(listbox->min, angle);
                }
                if (listbox->sec != NULL) {
                    angle = angle_s - listbox->start_angle;
                    widget_image_set_rotation(listbox->sec, angle);
                }
                if (listbox->page_time != NULL && listbox->time_bg != NULL) {
                    //同步时钟图标Alpha值
                    widget_set_alpha(listbox->page_time , widget_get_alpha(listbox->time_bg));
                }

            }
            break;

        case COMPO_TYPE_CARDBOX:
            {
                compo_cardbox_t *cardbox = (compo_cardbox_t *)compo;
                if (cardbox->text[0]) {
                    compo_set_roll(&cardbox->roll_cb, cardbox->text[0], false);     //滚动
                }
            }
            break;

        case COMPO_TYPE_FORM:
            {
                compo_form_t *frm = (compo_form_t *)compo;
                if (widget_get_visble(frm->time)) {
                    sprintf(time_str, "%02d:%02d", tm.hour, tm.min);
                    widget_text_set(frm->time, time_str);
                }
            }
            break;

        case COMPO_TYPE_NUMBER:
        case COMPO_TYPE_PICTUREBOX:
            if (compo->bond_data >= COMPO_BOND_TIME_MAP_M && compo->bond_data <= COMPO_BOND_TIME_MAP_S_MID) {
                bool flag_sec = ((compo->bond_data == COMPO_BOND_TIME_MAP_S) || (compo->bond_data == COMPO_BOND_TIME_MAP_S_MID));
                s16 angle = 0;
                if (compo->bond_data == COMPO_BOND_TIME_MAP_S || compo->bond_data == COMPO_BOND_TIME_MAP_S_MID) {
                    angle = angle_s;
                } else {
                    angle = angle_m;
                }
                u16 func_clock_time_map_r_get(bool is_sec);
                u16 r = func_clock_time_map_r_get(flag_sec);
                s32 x = muls_shift16(r << 16, COS(angle + 900));
                s32 y = muls_shift16(r << 16, SIN(angle + 900));
                float fx = (float)x / 65536;
                float fy = (float)y / 65536;

                compo_picturebox_set_pos((compo_picturebox_t*)compo, GUI_SCREEN_CENTER_X + (int)fx, GUI_SCREEN_CENTER_Y + (int)fy);
            } else {
                compo_set_bonddata(compo, tm);
            }
            break;

        case COMPO_TYPE_TEXTBOX:
            compo_set_bonddata(compo, tm);                                                  //设置绑定数据
            compo_textbox_t *textbox = (compo_textbox_t *)compo;
            compo_set_roll(&textbox->roll_cb, textbox->txt, textbox->multiline);            //滚动
            break;

        case COMPO_TYPE_ANIMATION:
            compo_animation_process((compo_animation_t *)compo);
            break;

        case COMPO_TYPE_RINGS:
            {
                s16 angle = 0;
                compo_rings_t *rings = (compo_rings_t *)compo;
                if (rings->time0.hour != NULL) {
                    angle = angle_h - rings->time0.start_angle;
                    widget_image_set_rotation(rings->time0.hour, angle);
                }
                if (rings->time0.min != NULL) {
                    angle = angle_m - rings->time0.start_angle;
                    widget_image_set_rotation(rings->time0.min, angle);
                }
                if (rings->time0.sec != NULL) {
                    angle = angle_s - rings->time0.start_angle;
                    widget_image_set_rotation(rings->time0.sec, angle);
                }
                if (rings->time0.page_time != NULL && rings->time0.time_bg != NULL) {
                    widget_set_alpha(rings->time0.page_time , widget_get_alpha(rings->time0.time_bg));
                }
                if (rings->time1.hour != NULL) {
                    angle = angle_h - rings->time1.start_angle;
                    widget_image_set_rotation(rings->time1.hour, angle);
                }
                if (rings->time1.min != NULL) {
                    angle = angle_m - rings->time1.start_angle;
                    widget_image_set_rotation(rings->time1.min, angle);
                }
                if (rings->time1.sec != NULL) {
                    angle = angle_s - rings->time1.start_angle;
                    widget_image_set_rotation(rings->time1.sec, angle);
                }
                if (rings->time1.page_time != NULL && rings->time1.time_bg != NULL) {
                    widget_set_alpha(rings->time1.page_time , widget_get_alpha(rings->time1.time_bg));
                }
            }
            break;

		case COMPO_TYPE_KALEIDOSCOPE:
            {
                s16 angle = 0;
                compo_kaleidoscope_t *kale = (compo_kaleidoscope_t *)compo;
                if (kale->hour != NULL) {
                    angle = angle_h - kale->start_angle;
                    widget_image_set_rotation(kale->hour, angle);
                }
                if (kale->min != NULL) {
                    angle = angle_m - kale->start_angle;
                    widget_image_set_rotation(kale->min, angle);
                }
                if (kale->sec != NULL) {
                    angle = angle_s - kale->start_angle;
                    widget_image_set_rotation(kale->sec, angle);
                }
                if (kale->page_time != NULL) {
                    //同步时钟图标Alpha值
                    widget_set_alpha(kale->page_time , widget_get_alpha(kale->time_bg));
                }
            }
            break;

        case COMPO_TYPE_DISKLIST:
            {
                s16 angle = 0;
                compo_disklist_t *disk = (compo_disklist_t*)compo;
                if (disk->hour != NULL) {
                    angle = angle_h - disk->start_angle;
                        widget_image_set_rotation(disk->hour, angle);
                    }
                    if (disk->min != NULL) {
                        angle = angle_m - disk->start_angle;
                        widget_image_set_rotation(disk->min, angle);
                    }
                    if (disk->sec != NULL) {
                        angle = angle_s - disk->start_angle;
                        widget_image_set_rotation(disk->sec, angle);
                    }
                    if (disk->page_time != NULL && disk->time_bg != NULL) {
                        widget_set_alpha(disk->page_time, widget_get_alpha(disk->time_bg));
                }
            }
            break;

#if VIDEO_PLAY_EN
        case COMPO_TYPE_VIDEO:
            {
                if (sys_cb.flag_swithing) {		//切换中
                    break;
                }

                compo_video_t *video = (compo_video_t *)compo;
                if (video->obuf) {
                    compo_video_process(video);
                }
//                printf("*");
                if ((api_video_play_get_times() >= api_video_play_get_total_times()) || (api_video_play_sta_get() == AVI_STA_END) || (api_video_play_sta_get() == AVI_STA_STOP)) {
                    if (!compo_video_play_control(video, true)) {
                        func_cb.sta = task_stack_pop();
                    }
                }
            }
            break;
#endif // VIDEO_PLAY_EN

        case COMPO_TYPE_GIF:
            {
                if (sys_cb.flag_swithing) {		//切换中
                    break;
                }
                compo_gif_t *gif = (compo_gif_t *)compo;
                compo_gif_process(gif);
            }

        default:
            break;
        }
        compo = compo_get_next(compo);                                                      //遍历组件
    }
}

/**
 * @brief 获取按键ID
 **/
int compo_get_button_id(void)
{
    point_t pt = ctp_get_sxy();
    component_t *compo = (component_t *)compo_pool_get_top();
    while (compo != NULL) {
        // WDT_CLR();
        if (compo->type == COMPO_TYPE_BUTTON) {
            compo_button_t *btn = (compo_button_t *)compo;
//            if (widget_get_visble(btn->widget)) {
    #if BOX_GUI_ROTATE_DISP
                rect_t rect = rotate_widget_get_absolute(btn->widget);
    #else
                rect_t rect = widget_get_absolute(btn->widget);
    #endif // BOX_GUI_ROTATE_DISP
                if (abs_s(pt.x - rect.x) * 2 <= rect.wid && abs_s(pt.y - rect.y) * 2 <= rect.hei) {
                    return btn->id;
                }
            // }
        }
        compo = compo_get_next(compo);          //遍历组件
    }
    return ID_NULL;
}
