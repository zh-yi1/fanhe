#include "ble_hid_service.h"

#if LE_HID_EN
bool ble_hid_send_touch_cmd(u8 touch, u8 id, u16 x, u16 y)
{
    ble_digitizer_report_typedef ble_digitizer_report;
    memset(&ble_digitizer_report, 0x00, sizeof(ble_digitizer_report_typedef));
    ble_digitizer_report.tip_switch = touch;
    ble_digitizer_report.in_range = touch;
    ble_digitizer_report.reserve = touch;
    ble_digitizer_report.contact_identifier = id;
    ble_digitizer_report.x = x;
    ble_digitizer_report.y = y;
    ble_digitizer_report.contact_count = touch;

    return ble_hid_task_enqueue_delay(BLE_HID_CMD_ID_DIGITIZER, (u8 *)&ble_digitizer_report, sizeof(ble_digitizer_report_typedef), BLE_HID_POLLING_INTERVAL_MS);
}

bool ble_hid_send_touch_cmd_delay(u8 touch, u8 id, u16 x, u16 y, u8 delay)
{
    ble_digitizer_report_typedef ble_digitizer_report;
    memset(&ble_digitizer_report, 0x00, sizeof(ble_digitizer_report_typedef));
    ble_digitizer_report.tip_switch = touch;
    ble_digitizer_report.in_range = touch;
    ble_digitizer_report.reserve = touch;
    ble_digitizer_report.contact_identifier = id;
    ble_digitizer_report.x = x;
    ble_digitizer_report.y = y;
    ble_digitizer_report.contact_count = touch;

    return ble_hid_task_enqueue_delay(BLE_HID_CMD_ID_DIGITIZER, (u8 *)&ble_digitizer_report, sizeof(ble_digitizer_report_typedef), delay);
}

//void ble_hid_send_touch_cmd_kick(u8 touch, u16 x, u16 y, u8 kick_cfg)
//{
//    ble_digitizer_report_typedef ble_digitizer_report;
//    ble_digitizer_report.tip_switch = touch;
//    ble_digitizer_report.in_range = touch;
//    ble_digitizer_report.contact_identifier = 0;
//    ble_digitizer_report.x = x;
//    ble_digitizer_report.y = y;
//    ble_digitizer_report.contact_count = touch;
//
//    ble_hid_report_for_handle_kick(ATT_CHARACTERISTIC_2a4d_02_VALUE_HANDLE, (u8*)&ble_digitizer_report, sizeof(ble_digitizer_report), kick_cfg);
//}


bool ble_hid_send_mouse_control(bool click, s8 wheel, s8 acpan)
{
    ble_mouse_report_t ble_mouse_report;
    memset(&ble_mouse_report, 0x00, sizeof(ble_mouse_report_t));

    ble_mouse_report.flag = 0;
    ble_mouse_report.control.button1 = click;
    ble_mouse_report.control.button2 = 0;
    ble_mouse_report.control.button3 = 0;
    ble_mouse_report.control.button4 = 0;
    ble_mouse_report.control.button5 = 0;

    if (wheel < -127) {
        wheel = -127;
    }
    if (wheel > 127) {
        wheel = 127;
    }
    if (acpan < -127) {
        acpan = -127;
    }
    if (acpan > 127) {
        acpan = 127;
    }

    ble_mouse_report.control.wheel = wheel;
    ble_mouse_report.control.acpan = acpan;
    return ble_hid_task_enqueue(BLE_HID_CMD_ID_MOUSE, (u8 *)&ble_mouse_report, sizeof(ble_mouse_report_t));
}

bool ble_hid_send_mouse_location(s16 x, s16 y)
{
    ble_mouse_report_t ble_mouse_report;
    memset(&ble_mouse_report, 0x00, sizeof(ble_mouse_report_t));
    ble_mouse_report.flag = 1;
    ble_mouse_report.location.xy[0] = x & 0xff;
    ble_mouse_report.location.xy[1] = ((x & 0xf00)>> 8) | ((y & 0x0f) << 4);
    ble_mouse_report.location.xy[2] = (y & 0xff0) >> 4;
    return ble_hid_task_enqueue(BLE_HID_CMD_ID_MOUSE, (u8 *)&ble_mouse_report, sizeof(ble_mouse_report_t));
}

bool ble_hid_send_mouse_location_delay(s16 x, s16 y, u8 delay)
{
    ble_mouse_report_t ble_mouse_report;
    memset(&ble_mouse_report, 0x00, sizeof(ble_mouse_report_t));
    ble_mouse_report.flag = 1;
    ble_mouse_report.location.xy[0] = x & 0xff;
    ble_mouse_report.location.xy[1] = ((x & 0xf00)>> 8) | ((y & 0x0f) << 4);
    ble_mouse_report.location.xy[2] = (y & 0xff0) >> 4;
    return ble_hid_task_enqueue_delay(BLE_HID_CMD_ID_MOUSE, (u8 *)&ble_mouse_report, sizeof(ble_mouse_report_t), delay);
}

bool ble_hid_send_consumer_cmd(u16 control_cmd)
{
    return ble_hid_task_enqueue(BLE_HID_CMD_ID_CONSUMER, (u8 *)&control_cmd, 2);
}

bool ble_hid_send_keyboard_cmd(u8 modifier, u8 key_code)
{
    ble_keyboard_report_typedef ble_keyboard_report;
    memset(&ble_keyboard_report, 0x00, sizeof(ble_keyboard_report_typedef));
    ble_keyboard_report.modifier = modifier;
    ble_keyboard_report.key_code = key_code;

    return ble_hid_task_enqueue(BLE_HID_CMD_ID_KEYBOARD, (u8 *)&ble_keyboard_report, sizeof(ble_keyboard_report_typedef));
}


#endif
