#include "include.h"
#include "app_ab_link.h"

#if FUNC_LUNCHBOX_UART_EN && ELUNCHBOX_PANEL_EN
#include "home_ui_shared.h"
#endif

#if SECURITY_PAY_EN
#include "alipay_bind.h"
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)
///////////////////////////////////////////////////////////////////////////
#define AB_MATE_VID             2           //广播包协议版本号
#define AB_MATE_BID             0x000000    //代理商和客户ID，0表示原厂bluetrum
#define ADV_VID_POS             (4 + 3)
#define ADV_MAC_POS             (7 + 3)
#define ADV_FMASK_POS           (13 + 3)
#define ADV_BID_POS             (14 + 3)

/**
 * ble tx buf set
 */
#if SECURITY_PAY_EN
#define MAX_NOTIFY_NUM          4
#define MAX_NOTIFY_LEN          110    //max=247
#else
#define MAX_NOTIFY_NUM          4
#define MAX_NOTIFY_LEN          247    //max=247, 饭盒协议帧需82+字节
#endif
#define NOTIFY_POOL_SIZE       (MAX_NOTIFY_LEN + sizeof(struct txbuf_tag)) * MAX_NOTIFY_NUM

/**
 * ble rx buf set
 */
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
#define BLE_CMD_BUF_LEN         4
#else
#define BLE_CMD_BUF_LEN         6
#endif
#define BLE_CMD_BUF_MASK        (BLE_CMD_BUF_LEN - 1)
#define BLE_RX_BUF_LEN          256

struct ble_cmd_t {
    u8 len;
    u16 handle;                //哪个 GATT 特征值被写入
    u8 buf[BLE_RX_BUF_LEN];    //数据内容 BLE_RX_BUF_LEN --> 256
};

struct ble_cmd_cb_t {
    struct ble_cmd_t cmd[BLE_CMD_BUF_LEN];  // 环形缓冲区数组 (6个槽位)
    u8 cmd_rptr;                            // 读指针 (主循环消费)
    u8 cmd_wptr;                            // 写指针 (中断产生)
    bool wakeup;                            // 是否有新数据待处理
};

static int gatt_callback_app(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len);
#if LE_AB_FOT_EN
static int gatt_callback_fota(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len);
#endif
#if LE_SERVICE_CHANGED
static int gatt_service_changed_callback(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len);
#endif
#if FUNC_CAMERA_TRANS_EN
void func_camera_jpeg_rx(u8 *buf, u16 len);
#endif
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
void alipay_iot_socket_rx_callback(uint8_t *ptr, uint16_t len);
#endif

