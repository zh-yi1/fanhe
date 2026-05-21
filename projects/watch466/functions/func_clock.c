#include "include.h"
#include "func.h"
#include "func_clock.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define DIALPLATE_NUM               (sizeof(dialplate_info) / sizeof(u32))
#define DIALPLATE_BTF_IDX           DIALPLATE_NUM - 1        //蝴蝶表盘默认最后一个
#define DIALPLATE_CUBE_IDX          DIALPLATE_NUM - 2        //立方体表盘默认倒数第二个
#define DIALPLATE_FISH_IDX          DIALPLATE_NUM - 3       //倒数第三个
#define DIALPLATE_WINDMILL_IDX      DIALPLATE_NUM - 4       //倒数第三个

const u32 dialplate_info[] = {


//    UI_BUF_DIALPLATE_2_BIN,
//    UI_BUF_DIALPLATE_5_BIN,
//    UI_BUF_DIALPLATE_4_BIN,
//    UI_BUF_DIALPLATE_7_BIN,
//     UI_BUF_DIALPLATE_8_BIN,                //精准时图

    
    UI_BUF_DIALPLATE_AVI_1_BIN,

    UI_BUF_DIALPLATE_1_BIN,
    UI_BUF_DIALPLATE_WINDMILL_BIN,
    UI_BUF_DIALPLATE_FISH_BIN,
    UI_BUF_DIALPLATE_CUBE_BIN,
    UI_BUF_DIALPLATE_BTF_BIN,
};

//表盘快捷按钮编号表
const u8 quick_btn_tbl[] =
{
    FUNC_NULL,
    FUNC_HEARTRATE,
    FUNC_BT,
    FUNC_ALARM_CLOCK,
    FUNC_BLOOD_OXYGEN,
    FUNC_BLOODSUGAR,
    FUNC_BLOOD_PRESSURE,
    FUNC_BREATHE,
    FUNC_CALCULATOR,

    FUNC_CAMERA,
    FUNC_TIMER,
    FUNC_SLEEP,
    FUNC_STOPWATCH,
    FUNC_WEATHER,
    FUNC_GAME,
    FUNC_STYLE,
    FUNC_ALTITUDE,
    FUNC_MAP,
    FUNC_MESSAGE,
    FUNC_SCAN,
    FUNC_VOICE,
#if SECURITY_PAY_EN
    FUNC_ALIPAY,
#else
    FUNC_NULL,
#endif // SECURITY_PAY_EN
    FUNC_COMPASS,
    FUNC_ADDRESS_BOOK,
    FUNC_SPORT,
    FUNC_CALL,
    FUNC_FINDPHONE,
    FUNC_CALENDAER,
    FUNC_ACTIVITY,
    FUNC_FLASHLIGHT,
    FUNC_SETTING,
};

int compo_get_animation_id(void);
void compo_animation_manual_next(compo_animation_t *animation);
void func_switch_to(u8 sta, u16 switch_mode);
compo_form_t *func_clock_cube_form_create(void);
compo_form_t *func_clock_butterfly_form_create(void);
compo_form_t *func_clock_windmill_form_create(void);
compo_form_t *func_clock_fish_form_create(void);
compo_form_t *func_clock_hourglass_form_create(void);
compo_form_t *func_clock_compass_form_create(void);
void func_clock_cube_message(size_msg_t msg);
void func_clock_butterfly_message(size_msg_t msg);
void func_clock_windmill_message(size_msg_t msg);
void func_clock_fish_message(size_msg_t msg);
void func_clock_hourglass_message(size_msg_t msg);
void func_clock_compass_message(size_msg_t msg);
void func_clock_cube_process(void);
void func_clock_butterfly_process(void);
void func_clock_windmill_process(void);
void func_clock_fish_process(void);
void func_clock_hourglass_process(void);
void func_clock_windmill_pbubbles_destory(void);
void func_clock_compass_process(void);

u8 func_clock_compass_init();
u16 func_clock_preview_get_type(void);

