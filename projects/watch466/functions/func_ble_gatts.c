/**
 * @file func_ble_gatts.c
 * @brief BLE GATT Server — 图片接收功能
 *
 * 功能概述：
 *   - 注册 128-bit UUID GATT Service（协议规范）
 *   - 实现 协议2 (CMD 0x01)：准备发送图片，打开文件
 *   - 实现 协议3 (CMD 0x02)：接收图片数据帧，直接写入 Flash FATFS
 *   - 实现 协议4 (CMD 0x03)：结束发送，关闭文件
 *   - 图片保存为 A:\HHMMSS.jpg（时间来自 rtc_clock_get）
 *
 * 协议参考：蓝牙通讯协议1.0.3_find_my.md
 * 参照：platform/bsp/bsp_ble/app_blue_fit.c 的 128-bit UUID 注册模式
 */

#include "include.h"
#include "func.h"

#if FUNC_BLE_GATTS_EN

//=============================================================================
// 组件 ID
//=============================================================================
enum {
    COMPO_ID_TXT_STATUS = 1,    // BLE 连接状态文本
    COMPO_ID_TXT_DATA,          // 传输状态/进度文本
    COMPO_ID_TXT_RESULT,        // 操作结果文本
};

//=============================================================================
// 协议常量
//=============================================================================
#define BLE_FRAME_HEADER            0x55AA
#define BLE_FRAME_HEADER_SIZE       2
#define BLE_FRAME_VERSION           0x00
#define BLE_FRAME_OVERHEAD          9       // 帧头2 + 版本1 + 消息标志1 + 命令字1 + 错误标志1 + 数据长度2 + 校验和1

#define BLE_CMD_PREPARE             0x01    // 协议2：准备发送图片
#define BLE_CMD_DATA                0x02    // 协议3：发送图片数据
#define BLE_CMD_FINISH              0x03    // 协议4：结束发送

#define BLE_IMG_TYPE_IMAGE          0x01    // 图片
#define BLE_IMG_TYPE_PERSONAL       0x02    // 个人信息（忽略）

//=============================================================================
// 传输状态枚举
//=============================================================================
typedef enum {
    BLE_IMG_STATE_IDLE = 0,     // 空闲
    BLE_IMG_STATE_READY,        // 已收到准备命令，文件已打开，等待数据
    BLE_IMG_STATE_RECEIVING,    // 正在接收数据帧并写入 Flash
} ble_img_state_t;

//=============================================================================
// 传输控制块（流式写入，不缓冲整个图片）
//=============================================================================
typedef struct {
    ble_img_state_t state;      // 当前状态
    u32 total_len;              // 图片总长度
    u32 total_packets;          // 总包数
    u8  img_type;               // 图片类型 (0x01=图片)
    u16 pixel_x;                // X 像素
    u16 pixel_y;                // Y 像素
    u32 received_bytes;         // 已接收字节数
    u16 next_seq;               // 下一个期望的帧序号
    FIL fp;                     // FATFS 文件句柄
    bool file_opened;           // 文件是否已打开
} ble_img_transfer_t;

//=============================================================================
// 帧结构（解析后的结果）
//=============================================================================
typedef struct {
    u8  version;
    u8  msg_flag;
    u8  cmd;
    u8  err_flag;
    u16 data_len;
    u8  *data;                  // 指向帧内数据区域的指针
} ble_frame_t;

//=============================================================================
// BLE GATTS UUID 定义（128-bit，参照 app_blue_fit.c 模式）
//=============================================================================

// 协议 Service UUID: 00010203-0405-0607-0809-0A0B0C0DFFE0
static const uint8_t img_service_uuid128[16] = {
    0xE0, 0xFF, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};
static const gatts_uuid_base_st uuid_img_service_base = {
    .type = BLE_GATTS_UUID_TYPE_128BIT,
    .uuid = img_service_uuid128,
};

// Write Characteristic UUID: 00010203-0405-0607-0809-0A0B0C0DFFE1
static const uint8_t img_write_uuid128[16] = {
    0xE1, 0xFF, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};
static const gatts_uuid_base_st uuid_img_write_base = {
    .props = ATT_WRITE | ATT_WRITE_WITHOUT_RESPONSE,
    .type = BLE_GATTS_UUID_TYPE_128BIT,
    .uuid = img_write_uuid128,
};

