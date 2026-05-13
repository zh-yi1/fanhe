#ifndef _BSP_HALT_H
#define _BSP_HALT_H

//HALT 定义 APP 0 - 0x7FFF
#define HALT_USER                               0           //用户自定义的HALT范围在0~0xFFF,请在Plugin/user_halt.h进行定义
#define HALT_FUNC                               0x1000
#define HALT_BSP                                0x4000
#define HALT_GUI                                0x5000

//HALT_FUNC
#define HALT_FUNC_HEAP                          (HALT_FUNC | 1)
#define HALT_FUNC_SORT                          (HALT_FUNC | 2)
#define HALT_FUNC_SWITCH                        (HALT_FUNC | 0x10)
#define HALT_FUNC_SWITCH_MODE                   (HALT_FUNC_SWITCH | 1)
#define HALT_FUNC_SWITCH_MENU_PTR               (HALT_FUNC_SWITCH | 2)
#define HALT_FUNC_SWITCH_MENU_MODE              (HALT_FUNC_SWITCH | 3)
#define HALT_FUNC_SWITCH_LR_PTR                 (HALT_FUNC_SWITCH | 4)
#define HALT_FUNC_SWITCH_LR_MODE                (HALT_FUNC_SWITCH | 5)
#define HALT_FUNC_SWITCH_ZOOM_PTR               (HALT_FUNC_SWITCH | 6)
#define HALT_FUNC_SWITCH_ZOOM_MODE              (HALT_FUNC_SWITCH | 7)
#define HALT_FUNC_SWITCH_ZOOM_PARAM             (HALT_FUNC_SWITCH | 8)
#define HALT_FUNC_SWITCH_UD_PTR                 (HALT_FUNC_SWITCH | 9)
#define HALT_FUNC_SWITCH_UD_MODE                (HALT_FUNC_SWITCH | 0x0a)

#define HALT_MSGBOX                             (HALT_FUNC | 0x80)
#define HALT_MSGBOX_MODE                        (HALT_MSGBOX | 1)

#define HALT_FUNC_MENU                          (HALT_FUNC | 0x100)
#define HALT_FUNC_MENU_SUBSTA                   (HALT_FUNC_MENU | 1)

//HALT_BSP
#define HALT_I18N_INIT                          (HALT_BSP | 1)
#define HALT_BSP_SYS_CTLBITS                    (HALT_BSP | 2)
#define HALT_MALLOC                             (HALT_BSP | 3)
#define HALT_BSP_PORT                           (HALT_BSP | 0x10)
#define HALT_BSP_PORT_INIT                      (HALT_BSP_PORT | 1)
#define HALT_BSP_PORT_IRQ                       (HALT_BSP_PORT | 2)

//HALT_GUI
#define HALT_GUI_COMPO                          (HALT_GUI | 0x100)
#define HALT_GUI_COMPO_UNKNOW_TYPE              (HALT_GUI_COMPO | 1)
#define HALT_GUI_COMPO_PTR                      (HALT_GUI_COMPO | 2)
#define HALT_GUI_COMPO_GET_NEXT                 (HALT_GUI_COMPO | 3)
#define HALT_GUI_COMPO_BUF_EXCEED               (HALT_GUI_COMPO | 5)
#define HALT_GUI_COMPO_POOL_CREATE              (HALT_GUI_COMPO | 6)
#define HALT_GUI_COMPO_POOL_CLEAR               (HALT_GUI_COMPO | 7)
#define HALT_GUI_COMPO_POOL_SETID               (HALT_GUI_COMPO | 8)
#define HALT_GUI_COMPO_POOL_BONDDATA            (HALT_GUI_COMPO | 9)
#define HALT_GUI_COMPO_POOL_SETTOP              (HALT_GUI_COMPO | 10)

#define HALT_GUI_COMPO_COMM                     (HALT_GUI_COMPO | 0x20)
#define HALT_GUI_COMPO_CREATE                   (HALT_GUI_COMPO_COMM | 1)
#define HALT_GUI_COMPO_ROLL                     (HALT_GUI_COMPO_COMM | 2)

#define HALT_GUI_COMPO_FORM                     (HALT_GUI_COMPO | 0x40)
#define HALT_GUI_COMPO_FORM_CREATE              (HALT_GUI_COMPO_FORM | 1)
#define HALT_GUI_COMPO_FORM_PTR                 (HALT_GUI_COMPO_FORM | 2)

