#include "include.h"
#include "../gui/components/compo_form.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

static u8 animation_id = 0;

/* 手表表盘多语言表已随 i18n 精简删除，饭盒工程不再使用 */
#if 0
//表盘文本编号表
const u8 text_str_tbl[] =
{
    [1] = STR_CALORIE,
    STR_STEPS,
    STR_DISTANCE,
    STR_HEART_RATE,
    STR_BLOOD_OXYGEN,
    STR_BLOOD_SUGAR,
    STR_WEATHER,
    STR_BAROMETER,
    STR_TEMPERATURE,
//    STR_CELSIUS,
    STR_ALTITUDE,
    STR_SPORTS,
    STR_YEAR,
    STR_MON,
    STR_MDAY,
    STR_HOUR,
    STR_MIN,
    STR_SEC,
};
#endif

void compo_set_bonddata(component_t *compo, tm_t tm);

//时针
void bsp_uitool_pointer_create(compo_form_t *frm, uitool_res_t *uitool_res, u32 res_addr)
{
    TRACE("UITOOL_TYPE_POINTER:%d, x:%d, y:%d, angle:%d\n", uitool_res->bond_type, uitool_res->x, uitool_res->y, uitool_res->param2);
    s16 pivot_x = uitool_res->param1 & 0xff;
    s16 pivot_y = uitool_res->param1 >> 16;
    s16 start_angle = uitool_res->param2 & 0xffff;
    bool anticlockwise = (uitool_res->param2 >> 16) & 0x01;
    compo_datetime_t *pointer = compo_datetime_create(frm, res_addr);
    compo_bonddata(pointer, uitool_res->bond_type);
    compo_datetime_set_pos(pointer, uitool_res->x, uitool_res->y);
//    compo_datetime_set_pos(pointer, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_datetime_set_center(pointer, pivot_x, pivot_y);
    compo_datetime_set_start_angle(pointer, start_angle);
    compo_datetime_set_anticlockwise(pointer, anticlockwise);
}