// Notify Characteristic UUID: 00010203-0405-0607-0809-0A0B0C0DFFEe
static const uint8_t img_notify_uuid128[16] = {
    0xEE, 0xFF, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};
static const gatts_uuid_base_st uuid_img_notify_base = {
    //.props = ATT_READ | ATT_NOTIFY | ATT_DYNAMIC,
    .props = ATT_NOTIFY,
    .type = BLE_GATTS_UUID_TYPE_128BIT,
    .uuid = img_notify_uuid128,
};

// Notify Characteristic 的 handle 存储
static gatts_service_base_st gatts_img_notify_handle;

//=============================================================================
// Notify TX Pool
//=============================================================================
#define IMG_NOTIFY_NUM          4
#define IMG_NOTIFY_LEN          256
#define IMG_NOTIFY_POOL_SIZE    (IMG_NOTIFY_LEN + sizeof(struct txbuf_tag)) * IMG_NOTIFY_NUM

static uint8_t img_notify_tx_pool[IMG_NOTIFY_POOL_SIZE];

//=============================================================================
// Write 回调环形缓冲区（2 个槽位，动态分配 cmd buffer）
//=============================================================================
#define IMG_RX_BUF_SIZE         2
#define IMG_RX_BUF_MASK         (IMG_RX_BUF_SIZE - 1)

struct img_rx_cmd_t {
    u16 len;
    u8  *buf;                   // 动态分配的 buffer
};

struct img_rx_cb_t {
    struct img_rx_cmd_t cmd[IMG_RX_BUF_SIZE];
    u8 rptr;
    u8 wptr;
    bool dirty;
};

static struct img_rx_cb_t img_rx_cb;

//=============================================================================
// 回调信息结构
//=============================================================================

static int gatt_callback_img_write(uint16_t con_handle, uint16_t handle, uint32_t offset, uint8_t *ptr, uint16_t len);

static ble_gatt_characteristic_cb_info_t gatts_img_write_cb_info = {
    .att_write_callback_func = gatt_callback_img_write,
};

static ble_gatt_characteristic_cb_info_t gatts_img_notify_cb_info = {
    .client_config = GATT_CLIENT_CONFIG_NOTIFY,
};

//=============================================================================
// 传输控制块实例
//=============================================================================
static ble_img_transfer_t img_xfer;

//=============================================================================
// 传输控制块重置
//=============================================================================
static void ble_img_xfer_reset(void)
{
    // 关闭已打开的文件
    if (img_xfer.file_opened) {
        fs_close(&img_xfer.fp);
        img_xfer.file_opened = false;
    }

    img_xfer.state = BLE_IMG_STATE_IDLE;
    img_xfer.total_len = 0;
    img_xfer.total_packets = 0;
    img_xfer.img_type = 0;
    img_xfer.pixel_x = 0;
    img_xfer.pixel_y = 0;
    img_xfer.received_bytes = 0;
    img_xfer.next_seq = 0;
}

//=============================================================================
// 帧校验和计算
//=============================================================================
static u8 ble_frame_checksum(const u8 *data, u16 len)
{
    u32 sum = 0;
    for (u16 i = 0; i < len; i++) {
        sum += data[i];
    }
    return (u8)(sum & 0xFF);
}

//=============================================================================
// 帧解析
//=============================================================================
static bool ble_frame_parse(const u8 *raw, u16 raw_len, ble_frame_t *frame)
{
    if (raw_len < BLE_FRAME_OVERHEAD) {
        printf("IMG: frame too short (%d)\n", raw_len);
        return false;
    }

    printf("%s: raw: %02x %02x %02x %02x %02x\n",
        __func__, raw[0], raw[1], raw[2], raw[3], raw[4]);

    u16 header = raw[0] | (raw[1] << 8);
    if (header != BLE_FRAME_HEADER) {
        printf("IMG: bad header 0x%04x\n", header);
        return false;
    }

    frame->version  = raw[2];
    frame->msg_flag = raw[3];
    frame->cmd      = raw[4];
    frame->err_flag = raw[5];
    frame->data_len = raw[6] | (raw[7] << 8);

    u16 expected_len = BLE_FRAME_OVERHEAD + frame->data_len;
    if (raw_len != expected_len) {
        printf("IMG: len mismatch, got %d, expected %d\n", raw_len, expected_len);
        return false;
    }

    u8 checksum = ble_frame_checksum(raw, raw_len - 1);
    if (checksum != raw[raw_len - 1]) {
        printf("IMG: checksum fail, calc=0x%02x, got=0x%02x\n", checksum, raw[raw_len - 1]);
        return false;
    }

    frame->data = (u8 *)&raw[8];
    return true;
}