#define HALT_GUI_COMPO_ICONLIST                 (HALT_GUI_COMPO | 0x80)
#define HALT_GUI_COMPO_ICONLIST_TYPE            (HALT_GUI_COMPO_ICONLIST | 1)
#define HALT_GUI_COMPO_ICONLIST_STYLE           (HALT_GUI_COMPO_ICONLIST | 2)
#define HALT_GUI_COMPO_ICONLIST_ADD             (HALT_GUI_COMPO_ICONLIST | 3)
#define HALT_GUI_COMPO_ICONLIST_ICONSIZE        (HALT_GUI_COMPO_ICONLIST | 4)
#define HALT_GUI_COMPO_ICONLIST_SET_ICONSIZE    (HALT_GUI_COMPO_ICONLIST | 5)
#define HALT_GUI_COMPO_ICONLIST_SET_FOCUS       (HALT_GUI_COMPO_ICONLIST | 6)
#define HALT_GUI_COMPO_ICONLIST_TIME_TYPE       (HALT_GUI_COMPO_ICONLIST | 7)
#define HALT_GUI_COMPO_ICONLIST_GETLINE         (HALT_GUI_COMPO_ICONLIST | 8)
#define HALT_GUI_COMPO_ICONLIST_TIME_IDX        (HALT_GUI_COMPO_ICONLIST | 9)

#define HALT_GUI_COMPO_LISTBOX                  (HALT_GUI_COMPO | 0x90)
#define HALT_GUI_COMPO_LISTBOX_TYPE             (HALT_GUI_COMPO_LISTBOX | 1)
#define HALT_GUI_COMPO_LISTBOX_STYLE            (HALT_GUI_COMPO_LISTBOX | 2)
#define HALT_GUI_COMPO_LISTBOX_SET_FOCUS        (HALT_GUI_COMPO_LISTBOX | 3)
#define HALT_GUI_COMPO_LISTBOX_CREATE           (HALT_GUI_COMPO_LISTBOX | 4)
#define HALT_GUI_COMPO_LISTBOX_SET              (HALT_GUI_COMPO_LISTBOX | 5)
#define HALT_GUI_COMPO_LISTBOX_SET_BGIMG        (HALT_GUI_COMPO_LISTBOX | 6)
#define HALT_GUI_COMPO_LISTBOX_SET_STA_ICON     (HALT_GUI_COMPO_LISTBOX | 7)
#define HALT_GUI_COMPO_LISTBOX_GET_Y            (HALT_GUI_COMPO_LISTBOX | 8)
#define HALT_GUI_COMPO_LISTBOX_UPDATE           (HALT_GUI_COMPO_LISTBOX | 9)
#define HALT_GUI_COMPO_LISTBOX_SELECT           (HALT_GUI_COMPO_LISTBOX | 0xa)
#define HALT_GUI_COMPO_LISTBOX_GET_SELECT       (HALT_GUI_COMPO_LISTBOX | 0xb)
#define HALT_GUI_COMPO_LISTBOX_ITEM_MODE        (HALT_GUI_COMPO_LISTBOX | 0xc)
#define HALT_GUI_COMPO_LISTBOX_SET_BITHOOK      (HALT_GUI_COMPO_LISTBOX | 0xd)
#define HALT_GUI_COMPO_LISTBOX_SET_BTN_ICON     (HALT_GUI_COMPO_LISTBOX | 0xe)
#define HALT_GUI_COMPO_LISTBOX_MOVE_CMD         (HALT_GUI_COMPO_LISTBOX | 0xf)
#define HALT_GUI_COMPO_LISTBOX_MOVE_STA         (HALT_GUI_COMPO_LISTBOX | 0x10)

#define HALT_GUI_COMPO_STACKLIST                (HALT_GUI_COMPO | 0xA0)
#define HALT_GUI_COMPO_STACKLIST_COUNT          (HALT_GUI_COMPO_STACKLIST | 1)


