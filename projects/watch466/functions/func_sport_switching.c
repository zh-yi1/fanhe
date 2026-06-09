#include "include.h"
#include "func.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define SPORT_ANIM_AMP_STEP             2       //图片放大步进数
#define SPORT_ANIM_RED_STEP             8       //图片缩小步进数
#define SPORT_ANIM_DELAY                30      //图片等待时间
#define SPORT_ANIM_ENTER_HEIGHT         50      //图片入场的高度
#define SPORT_ANIM_EXIT_HEIGHT          20      //图片退出的高度


typedef struct f_sport_switching_t_ {
    compo_picturebox_t *pic;
    u8 anim_sta;
    u8 anim_num;
    u8 anim_delay;

} f_sport_switching_t;

//ID
enum {
    COMPO_ID_PIC = 1,
};

//动画状态
enum {
    SPORT_ANIM_STA_IDLE,        //空闲，获取图片信息
    SPORT_ANIM_STA_START,       //开始入场
    SPORT_ANIM_STA_DELAY,       //等待
    SPORT_ANIM_STA_NEXT,        //跳转下一张图片
    SPORT_ANIM_STA_END,         //结束
};

#define ANIM_MAX_CNT                       ((int)(sizeof(anim_addr_tbl) / sizeof(anim_addr_tbl[0])))

static const u32 anim_addr_tbl[] = {    //替换图片即可
    // UI_BUF_SPORT_EXERCISING_3_BIN,
    // UI_BUF_SPORT_EXERCISING_2_BIN,
    // UI_BUF_SPORT_EXERCISING_1_BIN,
};

//创建运动开始界面窗体
compo_form_t *func_sport_switching_form_create(void)
{
    //新建窗体和背景
    compo_form_t *frm = compo_form_create(true);

    //创建图片
    compo_picturebox_t *pic;
    for (int i=0; i<ANIM_MAX_CNT; i++) {
        pic = compo_picturebox_create(frm, anim_addr_tbl[i]);
        compo_setid(pic, COMPO_ID_PIC + i);
        compo_picturebox_set_pos(pic, GUI_SCREEN_CENTER_X, GUI_SCREEN_HEIGHT);
        compo_picturebox_set_visible(pic, false);
    }

    return frm;
}

static void func_sport_animation(void)
{
    f_sport_switching_t *f_sport_switching = (f_sport_switching_t *)func_cb.f_cb;

    static area_t area_img;
    static rect_t rect_pic;

    static u32 ticks;
    if(tick_check_expire(ticks, 10)) {
        ticks = tick_get();

        switch (f_sport_switching->anim_sta) {
            case SPORT_ANIM_STA_IDLE:
                f_sport_switching->pic = compo_getobj_byid(COMPO_ID_PIC + f_sport_switching->anim_num);
                area_img = gui_image_get_size(anim_addr_tbl[f_sport_switching->anim_num]);
                rect_pic = compo_picturebox_get_location(f_sport_switching->pic);
                rect_pic.hei = SPORT_ANIM_ENTER_HEIGHT;
                rect_pic.wid = rect_pic.hei * area_img.wid / area_img.hei;
                compo_picturebox_set_size(f_sport_switching->pic, rect_pic.wid, rect_pic.hei);
                compo_picturebox_set_visible(f_sport_switching->pic, true);

//                printf("img-->w:%d, h:%d\n", area_img.wid, area_img.hei);
//                printf("rect-->x:%d, y:%d, w:%d, h:%d\n", rect_pic.x, rect_pic.y, rect_pic.wid, rect_pic.hei);
                f_sport_switching->anim_sta = SPORT_ANIM_STA_START;
                break;

            case SPORT_ANIM_STA_START:  //放大
                rect_pic.hei += SPORT_ANIM_AMP_STEP;
                rect_pic.wid = rect_pic.hei * area_img.wid / area_img.hei;
                rect_pic.y -=  GUI_SCREEN_CENTER_Y * SPORT_ANIM_AMP_STEP / abs(SPORT_ANIM_ENTER_HEIGHT - area_img.hei);

                compo_picturebox_set_pos(f_sport_switching->pic, rect_pic.x, rect_pic.y);
                compo_picturebox_set_size(f_sport_switching->pic, rect_pic.wid, rect_pic.hei);
//                printf("y:%d, w:%d, h:%d\n", rect_pic.y, rect_pic.wid, rect_pic.hei);
                if (rect_pic.y < GUI_SCREEN_CENTER_Y) {
                    rect_pic.hei = area_img.hei;
                    rect_pic.wid = area_img.wid;
                    rect_pic.x = GUI_SCREEN_CENTER_X;
                    rect_pic.y = GUI_SCREEN_CENTER_Y;

                    compo_picturebox_set_pos(f_sport_switching->pic, rect_pic.x, rect_pic.y);
                    compo_picturebox_set_size(f_sport_switching->pic, rect_pic.wid, rect_pic.hei);
                    f_sport_switching->anim_sta = SPORT_ANIM_STA_DELAY;
                }
                break;

            case SPORT_ANIM_STA_DELAY:
                if (++f_sport_switching->anim_delay > SPORT_ANIM_DELAY) {
                    f_sport_switching->anim_sta = SPORT_ANIM_STA_NEXT;
                }
                break;

            case SPORT_ANIM_STA_NEXT:   //缩小
                rect_pic.hei -= SPORT_ANIM_RED_STEP;
                rect_pic.wid = rect_pic.hei * area_img.wid / area_img.hei;
                rect_pic.y -=  GUI_SCREEN_CENTER_Y * SPORT_ANIM_RED_STEP / abs(SPORT_ANIM_EXIT_HEIGHT - area_img.hei);
//                printf("y:%d, w:%d, h:%d\n", rect_pic.y, rect_pic.wid, rect_pic.hei);

                compo_picturebox_set_pos(f_sport_switching->pic, rect_pic.x, rect_pic.y);
                compo_picturebox_set_size(f_sport_switching->pic, rect_pic.wid, rect_pic.hei);

                if (rect_pic.y < 0) {
                    if (++f_sport_switching->anim_num >= ANIM_MAX_CNT) {
                        f_sport_switching->anim_sta = SPORT_ANIM_STA_END;
                        break;
                    }
                    f_sport_switching->anim_delay = 0;
                    compo_picturebox_set_visible(f_sport_switching->pic, false);
                    f_sport_switching->anim_sta = SPORT_ANIM_STA_IDLE;
                }
                break;

            case SPORT_ANIM_STA_END:
                func_cb.sta = FUNC_SPORT_SUB_RUN;
                task_stack_pop();
                break;
        }
    }
}


//运动开始功能事件处理
static void func_sport_switching_process(void)
{
    func_sport_animation();
    func_process();
}

//运动开始功能消息处理
static void func_sport_switching_message(size_msg_t msg)
{
    switch (msg) {

    default:
//        func_message(msg);
        break;
    }
}

//进入运动开始功能
void func_sport_switching_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_sport_switching_t));
    func_cb.frm_main = func_sport_switching_form_create();
}

//退出运动开始功能
void func_sport_switching_exit(void)
{
    func_cb.last = FUNC_SPORT_SWITCH;
}

//运动开始功能
void func_sport_switching(void)
{
    printf("%s\n", __func__);
    func_sport_switching_enter();
    while (func_cb.sta == FUNC_SPORT_SWITCH) {
        func_sport_switching_process();
        func_sport_switching_message(msg_dequeue());
    }
    func_sport_switching_exit();
}
