#include "ble_hid_service.h"
#include "../ble.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if LE_HID_EN
static ble_hid_tasks_typedef ble_hid_tasks;

//绿联LP-817方案
static u8 report_map_android[]={
#if LE_HID_CONSUMER
    0x05, 0x0C,           // Usage Page (Consumer)
    0x09, 0x01,           // Usage (Consumer Control)
    0xA1, 0x01,           // Collection (Application)
    0x85, 0x03,           //   Report Id (3)
    0x15, 0x00,           //   Logical minimum (0)
    0x25, 0x01,           //   Logical minimum (1)
    0x75, 0x01,           //   Report Size (1)
    0x95, 0x05,           //   Report Count (5)
    0x09, 0xE9,           //   Usage (Volume Increment)
    0x09, 0xEA,           //   Usage (Volume Decrement)
    0x81, 0x06,           //   Input (Data,Value,Relative,Bit Field)
    0xC0,                 // End Collection
#endif //LE_HID_CONSUMER
#if LE_HID_MOUSE
    0x05, 0x01,           // Usage Page (Generic Desktop)
    0x09, 0x02,           // Usage (Mouse)
    0xA1, 0x01,           // Collection (Application)
    0x85, 0x04,           //   Report Id (4)
    0x09, 0x01,           //   Usage (Pointer)
    0xA1, 0x00,           //     Collention (Physical)
    0x95, 0x05,           //       Report Count (5)
    0x75, 0x01,           //       Report Size (1)
    0x05, 0x09,           //       Usage Page (Botton)
    0x19, 0x01,           //       Usage Page (Botton1)
    0x29, 0x05,           //       Usage Page (Botton5)
    0x15, 0x00,           //       Logical minimum (0)
    0x25, 0x01,           //       Logical minimum (1)
    0x81, 0x02,           //       Input (Data,Value,Absolute,Bit Field)
    0x95, 0x01,           //       Report Size (1)
    0x75, 0x03,           //       Report Count (5)
    0x81, 0x01,           //       Input (Data,Array,Absolute,Bit Field)
    0x75, 0x08,           //       Report Size (8)
    0x95, 0x01,           //       Report Count (1)
    0x05, 0x01,           //       Usage Page (Generic Desktop)
    0x09, 0x38,           //       Usage (wheel)
    0x15, 0x81,           //       Logical_Minimum (-127)
    0x25, 0x7f,           //       Logical_Maximum (127)
    0x81, 0x06,           //       Input (Data,Value,Relative,Bit Field)
    0x05, 0x0c,           //       Usage Page (Consumer)
    0x0a, 0x38, 0x02,     //       Usage (Acpan)
    0x95, 0x01,           //       Input (Data,Value,Relative,Bit Field)
    0xc0,                 //     End Collection
    0x85, 0x05,           //     Report Id (5)
    0x09, 0x01,           //     Usage (Consumer Control)
    0xA1, 0x00,           //     Collention (Physical)
    0x75, 0x0C,           //     Report Size (12)
    0x95, 0x02,           //     Report Count (2)
    0x05, 0x01,           //     Usage Page (Generic Desktop)
    0x09, 0x30,           //     Usage (X)
    0x09, 0x31,           //     Usage (Y)
    0x16, 0x01, 0xF8,     //     Logical_Minimum (-2047)
    0x26, 0xFF, 0x07,     //     Logical_Maximum (2047)
    0x81, 0x06,           //     Input (Data,Value,Relative,Bit Field)
    0xC0,                 //   End Collection
    0xC0,                 // End Collection
#endif //LE_HID_MOUSE
};

