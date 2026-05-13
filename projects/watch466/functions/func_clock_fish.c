#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define FISH_SHADOW_EN              0           //影子使能

#define FISH_NEED_CREATE_CNT        1
#define MASK_DATA_FISH_CNT          0xff        //鱼数量

enum {
    COMPO_ID_FINSH = 1,     //鱼组件ID
    COMPO_ID_PLANT,         //植物组件ID
    COMPO_ID_TIME_DOT,
};

//图片资源
static const u32 tbl_body[] = {
    UI_BUF_DIALPLATE_FISH_BODY1_BIN,
    UI_BUF_DIALPLATE_FISH_BODY2_BIN,
    UI_BUF_DIALPLATE_FISH_BODY3_BIN,
    UI_BUF_DIALPLATE_FISH_BODY4_BIN,
    UI_BUF_DIALPLATE_FISH_BODY5_BIN,
};

#if FISH_SHADOW_EN
static const u32 tbl_shadow[] = {
    UI_BUF_DIALPLATE_FISH_SHADOW1_BIN,
    UI_BUF_DIALPLATE_FISH_SHADOW2_BIN,
    UI_BUF_DIALPLATE_FISH_SHADOW3_BIN,
    UI_BUF_DIALPLATE_FISH_SHADOW4_BIN,
    UI_BUF_DIALPLATE_FISH_SHADOW5_BIN,
};
#endif

u16 func_clock_preview_get_type(void);

compo_form_t* func_clock_fish_form_create(void)
{
    compo_form_t* frm = compo_form_create(true);

    //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_FISH_POOL_BIN);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_picturebox_set_size(pic, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT); //原图适配320的，这里拉伸下

    pic = compo_picturebox_create(frm, UI_BUF_DIALPLATE_FISH_PLANT_BIN);
    area_t area = widget_image_get_size(pic->img);
    compo_picturebox_set_pos(pic, area.wid >> 1, GUI_SCREEN_HEIGHT - (area.hei >> 1));  //左下角
    compo_setid(pic, COMPO_ID_PLANT);

    if (func_cb.sta == FUNC_CLOCK) {
        f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
        f_clk->user_data &= ~MASK_DATA_FISH_CNT;
    }

//    path_test(frm);
//    compo_fish_t* fish = compo_fish_create(frm, tbl_body_, 5);
//    compo_setid(fish, COMPO_ID_FINSH);

    //新建文本
    compo_textbox_t *txt = compo_textbox_create(frm, 2);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X - 50, GUI_SCREEN_CENTER_Y - 140, 300, 70);
    compo_bonddata(txt, COMPO_BOND_HOUR);

    txt = compo_textbox_create(frm, 2);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X + 50, GUI_SCREEN_CENTER_Y - 140, 300, 70);
    compo_bonddata(txt, COMPO_BOND_MINUTE);

    txt = compo_textbox_create(frm, 10);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_24_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 70, 300, 70);
    compo_bonddata(txt, COMPO_BOND_DATE);

    txt = compo_textbox_create(frm, 1);
    compo_textbox_set_font(txt, UI_BUF_0FONT_FONT_NUM_46_BIN);
    compo_textbox_set_forecolor(txt, COLOR_BLACK);
    compo_textbox_set_location(txt, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 140, 300, 70);
    compo_textbox_set(txt, ":");
    compo_setid(txt, COMPO_ID_TIME_DOT);

    return frm;
}

void func_clock_fish_process(void)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    u8 fish_cnt = f_clk->user_data & MASK_DATA_FISH_CNT;

//    path_test_process();
    static u32 ticks = 0;
    if (fish_cnt == 0 || tick_check_expire(ticks, 10000)) {
        ticks = tick_get();
        if (fish_cnt < FISH_NEED_CREATE_CNT) {
            compo_fish_t* fish;
#if FISH_SHADOW_EN
            fish = compo_fish_create(func_cb.frm_main, tbl_body, tbl_shadow, 5);
#else
            fish = compo_fish_create(func_cb.frm_main, tbl_body, NULL, 5);
#endif
            compo_fish_set_frist_rotate(fish, 600*fish_cnt);
            compo_setid(fish, COMPO_ID_FINSH + fish_cnt);
            fish_cnt++;
            f_clk->user_data = (f_clk->user_data & ~MASK_DATA_FISH_CNT) | fish_cnt;
        }

        if ((func_cb.sta == FUNC_CLOCK) && (fish_cnt == FISH_NEED_CREATE_CNT)) {
            compo_picturebox_t *pic = compo_getobj_byid(COMPO_ID_PLANT);
            widget_set_top(pic->img, true);
        }
    }

    for (int i=0; i<f_clk->user_data; i++) {
        compo_fish_t* fish = compo_getobj_byid(COMPO_ID_FINSH + i);
        compo_fish_mcb_process(fish);
    }
}

void func_clock_fish_message(size_msg_t msg)
{
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    u8 fish_cnt = f_clk->user_data & MASK_DATA_FISH_CNT;

    static bool time_visible = 0;
    switch (msg) {
    case MSG_CTP_CLICK:
        {
            point_t pt = ctp_get_sxy();
            for (int i=0; i<fish_cnt; i++) {
                compo_fish_t* fish = compo_getobj_byid(COMPO_ID_FINSH + i);
                compo_fish_mcb_message(fish, pt, COMPO_FINSH_MCB_CLICK);
            }
        }
        break;

    case MSG_CTP_SHORT_UP:
//        func_clock_sub_pullup();                //上拉菜单
        func_switch_to(FUNC_CARD, FUNC_SWITCH_MENU_PULLUP_UP | FUNC_SWITCH_DOWN_BG_BLUR);  //上拉卡片界面
        break;

    case MSG_CTP_SHORT_RIGHT:
//        func_clock_sub_side();                  //右拉边菜单
        func_switch_to(FUNC_SIDEBAR, func_get_switching_mode_byidx(sys_cb.nav_index, false));  //右滑界面
        break;

    case MSG_CTP_SHORT_DOWN:
        func_clock_sub_dropdown();              //下拉菜单
        break;

    case MSG_CTP_LONG:
        if (func_clock_preview_get_type() == PREVIEW_ROTARY_STYLE) {
            func_cb.sta = FUNC_CLOCK_PREVIEW;
        } else {
            func_switch_to(FUNC_CLOCK_PREVIEW, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);                    //切换回主时钟
        }
        break;

    case MSG_SYS_500MS:
//        printf("MSG_SYS_500MS\n");
    //秒跳动处理
        {
            compo_textbox_t *txt = compo_getobj_byid(COMPO_ID_TIME_DOT);
            compo_textbox_set_visible(txt, time_visible);
            time_visible ^= 1;
        }
        break;

    default:
        func_message(msg);
        break;
    }
}