u16 func_clock_time_map_r_get(bool is_sec)
{
    return 0;
}

void func_clock_time_map_r_set(bool is_sec, u16 value)
{

}


u8 func_clock_get_max_dialplate_num(void)
{
    return (sizeof(dialplate_info) / sizeof(u32));
}

u32 func_clock_get_dialplate_info(u8 index)
{
    return dialplate_info[index];
}

u32 func_clock_get_dialplate_cube_idx(void)
{
    return DIALPLATE_CUBE_IDX;
}

u32 func_clock_get_dialplate_butterfly_idx(void)
{
    return DIALPLATE_BTF_IDX;
}

compo_form_t *func_clock_form_create(void)
{
    compo_form_t *frm;

	if (sys_cb.dialplate_index == DIALPLATE_FISH_IDX) {
        frm = func_clock_fish_form_create();
    } else if (sys_cb.dialplate_index == DIALPLATE_BTF_IDX) {
        frm = func_clock_butterfly_form_create();
    } else if (sys_cb.dialplate_index == DIALPLATE_CUBE_IDX) {
        frm = func_clock_cube_form_create();
    } else if (sys_cb.dialplate_index == DIALPLATE_WINDMILL_IDX) {
        frm = func_clock_windmill_form_create();
    } else {
        u32 base_addr = dialplate_info[sys_cb.dialplate_index];
        u16 compo_num = bsp_uitool_header_phrase(base_addr);
        if (!compo_num) {
            halt(HALT_GUI_DIALPLATE_HEAD);
        }

        frm = compo_form_create(true);
        bsp_uitool_create(frm, base_addr, compo_num);
        return frm;
    }
    return frm;
}

#if GUI_USE_BLUR
//截图
compo_form_t *func_clock_form_create_by_screenshoot(void)
{
    compo_form_t *frm = compo_form_create(true);
        //新建图像
    compo_picturebox_t *pic = compo_picturebox_create(frm, 0);
    compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    gui_set_ram_check(cur_scbuf, __func__);      //检测ram对不对
    compo_picturebox_set_ram(pic, cur_scbuf);
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    f_clk->blur_pic = pic;

    return frm;
}
#endif

//单击按钮
static void func_clock_button_click(void)
{
    u16 btn_id = compo_get_button_id();
    u16 animation_id = compo_get_animation_id();
    if (btn_id) {
        func_switch_to(quick_btn_tbl[btn_id], func_get_switching_mode_byidx(sys_cb.nav_index, false) | FUNC_SWITCH_AUTO);
    } else if (animation_id) {
        compo_animation_t *animation = compo_getobj_byid(animation_id);
        if (animation->bond_data == COMPO_BOND_IMAGE_CLICK) {
            compo_animation_manual_next(animation);
        } else if (animation->bond_data == COMPO_BOND_ANIMATION_AREA_CLICK) {
            compo_animation_click_set_vis(animation);
        }
    }

}


//子功能公共事件处理
void func_clock_sub_process(void)
{
    func_process();                                     //刷新UI
}

//子功能公共消息处理
void func_clock_sub_message(size_msg_t msg)
{
    func_message(msg);
}

void func_clock_swipe_up_to_football_menu(void)
{
    point_t pt = ctp_get_sxy();
    if (pt.y >= FUNC_CLOCK_SWIPE_UP_BOTTOM_ZONE) {
        func_switch_to_football_menu();
    }
}