//=============================================================================
// Notify 应答帧构建与发送
//=============================================================================
static bool ble_img_send_response(u8 cmd, u8 err_flag, const u8 *data, u16 data_len)
{
    if (!ble_is_connected()) {
        return false;
    }

    u16 frame_len = 8 + data_len + 1;
    if (frame_len > IMG_NOTIFY_LEN) {
        return false;
    }

    u8 buf[IMG_NOTIFY_LEN];
    buf[0] = 0xAA;
    buf[1] = 0x55;
    buf[2] = BLE_FRAME_VERSION;
    buf[3] = 0x00;
    buf[4] = cmd;
    buf[5] = err_flag;
    buf[6] = (u8)(data_len & 0xFF);
    buf[7] = (u8)((data_len >> 8) & 0xFF);

    if (data_len > 0 && data != NULL) {
        memcpy(&buf[8], data, data_len);
    }

    buf[8 + data_len] = ble_frame_checksum(buf, 8 + data_len);

    printf("IMG: resp cmd=0x%02x err=%d len=%d\n", cmd, err_flag, data_len);
    return ble_tx_notify(gatts_img_notify_handle.handle, buf, frame_len) == 0;
}

//=============================================================================
// 协议2 处理：CMD 0x01 准备发送图片 → 打开文件
//=============================================================================
static void ble_img_handle_prepare(const ble_frame_t *frame)
{
    printf("IMG: CMD 0x01 prepare, data_len=%d, state=%d\n", frame->data_len, img_xfer.state);

    // 数据格式：AAAA(2B) + BBBB(2B) + CC(1B) + XXXX(2B) + YYYY(2B) = 9 字节
    if (frame->data_len != 9) {
        printf("IMG: prepare data len error (%d)\n", frame->data_len);
        ble_img_send_response(BLE_CMD_PREPARE, 0x01, NULL, 0);
        return;
    }

    const u8 *d = frame->data;
    u32 total_len   = d[0] | (d[1] << 8);          // 2 字节总长度
    u32 total_pkts  = d[2] | (d[3] << 8);           // 2 字节包数
    u8  img_type    = d[4];
    u16 pixel_x     = d[5] | (d[6] << 8);
    u16 pixel_y     = d[7] | (d[8] << 8);

    printf("IMG: total=%lu, pkts=%lu, type=%d, %dx%d\n",
           total_len, total_pkts, img_type, pixel_x, pixel_y);

    if (img_type != BLE_IMG_TYPE_IMAGE) {
        printf("IMG: ignore type %d\n", img_type);
        return;
    }

    if (img_xfer.state != BLE_IMG_STATE_IDLE) {
        printf("IMG: busy, state=%d\n", img_xfer.state);
        ble_img_send_response(BLE_CMD_PREPARE, 0x04, NULL, 0);
        return;
    }

    // 挂载 Flash FATFS
    if (!bsp_flash_disk_mount()) {
        printf("IMG: FATFS mount failed\n");
        ble_img_send_response(BLE_CMD_PREPARE, 0x03, NULL, 0);
        return;
    }

    // 生成文件名并打开文件
    tm_t tm = rtc_clock_get();
    char filename[32];
    sprintf(filename, "A:\\PIC\\%02d%02d%02d.jpg", tm.hour, tm.min, tm.sec);
    printf("IMG: opening %s\n", filename);

    FRESULT res = fs_open(&img_xfer.fp, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        printf("IMG: fs_open failed, res=%d\n", res);
        ble_img_send_response(BLE_CMD_PREPARE, 0x03, NULL, 0);
        return;
    }

    // 初始化传输控制块
    img_xfer.state         = BLE_IMG_STATE_READY;
    img_xfer.total_len     = total_len;
    img_xfer.total_packets = total_pkts;
    img_xfer.img_type      = img_type;
    img_xfer.pixel_x       = pixel_x;
    img_xfer.pixel_y       = pixel_y;
    img_xfer.next_seq      = 0;
    img_xfer.received_bytes = 0;
    img_xfer.file_opened   = true;

    ble_img_send_response(BLE_CMD_PREPARE, 0x00, NULL, 0);
    printf("IMG: ready, file opened\n");
}

