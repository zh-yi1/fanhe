#ifndef __BLE_HID_SERVICE_H
#define __BLE_HID_SERVICE_H

#include "include.h"

#define BLE_HID_CMD_BUFFER_LENGTH           8
#define BLE_HID_CMD_QUEUE_SIZE              30
#define BLE_HID_POLLING_INTERVAL_MS         30
#define BLE_HID_TOUCH_BEGIN_MS              20
#define BLE_HID_TOUCH_FOLLOW_MS             15
#define BLE_HID_TOUCH_END_MS                1

/**
 * @brief The limit for x/y position, this value is determined by report map table.
 */
#define HID_DIGITIZER_X_MIN                 0
#define HID_DIGITIZER_X_MAX                 4096
#define HID_DIGITIZER_Y_MIN                 0
#define HID_DIGITIZER_Y_MAX                 4096

/**
 * @brief The limit for x/y.
 */
#define HID_MOUSE_X_MIN                     -2047
#define HID_MOUSE_X_MAX                     2047
#define HID_MOUSE_Y_MIN                     -2047
#define HID_MOUSE_Y_MAX                     2047

/**
 * @brief The keyboard modifier definition.
 */
#define HID_KM_NONE                         0x00
#define HID_KM_LEFT_CTRL                    0x01
#define HID_KM_LEFT_SHIFT                   0x02
#define HID_KM_LEFT_ALT                     0x04
#define HID_KM_LEFT_GUI                     0x08
#define HID_KM_RIGHT_CTRL                   0x01
#define HID_KM_RIGHT_SHIFT                  0x02
#define HID_KM_RIGHT_ALT                    0x04
#define HID_KM_RIGHT_GUI                    0x08

typedef enum {
    BLE_HID_CMD_ID_CONSUMER,
    BLE_HID_CMD_ID_DIGITIZER,
    BLE_HID_CMD_ID_MOUSE,
    BLE_HID_CMD_ID_KEYBOARD,
} BLE_HID_CMD_ID_TYPEDEF;

typedef struct PACKED {
    uint8_t         tip_switch  : 1;
    uint8_t         in_range    : 1;
    uint8_t         reserve     : 6;
    uint8_t         contact_identifier;
    uint16_t        x;              // X position
    uint16_t        y;              // Y position
    uint8_t         contact_count;
} ble_digitizer_report_typedef;

typedef struct PACKED {
    uint8_t         button1 : 1;    // Left Button
    uint8_t         button2 : 1;    // Right Button
    uint8_t         button3 : 1;    // Central Button
    uint8_t         button4 : 1;
    uint8_t         button5 : 1;
    uint8_t         reserve : 3;
    int8_t          wheel;
    int8_t          acpan;
} ble_mouse_report_control_typedef;

typedef struct PACKED {
    uint8_t         xy[3];
} ble_mouse_report_location_typedef;

typedef struct PACKED {
    uint8_t flag;                   // 0:control cmd; 1:location cmd
    ble_mouse_report_control_typedef control;
    ble_mouse_report_location_typedef location;
} ble_mouse_report_t;

typedef struct {
    uint8_t modifier;
    uint8_t key_code;
} ble_keyboard_report_typedef;

typedef struct {
    BLE_HID_CMD_ID_TYPEDEF  id;
    uint8_t                 len;
    uint8_t                 delay_ms;
    uint8_t                 buffer[BLE_HID_CMD_BUFFER_LENGTH];
} ble_hid_cmd_typedef;

typedef struct {
    uint8_t                 head_idx;
    uint8_t                 rear_idx;
    uint8_t                 vaild_flag;
    ble_hid_cmd_typedef     queue[BLE_HID_CMD_QUEUE_SIZE];
} ble_hid_tasks_typedef;

#define BLE_HID_MSG_ID_SELFIE           0x0000
#define BLE_HID_MSG_ID_TIKTOK           0x0100
#define BLE_HID_MSG_ID_TEST             0x0200
#define BLE_HID_MSG_ID_MASK             0x0f00

typedef enum {
    BLE_HID_MSG_NONE,

    /* Selfie Device Message */
    BLE_HID_MSG_PHOTOGRAPH              = 0x0001,
    BLE_HID_MSG_OPEN_OR_SWITCH_CAMERA,
    BLE_HID_MSG_VCR_START,
    BLE_HID_MSG_VCR_STOP,
    BLE_HID_MSG_ZOOM_IN,
    BLE_HID_MSG_ZOOM_IN_STOP,
    BLE_HID_MSG_ZOOM_OUT,
    BLE_HID_MSG_ZOOM_OUT_STOP,
    /* Only Android Used */
    BLE_HID_MSG_ZOOM_IN_PREPARE,
    BLE_HID_MSG_ZOOM_OUT_PREPARE,
    /* Only IOS Used */
    BLE_HID_MSG_FOCUS_PLATE,

    /* TikTok Page turner Message */
    BLE_HID_MSG_CLICK                   = 0x0101,
    BLE_HID_MSG_DOUBLE,
    BLE_HID_MSG_UP,             //上一个视频
    BLE_HID_MSG_DOWN,           //下一个视频
    BLE_HID_MSG_LEFT,
    BLE_HID_MSG_RIGHT,
    BLE_HID_MSG_ROUGH_MOVE,
    BLE_HID_MSG_CAREFUL_MOVE,
    BLE_HID_MSG_VOLUME_UP,
    BLE_HID_MSG_VOLUME_DOWN,
    BLE_HID_MSG_PP_HOLD,
    BLE_HID_MSG_K1_HOLD,
    BLE_HID_MSG_K2_HOLD,

    BLE_HID_MSG_MASK                    = 0x0fff,
} BLE_HID_MSG_TYPEDEF;

typedef enum {
    PEER_DEVICE_TYPE_ANDROID            = 0x01,
    PEER_DEVICE_TYPE_IOS,
} peer_device_type_t;

peer_device_type_t ble_hid_peer_device_type_get(void);
bool ble_hid_peer_device_is_ios(void);

int ble_hid_service_init(void);
bool ble_hid_task_enqueue(BLE_HID_CMD_ID_TYPEDEF id, u8 *buffer, u8 len);
bool ble_hid_task_enqueue_delay(BLE_HID_CMD_ID_TYPEDEF id, u8 *buffer, u8 len, u8 delay);
uint ble_hid_task_get_cnt(void);
void ble_hid_service_proc(void);

bool ble_hid_send_touch_cmd(u8 touch, u8 id, u16 x, u16 y);
bool ble_hid_send_touch_cmd_delay(u8 touch, u8 id, u16 x, u16 y, u8 delay);
//void ble_hid_send_touch_cmd_kick(u8 touch, u16 x, u16 y, u8 kick_cfg);
bool ble_hid_send_consumer_cmd(u16 control_cmd);
bool ble_hid_send_keyboard_cmd(u8 modifier, u8 key_code);
bool ble_hid_send_mouse_control(bool click, s8 wheel, s8 acpan);
bool ble_hid_send_mouse_location(s16 x, s16 y);
bool ble_hid_send_mouse_location_delay(s16 x, s16 y, u8 delay);


//抖音神器事件
bool ble_hid_event_tiktok(u16 msg);
bool ble_hid_event_selfie(u16 msg);

void ble_hid_tiktok_mouse_location_init(void);

#endif
