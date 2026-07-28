#ifndef _GENERAL_TITLE_UI_H
#define _GENERAL_TITLE_UI_H

/* 通用顶部标题栏：页面标题、蓝牙、电量
 * 与 general_ui（左上角时间）相对：左上角显示页面标题。
 *
 * 使用方式：
 *   1. 在 xxx_form_create() 中调用 general_title_bar_create(frm, &bar, title)
 *   2. 在 xxx_enter() 中调用 general_title_bar_attach()
 *   3. 在 xxx_exit() 中调用 general_title_bar_detach()
 *
 * func_process() 内部已调用 home_ui_shared_ble_status_poll() 和
 * home_ui_shared_battery_chg_poll()，页面无需重复调用。
 */

#define GENERAL_TB_Y                    20
#define GENERAL_TB_LEFT_MARGIN          12
#define GENERAL_TB_RIGHT_MARGIN         10
#define GENERAL_TB_ICON_GAP             6

/* composet ID，与 general_status_bar 错开 */
enum {
    GENERAL_TB_ID_TXT_TITLE = 0x210,
    GENERAL_TB_ID_PIC_BT,
    GENERAL_TB_ID_PIC_BAT,
};

typedef struct
{
    compo_textbox_t *txt_title;
    compo_picturebox_t *pic_bt;
    compo_picturebox_t *pic_bat;
} general_title_bar_t;

/**
 * 在 form 上创建标题栏所有控件。
 * @param frm    所属 form
 * @param bar    标题栏实例（调用方分配）
 * @param title  页面标题（i18n 或字面量，NULL 则不创建标题文本）
 */
void general_title_bar_create(compo_form_t *frm, general_title_bar_t *bar, const char *title);

/**
 * 运行时更新标题文字。
 */
void general_title_bar_set_title(general_title_bar_t *bar, const char *title);

/**
 * 绑定控件指针（form 重建后调用，如 resume/wakeup 场景）。
 */
void general_title_bar_bind(general_title_bar_t *bar);

/**
 * 页面 enter 时调用：绑定蓝牙/电量图标。
 */
void general_title_bar_attach(general_title_bar_t *bar);

/**
 * 页面 exit 时调用：解绑电池 pic。
 */
void general_title_bar_detach(void);

#endif
