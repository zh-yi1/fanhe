#include "include.h"
#include "ab_common.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if (USE_APP_TYPE == APP_BLUE_FIT)

///消息推送类
//0xa0:消息推送
void ble_app_set_message_switch(u8 sw)
{
    app_data.sector0.sys_sw.sw.notification = sw & 0x01;                //消息提醒总开关
    app_data.sector0.sys_sw.sw.notify_screen_light = (sw & 0x02) >> 1;  //消息提醒亮屏开关
    app_data.sector0.sys_sw.sw.message_vibration = (sw & 0x04) >> 2;    //消息提醒震动开关
//    printf("notification: %d\n", app_data.sector0.sys_sw.sw.notification);
//    printf("notify_screen_light: %d\n", app_data.sector0.sys_sw.sw.notify_screen_light);
//    printf("message_vibration: %d\n", app_data.sector0.sys_sw.sw.message_vibration);
}

//0xa4:联系人设置
u8 get_contact_num(void)
{
    u8 num = 0;
    for (int i = 0; i < CONTACTS_MAX_NUM; i++) {
        if (app_data.sector1.contact_info[i].num_len) {
            num++;
        }
    }
    return num;
}

void contact_msg_enqueue(void)
{
//    if (func_cb.sta == FUNC_GTELBOOK) {
//        msg_enqueue(EVT_TELBOOL_UPDATE);
//    }
}

u8 ble_upload_contacts_data(void)
{
//    TRACE("%s\r\n", __func__);
    uint8_t ofs = 0, data_len = 0, frame = 0;
    uint8_t contact_num = get_contact_num();
    contact_info_t *contact_info;
    uint8_t upload_data_arr[100] = {0x00};

    if (contact_num == 0) {
        return CMD_RESULT_FAILE;
    }

    for (int i = 0; i < CONTACTS_MAX_NUM; i++) {
        contact_info = &app_data.sector1.contact_info[i];
        if (contact_info->num_len) {
//            upload_data_arr[0]     = ble_seq_num_get();
            upload_data_arr[1]     = 0xa4;
            upload_data_arr[2]     = 0x02;
            upload_data_arr[3]     = (contact_num << 4) | frame;

            ofs = INSTRUCT_HEADER_LEN + 1;

            upload_data_arr[ofs++] = i;
            data_len = strlen((char*)contact_info->name);
            upload_data_arr[ofs++] = data_len;
            strncpy((char *)&upload_data_arr[ofs], (char*)contact_info->name, data_len);
            ofs += data_len;

            data_len = contact_info->num_len%2 == 0? contact_info->num_len/2 : contact_info->num_len/2 + 1;
            upload_data_arr[ofs++] = data_len;
            memcpy(&upload_data_arr[ofs], contact_info->num_buf, data_len);
            ofs += data_len;

            upload_data_arr[4]     = ofs - INSTRUCT_HEADER_LEN;

            app_protocol_tx(upload_data_arr, ofs);
            memset(upload_data_arr, 0, ofs);
            frame++;

        }
    }
    return CMD_RESULT_OK;
}

u8 get_contact_free_index(void)
{
    u8 idx = 0xff;
    for (int i = 0; i < CONTACTS_MAX_NUM; i++) {
        if (app_data.sector1.contact_info[i].num_len == 0) {
            idx = i;
            break;
        }
    }

    return idx;
}

u8 contact_info_set(uint8_t *data, uint8_t len)
{
//    uint8_t data_len = 0, offset = 0;
//    uint8_t idx = data[offset++];
//
////    TRACE("%s:idx[%d] offset[%d]\n", __func__, idx, offset);
//
////    if (idx == 0) {
////        contact_delete_all();
////    }
//
//    if (idx > CONTACTS_MAX_NUM || app_data.sector1.contact_info[idx].num_len) {
//        return CMD_RESULT_IDX_INVALID;
//    }
//
//    //print_r(data, len);
//    while (offset < len) {
//        data_len = data[offset++];
//        contact_name_set(idx, &data[offset], data_len);
//        offset += data_len;
//        data_len = data[offset++];
//        contact_number_set(idx, &data[offset], len - offset, data_len);
//        offset += data_len;
//    }
//
//    contact_msg_enqueue();

    return CMD_RESULT_OK;
}

u8 contact_info_edit(uint8_t *data, uint8_t len)
{
//    uint8_t data_len = 0, offset = 0;
//    uint8_t idx = data[offset++];
//
////    TRACE("%s:idx[%d] offset[%d]\n", __func__, idx, offset);
//    if (idx > CONTACTS_MAX_NUM) {
//        return CMD_RESULT_IDX_INVALID;
//    }
//
//    if (app_data.sector1.contact_info[idx].num_len == 0) { //编号不存在
//        return CMD_RESULT_IDX_INVALID;
//    }
//
//    //print_r(data, len);
//    while (offset < len) {
//        data_len = data[offset++];
//        contact_name_set(idx, &data[offset], data_len);
//        offset += data_len;
//        data_len = data[offset++];
//        contact_number_set(idx, &data[offset], len - offset, data_len);
//        offset += data_len;
//    }
//
//    contact_msg_enqueue();

    return CMD_RESULT_OK;
}