static u8 report_map_ios[]={
#if LE_HID_CONSUMER
    0x05, 0x0C,           // Usage Page (Consumer)
    0x09, 0x01,           // Usage (Consumer Control)
    0xA1, 0x01,           // Collection (Application)
    0x85, 0x03,           //   Report Id (3)
    0x15, 0x00,           //   Logical minimum (0)
    0x25, 0x01,           //   Logical minimum (1)
    0x75, 0x01,           //   Report Size (1)
    0x95, 0x05,           //   Report Count (5)
    0x09, 0xE9,           //   Usage (Volume Increment)
    0x09, 0xEA,           //   Usage (Volume Decrement)
    0x81, 0x06,           //   Input (Data,Value,Relative,Bit Field)
    0xC0,                 // End Collection
#endif //LE_HID_CONSUMER
#if LE_HID_MOUSE
    0x05, 0x01,         // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,         // USAGE (Mouse)
    0xa1, 0x01,         // COLLECTION (Application)
    0x85, 0x04,         //     REPORT_ID (4)
    0x09, 0x01,         //   USAGE (Pointer)
    0xa1, 0x00,         //   COLLECTION (Physical)
    0x95, 0x05,         //     REPORT_COUNT (3)
    0x75, 0x01,         //     REPORT_SIZE (1)
    0x05, 0x09,         //     USAGE_PAGE (Button)
    0x19, 0x01,         //     USAGE_MINIMUM (Button 1)
    0x29, 0x05,         //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,         //     LOGICAL_MINIMUM (0)
    0x25, 0x01,         //     LOGICAL_MAXIMUM (1)
    0x81, 0x02,         //     INPUT (Data,Var,Abs)
    0x95, 0x01,         //     REPORT_COUNT (1)
    0x75, 0x03,         //     REPORT_SIZE (5)
    0x81, 0x01,         //     INPUT (Cnst,Ary,Abs)
    0x75, 0x08,         //     REPORT SIZE (8)
    0x95, 0x01,         //     REPORT COUNT (1)
    0x05, 0x01,         //     USAGE_PAGE (Generic Desktop)
    0x09, 0x38,         //     USAGE (wheel)
    0x15, 0x81,         //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,         //     LOGICAL_MAXIMUM (127)
    0x81, 0x06,         //     INPUT (Cnst,Ary,Abs)
    0x05, 0x0c,         //     USAGE PAGE (Consumer)
    0x0a, 0x38, 0x02,   //     USAGE (Acpan)
    0x95, 0x01,         //     LOGICAL_MINIMUM (-127)
    0x81, 0x06,         //     INPUT (Cnst,Ary,Abs)
    0xC0,               //   END_COLLECTION
    0x85, 0x05,         //     Report ID (5)
    0x09, 0x01,         //     Usage (Consumer Control)
    0xA1, 0x00,         //     COLLECTION (Physical)
    0x75, 0x0c,         //       REPORT_SIZE (12)
    0x95, 0x02,         //       REPORT_COUNT (2)
    0x05, 0x01,         //       USAGE_PAGE (Generic Desktop)
    0x09, 0x30,         //       USAGE (X)
    0x09, 0x31,         //       USAGE (Y)
    0x16, 0x01, 0xf8,   //       LOGICAL_MINIMUM (-2047)
    0x26, 0xff, 0x07,   //       LOGICAL_MAXIMUM (2047)
    0x81, 0x06,         //       INPUT (Data,Var,Rel)
    0xc0,               //     END_COLLECTION
    0xc0,               // END_COLLECTION
#endif //LE_HID_MOUSE
#if LE_HID_DIGITIZER
	0x05, 0x0D,        //   Usage Page (Digitizer)
    0x09, 0x02,        //   Usage (Pen)
    0xA1, 0x01,        //   Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x22,        //   Usage (Finger)
    0xA1, 0x02,        //   Collection (Logical)
    0x09, 0x42,        //   Usage (Tip Switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x32,        //   Usage (In Range)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //   Report Size (8)
    0x09, 0x51,        //   Usage (Contact identifier)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x26, 0xFF, 0x0F,  //   Logical Maximum (4095)
    0x75, 0x10,        //   Report Size (16)
    0x55, 0x0E,        //   Unit Exponent (-2)
    0x65, 0x33,        //   Unit (System: English Linear, Length: Inch)
    0x09, 0x30,        //   Usage (X)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0xB5, 0x04,  //   Physical Maximum (1205)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x46, 0x8A, 0x03,  //   Physical Maximum (906)
    0x09, 0x31,        //   Usage (Y)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0x05, 0x0D,        //   Usage Page (Digitizer)
    0x09, 0x54,        //   Usage (Contact count)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x08,        //   Report ID (8)
    0x09, 0x55,        //   Usage (0x55)
    0x25, 0x05,        //   Logical Maximum (5)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
#endif // LE_HID_DIGITIZER
};

