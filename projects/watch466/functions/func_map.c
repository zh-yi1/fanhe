#include "include.h"
#include "func.h"
#include "awk_pr.h"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

void awk_map_tile_init(compo_form_t *frm);
typedef struct f_map_t_ {
    bool awk_is_touch_move;
} f_map_t;

//创建地图窗体
compo_form_t *func_map_form_create(void)
{
    //新建窗体
    compo_form_t *frm = compo_form_create(true);

    //设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, i18n[STR_MAP]);

	//创建按键
    compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_ICON_MAP_BIN);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, 200);

#if AWK_MAP_EN
    awk_map_tile_init(frm);
#endif
    return frm;
}

//地图功能事件处理
static void func_map_process(void)
{
    func_process();

#if AWK_MAP_EN
    f_map_t *f_cb = func_cb.f_cb;
    if (f_cb->awk_is_touch_move) {
        s32 sx, sy, x, y;

        static s32 x_pr = 0, y_pr = 0;
        if (ctp_get_cur_point(&sx, &sy, &x, &y)) {
            if ((x_pr != x) || (y_pr != y)) {
                // printf("%d %d\n", awk_tp_vx2sx(x), awk_tp_vy2sy(y));
                x_pr = x;
                y_pr = y;
                awk_map_touch_update(awk_get_map_id(),
                                     awk_tp_vx2sx(x),
                                     awk_tp_vy2sy(y));
            }
        } else {
            f_cb->awk_is_touch_move = false;
            awk_map_touch_end(awk_get_map_id(),
                              awk_tp_vx2sx(x),
                              awk_tp_vy2sy(y));
        }
    }
#endif
}

//地图功能消息处理
static void func_map_message(size_msg_t msg)
{
#if AWK_MAP_EN
    f_map_t *f_cb = func_cb.f_cb;
#endif
    switch (msg) {
#if AWK_MAP_EN
    case MSG_CTP_TOUCH:
        f_cb->awk_is_touch_move = true;
        awk_map_touch_begin(awk_get_map_id(), awk_tp_vx2sx(ctp_get_sxy().x), awk_tp_vy2sy(ctp_get_sxy().y));
        break;
    case MSG_CTP_CLICK:
        break;
    case MSG_CTP_SHORT_UP:
        break;
    case MSG_CTP_SHORT_DOWN:
        break;
    case MSG_CTP_LONG:
        break;
    case MSG_CTP_SHORT_LEFT:
        break;
    case MSG_CTP_SHORT_RIGHT:
        break;
    case MSG_QDEC_FORWARD:
        awk_map_set_zoom(1);
        break;
    case MSG_QDEC_BACKWARD:
        awk_map_set_zoom(0);
        break;
#endif
    default:
        func_message(msg);
        break;
    }
}

//进入地图功能
void func_map_enter(void)
{
    func_cb.f_cb = func_zalloc(sizeof(f_map_t));
    func_cb.frm_main = func_map_form_create();

#if AWK_MAP_EN
    bt_audio_bypass();
    awk_map_init();
#endif
}

//退出地图功能
void func_map_exit(void)
{
    func_cb.last = FUNC_MAP;
#if AWK_MAP_EN
    awk_map_uninit();
    bt_audio_enable();
#endif
}

//地图功能
void func_map(void)
{
    printf("%s\n", __func__);
    func_map_enter();
    while (func_cb.sta == FUNC_MAP) {
#if AWK_MAP_EN
        awk_map_flush();
#endif
        func_map_process();
        func_map_message(msg_dequeue());
    }
    func_map_exit();
}