//时钟表盘功能事件处理
static void func_clock_process(void)
{
    if (sys_cb.dialplate_index == DIALPLATE_FISH_IDX) {
        // printf(">>>>>>>>>>>>>\n");
        func_clock_fish_process();
    } else if (sys_cb.dialplate_index == DIALPLATE_CUBE_IDX) {
        func_clock_cube_process();
    } else if (sys_cb.dialplate_index == DIALPLATE_BTF_IDX) {
        func_clock_butterfly_process();
    } else if (sys_cb.dialplate_index == DIALPLATE_WINDMILL_IDX) {
        func_clock_windmill_process();
    }

    func_process();                                  //刷新UI

#if VIDEO_PLAY_EN
    f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    if (video && !f_clk->flag_video_start && tick_check_expire(f_clk->tick_video_start, 100)) {
        printf("video clock create\n");
        if (video) {
    //        bsp_video_play_init(GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT, video->res_addr);
            compo_video_play(video, NULL);
            f_clk->flag_video_start = true;
            /* obuf 由解码异步填充；勿在 obuf==NULL 时调 compo_video_play_control(true)：
             * 旧 compo_video_play_control 曾 do{}while(obuf==NULL) 死等导致 WDT，且 next==1 会误改 file_num。 */
        }
    }
#endif // VIDEO_PLAY_EN
}

static void func_clock_message_nomal(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_LONG_UP:
        func_clock_swipe_up_to_football_menu();         //自底部上长滑进入足球菜单
        break;

    case MSG_CTP_SHORT_RIGHT:
        func_switch_to(FUNC_SIDEBAR, func_get_switching_mode_byidx(sys_cb.nav_index, false));  //右滑界面
        break;

    case MSG_CTP_SHORT_DOWN:
        func_clock_sub_dropdown();              //下拉菜单
        break;

    case MSG_CTP_CLICK:
        func_clock_button_click();
        break;

    case MSG_CTP_LONG:
        if (func_clock_preview_get_type() == PREVIEW_ROTARY_STYLE) {
            func_cb.sta = FUNC_CLOCK_PREVIEW;
        } else {
            func_switch_to(FUNC_CLOCK_PREVIEW, FUNC_SWITCH_ZOOM_FADE_ENTER | FUNC_SWITCH_AUTO);                    //切换回主时钟
        }
        break;

    default:
        func_message(msg);
        break;
    }
}

//时钟表盘功能消息处理
static void func_clock_message(size_msg_t msg)
{

    switch (sys_cb.dialplate_index) {

    case DIALPLATE_CUBE_IDX:
        func_clock_cube_message(msg);
        break;

    case DIALPLATE_BTF_IDX:
        func_clock_butterfly_message(msg);
        break;

    case DIALPLATE_FISH_IDX:
        func_clock_fish_message(msg);
        break;

    case DIALPLATE_WINDMILL_IDX:
        func_clock_windmill_message(msg);
        break;

    default:
        func_clock_message_nomal(msg);
        break;
    }

}


//进入时钟表盘功能
void func_clock_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_clock_t));
    {
        f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
        f_clk->sta = FUNC_CLOCK_MAIN;
    }
    func_cb.frm_main = func_clock_form_create();

#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    if (video) {
        f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
        f_clk->tick_video_start = tick_get();
        f_clk->flag_video_start = false;
    }

#endif // VIDEO_PLAY_EN

}

//退出时钟表盘功能
void func_clock_exit(void)
{
    if (sys_cb.dialplate_index == DIALPLATE_BTF_IDX) {
        tft_set_temode(DEFAULT_TE_MODE);
    } else if(sys_cb.dialplate_index == DIALPLATE_WINDMILL_IDX) {
        func_clock_windmill_pbubbles_destory();
    }

    //tft_set_baud(3, 4);
    func_cb.last = FUNC_CLOCK;

#if VIDEO_PLAY_EN
    compo_video_t *video = compo_getobj_bytype(COMPO_TYPE_VIDEO);
    if (video) {
        f_clock_t *f_clk = (f_clock_t *)func_cb.f_cb;
        compo_video_exit(video);
        f_clk->tick_video_start = 0;
        f_clk->flag_video_start = false;
    }
#endif // VIDEO_PLAY_EN

}

//时钟表盘功能
void func_clock(void)
{
    printf("%s\n", __func__);
//sys_cb.dialplate_index = DIALPLATE_CUBE_IDX;
    func_clock_enter();
    while (func_cb.sta == FUNC_CLOCK) {
        func_clock_process();
        func_clock_message(msg_dequeue());
    }
    func_clock_exit();
}
