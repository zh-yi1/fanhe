#include "ble_hid_service.h"
#include "hid_usage.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if LE_HID_EN

//定义该变量配置ble_hid兼容性
const uint8_t cfg_ble_hid_compatibility = BIT(0); //bit0=single_tx, 兼容抖音翻页失败

void ancs_search_init_do(void);
//初始化回调函数
void ancs_search_init(void)
{
    ancs_search_init_do();
}

//更新参数回调函数
void ble_update_con_param_response(uint16_t con_handle, uint8_t result)
{
    //result:0=accept, 1=reject, other=reserved
    uint16_t interval = ble_get_conn_interval();

//    ble_get_update_con_param(&interval, NULL, NULL);
    printf("====>updata_con_param_respone(%d), %s\n", interval, (result!=0)? "reject" : "accept");

    if(result != 0) {
        if(ble_get_conn_interval() > 30) {
            if(interval < 9 && PEER_DEVICE_TYPE_IOS != ble_hid_peer_device_type_get()) {
                ble_update_conn_param(9, 12, 500);  //11.25ms
            } else if(interval < 12) {
                ble_update_conn_param(12, 12, 500); //15ms
            } else if(interval < 24) {
                ble_update_conn_param(24, 12, 500); //30ms
            }
        }
    }
}

void ble_hid_tiktok_mouse_location_init(void)
{
    if (PEER_DEVICE_TYPE_IOS == ble_hid_peer_device_type_get()) {
        ble_hid_send_mouse_control(0, 0, 0);
        ble_hid_send_mouse_location(HID_MOUSE_X_MIN, HID_MOUSE_Y_MIN);
        ble_hid_send_mouse_location(HID_MOUSE_X_MIN, HID_MOUSE_Y_MIN);
        ble_hid_send_mouse_location(0, HID_MOUSE_Y_MIN);
        ble_hid_send_mouse_location(60, 120);
        ble_hid_send_mouse_control(0, 0, 0);
        ble_hid_send_mouse_control(0, 0, 0);
    }
}