static const uint8_t adv_data_const[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x06,

    // Manufacturer Specific Data
    //len type  CID         VID   PID          MAC                                 FMASK  BID
    0x10, 0xff, 0x42, 0x06, AB_MATE_VID, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t scan_data_const_name[] = {
    // Complete Local Name
    0x0C, 0x09, 'B', 'T', '5', '6', '8', '0', '-', 'B', 'L', 'E',
};

static const uint8_t scan_data_const[] = {
    0x09, 0x16, 0x02, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t uart_service_primay_uuid128[16] = {
    0xfb, 0x34, 0x9b, 0x5f,
    0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00,
    0x30, 0xae, 0x00, 0x00
};

static const uint8_t tx_uuid128[16] = {
    0xfb, 0x34, 0x9b, 0x5f,
    0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00,
    0x04, 0xae, 0x00, 0x00
};

static const uint8_t rx_uuid128[16] = {
    0xfb, 0x34, 0x9b, 0x5f,
    0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00,
    0x03, 0xae, 0x00, 0x00
};

static const gatts_uuid_base_st uuid_tx_primay_base = {
    .type = BLE_GATTS_UUID_TYPE_128BIT,
    .uuid = uart_service_primay_uuid128,
};

static const gatts_uuid_base_st gatt_tx_base = {
    .props = ATT_NOTIFY,
    .type = BLE_GATTS_UUID_TYPE_128BIT,
    .uuid = tx_uuid128,
};

static const gatts_uuid_base_st gatt_rx_base = {
    .props = ATT_WRITE | ATT_WRITE_WITHOUT_RESPONSE,
    .type = BLE_GATTS_UUID_TYPE_128BIT,
    .uuid = rx_uuid128,
};

static gatts_service_base_st gatts_tx_base;
static gatts_service_base_st gatts_rx_base;

static ble_gatt_characteristic_cb_info_t gatts_app_protocol_rx_cb_info = {
    .att_write_callback_func = gatt_callback_app,
};

static ble_gatt_characteristic_cb_info_t gatts_app_protocol_tx_cb_info = {
    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
};

#if SECURITY_PAY_EN
//支付宝
static const uint8_t alipay_service_primay_uuid128[16] = {
    0xfb, 0x34, 0x9b, 0x5f,
    0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00,
    0x02, 0x38, 0x00, 0x00
};

static const uint8_t alipay_tx_uuid128[16] = {
    0xfb, 0x34, 0x9b, 0x5f,
    0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00,
    0x02, 0x4a, 0x00, 0x00
};

static gatts_service_base_st alipay_gatts_tx_base;
static gatts_service_base_st alipay_gatts_tx_base;

static ble_gatt_characteristic_cb_info_t gatts_alipay_cb_info = {
    .att_write_callback_func = gatt_callback_app,
};

static const gatts_uuid_base_st alipay_uuid_tx_primay_base = {
    .type = BLE_GATTS_UUID_TYPE_128BIT,
    .uuid = alipay_service_primay_uuid128,
};

static const gatts_uuid_base_st alipay_gatt_tx_base = {
    .props = ATT_READ|ATT_WRITE|ATT_NOTIFY,
    .type = BLE_GATTS_UUID_TYPE_128BIT,
    .uuid = alipay_tx_uuid128,
};
#endif

#if LE_AB_FOT_EN
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

//static ble_gatt_characteristic_cb_info_t gatts_app_notify_cb_info = {
//    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
//};

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

#if LE_SERVICE_CHANGED
//GATT primary
static const uint8_t gatt_primay_uuid16[2] = {0x01, 0x18};
static const gatts_uuid_base_st uuid_gatt_primay_base = {
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = gatt_primay_uuid16,
};

//GATT Service Changed
static const uint8_t service_changed_uuid16[2] = {0x05, 0x2a};
static const gatts_uuid_base_st gatt_service_changed_base = {
    .props = ATT_INDICATE,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = service_changed_uuid16,
};
static gatts_service_base_st gatts_service_changed_base;

static ble_gatt_characteristic_cb_info_t gatt_service_changed_base_cb_info = {
    .client_config = GATT_CLIENT_CONFIG_INDICATE,
    .att_read_callback_func = gatt_service_changed_callback,
};
#endif

AT(.ble_cache.att)
static uint8_t notify_tx_pool[NOTIFY_POOL_SIZE];

AT(.ble_cache.cmd)
static struct ble_cmd_cb_t ble_cmd_cb;

#if LE_AB_FOT_EN
u16 att_get_max_mtu(void)
{
    return 422;
}
#endif

u32 ble_get_scan_data(u8 *scan_buf, u32 buf_size)
{
    u8 ble_addr[6];
    memset(scan_buf, 0, buf_size);
    u32 data_len = sizeof(scan_data_const_name);
    memcpy(scan_buf, scan_data_const_name, data_len);

    //读取BLE配置的蓝牙名称
    int len_name;
    len_name = strlen(xcfg_cb.le_name);

    if (len_name > 0) {
        memcpy(&scan_buf[2], xcfg_cb.le_name, len_name);
        data_len = 2 + len_name;
        scan_buf[0] = len_name + 1;
    }

    memcpy(&scan_buf[data_len], scan_data_const, sizeof(scan_data_const));
    data_len += sizeof(scan_data_const);

    ble_get_local_bd_addr(ble_addr);
	memcpy(scan_buf + data_len - 6, ble_addr, 6);

//    printf("--->scan_buf:");
//    print_r(scan_buf, data_len);

    return data_len;
}

bool ble_change_name(char *le_name)
{
    char name_cache[32] = {0};
    uint8_t scan_data_len = 0;
    uint8_t scan_data[31] = {0};
    memcpy(name_cache, xcfg_cb.le_name, sizeof(xcfg_cb.le_name));
    memset(xcfg_cb.le_name, 0, sizeof(xcfg_cb.le_name));
    memcpy(xcfg_cb.le_name, le_name, strlen(le_name));
    scan_data_len = ble_get_scan_data(scan_data, 31);
    bool ret = ble_set_scan_rsp_data((uint8_t*)scan_data, scan_data_len);
    if (!ret) {
        printf("scan data over length!\n");
        memcpy(xcfg_cb.le_name, name_cache, sizeof(name_cache));
    }
    return ret;
}

u32 ble_get_adv_data(u8 *adv_buf, u32 buf_size)
{
//    printf("%s\n", __func__);
    u8 edr_addr[6];
    u32 data_len = sizeof(adv_data_const);
    u32 bid = AB_MATE_BID;

    memset(adv_buf, 0, buf_size);

    // get adv const
    memcpy(adv_buf, adv_data_const, data_len);

    // get mac addr
    ble_get_local_bd_addr(edr_addr);

    //广播包协议从版本1之后，经典蓝牙地址都做个简单的加密操作，不直接暴露地址
    if (AB_MATE_VID > 1) {
        for (u8 i = 0; i < 6; i++) {
            edr_addr[i] ^= 0xAD;
        }
    }

    memcpy(&adv_buf[ADV_MAC_POS], edr_addr, 6);
    memcpy(&adv_buf[ADV_BID_POS], &bid, 3);

//    printf("--->adv_buf:");
//    print_r(adv_buf, data_len);

    return data_len;
}

void ble_txpkt_init(void)
{
    ble_txpkt_init_with_pool(ble_send_kick, notify_tx_pool, MAX_NOTIFY_NUM, MAX_NOTIFY_LEN);
}

/**
 * @brief 通过 BLE Notify 发送数据帧（饭盒 & 普通双协议兼容）
 *
 * 双协议兼容处理：
 *   - 饭盒帧（0x55AA 开头）：帧头本身就是协议标识，不能覆写 buf[0]
 *   - 普通帧（非饭盒）：SDK 默认格式要求 buf[0] 存放序号(seq_num 0~15)
 *
 * 最终调用 ble_tx_notify() 通过 TX Characteristic（...ae04...）发出。
 *
 * @param buf  待发送的数据帧
 * @param len  帧长度
 * @return ble_tx_notify() 的返回值（0=成功）
 */
int app_protocol_tx(u8 *buf, u8 len)
{
    if (!ble_is_connect()) {
        return false;
    }

    // 饭盒协议帧(0x55AA)不覆写序号，保持帧头完整
    bool lunchbox_frame = false;
#if FUNC_LUNCHBOX_UART_EN
    if (buf[0] == 0x55 && buf[1] == 0xAA)
        lunchbox_frame = true;
#endif

#if FUNC_CAMERA_TRANS_EN
	if ((buf[0] != 0xaa) && (buf[1] != 55))
#endif
    if (!lunchbox_frame)
    {
        static u8 seq_num = 0;
        buf[0] = seq_num;
        seq_num++;
        if (seq_num > 0xf) {
            seq_num = 0;
        }
    }

    printf("BLE==>TX [%d]: ",len);
    print_r(buf, len);

    int ret = ble_tx_notify(gatts_tx_base.handle, buf, len); //给APP
    if (ret != 0) {
        printf("BLE==>TX FAILED, ret=%d, handle=0x%04x\n", ret, gatts_tx_base.handle);
    }
    return ret;
}

#if SECURITY_TRANSITCODE_EN
int alipay_iot_socket_tx(u8 *buf, u8 len)
{
    return ble_tx_notify(gatts_tx_base.handle, buf, len);
}
#endif

/**
 * @brief BLE Write 回调 — 手机往 RX Characteristic 写数据时触发
 *
 * 本函数在蓝牙中断上下文中执行，必须快速返回，不做耗时操作。
 * 策略：仅拷贝数据到环形缓冲区 ble_cmd_cb，标记 wakeup=true，
 *       由主循环中的 ble_app_watch_process() 取走并处理。
 *
 * @param con_handle  BLE 连接句柄（SDK 内部用）
 * @param handle      写入的特征值 handle（据此区分是饭盒通道还是支付宝等）
 * @param flag        操作标志（读/写/通知等）
 * @param ptr         手机发来的原始数据指针（比如 13 字节的饭盒协议帧）
 * @param len         数据长度（字节数）
 * @return 0
 */
static int gatt_callback_app(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len)
{
    u8 wptr = ble_cmd_cb.cmd_wptr & BLE_CMD_BUF_MASK;

//    printf("BLE_RX len[%d] handle[%d]\n", len, handle);
//    print_r(ptr, len);

    ble_cmd_cb.cmd_wptr++;                            //环形缓冲区写指针+1
    if (len > BLE_RX_BUF_LEN) {
        len = BLE_RX_BUF_LEN;
    }
    memcpy(ble_cmd_cb.cmd[wptr].buf, ptr, len);       //将发到单片机的数据拷贝过来
    ble_cmd_cb.cmd[wptr].len = len;                   //记录数据长度
    ble_cmd_cb.cmd[wptr].handle = handle;             // 记录是哪个特征值-->写入
    ble_cmd_cb.wakeup = true;                         //标记有新数据

    return 0;
}

/**
 * @brief BLE 数据路由分发 — 判断数据该走哪个协议处理
 *
 * 数据从 gatt_callback_app() 存入环形缓冲区，由主循环取出后交此函数分发。
 * 路由规则（按优先级）：
 *   1. 前两字节 0x55 0xAA → 饭盒协议帧 → lunchbox_ble_rx_handle()
 *   2. 相机模式帧         → func_camera_jpeg_rx()
 *   3. 支付宝模式帧       → alipay_iot_socket_rx_callback()
 *   4. 其他               → ble_uart_service_write()（透传串口）
 *
 * @param ptr  数据指针
 * @param len  数据长度
 */
static void ble_app_blue_fit_rx_callback(u8 *ptr, u16 len)
{
    //printf("BLE rx len=%d: %02x %02x %02x\n", len, ptr[0], ptr[1], ptr[2]);
//    print_r(ptr, len);

#if FUNC_LUNCHBOX_UART_EN
    // 饭盒协议帧：0x55AA 帧头，或缓冲区有待处理数据时继续路由
    // (文件发送时 BLE 栈按 MTU 分包，后续包不以 55 AA 开头，需依赖 pending 状态)
    if ((len >= 2 && ptr[0] == 0x55 && ptr[1] == 0xAA)
        || lunchbox_ble_rx_pending()) {
        lunchbox_ble_rx_handle(ptr, len);
        return;
    }
#endif

#if FUNC_CAMERA_TRANS_EN
	if (func_cb.sta == FUNC_CAMERA) {
		func_camera_jpeg_rx(ptr, len);
	} else
#endif
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
	if (func_cb.sta == FUNC_ALIPAY) {
        alipay_iot_socket_rx_callback(ptr, len);
	} else
#endif
	{
        ble_uart_service_write(ptr, len);
	}
}

#if SECURITY_PAY_EN
static void gatt_alipay_rx(u8 *cmd, u16 len)
{
    if (func_cb.sta == FUNC_ALIPAY) {
        printf("%s: addr: 0x%x, len: %d\n",__func__, cmd, len);
        print_r(cmd, len);
        reset_sleep_delay();
        alipay_ble_recv_data_handle(cmd, (u32)len);
    }
}

u8 gatt_alipay_tx(u8 *buf, u16 len)
{
    printf("%s: %d\n",__func__, len);
    if (!ble_is_connect() || func_cb.sta != FUNC_ALIPAY) {
        return false;
    }
    print_r(buf, len);
    int res = ble_tx_notify(alipay_gatts_tx_base.handle, buf, len);
    delay_5ms(30);
    if (res == 0) {
        printf("ble_tx_notify success\n");
    }
    u8 timeout_cnt = 0;
    //发送失败重发
    while (res != 0) {
        timeout_cnt++;
        WDT_CLR();
        res = ble_tx_notify(alipay_gatts_tx_base.handle, buf, len);
        delay_5ms(30);
        printf("ble_tx_notify retry: %d\n", res);
        if (timeout_cnt == 10) {
            printf("alipay ble tx error: %d\n", res);
            break;
        }
    }

    return res;
}

#endif

#if LE_AB_FOT_EN
static int gatt_callback_fota(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len)
{
    if(fot_app_connect_auth(ptr,len)){
        fot_recv_proc(ptr,len);
    }
    return 0;
}

bool ble_fot_send_packet(u8 *buf, u8 len)
{
    return ble_tx_notify(gatts_app_notify_base.handle, buf, len);
}
#endif

/**
 * @brief BLE 接收数据处理 — 主循环中调用，从环形缓冲区取出数据并分发
 *
 * 环形缓冲区原理（生产者-消费者模型）：
 *   - cmd_wptr（写指针）：gatt_callback_app() 写入后递增（中断上下文）
 *   - cmd_rptr（读指针）：本函数读取后递增（主循环上下文）
 *   - wptr == rptr → 缓冲区空，无新数据，直接返回
 *   - wptr != rptr → 有新数据，取出处理
 *
 * 根据 handle 区分数据来源：
 *   - gatts_rx_base.handle   → 饭盒协议通道 → ble_app_blue_fit_rx_callback()
 *   - alipay_gatts_tx_base   → 支付宝通道    → gatt_alipay_rx()
 */
//----------------------------------------------------------------------------
void ble_app_watch_process(void)
{
    if (ble_cmd_cb.cmd_rptr == ble_cmd_cb.cmd_wptr) {
        ble_cmd_cb.wakeup = false; //清除唤醒标志-->没数据
        return;
    }

    u8 rptr = ble_cmd_cb.cmd_rptr & BLE_CMD_BUF_MASK;  //&5等价于%6（取模运算），把不断递增的读指针映射回 0~5 的数组下标
    ble_cmd_cb.cmd_rptr++;
    u8 *ptr = ble_cmd_cb.cmd[rptr].buf;                //取出指向本次数据内容的指针
    u8 len = ble_cmd_cb.cmd[rptr].len;                 //取出数据长度（字节数）
    u16 handle = ble_cmd_cb.cmd[rptr].handle;          //取出特征值 handle —— 数据是从哪个 GATT Characteristic 写入的。

    if (handle == gatts_rx_base.handle) {              //判断通道
        ble_app_blue_fit_rx_callback(ptr, len);        // → 饭盒协议帧 0x55AA
    }
#if SECURITY_PAY_EN
    if (handle == alipay_gatts_tx_base.handle) {
        gatt_alipay_rx(ptr, len);
    }
#endif
}

AT(.com_text.sleep.app.wakeup)
bool ble_app_watch_need_wakeup(void)
{
    return false;
}

#if LE_SERVICE_CHANGED
static int gatt_service_changed_callback(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len)
{
    if(handle == gatts_service_changed_base.handle){
        if (GET_LE16(&ptr[0])) {
            uint8_t buff[]={0x01, 0x00, 0xff, 0xff};        //handle range : 0x0001 - 0xffff
            ble_tx_indication(gatts_service_changed_base.handle, buff, 4);
        }
    }
    return 0;
}
#endif

/**
 * @brief 注册 BLE GATT 服务 — 构建 MCU 的"属性表"
 *
 * GATT 层级结构（类比写字楼）：
 *   Profile（整栋楼）
 *   └── Service: 饭盒通信部（UUID ...ae30...）
 *       ├── Characteristic TX: MCU→手机 Notify（UUID ...ae04...）
 *       └── Characteristic RX: 手机→MCU Write  （UUID ...ae03...）
 *
 * 注册顺序必须严格（SDK 要求）：
 *   1. ble_gatts_service_add()          — 先创建 Service
 *   2. ble_gatts_characteristic_add()   — 添加 TX 特征值（ATT_NOTIFY）
 *   3. ble_gatts_characteristic_add()   — 添加 RX 特征值（ATT_WRITE）
 *
 * TX: ATT_NOTIFY → MCU 可主动向手机推送数据，手机订阅后生效
 * RX: ATT_WRITE | ATT_WRITE_WITHOUT_RESPONSE → 手机可写数据，不需 MCU 确认
 */
static void ble_app_gatts_service_init(void)
{
    int ret = 0;

    ble_set_gap_name(xcfg_cb.le_name, strlen(xcfg_cb.le_name) + 1);

    ret |= ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 uuid_tx_primay_base.uuid,
                                 uuid_tx_primay_base.type,
                                 NULL);                        //创建service

    ret |= ble_gatts_characteristic_add(gatt_tx_base.uuid,
                                         gatt_tx_base.type,
                                         gatt_tx_base.props,
                                         &gatts_tx_base.handle,
                                         &gatts_app_protocol_tx_cb_info);      //characteristic

    ret |= ble_gatts_characteristic_add(gatt_rx_base.uuid,
                                         gatt_rx_base.type,
                                         gatt_rx_base.props,
                                         &gatts_rx_base.handle,
                                         &gatts_app_protocol_rx_cb_info);      //characteristic

#if SECURITY_PAY_EN
    //alipay
    ret |= ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 alipay_uuid_tx_primay_base.uuid,
                                 alipay_uuid_tx_primay_base.type,
                                 NULL);

    ret |= ble_gatts_characteristic_add(alipay_gatt_tx_base.uuid,
                                        alipay_gatt_tx_base.type,
                                        alipay_gatt_tx_base.props,
                                        &alipay_gatts_tx_base.handle,
                                        &gatts_alipay_cb_info);      //characteristic
#endif

#if LE_SERVICE_CHANGED
    //GATT
    ret |= ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 uuid_gatt_primay_base.uuid,
                                 uuid_gatt_primay_base.type,
                                 NULL);            //PRIMARY

    ret |= ble_gatts_characteristic_add(gatt_service_changed_base.uuid,
                                        gatt_service_changed_base.type,
                                        gatt_service_changed_base.props,
                                        &gatts_service_changed_base.handle,
                                        &gatt_service_changed_base_cb_info);