//----------------------------------------------------------------------------
//
// Device Information service
static const uint8_t device_info_service_primay_uuid[2] = {0x0a, 0x18};
static const gatts_uuid_base_st device_info_primary_base = {
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = device_info_service_primay_uuid,
};

// PnP ID
static const uint8_t pnp_id_uuid[2] = {0x50, 0x2a};
static const gatts_uuid_base_st uuid_pnp_id_base = {
    .props = ATT_READ | ATT_INDICATE,
    .type  = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid  = pnp_id_uuid,
};

const uint8_t pnp_info[7] = { 0x01, 0x05, 0x0e, 0x00, 0x0a, 0x02, 0x40};
static int pnp_id_read_callback(uint16_t con_handle, uint16_t attribute_handle, uint32_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    uint8_t pnp_id_len = sizeof(pnp_info);
    const uint8_t *p_pnp_id = pnp_info;

    if (buffer) {
        if (pnp_id_len < buffer_size) {
            pnp_id_len = buffer_size;
         }
        memcpy(buffer, p_pnp_id, pnp_id_len);
        return pnp_id_len;
    }
    return 0;
}

static ble_gatt_characteristic_cb_info_t pnp_id_read_base_cb_info = {
    .value_len = sizeof(pnp_info),
    .att_read_callback_func = pnp_id_read_callback,
};

//----------------------------------------------------------------------------
//
//Human Interface Device(HID) service
const uint8_t hid_service_primary_uuid16[2]={0x12, 0x18};
static const gatts_uuid_base_st uuid_hid_service_primary_base = {
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = hid_service_primary_uuid16,
};

//hid information
const uint8_t hid_information_uuid16[2]={0x4a, 0x2a};
static const gatts_uuid_base_st uuid_hid_information_base = {
    .props = ATT_READ| ATT_DYNAMIC,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = hid_information_uuid16,
};

//hid report map
const uint8_t hid_report_map_uuid16[2]={0x4b, 0x2a};
static const gatts_uuid_base_st uuid_hid_report_map_base = {
    .props = ATT_READ | ATT_DYNAMIC | 0xF000,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = hid_report_map_uuid16,
};

//hid control point
const uint8_t hid_control_point_uuid16[2]={0x4c, 0x2a};
static const gatts_uuid_base_st uuid_hid_control_point_base = {
    .props = ATT_WRITE,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = hid_control_point_uuid16,
};

//hid input report
const uint8_t hid_input_uuid16[2]={0x4d,0x2a};
static const gatts_uuid_base_st uuid_hid_input_base = {
    .props = ATT_NOTIFY | ATT_READ,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = hid_input_uuid16,
};

static gatts_service_base_st hid_rpt1_gatts;
static gatts_service_base_st hid_rpt2_gatts;
static gatts_service_base_st hid_rpt3_gatts;
static gatts_service_base_st hid_rpt4_gatts;
static gatts_service_base_st hid_rpt5_gatts;

static const uint8_t hid_descriptor_data_01[]      = {0x0a, 0x00, 0x02, 0x00, 0x21, 0x00, 0x08, 0x29, 0x01, 0x01};
static const uint8_t hid_descriptor_data_02[]      = {0x0a, 0x00, 0x02, 0x00, 0x21, 0x00, 0x08, 0x29, 0x02, 0x01};
static const uint8_t hid_descriptor_data_03[]      = {0x0a, 0x00, 0x02, 0x00, 0x21, 0x00, 0x08, 0x29, 0x03, 0x01};
static const uint8_t hid_descriptor_data_04[]      = {0x0a, 0x00, 0x02, 0x00, 0x21, 0x00, 0x08, 0x29, 0x04, 0x01};
static const uint8_t hid_descriptor_data_05[]      = {0x0a, 0x00, 0x02, 0x00, 0x21, 0x00, 0x08, 0x29, 0x05, 0x01};

