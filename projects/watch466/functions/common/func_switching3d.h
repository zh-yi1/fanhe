#ifndef _SWITCHING3D_H
#define _SWITCHING3D_H

enum FUNC_SWITCH_MODE3D {
    FUNC_SWITCH3D_LR_LEFT = FUNC_SWITCH3D,                          //3D左右切换
    FUNC_SWITCH3D_LR_RIGHT,
    FUNC_SWITCH3D_LR_IN_LEFT,                                       //3D左右内切换
    FUNC_SWITCH3D_LR_IN_RIGHT,
    FUNC_SWITCH3D_LR_FLIP_LEFT,                                     //3D平面左右翻转
    FUNC_SWITCH3D_LR_FLIP_RIGHT,
    FUNC_SWITCH3D_LR_ROTA_LEFT,                                     //3D左右旋转切换
    FUNC_SWITCH3D_LR_ROTA_RIGHT,
    FUNC_SWITCH3D_LR_FOLD_LEFT,                                     //3D左右翻页效果
    FUNC_SWITCH3D_LR_FOLD_RIGHT,  
    FUNC_SWITCH3D_LR_PLATE_FLIP_LEFT,                               //3D立体左右翻转
    FUNC_SWITCH3D_LR_PLATE_FLIP_RIGHT,
    FUNC_SWITCH3D_LR_DRIFT_LEFT,                                    //3D左右漂移翻转
    FUNC_SWITCH3D_LR_DRIFT_RIGHT,
};

void func_switch_screenshot(void *cur_scbuf, void *next_scbuf, u8 next_sta);

//创建3D窗体，mode：3D样式
void func_switching3d_form_create(u16 switch_mode, u16 sta, compo_form_t **cur_frm, compo_form_t **new_frm);

bool func_switching3d(u16 switch_mode, bool flag_auto, void *param);

#endif