#endif

#if LE_HID_EN
    ret |= ble_hid_service_init();
#endif

#if LE_AB_FOT_EN
    ret |= ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 uuid_app_primay_base.uuid,
                                 uuid_app_primay_base.type,
                                 NULL);            //PRIMARY

    ret |= ble_gatts_characteristic_add(uuid_app_write_base.uuid,
                                        uuid_app_write_base.type,
                                        uuid_app_write_base.props,
                                        NULL,
                                        NULL);      //characteristic

    ret |= ble_gatts_characteristic_add(uuid_app_notify_base.uuid,
                                        uuid_app_notify_base.type,
                                        uuid_app_notify_base.props,
                                        &gatts_app_notify_base.handle,
                                        NULL);      //characteristic

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
//
#if FUNC_LUNCHBOX_UART_EN
/**
 * @brief BLE 发送包装 — 饭盒协议层 → 蓝牙发送的桥梁
 *
 * 注册给饭盒协议层（lunchbox_ble_set_tx_fn），使 lb_send_frame()
 * 组帧后通过本函数走 BLE Notify。
 * 调用链: lb_send_frame() → lb_ble_tx_wrapper() → app_protocol_tx() → ble_tx_notify()
 */
static void lb_ble_tx_wrapper(u8 *data, u16 len) { app_protocol_tx(data, (u8)len); }
#endif

