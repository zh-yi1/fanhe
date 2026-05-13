#include "include.h"
#include "app_ab_link.h"
#include "bt_fota.h"

#if (USE_APP_TYPE == APP_AB_LINK)
///////////////////////////////////////////////////////////////////////////

#define MAX_NOTIFY_NUM          10
#define MAX_NOTIFY_LEN          256     //max=247
#define NOTIFY_POOL_SIZE       (MAX_NOTIFY_LEN + sizeof(struct txbuf_tag)) * MAX_NOTIFY_NUM

#define BLE_CMD_BUF_LEN         4
#define BLE_CMD_BUF_MASK        (BLE_CMD_BUF_LEN - 1)
#define BLE_RX_BUF_LEN          20

struct ble_cmd_t {
    u8 len;
    u16 handle;
    u8 buf[BLE_RX_BUF_LEN];
};

struct ble_cmd_cb_t {
    struct ble_cmd_t cmd[BLE_CMD_BUF_LEN];
    u8 cmd_rptr;
    u8 cmd_wptr;
    bool wakeup;
};

static int gatt_callback_app(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len);

static const uint8_t adv_data_const[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x02,
    // Manufacturer Specific Data
    0x03, 0xff, 0x42, 0x06,
    // Complete Local Name
    0x09, 0xff, 'B', 'l', 'u', 'e', 't', 'r', 'u', 'm',
};

static const uint8_t scan_data_const[] = {
    // Name
    0x08, 0x09, 'B', 'L', 'E', '-', 'B', 'O', 'X',
};

//----------------------------------------------------------------------------
//app service
static const uint8_t app_primay_uuid16[2] = {0x12, 0xff};
static const gatts_uuid_base_st uuid_app_primay_base = {
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = app_primay_uuid16,
};

static const uint8_t app_write_uuid16[2] = {0x13, 0xff};
static const gatts_uuid_base_st uuid_app_write_base = {
    .props = ATT_WRITE,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = app_write_uuid16,
};

static const uint8_t app_notify_uuid16[2] = {0x14, 0xff};
static const gatts_uuid_base_st uuid_app_notify_base = {
    .props = ATT_NOTIFY,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = app_notify_uuid16,
};
static gatts_service_base_st gatts_app_notify_base;

static ble_gatt_characteristic_cb_info_t gatts_app_notify_cb_info = {
    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
};


static ble_gatt_characteristic_cb_info_t gatts_test_cb_info = {
    .att_write_callback_func = gatt_callback_app,
    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
};

static ble_gatt_characteristic_cb_info_t gatts_app_write_cb_info = {
    .att_write_callback_func = gatt_callback_app,
};

//----------------------------------------------------------------------------
//battery service
//static const uint8_t battery_primay_uuid16[2] = {0x0f, 0x18};
//static const gatts_uuid_base_st uuid_battery_primay_base = {
//    .type = BLE_GATTS_UUID_TYPE_16BIT,
//    .uuid = battery_primay_uuid16,
//};
//
//static const uint8_t battery_uuid16[2] = {0x19, 0x2a};
//static const gatts_uuid_base_st uuid_battery_base = {
//    .props = ATT_READ|ATT_NOTIFY,
//    .type = BLE_GATTS_UUID_TYPE_16BIT,
//    .uuid = battery_uuid16,
//};
//static gatts_service_base_st gatts_battery_base;

//----------------------------------------------------------------------------
//test service
static const uint8_t test_primay_uuid16[2] = {0x10, 0xff};
static const gatts_uuid_base_st uuid_test_primay_base = {
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = test_primay_uuid16,
};

static const uint8_t test_uuid16[2] = {0xf1, 0xff};
static const gatts_uuid_base_st uuid_test_base = {
    .props = ATT_NOTIFY|ATT_WRITE,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = test_uuid16,
};
static gatts_service_base_st gatts_tests_base;

//----------------------------------------------------------------------------
#if LE_AB_FOT_EN
static int gatt_callback_fota(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len)
{
    if (fot_app_connect_auth(ptr,len)) {
        fot_recv_proc(ptr,len);
    }
    return 0;
}

bool ble_fot_send_packet(u8 *buf, u8 len)
{
    return ble_tx_notify(gatts_app_notify_base.handle, buf, len);
}
#endif


#if LE_AB_FOT_EN
static const uint8_t fota_uuid16[2] = {0x15, 0xff};
static const gatts_uuid_base_st uuid_fota_base = {
    .props = ATT_READ|ATT_WRITE_WITHOUT_RESPONSE,
    .type  = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid  = fota_uuid16,
};

static ble_gatt_characteristic_cb_info_t gatts_fota_cb_info = {
    .att_write_callback_func = gatt_callback_fota,
};
#endif

AT(.ble_cache.att)
static uint8_t notify_tx_pool[NOTIFY_POOL_SIZE];

AT(.ble_cache.cmd)
static struct ble_cmd_cb_t ble_cmd_cb;

u32 ble_get_scan_data(u8 *scan_buf, u32 buf_size)
{
    memset(scan_buf, 0, buf_size);
    u32 data_len = sizeof(scan_data_const);
    memcpy(scan_buf, scan_data_const, data_len);

    //读取BLE配置的蓝牙名称
    int len;
    len = strlen(xcfg_cb.le_name);
    if (len > 0) {
        memcpy(&scan_buf[2], xcfg_cb.le_name, len);
        data_len = 2 + len;
        scan_buf[0] = len + 1;
    }
    return data_len;
}