//=============================================================================
// 协议3 处理：CMD 0x02 接收图片数据帧 → 直接写入文件
//=============================================================================
static void ble_img_handle_data(const ble_frame_t *frame)
{
    if (img_xfer.state != BLE_IMG_STATE_READY && img_xfer.state != BLE_IMG_STATE_RECEIVING) {
        return;
    }

    if (frame->data_len < 2) {
        u8 fail_seq[2] = {0x00, 0x00};
        ble_img_send_response(BLE_CMD_DATA, 0x01, fail_seq, 2);
        return;
    }

    const u8 *d = frame->data;
    u16 frame_seq = d[0] | (d[1] << 8);
    u16 frame_data_len = frame->data_len - 2;
    const u8 *frame_data = &d[2];

    if (frame_seq != img_xfer.next_seq) {
        printf("IMG: seq mismatch, expected %d, got %d\n", img_xfer.next_seq, frame_seq);
        u8 fail_seq[2];
        fail_seq[0] = (u8)(img_xfer.next_seq & 0xFF);
        fail_seq[1] = (u8)((img_xfer.next_seq >> 8) & 0xFF);
        ble_img_send_response(BLE_CMD_DATA, 0x01, fail_seq, 2);
        return;
    }

    // 直接写入 Flash 文件
    UINT bw;
    FRESULT res = fs_write(&img_xfer.fp, frame_data, frame_data_len, &bw);
    if (res != FR_OK || bw != frame_data_len) {
        printf("IMG: fs_write fail, res=%d, bw=%d/%d\n", res, bw, frame_data_len);
        u8 fail_seq[2];
        fail_seq[0] = (u8)(frame_seq & 0xFF);
        fail_seq[1] = (u8)((frame_seq >> 8) & 0xFF);
        ble_img_send_response(BLE_CMD_DATA, 0x02, fail_seq, 2);
        return;
    }

    img_xfer.received_bytes += frame_data_len;
    img_xfer.next_seq++;
    img_xfer.state = BLE_IMG_STATE_RECEIVING;

    // 成功不发送应答
    printf("IMG: rx seq=%d, wrote %d, total=%lu/%lu\n",
           frame_seq, frame_data_len, img_xfer.received_bytes, img_xfer.total_len);
}

//=============================================================================
// 协议4 处理：CMD 0x03 结束发送 → 关闭文件
//=============================================================================
static void ble_img_handle_finish(const ble_frame_t *frame)
{
    printf("IMG: CMD 0x03 finish, state=%d, received=%lu\n", img_xfer.state, img_xfer.received_bytes);

    if (img_xfer.state != BLE_IMG_STATE_RECEIVING && img_xfer.state != BLE_IMG_STATE_READY) {
        ble_img_send_response(BLE_CMD_FINISH, 0x01, NULL, 0);
        return;
    }

    if (img_xfer.file_opened) {
        fs_close(&img_xfer.fp);
        img_xfer.file_opened = false;
        printf("IMG: file closed, %lu bytes written\n", img_xfer.received_bytes);
    }

    // 枚举 A:\PIC 目录文件
    {
        FRESULT res;
        FILINFO fno;
        u32 pic_cnt = 0;
        res = fs_findfirst(&fno, "A:\\PIC", "*", D_FILE, NULL);
        while (res == FR_OK && fno.fname[0]) {
            printf("IMG: PIC file: %s\n", fno.fname);
            pic_cnt++;
            res = fs_findnext(&fno);
        }
        printf("IMG: PIC has %d files\n", pic_cnt);
    }

    ble_img_send_response(BLE_CMD_FINISH, 0x00, NULL, 0);
    ble_img_xfer_reset();
}

//=============================================================================
// 帧命令分发
//=============================================================================
static void ble_img_dispatch_frame(const ble_frame_t *frame)
{
    switch (frame->cmd) {
    case BLE_CMD_PREPARE:
        ble_img_handle_prepare(frame);
        break;
    case BLE_CMD_DATA:
        ble_img_handle_data(frame);
        break;
    case BLE_CMD_FINISH:
        ble_img_handle_finish(frame);
        break;
    default:
        printf("IMG: unknown cmd 0x%02x\n", frame->cmd);
        break;
    }
}