const u8 hid_info[]={0x11, 0x01, 0x00, 0x01};
static int hid_info_read_callback(uint16_t con_handle, uint16_t attribute_handle, uint32_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    u8 hid_info_len = sizeof(hid_info);
    const u8 *p_hid_info = hid_info;

    if (buffer) {
        if (hid_info_len < buffer_size) {
            hid_info_len = buffer_size;
         }
        memcpy(buffer, p_hid_info, hid_info_len);
        return hid_info_len;
    }
    return 0;
}

static ble_gatt_characteristic_cb_info_t hid_info_read_base_cb_info = {
    .value_len = sizeof(hid_info),
    .att_read_callback_func = hid_info_read_callback,
};

static int report_map_read_callback(uint16_t con_handle, uint16_t attribute_handle, uint32_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    u8 hid_report_map_len = 0;
    const u8 *p_hid_report_map;
    bool is_ios = ble_hid_peer_device_is_ios();

    if (is_ios) {
        hid_report_map_len = sizeof(report_map_ios);
        p_hid_report_map = report_map_ios;
    }
    else {
        hid_report_map_len = sizeof(report_map_android);
        p_hid_report_map = report_map_android;
    }

     if (buffer) {
        if (hid_report_map_len < buffer_size) {
            hid_report_map_len = buffer_size;
            }
            memcpy(buffer, p_hid_report_map, hid_report_map_len);
            return hid_report_map_len;
        }
    return 0;
}

int hid_report_write_callback(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *buffer, uint16_t buffer_size)
{
    TRACE("hid_report_write_callback 0x%04x data[%d]: ", handle, buffer_size);
    TRACE_R(buffer, buffer_size);

    if(handle == hid_rpt1_gatts.handle ){
        ble_update_conn_param(9, 12, 500);  //11.25ms
    } else if (handle == hid_rpt5_gatts.handle){
        ble_update_conn_param(9, 12, 500);  //11.25ms
        ble_hid_tiktok_mouse_location_init();
    } else if (handle == hid_rpt2_gatts.handle){
        ble_update_conn_param(9, 12, 500);  //11.25ms
    }
    return 0;
}

static ble_gatt_characteristic_cb_info_t report_map_read_base_cb_info = {
    .att_read_callback_func = report_map_read_callback,
};

static ble_gatt_characteristic_cb_info_t hid_input_base_cb_info = {
    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
    .att_write_callback_func = hid_report_write_callback,
};

