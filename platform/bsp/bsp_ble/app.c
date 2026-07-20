#include "include.h"
#include "app.h"

void ble_app_init(void)
{
#if (USE_APP_TYPE == APP_AB_LINK)
    ble_app_ab_link_init();
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)
    ble_app_watch_init();  //蓝牙信息初始化
#endif

#if LE_ANCS_CLIENT_EN
    ble_ancs_var_init(); //ancs_msg_cb初始化
#endif
}

//BLE数据处理
void ble_app_process(void)
{
#if (USE_APP_TYPE == APP_AB_LINK)
    ble_app_ab_link_process();
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)
    ble_app_watch_process();  //ble
#endif
}

AT(.com_text.sleep.app.wakeup)
bool ble_app_need_wakeup(void)
{
    bool ret = false;

#if (USE_APP_TYPE == APP_AB_LINK)
    ret |= ble_app_ab_link_need_wakeup();
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)
    ret |= ble_app_watch_need_wakeup();
#endif

    return ret;
}

void ble_app_disconnect_callback(void)
{
#if (USE_APP_TYPE == APP_BLUE_FIT)
    ble_app_watch_disconnect_callback();
#endif
}

//一键双连应用示例
void app_once_connect(u8 ble_app_ios)
{
#if LE_SM_SC_EN
    if (app_phone_type_get() != ble_app_ios) {
        app_phone_type_set(ble_app_ios);
        bsp_change_bt_mac();
    }

    if (ble_app_ios && !bt_is_connected()) {        //IOS
        if (bt_nor_get_link_info(NULL)) {           //如果存在配对信息
            sys_cb.bt_reconn_flag = true;
            bt_connect();
        } else {
            ble_bt_connect();                       //一键双联
        }
    }
#endif // LE_SM_SC_EN
}

void ble_app_connect_callback(void)
{
#if (USE_APP_TYPE == APP_BLUE_FIT)
    ble_app_watch_connect_callback();
#endif

//    //一键双链接IOS示例程序,需要打开宏LE_PAIR_EN, LE_SM_SC_EN
//    app_once_connect(true);
//    //一键双链接安卓示例程序,需要打开宏LE_PAIR_EN, LE_SM_SC_EN
//    app_once_connect(false);
#if LE_AB_FOT_EN
    fot_ble_disconnect_callback();
#endif
}

void ble_app_client_cfg_callback(u16 handle, u8 cfg)
{
#if (USE_APP_TYPE == APP_BLUE_FIT)
    ble_app_watch_client_cfg_callback(handle, cfg);
#endif
}

void ble_app_ancs_client_notifiy_callback(u8 id, const char *att_name, const char *att_content)
{
#if LE_ANCS_CLIENT_EN
    ble_app_watch_ancs_client_notifiy_process(id, att_name, att_content);
#endif
}
