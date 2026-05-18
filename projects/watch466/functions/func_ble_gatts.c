/**
 * @file func_ble_gatts.c
 * @brief BLE GATT Server Demo — 演示自定义 GATT Service 的完整生命周期
 *
 * 功能概述：
 *   - 注册一个 Primary GATT Service (UUID: 0xFF20)
 *   - 包含 Read (0xFF21)、Write (0xFF22)、Notify (0xFF23) 三个 Characteristic
 *   - 屏幕显示 BLE 连接状态、接收数据、Notify 发送计数
 *   - 按钮触发 Notify 发送
 *
 * 参照：platform/bsp/bsp_ble/app_ab_link.c 的 GATTS 注册模式
 */

#include "include.h"
#include "func.h"

#if FUNC_BLE_GATTS_EN

//=============================================================================
// 组件 ID
//=============================================================================
enum {
    COMPO_ID_TXT_STATUS = 1,    // BLE 连接状态文本
    COMPO_ID_TXT_DATA,          // 接收数据文本
    COMPO_ID_TXT_COUNT,         // Notify 发送计数文本
    COMPO_ID_BTN_SEND,          // 发送按钮
};

//=============================================================================
// BLE GATTS UUID 定义（参照 app_ab_link.c 模式）
//=============================================================================

// Demo Service UUID: 0xFF20
static const uint8_t demo_service_uuid16[2] = {0x20, 0xFF};
static const gatts_uuid_base_st uuid_demo_service_base = {
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = demo_service_uuid16,
};

// Read Characteristic UUID: 0xFF21
static const uint8_t demo_read_uuid16[2] = {0x21, 0xFF};
static const gatts_uuid_base_st uuid_demo_read_base = {
    .props = ATT_READ | ATT_DYNAMIC,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = demo_read_uuid16,
};

// Write Characteristic UUID: 0xFF22
static const uint8_t demo_write_uuid16[2] = {0x22, 0xFF};
static const gatts_uuid_base_st uuid_demo_write_base = {
    .props = ATT_WRITE,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = demo_write_uuid16,
};

// Notify Characteristic UUID: 0xFF23
static const uint8_t demo_notify_uuid16[2] = {0x23, 0xFF};
static const gatts_uuid_base_st uuid_demo_notify_base = {
    .props = ATT_NOTIFY | ATT_DYNAMIC,
    .type = BLE_GATTS_UUID_TYPE_16BIT,
    .uuid = demo_notify_uuid16,
};

// Notify Characteristic 的 handle 存储
static gatts_service_base_st gatts_demo_notify_handle;

//=============================================================================
// Notify TX Pool（参照 app_ab_link.c 的分配模式）
//=============================================================================
#define DEMO_NOTIFY_NUM         4
#define DEMO_NOTIFY_LEN         20
#define DEMO_NOTIFY_POOL_SIZE   (DEMO_NOTIFY_LEN + sizeof(struct txbuf_tag)) * DEMO_NOTIFY_NUM

// 注意：不使用 AT(.ble_cache.att)，因为在 APP_BLUE_FIT 模式下 ble_cache 放入 bram（仅 40K），
// 此处改用默认 bss 段（cram），避免 bram 溢出
static uint8_t demo_notify_tx_pool[DEMO_NOTIFY_POOL_SIZE];

//=============================================================================
// 回调信息结构（参照 app_ab_link.c）
//=============================================================================

// Read 回调前置声明
static int gatt_callback_demo_read(uint16_t con_handle, uint16_t handle, uint32_t offset, uint8_t *ptr, uint16_t len);

// Write 回调前置声明
static int gatt_callback_demo_write(uint16_t con_handle, uint16_t handle, uint32_t offset, uint8_t *ptr, uint16_t len);

// Read Characteristic 回调信息
static ble_gatt_characteristic_cb_info_t gatts_demo_read_cb_info = {
    .att_read_callback_func = gatt_callback_demo_read,
};

// Write Characteristic 回调信息
static ble_gatt_characteristic_cb_info_t gatts_demo_write_cb_info = {
    .att_write_callback_func = gatt_callback_demo_write,
};

// Notify Characteristic 回调信息
static ble_gatt_characteristic_cb_info_t gatts_demo_notify_cb_info = {
    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
};