//=============================================================================
// Write 回调（蓝牙线程上下文 — 仅保存指针和长度，不拷贝）
//=============================================================================
static int gatt_callback_img_write(uint16_t con_handle, uint16_t handle, uint32_t offset, uint8_t *ptr, uint16_t len)
{
    // 检查环形缓冲区是否已满
    u8 next_wptr = (img_rx_cb.wptr + 1) & IMG_RX_BUF_MASK;
    if (next_wptr == (img_rx_cb.rptr & IMG_RX_BUF_MASK)) {
        printf("IMG: rx ring full, drop\n");
        return 0;
    }

    // 动态分配 buffer 并拷贝数据
    u8 *dyn_buf = func_zalloc(len);
    if (dyn_buf == NULL) {
        printf("IMG: alloc fail, len=%d\n", len);
        return 0;
    }

    memcpy(dyn_buf, ptr, len);

    u8 wptr = img_rx_cb.wptr & IMG_RX_BUF_MASK;
    img_rx_cb.cmd[wptr].buf = dyn_buf;
    img_rx_cb.cmd[wptr].len = len;
    img_rx_cb.wptr++;
    img_rx_cb.dirty = true;

    return 0;
}

//=============================================================================
// GATT Service 初始化（由 ble_init_att() 调用）
//=============================================================================
void ble_gatts_demo_service_init(void)
{
    int ret = 0;

    printf("IMG_GATTS: service init start\n");

    ble_txpkt_init_with_pool(ble_send_kick, img_notify_tx_pool, IMG_NOTIFY_NUM, IMG_NOTIFY_LEN);

    memset(&img_rx_cb, 0, sizeof(img_rx_cb));
    ble_img_xfer_reset();

    ret |= ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                  uuid_img_service_base.uuid,
                                  uuid_img_service_base.type,
                                  NULL);

    ret |= ble_gatts_characteristic_add(uuid_img_write_base.uuid,
                                         uuid_img_write_base.type,
                                         uuid_img_write_base.props,
                                         NULL,
                                         &gatts_img_write_cb_info);

    ret |= ble_gatts_characteristic_add(uuid_img_notify_base.uuid,
                                         uuid_img_notify_base.type,
                                         uuid_img_notify_base.props,
                                         &gatts_img_notify_handle.handle,
                                         &gatts_img_notify_cb_info);

    if (ret != BLE_GATTS_SUCCESS) {
        printf("IMG_GATTS: init FAILED, err=%d\n", ret);
    } else {
        printf("IMG_GATTS: init OK, handle=0x%04x\n", gatts_img_notify_handle.handle);
    }
}

//=============================================================================
// BLE 断开连接时的资源清理
//=============================================================================
void ble_gatts_disconnect_cleanup(void)
{
    if (img_xfer.state != BLE_IMG_STATE_IDLE) {
        printf("IMG: BLE disconnect, reset (state=%d)\n", img_xfer.state);
        ble_img_xfer_reset();
    }
}

//=============================================================================
// 屏幕 UI — 控制块
//=============================================================================
typedef struct f_ble_gatts_t_ {
    char status_text[32];
    char result_text[48];
} f_ble_gatts_t;

//=============================================================================
// 状态文本
//=============================================================================
static const char *ble_img_state_str(ble_img_state_t state)
{
    switch (state) {
    case BLE_IMG_STATE_IDLE:      return "Idle";
    case BLE_IMG_STATE_READY:     return "Ready";
    case BLE_IMG_STATE_RECEIVING: return "Receiving...";
    default:                      return "Unknown";
    }
}