/**
 * @brief 蓝牙初始化入口 — 上电时调用一次
 *
 * 执行步骤：
 *   1. 读取芯片 MAC 地址
 *   2. 拼蓝牙广播名：AR0MA-NY_XXXX（XXXX = MAC 后 2 字节大写）
 *   3. 把名字写入扫描响应包 → 手机扫描时可见
 *   4. 注册 GATT Service + TX/RX Characteristic
 *   5. 告诉饭盒协议层"蓝牙通道可用"（注册 lb_ble_tx_wrapper）
 *
 * 此后 lb_send_frame() 组帧后通过 lb_ble_tx_wrapper → ble_tx_notify() 走蓝牙发出。
 */
void ble_app_watch_init(void)
{
    // 蓝牙名称: AR0MA-NY_xxxx (xxxx = MAC 后两字节大写十六进制)
    u8 ble_addr[6];
    char ble_name[16];
    ble_get_local_bd_addr(ble_addr);
    sprintf(ble_name, "AR0MA-NY_%02X%02X", ble_addr[4], ble_addr[5]);
    ble_change_name(ble_name);

    ble_app_gatts_service_init();
#if FUNC_LUNCHBOX_UART_EN
    lunchbox_ble_set_tx_fn(lb_ble_tx_wrapper);
#endif

    // 显式设置 BLE 空口地址为 flash 持久化的固定地址
    // 避免 SDK 库每次初始化时生成随机地址导致手机无法重连
    bt_ctrl0_msg(BT_CTL0_BLE_SET_BLE_ADDR);
}