int ble_hid_service_init(void)
{
    int ret = BLE_GATTS_SUCCESS;

    memset(&ble_hid_tasks, 0x00, sizeof(ble_hid_tasks_typedef));

    //device infomation service
    ret = ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                device_info_primary_base.uuid,
                                device_info_primary_base.type,
                                NULL);            //PRIMARY

    ret = ble_gatts_characteristic_add(uuid_pnp_id_base.uuid,
                                       uuid_pnp_id_base.type,
                                       uuid_pnp_id_base.props,
                                       NULL,
                                       &pnp_id_read_base_cb_info);      //characteristic---2a50:pnp_id

    //hid service
    ret = ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                uuid_hid_service_primary_base.uuid,
                                uuid_hid_service_primary_base.type,
                                NULL);            //PRIMARY

    ret = ble_gatts_characteristic_add(uuid_hid_information_base.uuid,
                                       uuid_hid_information_base.type,
                                       uuid_hid_information_base.props,
                                       NULL,
                                       &hid_info_read_base_cb_info);        //characteristic---2a4a:hid_information

    ret = ble_gatts_characteristic_add(uuid_hid_report_map_base.uuid,
                                       uuid_hid_report_map_base.type,
                                       uuid_hid_report_map_base.props,
                                       NULL,
                                       &report_map_read_base_cb_info);      //characteristic---2a4b:report_map

    ret = ble_gatts_characteristic_add(uuid_hid_control_point_base.uuid,
                                       uuid_hid_control_point_base.type,
                                       uuid_hid_control_point_base.props,
                                       NULL,
                                       NULL);                               //characteristic---2a4c:control_point

    ret = ble_gatts_characteristic_add(uuid_hid_input_base.uuid,
                                       uuid_hid_input_base.type,
                                       uuid_hid_input_base.props,
                                       &hid_rpt1_gatts.handle,
                                       &hid_input_base_cb_info);            //characteristic---2a4d_01_digitizer

    ret = ble_gatts_attribute_add(hid_descriptor_data_01, NULL);            //---2a4d_report_reference_01

    ret = ble_gatts_characteristic_add(uuid_hid_input_base.uuid,
                                       uuid_hid_input_base.type,
                                       uuid_hid_input_base.props,
                                       &hid_rpt2_gatts.handle,
                                       &hid_input_base_cb_info);            //characteristic---2a4d_02_keyboard

    ret = ble_gatts_attribute_add(hid_descriptor_data_02, NULL);            //---2a4d_report_reference_02

    ret = ble_gatts_characteristic_add(uuid_hid_input_base.uuid,
                                       uuid_hid_input_base.type,
                                       uuid_hid_input_base.props,
                                       &hid_rpt3_gatts.handle,
                                       &hid_input_base_cb_info);            //characteristic---2a4d_03_consumer

    ret = ble_gatts_attribute_add(hid_descriptor_data_03, NULL);            //---2a4d_report_reference_03

    ret = ble_gatts_characteristic_add(uuid_hid_input_base.uuid,
                                       uuid_hid_input_base.type,
                                       uuid_hid_input_base.props,
                                       &hid_rpt4_gatts.handle,
                                      &hid_input_base_cb_info);             //characteristic---2a4d_04_mouse_control_report

    ret = ble_gatts_attribute_add(hid_descriptor_data_04, NULL);            //---2a4d_report_reference_04

    ret = ble_gatts_characteristic_add(uuid_hid_input_base.uuid,
                                       uuid_hid_input_base.type,
                                       uuid_hid_input_base.props,
                                       &hid_rpt5_gatts.handle,
                                       &hid_input_base_cb_info);            //characteristic---2a4d_05_mouse_location_report

    ret = ble_gatts_attribute_add(hid_descriptor_data_05, NULL);            //---2a4d_report_reference_05

    return ret;
}

bool ble_hid_task_enqueue(BLE_HID_CMD_ID_TYPEDEF id, u8 *buffer, u8 len)
{
    u8 next_head = (ble_hid_tasks.head_idx + 1) % BLE_HID_CMD_QUEUE_SIZE;

    if (next_head == ble_hid_tasks.rear_idx) {
        /* Queue is full, do nothing. */
        return false;
    } else {
        ble_hid_tasks.queue[ble_hid_tasks.head_idx].id = id;
        ble_hid_tasks.queue[ble_hid_tasks.head_idx].len = len;
        ble_hid_tasks.queue[ble_hid_tasks.head_idx].delay_ms = 30;
        memcpy(ble_hid_tasks.queue[ble_hid_tasks.head_idx].buffer, buffer, len);
        ble_hid_tasks.head_idx = next_head;
        ble_hid_tasks.vaild_flag = true;
    }

    return true;
}

bool ble_hid_task_enqueue_delay(BLE_HID_CMD_ID_TYPEDEF id, u8 *buffer, u8 len, u8 delay)
{
    u8 next_head = (ble_hid_tasks.head_idx + 1) % BLE_HID_CMD_QUEUE_SIZE;

    if (next_head == ble_hid_tasks.rear_idx) {
		printf(" >>>> Queue is full, do nothing\n");
        /* Queue is full, do nothing. */
        return false;
    } else {
        ble_hid_tasks.queue[ble_hid_tasks.head_idx].id = id;
        ble_hid_tasks.queue[ble_hid_tasks.head_idx].len = len;
        ble_hid_tasks.queue[ble_hid_tasks.head_idx].delay_ms = delay;
        memcpy(ble_hid_tasks.queue[ble_hid_tasks.head_idx].buffer, buffer, len);
        ble_hid_tasks.head_idx = next_head;
        ble_hid_tasks.vaild_flag = true;
    }

    return true;
}