//=============================================================================
// Write 数据环形缓冲区（参照 app_ab_link.c 的 ble_cmd_cb_t 模式）
//=============================================================================
#define DEMO_RX_BUF_LEN         20
#define DEMO_RX_BUF_SIZE        4
#define DEMO_RX_BUF_MASK        (DEMO_RX_BUF_SIZE - 1)

struct demo_rx_cmd_t {
    u8 len;
    u8 buf[DEMO_RX_BUF_LEN];
};

struct demo_rx_cb_t {
    struct demo_rx_cmd_t cmd[DEMO_RX_BUF_SIZE];
    u8 rptr;
    u8 wptr;
    bool dirty;     // 有新数据标志
};

static struct demo_rx_cb_t demo_rx_cb;

//=============================================================================
// Read 回调实现
//=============================================================================
static const char demo_read_data[] = "BLE GATTS Demo v1.0";

static int gatt_callback_demo_read(uint16_t con_handle, uint16_t handle, uint32_t offset, uint8_t *ptr, uint16_t len)
{
    printf("GATTS_Demo Read: handle=0x%04x, offset=%d, req_len=%d\n", handle, (int)offset, (int)len);

    // 返回设备状态字符串
    u32 data_len = strlen(demo_read_data);
    if (offset < data_len) {
        u32 remain = data_len - offset;
        u32 copy_len = (remain < len) ? remain : len;
        memcpy(ptr, &demo_read_data[offset], copy_len);
        printf("GATTS_Demo Read resp: \"%s\", copy=%d\n", &demo_read_data[offset], (int)copy_len);
        return copy_len;
    }
    printf("GATTS_Demo Read: offset beyond data, return 0\n");
    return 0;
}

//=============================================================================
// Write 回调实现
//=============================================================================
static int gatt_callback_demo_write(uint16_t con_handle, uint16_t handle, uint32_t offset, uint8_t *ptr, uint16_t len)
{
    printf("GATTS_Demo Write: handle=0x%04x, len=%d, data:\n", handle, (int)len);
    print_r(ptr, len);

    // 存入环形缓冲区（蓝牙线程上下文，不操作 GUI）
    u8 wptr = demo_rx_cb.wptr & DEMO_RX_BUF_MASK;
    demo_rx_cb.wptr++;

    // 截断到缓冲区容量
    u8 copy_len = (len > DEMO_RX_BUF_LEN) ? DEMO_RX_BUF_LEN : (u8)len;
    memcpy(demo_rx_cb.cmd[wptr].buf, ptr, copy_len);
    demo_rx_cb.cmd[wptr].len = copy_len;
    demo_rx_cb.dirty = true;

    return 0;
}

//=============================================================================
// Notify 发送接口
//=============================================================================
bool ble_gatts_demo_send_notify(u8 *buf, u16 len)
{
    if (!ble_is_connect()) {
        printf("GATTS_Demo Notify FAIL: not connected\n");
        return false;
    }
    printf("GATTS_Demo Notify: handle=0x%04x, len=%d\n", gatts_demo_notify_handle.handle, (int)len);
    int ret = ble_tx_notify(gatts_demo_notify_handle.handle, buf, len);
    if (ret != 0) {
        printf("GATTS_Demo Notify FAIL: ret=%d\n", ret);
    }
    return (ret == 0);
}