/**
 * @brief 蓝牙断开连接回调 — 清除绑定状态
 */
void ble_app_watch_disconnect_callback(void)
{
    bind_sta_set(BIND_NULL);
#if FUNC_LUNCHBOX_UART_EN && ELUNCHBOX_PANEL_EN
    home_ui_shared_ble_link_notify();
#endif
}

/**
 * @brief 蓝牙连接成功回调 — 当前为空，可在此添加连接后的初始化操作
 */
void ble_app_watch_connect_callback(void)
{
#if FUNC_LUNCHBOX_UART_EN
    // BLE 连接成功后主动上报时间戳给 APP (蓝牙通讯协议1.0.8 §3.3)
    // 帧格式: 0x03 状态上报, DataPoint dpid=11(时间戳) value=4B Unix时间戳
    lunchbox_ble_on_connected();
#endif
#if FUNC_LUNCHBOX_UART_EN && ELUNCHBOX_PANEL_EN
    home_ui_shared_ble_link_notify();
#endif
}

/**
 * @brief 客户端配置变更回调 — 手机订阅/取消订阅 Notify 时触发
 *
 * @param handle  发生配置变更的特征值 handle
 * @param cfg     订阅状态：非 0 = 已订阅，0 = 取消订阅
 *
 * 手机订阅 TX Notify 后，MCU 才可以通过 ble_tx_notify() 向手机推送数据。
 */
void ble_app_watch_client_cfg_callback(u16 handle, u8 cfg)
{
    printf("BLE CCCD: handle=0x%04x, cfg=%d (TX handle=0x%04x)\n", handle, cfg, gatts_tx_base.handle);
    if (cfg) {
#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
        if (func_cb.sta != FUNC_ALIPAY)
#endif
        {
            // 饭盒项目不需要手表状态同步，关闭 ab_app_sync_info
            // ab_app_sync_info();
        }
    }
}

#endif