#define HALT_GUI_COMPO_ROTARY                   (HALT_GUI_COMPO | 0xB0)
#define HALT_GUI_COMPO_ROTARY_TYPE              (HALT_GUI_COMPO_ROTARY | 1)
#define HALT_GUI_COMPO_ROTARY_CREATE            (HALT_GUI_COMPO_ROTARY | 2)
#define HALT_GUI_COMPO_ROTARY_MOVE_CMD          (HALT_GUI_COMPO_ROTARY | 3)
#define HALT_GUI_COMPO_ROTARY_MOVE_STA          (HALT_GUI_COMPO_ROTARY | 4)

#define HALT_GUI_COMPO_ARC                      (HALT_GUI_COMPO | 0xC0)
#define HALT_GUI_COMPO_ARC_FAIL                 (HALT_GUI_COMPO_ARC | 0)
#define HALT_GUI_COMPO_ARC_NULL                 (HALT_GUI_COMPO_ARC | 1)
#define HALT_GUI_COMPO_ARC_OUT_OF_RANGE         (HALT_GUI_COMPO_ARC | 2)
#define HALT_GUI_COMPO_ARC_TYPE                 (HALT_GUI_COMPO_ARC | 3)

#define HALT_GUI_COMPO_CHARTBOX                 (HALT_GUI_COMPO | 0xD0)
#define HALT_GUI_COMPO_CHARTBOX_PIXEL           (HALT_GUI_COMPO_CHARTBOX | 0)
#define HALT_GUI_COMPO_CHARTBOX_NOSIZE          (HALT_GUI_COMPO_CHARTBOX | 1)
#define HALT_GUI_COMPO_CHARTBOX_REALNUM         (HALT_GUI_COMPO_CHARTBOX | 2)

#define HALT_GUI_COMPO_FOOTBALL                 (HALT_GUI_COMPO | 0xE0)
#define HALT_GUI_COMPO_FOOTBALL_TYPE            (HALT_GUI_COMPO_FOOTBALL | 1)
#define HALT_GUI_COMPO_FOOTBALL_CREATE          (HALT_GUI_COMPO_FOOTBALL | 2)
#define HALT_GUI_COMPO_FOOTBALL_MOVE_CMD        (HALT_GUI_COMPO_FOOTBALL | 3)
#define HALT_GUI_COMPO_FOOTBALL_MOVE_STA        (HALT_GUI_COMPO_FOOTBALL | 4)

#define HALT_GUI_COMPO_BUTTERFLY                (HALT_GUI_COMPO | 0xF0)
#define HALT_GUI_COMPO_BUTTERFLY_TYPE           (HALT_GUI_COMPO_BUTTERFLY | 1)
#define HALT_GUI_COMPO_BUTTERFLY_CREATE         (HALT_GUI_COMPO_BUTTERFLY | 2)
#define HALT_GUI_COMPO_BUTTERFLY_MOVE_CMD       (HALT_GUI_COMPO_BUTTERFLY | 3)
#define HALT_GUI_COMPO_BUTTERFLY_MOVE_STA       (HALT_GUI_COMPO_BUTTERFLY | 4)

#define HALT_GUI_COMPO_CUBE                     (HALT_GUI_COMPO | 0xF0)
#define HALT_GUI_COMPO_CUBE_TYPE                (HALT_GUI_COMPO_CUBE | 0x0A)
#define HALT_GUI_COMPO_CUBE_CREATE              (HALT_GUI_COMPO_CUBE | 0x0B)
#define HALT_GUI_COMPO_CUBE_MOVE_CMD            (HALT_GUI_COMPO_CUBE | 0x0C)
#define HALT_GUI_COMPO_CUBE_MOVE_STA            (HALT_GUI_COMPO_CUBE | 0x0D)

#define HALT_GUI_COMPO2                         (HALT_GUI | 0x200)
#define HALT_GUI_COMPO_NUMBER                   (HALT_GUI_COMPO2 | 0x10)
#define HALT_GUI_COMPO_NUMBER_CNT               (HALT_GUI_COMPO_NUMBER | 1)
#define HALT_GUI_COMPO_NUMBER_VALUE             (HALT_GUI_COMPO_NUMBER | 2)

#define HALT_GUI_DIALPLATE                      (HALT_GUI | 0x300)
#define HALT_GUI_DIALPLATE_HEAD                 (HALT_GUI_DIALPLATE | 1)
#define HALT_GUI_DIALPLATE_TYPE                 (HALT_GUI_DIALPLATE | 2)

void halt(u32 halt_no);

#endif