//=============================================================================
// GATT Service 初始化（由 ble_init_att() 调用）
//=============================================================================
void ble_gatts_demo_service_init(void)
{
    int ret = 0;

    printf("GATTS_Demo: service init start\n");

    // 初始化 Notify TX Pool
    ble_txpkt_init_with_pool(ble_send_kick, demo_notify_tx_pool, DEMO_NOTIFY_NUM, DEMO_NOTIFY_LEN);
    printf("GATTS_Demo: TX pool init OK (%d x %d)\n", DEMO_NOTIFY_NUM, DEMO_NOTIFY_LEN);

    // 初始化 Write 接收缓冲区
    memset(&demo_rx_cb, 0, sizeof(demo_rx_cb));

    // 注册 Demo Service (UUID: 0xFF20)
    ret |= ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                  uuid_demo_service_base.uuid,
                                  uuid_demo_service_base.type,
                                  NULL);
    printf("GATTS_Demo: service add (0xFF20) ret=%d\n", ret);

    // 注册 Read Characteristic (UUID: 0xFF21, ATT_READ | ATT_DYNAMIC)
    ret |= ble_gatts_characteristic_add(uuid_demo_read_base.uuid,
                                         uuid_demo_read_base.type,
                                         uuid_demo_read_base.props,
                                         NULL,
                                         &gatts_demo_read_cb_info);
    printf("GATTS_Demo: read char add (0xFF21) ret=%d\n", ret);

    // 注册 Write Characteristic (UUID: 0xFF22, ATT_WRITE)
    ret |= ble_gatts_characteristic_add(uuid_demo_write_base.uuid,
                                         uuid_demo_write_base.type,
                                         uuid_demo_write_base.props,
                                         NULL,
                                         &gatts_demo_write_cb_info);
    printf("GATTS_Demo: write char add (0xFF22) ret=%d\n", ret);

    // 注册 Notify Characteristic (UUID: 0xFF23, ATT_NOTIFY | ATT_DYNAMIC)
    ret |= ble_gatts_characteristic_add(uuid_demo_notify_base.uuid,
                                         uuid_demo_notify_base.type,
                                         uuid_demo_notify_base.props,
                                         &gatts_demo_notify_handle.handle,
                                         &gatts_demo_notify_cb_info);
    printf("GATTS_Demo: notify char add (0xFF23) ret=%d, handle=0x%04x\n", ret, gatts_demo_notify_handle.handle);

    if (ret != BLE_GATTS_SUCCESS) {
        printf("GATTS_Demo: init FAILED, err=%d\n", ret);
    } else {
        printf("GATTS_Demo: init OK, notify_handle=0x%04x\n", gatts_demo_notify_handle.handle);
    }
}

//=============================================================================
// 屏幕 UI — 控制块
//=============================================================================
typedef struct f_ble_gatts_t_ {
    u32 notify_count;       // Notify 发送计数
    bool not_connected_tip; // "未连接" 提示闪烁状态
    u8 tip_timer;           // 提示计时器
} f_ble_gatts_t;

//=============================================================================
// 屏幕创建
//=============================================================================
compo_form_t *func_ble_gatts_form_create(void)
{
    // 新建窗体
    compo_form_t *frm = compo_form_create(true);

    // 设置标题栏
    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, "BLE GATTS Demo");

    // 连接状态文本
    compo_textbox_t *txt_status = compo_textbox_create(frm, 20);
    compo_setid(txt_status, COMPO_ID_TXT_STATUS);
    compo_textbox_set_pos(txt_status, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 120);
    compo_textbox_set(txt_status, "Disconnected");

    // 接收数据文本
    compo_textbox_t *txt_data = compo_textbox_create(frm, 30);
    compo_setid(txt_data, COMPO_ID_TXT_DATA);
    compo_textbox_set_pos(txt_data, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 40);
    compo_textbox_set(txt_data, "No Data");

    // Notify 发送计数文本
    compo_textbox_t *txt_count = compo_textbox_create(frm, 20);
    compo_setid(txt_count, COMPO_ID_TXT_COUNT);
    compo_textbox_set_pos(txt_count, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 40);
    compo_textbox_set(txt_count, "Sent: 0");

    // 发送按钮
    compo_button_t *btn = compo_button_create_by_image(frm, UI_BUF_COMMON_BUTTON2_BIN);
    compo_setid(btn, COMPO_ID_BTN_SEND);
    compo_button_set_pos(btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 140);
    // 按钮上的文字
    compo_textbox_t *txt_btn = compo_textbox_create(frm, 10);
    compo_textbox_set_pos(txt_btn, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 140);
    compo_textbox_set(txt_btn, "Send Notify");

    return frm;
}

//=============================================================================
// 进入/退出回调
//=============================================================================
//进入 BLE GATTS Demo
void func_ble_gatts_enter(void)
{
    printf("GATTS_Demo: screen enter\n");
    func_cb.f_cb = func_zalloc(sizeof(f_ble_gatts_t));
    func_cb.frm_main = func_ble_gatts_form_create();
    printf("GATTS_Demo: ble_connected=%d\n", ble_is_connected());
}

//退出 BLE GATTS Demo
void func_ble_gatts_exit(void)
{
    printf("GATTS_Demo: screen exit\n");
    func_cb.last = FUNC_BLE_GATTS;
}