u32 ble_get_adv_data(u8 *adv_buf, u32 buf_size)
{
    memset(adv_buf, 0, buf_size);
    u32 data_len = sizeof(adv_data_const);
    memcpy(adv_buf, adv_data_const, data_len);

//    //读取BLE配置的蓝牙名称
//    int len;
//    len = strlen(xcfg_cb.le_name);
//    if (len > 0) {
//        memcpy(&adv_buf[9], xcfg_cb.le_name, len);
//        adv_buf[7] = len + 1;
//        data_len = 9 + len;
//    }
    return data_len;
}

void ble_txpkt_init(void)
{
    ble_txpkt_init_with_pool(ble_send_kick, notify_tx_pool, MAX_NOTIFY_NUM, MAX_NOTIFY_LEN);
}

bool ble_app_ab_link_send_packet(u8 *buf, u16 len)
{
//    printf("%s:0x%x,%d\n", __func__, buf, len);
    return ble_tx_notify(gatts_tests_base.handle, buf, len);
}

//----------------------------------------------------------------------------
//app wirte callback
static int gatt_callback_app(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len)
{
    u8 wptr = ble_cmd_cb.cmd_wptr & BLE_CMD_BUF_MASK;

    printf("BLE_RX [%d]: \n", len);
    print_r(ptr, len);

    ble_cmd_cb.cmd_wptr++;
    if (len > BLE_RX_BUF_LEN) {
        len = BLE_RX_BUF_LEN;
    }
    memcpy(ble_cmd_cb.cmd[wptr].buf, ptr, len);
    ble_cmd_cb.cmd[wptr].len = len;
    ble_cmd_cb.cmd[wptr].handle = handle;
    ble_cmd_cb.wakeup = true;

    return 0;
}

//BTControl APP TX Test(仅用于测试)
static void ble_app_btcontrol_tx_test(void)
{
    int i;
    u8 ble_test_buf[66];
    for (i = 1; i < 66; i++) {
        ble_test_buf[i-1] = i;
    }
    for (i = 1; i < MAX_NOTIFY_LEN; i++) {
        if (i > 200) {
            break;
        }
        if (ble_is_connect()) {
            ble_test_buf[0] = i;
            ble_app_ab_link_send_packet(ble_test_buf, i);
            delay_5ms(8);
            WDT_CLR();
        }
    }
}

static void ble_app_ab_link_rx_callback(u8 *ptr, u16 size)
{
    if ((size == 4) && (ptr[0] == 0x01) && (ptr[1] == 0x06)) {
        ble_app_btcontrol_tx_test();
    } else {
        bt_spp_cmd_process(ptr, size, 1);
    }
}

void ble_app_ab_link_process(void)
{
    if (ble_cmd_cb.cmd_rptr == ble_cmd_cb.cmd_wptr) {
        ble_cmd_cb.wakeup = false;
        return;
    }
    u8 rptr = ble_cmd_cb.cmd_rptr & BLE_CMD_BUF_MASK;
    ble_cmd_cb.cmd_rptr++;
    u8 *ptr = ble_cmd_cb.cmd[rptr].buf;
    u8 len = ble_cmd_cb.cmd[rptr].len;
    u16 handle = ble_cmd_cb.cmd[rptr].handle;

    if (handle == gatts_tests_base.handle) {             //兼容旧版APP
        ble_app_ab_link_rx_callback(ptr, len);
    }
}

AT(.com_text.sleep.app.wakeup)
bool ble_app_ab_link_need_wakeup(void)
{
    return ble_cmd_cb.wakeup;
}



//----------------------------------------------------------------------------
static void ble_app_gatts_service_init(void)
{
    int ret = 0;

    ble_set_gap_name(xcfg_cb.le_name, strlen(xcfg_cb.le_name) + 1);
    //battery service
//    ret = ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
//                                uuid_battery_primay_base.uuid,
//                                uuid_battery_primay_base.type,
//                                NULL);            //PRIMARY
//
//    ret = ble_gatts_characteristic_add(uuid_battery_base.uuid,
//                                       uuid_battery_base.type,
//                                       uuid_battery_base.props,
//                                       &gatts_battery_base.handle,
//                                       NULL);      //characteristic

    //Test Service
    ret |= ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 uuid_test_primay_base.uuid,
                                 uuid_test_primay_base.type,
                                 NULL);            //PRIMARY

    ret |= ble_gatts_characteristic_add(uuid_test_base.uuid,
                                       uuid_test_base.type,
                                       uuid_test_base.props,
                                       &gatts_tests_base.handle,
                                       &gatts_test_cb_info);      //characteristic
    //APP Service
    ret |= ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 uuid_app_primay_base.uuid,
                                 uuid_app_primay_base.type,
                                 NULL);            //PRIMARY

    ret |= ble_gatts_characteristic_add(uuid_app_write_base.uuid,
                                        uuid_app_write_base.type,
                                        uuid_app_write_base.props,
                                        NULL,
                                        &gatts_app_write_cb_info);      //characteristic

    ret |= ble_gatts_characteristic_add(uuid_app_notify_base.uuid,
                                        uuid_app_notify_base.type,
                                        uuid_app_notify_base.props,
                                        &gatts_app_notify_base.handle,
                                        &gatts_app_notify_cb_info);      //characteristic

#if LE_AB_FOT_EN
    ret |= ble_gatts_characteristic_add(uuid_fota_base.uuid,
                                        uuid_fota_base.type,
                                        uuid_fota_base.props,
                                        NULL,
                                        &gatts_fota_cb_info);      //characteristic
#endif

    if (ret != BLE_GATTS_SUCCESS) {
        printf("gatt err: %d\n", ret);
        return;
    }
}

//----------------------------------------------------------------------------
void ble_app_ab_link_init(void)
{
    memset(&ble_cmd_cb, 0, sizeof(ble_cmd_cb));
    ble_app_gatts_service_init();
}

#endif