bool ble_hid_event_tiktok(u16 msg)
{
    printf("===>current interval = %d\n", ble_get_conn_interval());
    if (ble_get_conn_interval() > 30) {
        return false;
    }

    if(ble_hid_task_get_cnt() + 11 < BLE_HID_CMD_QUEUE_SIZE) {
        printf("===>task_queue, full\n");
        return false;
    }

    int android_pos_x = 1904;
    int android_pos_y = 1904;

    uint8_t is_android = (PEER_DEVICE_TYPE_ANDROID == ble_hid_peer_device_type_get());
    TRACE("is_android=%d\n", is_android);

    static u16 pre_msg = BLE_HID_MSG_CLICK;
    if (pre_msg != msg) {
        pre_msg = msg;
        //printf("--->location_init\n");
        ble_hid_tiktok_mouse_location_init();
    }

    switch (msg & BLE_HID_MSG_MASK) {
    // 单击
    case BLE_HID_MSG_CLICK:
        TRACE("BLE_HID_MSG_CLICK\n");
        if (is_android) {
            //定位
            ble_hid_send_mouse_location(HID_MOUSE_X_MIN, HID_MOUSE_Y_MAX);
            ble_hid_send_mouse_location(160, -381);
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);
            //点击
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);
        }
        else {
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);
        }
        break;

    // 双击
    case BLE_HID_MSG_DOUBLE:
        TRACE("BLE_HID_MSG_DOUBLE\n");
        if (is_android) {
            //定位
            ble_hid_send_mouse_location_delay(HID_MOUSE_X_MIN, HID_MOUSE_Y_MAX, 0);
            ble_hid_send_mouse_location_delay(120, -350, 0);
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);

            //点击
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);

            //等待
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);

            //点击
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);
        }
        else {
            //点击
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);

            //等待
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);

            //点击
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);
        }
        break;

    // 连点
    case BLE_HID_MSG_PP_HOLD:
        TRACE("BLE_HID_MSG_PP_HOLD\n");
        if (is_android) {
            ble_hid_send_mouse_location(HID_MOUSE_X_MIN, HID_MOUSE_Y_MAX);
            ble_hid_send_mouse_location(160, -381);
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);

            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);
        }
        else {
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_control(0, 0, 0);
        }
        break;

    case BLE_HID_MSG_K1_HOLD:
        TRACE("BLE_HID_MSG_K1_HOLD\n");
        ble_hid_send_consumer_cmd(HID_CCC_VCR_PLUS);
        ble_hid_send_consumer_cmd(HID_CCC_RELEASE);
        break;

    case BLE_HID_MSG_K2_HOLD:
        TRACE("BLE_HID_MSG_K2_HOLD\n");
        ble_hid_send_consumer_cmd(HID_CCC_AC_HOME);
        ble_hid_send_consumer_cmd(HID_CCC_RELEASE);
        break;

    case BLE_HID_MSG_VOLUME_UP:
        TRACE("BLE_HID_MSG_VOLUME_UP\n");
        ble_hid_send_consumer_cmd(HID_CCC_VOLUME_INCREMENT);
        ble_hid_send_consumer_cmd(HID_CCC_RELEASE);
        break;

    case BLE_HID_MSG_VOLUME_DOWN:
        TRACE("BLE_HID_MSG_VOLUME_DOWN\n");
        ble_hid_send_consumer_cmd(HID_CCC_VOLUME_DECREMENT);
        ble_hid_send_consumer_cmd(HID_CCC_RELEASE);
        break;

    //上一个视频
    case BLE_HID_MSG_UP:
        TRACE("BLE_HID_MSG_UP\n");
        if (is_android) {
            ble_hid_send_mouse_location_delay(HID_MOUSE_X_MIN, HID_MOUSE_Y_MAX, 0);
            ble_hid_send_mouse_location_delay(160, -381, 0);
            ble_hid_send_mouse_location_delay(0, -82, 0);
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_location_delay(0, 80, 0);
            ble_hid_send_mouse_location_delay(0, 80, 0);
            ble_hid_send_mouse_location_delay(0, 80, 0);
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_location_delay(HID_MOUSE_X_MIN, HID_MOUSE_Y_MAX, 0);
        }
        else {
            ble_hid_send_mouse_control(0, -46, 0);
            ble_hid_send_mouse_control(0, -46, 0);
            ble_hid_send_mouse_control(0, -6, 0);

            ble_hid_send_touch_cmd(1, 6, android_pos_x, 1400);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 2300);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 2600);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 2900);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 3200);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 3500);
            ble_hid_send_touch_cmd(0, 6, android_pos_x, 3756);
        }
        break;

    //下一个视频
    case BLE_HID_MSG_DOWN:
        TRACE("BLE_HID_MSG_DOWN\n");
        if (is_android) {
            ble_hid_send_mouse_location_delay(HID_MOUSE_X_MIN, HID_MOUSE_Y_MAX, 0);
            ble_hid_send_mouse_location_delay(160, -381, 0);
            ble_hid_send_mouse_location_delay(0, 80, 0);
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_location_delay(0, -82, 0);
            ble_hid_send_mouse_location_delay(0, -82, 0);
            ble_hid_send_mouse_location_delay(0, -82, 0);
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_location_delay(HID_MOUSE_X_MIN, HID_MOUSE_Y_MAX, 0);
        }
        else {
            ble_hid_send_mouse_control(0, 50, 0);
            ble_hid_send_mouse_control(0, 50, 0);
            ble_hid_send_mouse_control(0, 40, 0);

            ble_hid_send_touch_cmd(1, 6, android_pos_x, 3500);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 3200);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 2900);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 2600);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 2300);
            ble_hid_send_touch_cmd(1, 6, android_pos_x, 2000);
            ble_hid_send_touch_cmd(0, 6, android_pos_x, 1400);
        }
        break;

    case BLE_HID_MSG_LEFT:
        TRACE("BLE_HID_MSG_LEFT\n");
        if (is_android) {
            ble_hid_send_touch_cmd(1, 0, 1500, android_pos_y);
            ble_hid_send_touch_cmd(1, 0, 2000, android_pos_y);
            ble_hid_send_touch_cmd(1, 0, 2500, android_pos_y);
            ble_hid_send_touch_cmd(1, 0, 3000, android_pos_y);
            ble_hid_send_touch_cmd(1, 0, 3600, android_pos_y);
            ble_hid_send_touch_cmd(0, 0, 3800, android_pos_y);
            ble_hid_send_touch_cmd(0, 0, 3800, android_pos_y);
        }
        else {
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_location(60, 0);
            ble_hid_send_mouse_location(60, 0);
            ble_hid_send_mouse_location(60, 0);
            ble_hid_send_mouse_location(60, 0);
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_location(-60, 0);
            ble_hid_send_mouse_location(-60, 0);
            ble_hid_send_mouse_location(-60, 0);
            ble_hid_send_mouse_location(-60, 0);
            ble_hid_send_mouse_control(0, 0, 0);
        }
        break;

    case BLE_HID_MSG_RIGHT:
        TRACE("BLE_HID_MSG_RIGHT\n");
        if (is_android) {
            ble_hid_send_touch_cmd(1, 0, 2500, android_pos_y);
            ble_hid_send_touch_cmd(1, 0, 2000, android_pos_y);
            ble_hid_send_touch_cmd(1, 0, 1500, android_pos_y);
            ble_hid_send_touch_cmd(1, 0, 1000, android_pos_y);
            ble_hid_send_touch_cmd(1, 0, 500,  android_pos_y);
            ble_hid_send_touch_cmd(0, 0, 300,  android_pos_y);
            ble_hid_send_touch_cmd(0, 0, 300,  android_pos_y);
        }
        else {
            ble_hid_send_mouse_control(1, 0, 0);
            ble_hid_send_mouse_location(-60, 0);
            ble_hid_send_mouse_location(-60, 0);
            ble_hid_send_mouse_location(-60, 0);
            ble_hid_send_mouse_location(-60, 0);
            ble_hid_send_mouse_control(0, 0, 0);
            ble_hid_send_mouse_location(60, 0);
            ble_hid_send_mouse_location(60, 0);
            ble_hid_send_mouse_location(60, 0);
            ble_hid_send_mouse_location(60, 0);
            ble_hid_send_mouse_control(0, 0, 0);
        }
        break;

    default:
        /* event invalid */
        return false;
    }

    return true;

}
#else
bool ble_hid_event_tiktok(u16 msg){return 0;}
#endif