//=============================================================================
// Process 循环（每个 tick 调用）
//=============================================================================
void func_ble_gatts_process(void)
{
    func_process();

    f_ble_gatts_t *fst = (f_ble_gatts_t *)func_cb.f_cb;
    if (fst == NULL) {
        return;
    }

    // 更新连接状态
    compo_textbox_t *txt_status = compo_getobj_byid(COMPO_ID_TXT_STATUS);
    if (txt_status) {
        if (ble_is_connected()) {
            compo_textbox_set(txt_status, "Connected");
        } else {
            compo_textbox_set(txt_status, "Disconnected");
        }
    }

    // 检查是否有新的 Write 数据
    if (demo_rx_cb.rptr != demo_rx_cb.wptr) {
        u8 rptr = demo_rx_cb.rptr & DEMO_RX_BUF_MASK;
        demo_rx_cb.rptr++;

        u8 *ptr = demo_rx_cb.cmd[rptr].buf;
        u8 len = demo_rx_cb.cmd[rptr].len;

        printf("GATTS_Demo: process rx data, len=%d\n", len);

        // 更新接收数据文本（显示为 HEX）
        compo_textbox_t *txt_data = compo_getobj_byid(COMPO_ID_TXT_DATA);
        if (txt_data) {
            char hex_buf[DEMO_RX_BUF_LEN * 3 + 1];
            u32 pos = 0;
            for (u8 i = 0; i < len && pos < sizeof(hex_buf) - 4; i++) {
                pos += sprintf(&hex_buf[pos], "%02X ", ptr[i]);
            }
            hex_buf[pos] = '\0';
            compo_textbox_set(txt_data, hex_buf);
        }
    }

    // "未连接"提示闪烁（点击发送按钮但未连接时）
    if (fst->not_connected_tip) {
        fst->tip_timer++;
        if (fst->tip_timer > 30) {  // 约 300ms
            fst->not_connected_tip = false;
            fst->tip_timer = 0;
            compo_textbox_t *txt_count = compo_getobj_byid(COMPO_ID_TXT_COUNT);
            if (txt_count) {
                char buf[20];
                sprintf(buf, "Sent: %d", (int)fst->notify_count);
                compo_textbox_set(txt_count, buf);
            }
        }
    }
}

//=============================================================================
// 按钮点击处理
//=============================================================================
static void func_ble_gatts_button_click(void)
{
    int id = compo_get_button_id();
    printf("GATTS_Demo: button click, id=%d\n", id);
    f_ble_gatts_t *fst = (f_ble_gatts_t *)func_cb.f_cb;

    switch (id) {
    case COMPO_ID_BTN_SEND: {
        if (!ble_is_connected()) {
            // 未连接提示
            printf("GATTS_Demo: send btn clicked but not connected\n");
            fst->not_connected_tip = true;
            fst->tip_timer = 0;
            compo_textbox_t *txt_count = compo_getobj_byid(COMPO_ID_TXT_COUNT);
            if (txt_count) {
                compo_textbox_set(txt_count, "Not Connected!");
            }
            break;
        }

        // 构建 Notify 数据
        char notify_buf[32];
        int len = sprintf(notify_buf, "Notify #%d", (int)(fst->notify_count + 1));

        if (ble_gatts_demo_send_notify((u8 *)notify_buf, len)) {
            fst->notify_count++;
            printf("GATTS_Demo: notify sent OK, count=%d\n", (int)fst->notify_count);
            // 更新计数显示
            compo_textbox_t *txt_count = compo_getobj_byid(COMPO_ID_TXT_COUNT);
            if (txt_count) {
                char buf[20];
                sprintf(buf, "Sent: %d", (int)fst->notify_count);
                compo_textbox_set(txt_count, buf);
            }
        }
        break;
    }
    default:
        break;
    }
}

//=============================================================================
// 消息处理
//=============================================================================
void func_ble_gatts_message(size_msg_t msg)
{
    switch (msg) {
    case MSG_CTP_CLICK:
        func_ble_gatts_button_click();
        break;
    default:
        func_message(msg);
        break;
    }
}

//=============================================================================
// 主函数（由 func_tbl.h 的 tbl_func_entry 调用）
//=============================================================================
void func_ble_gatts(void)
{
    printf("GATTS_Demo: main loop start\n");
    func_ble_gatts_enter();
    while (func_cb.sta == FUNC_BLE_GATTS) {
        func_ble_gatts_process();
        func_ble_gatts_message(msg_dequeue());
    }
    func_ble_gatts_exit();
}

#endif // FUNC_BLE_GATTS_EN