//=============================================================================
// 屏幕创建
//=============================================================================
compo_form_t *func_ble_gatts_form_create(void)
{
    compo_form_t *frm = compo_form_create(true);

    compo_form_set_mode(frm, COMPO_FORM_MODE_SHOW_TITLE);
    compo_form_set_title_center(frm, true);
    compo_form_set_title(frm, "BLE Image Rx");

    compo_textbox_t *txt_status = compo_textbox_create(frm, 30);
    compo_setid(txt_status, COMPO_ID_TXT_STATUS);
    compo_textbox_set_pos(txt_status, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y - 100);
    compo_textbox_set(txt_status, "Disconnected");

    compo_textbox_t *txt_data = compo_textbox_create(frm, 40);
    compo_setid(txt_data, COMPO_ID_TXT_DATA);
    compo_textbox_set_pos(txt_data, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y);
    compo_textbox_set(txt_data, "No Transfer");

    compo_textbox_t *txt_result = compo_textbox_create(frm, 48);
    compo_setid(txt_result, COMPO_ID_TXT_RESULT);
    compo_textbox_set_pos(txt_result, GUI_SCREEN_CENTER_X, GUI_SCREEN_CENTER_Y + 100);
    compo_textbox_set(txt_result, "");

    return frm;
}

//=============================================================================
// 进入/退出
//=============================================================================
void func_ble_gatts_enter(void)
{
    printf("IMG_GATTS: screen enter\n");
    func_cb.f_cb = func_zalloc(sizeof(f_ble_gatts_t));
    func_cb.frm_main = func_ble_gatts_form_create();
}

void func_ble_gatts_exit(void)
{
    printf("IMG_GATTS: screen exit\n");
    func_cb.last = FUNC_BLE_GATTS;
}

//=============================================================================
// Process 循环
//=============================================================================
void func_ble_gatts_process(void)
{
    func_process();

    f_ble_gatts_t *fst = (f_ble_gatts_t *)func_cb.f_cb;
    if (fst == NULL) {
        return;
    }

    // BLE 断开清理
    if (!ble_is_connected()) {
        ble_gatts_disconnect_cleanup();
    }

    // 连接状态
    compo_textbox_t *txt_status = compo_getobj_byid(COMPO_ID_TXT_STATUS);
    if (txt_status) {
        if (ble_is_connected()) {
            char buf[32];
            sprintf(buf, "Connected [%s]", ble_img_state_str(img_xfer.state));
            compo_textbox_set(txt_status, buf);
        } else {
            compo_textbox_set(txt_status, "Disconnected");
        }
    }

    // 处理 ring buffer 中的帧
    while (img_rx_cb.rptr != img_rx_cb.wptr) {
        u8 rptr = img_rx_cb.rptr & IMG_RX_BUF_MASK;
        img_rx_cb.rptr++;

        u8 *ptr = img_rx_cb.cmd[rptr].buf;
        u16 len = img_rx_cb.cmd[rptr].len;

        if (ptr != NULL) {
            ble_frame_t frame;
            if (ble_frame_parse(ptr, len, &frame)) {
                ble_img_dispatch_frame(&frame);
            }
            // 释放动态分配的 buffer
            func_free(ptr);
            img_rx_cb.cmd[rptr].buf = NULL;
        }
    }

    // 更新进度
    compo_textbox_t *txt_data = compo_getobj_byid(COMPO_ID_TXT_DATA);
    if (txt_data) {
        if (img_xfer.state == BLE_IMG_STATE_IDLE) {
            compo_textbox_set(txt_data, "No Transfer");
        } else if (img_xfer.state == BLE_IMG_STATE_READY) {
            char buf[40];
            sprintf(buf, "Ready: %luB %dx%d",
                    img_xfer.total_len, img_xfer.pixel_x, img_xfer.pixel_y);
            compo_textbox_set(txt_data, buf);
        } else if (img_xfer.state == BLE_IMG_STATE_RECEIVING) {
            char buf[40];
            u32 pct = (img_xfer.total_len > 0) ?
                      (img_xfer.received_bytes * 100 / img_xfer.total_len) : 0;
            sprintf(buf, "%lu/%luB (%lu%%)",
                    img_xfer.received_bytes, img_xfer.total_len, pct);
            compo_textbox_set(txt_data, buf);
        }
    }
}

//=============================================================================
// 消息处理
//=============================================================================
void func_ble_gatts_message(size_msg_t msg)
{
    switch (msg) {
    default:
        func_message(msg);
        break;
    }
}

//=============================================================================
// 主函数
//=============================================================================
void func_ble_gatts(void)
{
    printf("IMG_GATTS: main loop start\n");
    func_ble_gatts_enter();
    while (func_cb.sta == FUNC_BLE_GATTS) {
        func_ble_gatts_process();
        func_ble_gatts_message(msg_dequeue());
    }
    func_ble_gatts_exit();
}

#endif // FUNC_BLE_GATTS_EN