u8 contact_info_delete(uint8_t index)
{
    uint8_t idx = index;

    if (idx > CONTACTS_MAX_NUM) {
        return CMD_RESULT_IDX_INVALID;
    }

//    TRACE("%s:idx[%d] num_len[%d]\n", __func__, idx, app_data.sector1.contact_info[idx].num_len);
    if (app_data.sector1.contact_info[idx].num_len == 0) { //编号不存在
        return CMD_RESULT_IDX_INVALID;
    }

    app_data.sector1.contact_info[idx].num_len = 0;
    memset(app_data.sector1.contact_info[idx].name, 0, strlen((char*)app_data.sector1.contact_info[idx].name));

    contact_msg_enqueue();

    return CMD_RESULT_OK;
}

void contact_delete_all(void)
{
    memset(&app_data.sector1.contact_info, 0, sizeof(contact_info_t)*CONTACTS_MAX_NUM);
    contact_msg_enqueue();
//    TRACE("%s:idx[%d] num_len[%d]\n", __func__, 0, app_data.sector1.contacts_all.info[0].num_len);
}

//0xa1:天气信息
u8 ble_weather_info_sync(uint8_t* protocol_data, uint16_t protocol_len)
{
    uint8_t date_type = 0, cmd_type = 0, len = 0, offset = 0;
    weather_oneday_info_t *weather_day;
    weather_condition_t *weather = &app_data.sector0.weather;

    date_type = *(protocol_data + offset++);
    if (date_type == 2) {
        weather_day = &weather->third_day;
    } else if (date_type == 1) {
        weather_day = &weather->second_day;
    } else {
        weather_day = &weather->first_day;
    }
//    print_r(protocol_data, protocol_len);

    while (offset < protocol_len) {
        cmd_type = *(protocol_data + offset++);
        len      = *(protocol_data + offset++);

        if (cmd_type > 0x0b) {
            return CMD_RESULT_CMD_INVALID;
        }

//        printf("date_type[%d] cmd_type[%d] len[%d] offset[%d]\n", date_type, cmd_type, len, offset);
        switch (cmd_type) {
            case 0x00://天气类型
                weather_day->oneday_weather_type = protocol_data[offset];
                printf("weather_day->oneday_weather_type[%d]\n", weather_day->oneday_weather_type);
                break;
            case 0x01://当前温度
                weather_day->current_temp = protocol_data[offset];
                printf("weather_day->current_temp[%d]\n", weather_day->current_temp);
                break;
            case 0x02://最小温度
                weather_day->min_temp = protocol_data[offset];
                printf("weather_day->min_temp[%d]\n", weather_day->min_temp);
                break;
            case 0x03://最大温度
                weather_day->max_temp = protocol_data[offset];
                printf("weather_day->max_temp[%d]\n", weather_day->max_temp);
                break;
            case 0x04:{ //地点
                uint16_t length = len;
                memset(weather->location, 0, sizeof(weather->location));
                length = length > WEATHER_LOCATION_MAX? WEATHER_LOCATION_MAX : length;
                weather->location_len = length;
                memcpy(&weather->location, protocol_data + offset, length);
                break;
            }
            case 0x05://湿度
                break;
            case 0x06://风速
                break;
            case 0x07://紫外线指数
                break;
            case 0x08://气压
                break;
            case 0x09://日出时间
                break;
            case 0x0a://日落时间
                break;
            case 0x0b://降雨概率
                break;
            default:
                break;
        }
        offset += len;
    }
    //这里要将天气信息写入flash
//    msg_enqueue(EVT_WEATHER_UPDATE);
    return CMD_RESULT_OK;
}

#endif

#if FLASHDB_EN
void msg_tsdb_append_user(const char *name, const char *title, const char *message)
{
    struct msg_ctrl data = {0};

    data.num = 3;
    data.name.type = MSG_CTRL_TYPE_STR;
    data.name.addr = (void *)name;
    data.name.len = strlen(data.name.addr) + 1;

    data.title.type = MSG_CTRL_TYPE_STR;
    data.title.addr = (void *)title;
    data.title.len = strlen(data.title.addr) + 1;

    data.message.type = MSG_CTRL_TYPE_STR;
    data.message.addr = (void *)message;
    data.message.len = strlen(data.message.addr) + 1;

    msg_tsdb_append_raw(&data);
}

static void msg_tsdb_read_do(u32 addr, u16 addr_len, char *buf, u16 len)
{
    if (addr_len > len) {
        os_spiflash_read(buf, addr, len);
        buf[len - 1] = '\0';
    } else {
        os_spiflash_read(buf, addr, addr_len);
    }
}

int msg_tsdb_read_init(u32 addr, struct msg_ctrl *msg)
{
    os_spiflash_read(msg, addr, sizeof(struct msg_ctrl));
    // printf("%x %x %x %x %x %x %x %x\n", addr, msg->num, msg->name.addr, msg->name.len, msg->title.addr, msg->title.len, msg->message.addr, msg->message.len);
    // print_r(msg, sizeof(struct msg_ctrl));

    if (msg->num != 3) {
        return -1;
    }
    return 0;
}

void msg_tsdb_read_name(struct msg_ctrl *msg, void *buf, u16 len)
{
    msg_tsdb_read_do((u32)msg->name.addr, msg->name.len, buf, len);
}

void msg_tsdb_read_title(struct msg_ctrl *msg, void *buf, u16 len)
{
    msg_tsdb_read_do((u32)msg->title.addr, msg->title.len, buf, len);
}

void msg_tsdb_read_message(struct msg_ctrl *msg, void *buf, u16 len)
{
    msg_tsdb_read_do((u32)msg->message.addr, msg->message.len, buf, len);
}
#endif