ALIGNED(128)
uint ble_hid_task_get_cnt(void)
{
    uint cnt = BLE_HID_CMD_QUEUE_SIZE;

    GLOBAL_INT_DISABLE();
    if(ble_hid_tasks.vaild_flag) {
        cnt = (ble_hid_tasks.head_idx + BLE_HID_CMD_QUEUE_SIZE - ble_hid_tasks.rear_idx) % BLE_HID_CMD_QUEUE_SIZE;
    }
    GLOBAL_INT_RESTORE();

    return cnt;
}

// 循环中不断执行
void ble_hid_service_proc(void)
{
    static u32 delay_cnt;
    int ret = 0;
//    printf(" >>>>  %s--%d \n",__func__,__LINE__);
    if (ble_hid_tasks.head_idx != ble_hid_tasks.rear_idx) {
//		printf(" >>>>  %s--%d \n",__func__,__LINE__);
        ble_hid_cmd_typedef *ble_hid_cmd = &ble_hid_tasks.queue[ble_hid_tasks.rear_idx];
        if (tick_check_expire(delay_cnt, ble_hid_cmd->delay_ms)) {
            if(ble_hid_tasks.head_idx == (ble_hid_tasks.rear_idx + 1)%30) {
                printf("D(%d) \n", tick_get() - delay_cnt);
            } else {
                printf("C(%d) \n", tick_get() - delay_cnt);
            }

            switch (ble_hid_cmd->id) {
#if LE_HID_CONSUMER
            case BLE_HID_CMD_ID_CONSUMER:
                TRACE("[TASK] consumer report:");
                TRACE_R(ble_hid_cmd->buffer, ble_hid_cmd->len);
                ret = ble_tx_notify(hid_rpt3_gatts.handle, ble_hid_cmd->buffer, ble_hid_cmd->len);
                break;
#endif

#if LE_HID_DIGITIZER
            case BLE_HID_CMD_ID_DIGITIZER:
                TRACE("[TASK] digitizer report:");
                TRACE_R(ble_hid_cmd->buffer, ble_hid_cmd->len);
                ret = ble_tx_notify(hid_rpt1_gatts.handle, ble_hid_cmd->buffer, ble_hid_cmd->len);
                break;
#endif

#if LE_HID_MOUSE
            case BLE_HID_CMD_ID_MOUSE:
                TRACE("[TASK] mouse report:");
                TRACE_R(ble_hid_cmd->buffer, ble_hid_cmd->len);

                //flag
                if (ble_hid_cmd->buffer[0] == 1) {
                    //ble_mouse_report_location_typedef
                    ret = ble_tx_notify(hid_rpt5_gatts.handle, &ble_hid_cmd->buffer[4], 3);
                } else {
                    //ble_mouse_report_control_typedef
                    ret = ble_tx_notify(hid_rpt4_gatts.handle, &ble_hid_cmd->buffer[1], 3);
                }
            break;

#endif

#if LE_HID_KEYBOARD
            case BLE_HID_CMD_ID_KEYBOARD:
                TRACE("[TASK] keyboard report:");
                TRACE_R(ble_hid_cmd->buffer, ble_hid_cmd->len);
                ret = ble_tx_notify(hid_rpt2_gatts.handle, ble_hid_cmd->buffer, ble_hid_cmd->len);
                break;
#endif

            default:
                break;
            }

            if(ret != 0x57) { //0x57=ACL_BUFFERS_FULL
                delay_cnt = tick_get();
                ble_hid_tasks.rear_idx = (ble_hid_tasks.rear_idx + 1) % BLE_HID_CMD_QUEUE_SIZE;
                if(ble_hid_tasks.rear_idx == ble_hid_tasks.head_idx) {
                    ble_hid_tasks.vaild_flag = false;
                }
            } else {
                delay_cnt = tick_get() - (BLE_HID_POLLING_INTERVAL_MS-1);
            }
        }
    }else{
//		printf(" >>>> head_idx: %d rear_idx --%d \n",ble_hid_tasks.head_idx,ble_hid_tasks.rear_idx);

	}
}

#endif