//图像
void bsp_uitool_image_create(compo_form_t *frm, uitool_res_t *uitool_res, u32 res_addr)
{
    u8 interval   = BYTE2(uitool_res->param1);
    s16 click_wid = (BYTE0(uitool_res->param2) << 8) | BYTE3(uitool_res->param1);
    s16 click_hei = (BYTE2(uitool_res->param2) << 8) | BYTE1(uitool_res->param2);

    TRACE("UITOOL_TYPE_IMAGE:%d, res_addr:%x, x:%d, y:%d\n", uitool_res->bond_type, uitoolres->res_addr, uitool_res->x, uitool_res->y);

    switch (uitool_res->bond_type) {
        case COMPO_BOND_IMAGE_STATIC:
            compo_form_add_image(frm, res_addr, uitool_res->x, uitool_res->y);
            break;

        case COMPO_BOND_IMAGE_CLICK:
            {
                compo_animation_t *animation = compo_animation_create(frm, res_addr);
                compo_animation_set_click_area(animation, uitool_res->x, uitool_res->y, click_wid, click_hei);
                compo_animation_set_pos(animation, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
                compo_animation_set_radix(animation, uitool_res->res_num);
                compo_animation_set_interval(animation, 0);
                compo_bonddata(animation, uitool_res->bond_type);
                compo_setid(animation, ++animation_id);

            }
            break;

        case COMPO_BOND_ANIMATION_STATIC:
            {
                compo_animation_t *animation = compo_animation_create(frm, res_addr);
                compo_animation_set_pos(animation, uitool_res->x, uitool_res->y);
                compo_animation_set_radix(animation, uitool_res->res_num);
                compo_animation_set_interval(animation, interval);
                compo_bonddata(animation, uitool_res->bond_type);
            }
            break;

        case COMPO_BOND_ANIMATION_AREA_CLICK:
            {
                for (u8 i = 0; i < COMPO_ANIMATION_CLICK_NUM; i++) {
                    compo_animation_t *animation = compo_animation_create(frm, res_addr);
                    compo_animation_set_click_area(animation, uitool_res->x, uitool_res->y, click_wid, click_hei);
                    compo_animation_set_pos(animation, 0, 0);
                    compo_animation_set_radix(animation, uitool_res->res_num);
                    compo_animation_set_interval(animation, interval);
                    compo_animation_set_visible(animation, false);
                    compo_bonddata(animation, uitool_res->bond_type);
                    compo_setid(animation, ++animation_id);
                }
            }
            break;

        case COMPO_BOND_ANIMATION_CLICK:
            {

            }
            break;

        case COMPO_BOND_IMAGE_WEATHER:
        case COMPO_BOND_BLE_STA:
        case COMPO_BOND_BT_STA:
        case COMPO_BOND_BTBLE_STA:
        case COMPO_BOND_DISTANCE_PROGRESS:
        case COMPO_BOND_KCAL_PROGRESS:
        case COMPO_BOND_STEPS_PROGRESS:
        case COMPO_BOND_VBAT_PROGRESS:
            {
                compo_picturebox_t *pic;
                pic = compo_picturebox_create(frm, res_addr);
                compo_picturebox_cut(pic, 0, uitool_res->res_num); //默认第1张图
                compo_picturebox_set_pos(pic, uitool_res->x, uitool_res->y);
                compo_bonddata(pic, uitool_res->bond_type);
                printf("type[%d]\n", uitool_res->bond_type);
            }
            break;

        case COMPO_BOND_TMIE_MAP_BG_M:
        case COMPO_BOND_TMIE_MAP_BG_S:
        case COMPO_BOND_TIME_MAP_M:
        case COMPO_BOND_TIME_MAP_S:
        case COMPO_BOND_TIME_MAP_M_MID:
        case COMPO_BOND_TIME_MAP_S_MID:
        case COMPO_BOND_TIME_MAP_LEFT_UP:
        case COMPO_BOND_TIME_MAP_RIGHT_UP:
        case COMPO_BOND_TIME_MAP_LEFT_DOWN:
        case COMPO_BOND_TIME_MAP_RIGHT_DOWN:
            {

                compo_picturebox_t *pic = compo_picturebox_create(frm, res_addr);
                compo_picturebox_set_pos(pic, uitool_res->x, uitool_res->y);
                compo_bonddata(pic, uitool_res->bond_type);
                compo_setid(pic, uitool_res->bond_type);
                if (uitool_res->bond_type >= COMPO_BOND_TIME_MAP_M && uitool_res->bond_type <= COMPO_BOND_TIME_MAP_S_MID) {
                    compo_picturebox_set_visible(pic, false);
                }
            }
            break;

        default:
            break;
    }
}

//文本
void bsp_uitool_text_create(compo_form_t *frm, uitool_res_t *uitool_res, u32 res_addr)
{
    u8 index = BYTE0(uitool_res->param1);
    TRACE("UITOOL_TYPE_TEXT:%d\n", index);
    compo_textbox_t *txt = compo_textbox_create(frm, 10);
    compo_textbox_set_location(txt, uitool_res->x, uitool_res->y, 80, 40);
#if 0
    /* 依赖已删除的手表 STR_* / text_str_tbl */
    compo_textbox_set(txt, i18n[text_str_tbl[index]]);
#else
    (void)index;
    (void)res_addr;
    compo_textbox_set(txt, "");
#endif
}

//图片/字库数字
void bsp_uitool_num_create(compo_form_t *frm, uitool_res_t *uitool_res, u32 res_addr)
{
    bool use_num = uitool_res->param1 & BIT(0);
    u8 max_cnt = 0;
    u8 bond_compo_type = COMPO_TYPE_NONE;
    TRACE("UITOOL_TYPE_NUM:%d, x:%d, y:%d, res_num:%d\n", uitool_res->bond_type, uitool_res->x, uitool_res->y, uitool_res->res_num);
    if (use_num) {
        switch (uitool_res->bond_type) {
            case COMPO_BOND_HOUR_H:
            case COMPO_BOND_MINUTE_H:
            case COMPO_BOND_HOUR_L:
            case COMPO_BOND_MINUTE_L:
            case COMPO_BOND_WEEKDAY:
                bond_compo_type = COMPO_TYPE_NUMBER;
                max_cnt = 1;
                break;

            case COMPO_BOND_TEMPERATURE_UNIT:
                bond_compo_type = COMPO_TYPE_PICTUREBOX;
                break;

            case COMPO_BOND_DISTANCE_UNIT:
                bond_compo_type = COMPO_TYPE_PICTUREBOX;
                break;

            case COMPO_BOND_YEAD:
                bond_compo_type = COMPO_TYPE_NUMBER;
                max_cnt = 4;
                break;

            default:
                bond_compo_type = COMPO_TYPE_NUMBER;
                max_cnt = 2;
                break;
        }

        if (bond_compo_type == COMPO_TYPE_NUMBER) {
            //数字
            bool minus = uitool_res->param1 & BIT(1);
            bool num_part_en = uitool_res->param1 & BIT(2);
            s16 delt_x = num_part_en ? ((uitool_res->param1 >> 8) & 0xffff) : 0;
            s16 delt_y = num_part_en ? ((uitool_res->param1 >> 24) & 0xffff) : 0;
            u8 cnt = num_part_en ? 5 : 1;
            max_cnt = num_part_en ? 1 : max_cnt;
            compo_number_t *num = NULL;
            for(u8 i=0;i<cnt;i++) {
                num = compo_number_create(frm, res_addr, max_cnt);
                compo_number_set_radix(num, uitool_res->res_num, minus);
                if (num != NULL) {
                    num->num_part = num_part_en ? i + 1 : 0;
                }
                //个位中心坐标
                compo_number_set_pos(num, uitool_res->x - delt_x * i, uitool_res->y + delt_y * i);
                compo_number_set_zfill(num, true);
                //compo_number_set_visible(num, false);
                compo_bonddata(num, uitool_res->bond_type);
                compo_set_bonddata((component_t *)num, time_to_tm(compo_cb.rtc_cnt));
            }
        } else if (bond_compo_type == COMPO_TYPE_PICTUREBOX) {
            //图像
            compo_picturebox_t *pic;
            pic = compo_picturebox_create(frm, res_addr);
            compo_picturebox_cut(pic, 0, uitool_res->res_num); //默认第1张图
            compo_picturebox_set_pos(pic, uitool_res->x, uitool_res->y);
            compo_bonddata(pic, uitool_res->bond_type);
        }
    } else {
        max_cnt = 2;
        compo_textbox_t *txt = compo_textbox_create(frm, max_cnt);
        compo_textbox_set_font(txt, res_addr);
        compo_textbox_set_location(txt, uitool_res->x, uitool_res->y, 80, widget_text_get_max_height()*2);
//        compo_textbox_set_visible(txt, false);
        compo_bonddata(txt, uitool_res->bond_type);
    }
}

//区域
void bsp_uitool_area_create(compo_form_t *frm, uitool_res_t *uitool_res, u32 res_addr)
{
    u8 index = uitool_res->param2 & 0xff;
    s16 wid = uitool_res->param1 & 0xff;
    s16 hei = uitool_res->param1 >> 16;
    TRACE("UITOOL_TYPE_AREA:%d, %d, %d\n", index, wid, hei);
    compo_button_t *btn;
    btn = compo_button_create(frm);
    compo_setid(btn, index);
    compo_button_set_location(btn, uitool_res->x, uitool_res->y, wid, hei);
}

//图像
void bsp_uitool_video_create(compo_form_t *frm, uitool_res_t *uitool_res, u32 res_addr)
{
    s16 wid = (BYTE0(uitool_res->param2) << 8) | BYTE3(uitool_res->param1);
    s16 hei = (BYTE2(uitool_res->param2) << 8) | BYTE1(uitool_res->param2);

    printf("UITOOL_TYPE_VIDEO:%d, res_addr:%x, x:%d, y:%d\n", uitool_res->bond_type, uitool_res->res_addr, wid, hei);

#if VIDEO_PLAY_EN
    u32 video_cover_size = (BYTE2(uitool_res->param1) << 16) | (BYTE1(uitool_res->param1) << 8) | BYTE0(uitool_res->param1);
    compo_video_t *video = compo_video_create(frm, res_addr + video_cover_size);
    if (wid == hei) {
        compo_video_set_size(video, GUI_SCREEN_WIDTH, GUI_SCREEN_WIDTH);
    } else {
        compo_video_set_size(video, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
    }
    compo_video_set_pos(video, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    widget_set_visible(video->img, false);
    video->cover = compo_picturebox_create(frm, res_addr);
    compo_picturebox_set_pos(video->cover, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
#endif
}


u16 bsp_uitool_header_phrase(u32 base_addr)
{
    uitool_header_t uitool_header;

    os_spiflash_read(&uitool_header, base_addr, UITOOL_HEADER);
    TRACE("sig:%x, ver:%d, size:%x, num:%x\n", uitool_header.sig, uitool_header.ver, uitool_header.size, uitool_header.num);
    TRACE("user id:%d, index:%d, wid:%d, hei:%d\n", uitool_header.user_id, uitool_header.index, uitool_header.wid, uitool_header.hei);
//    print_r(uitool_header.user_extend, 32);
	if (uitool_header.sig != UITOOL_HEADER_FORMAT) {
		printf("UITOOL Format Uncorrect:%x, %x\n", uitool_header.sig, UITOOL_HEADER_FORMAT);
		return false;
	}
	return uitool_header.num;
}

void bsp_uitool_create(compo_form_t *frm, u32 base_addr, u16 compo_num)
{
    uitool_res_t uitool_res;
	memset(&uitool_res, 0, sizeof(uitool_res_t));
	animation_id = 0;

    for(u16 i=0;i<compo_num;i++) {
        os_spiflash_read(&uitool_res, base_addr + UITOOL_HEADER + i * UITOOL_RES_HEADER, UITOOL_RES_HEADER);
        u32 res_addr = base_addr + uitool_res.res_addr;
         switch (uitool_res.res_type) {
            case UITOOL_TYPE_POINTER:
                bsp_uitool_pointer_create(frm, &uitool_res, res_addr);
                break;

            case UITOOL_TYPE_IMAGE:
                bsp_uitool_image_create(frm, &uitool_res, res_addr);
                break;

            case UITOOL_TYPE_TEXT:
                bsp_uitool_text_create(frm, &uitool_res, res_addr);
                break;

            case UITOOL_TYPE_NUM:
                bsp_uitool_num_create(frm, &uitool_res, res_addr);
                break;

            case UITOOL_TYPE_AREA:
                bsp_uitool_area_create(frm, &uitool_res, res_addr);
                break;

            case UITOOL_TYPE_VIDEO:
                bsp_uitool_video_create(frm, &uitool_res, res_addr);
                break;

            default:
                TRACE("HALT_GUI_DIALPLATE_TYPE:%d\n", uitool_res.res_type);
                halt(HALT_GUI_DIALPLATE_TYPE);
                break;
            }
    }
}

