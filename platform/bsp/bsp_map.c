#include "include.h"

int nibble_for_char(char c);

static uint16_t get_number(uint8_t *ptr, uint8_t start, uint8_t *end)
{
    uint8_t pos = 0;
    uint16_t number = 0;

    while (ptr[start + pos] != '/') {
        number *= 10;
        number += nibble_for_char(ptr[start + pos]);
        pos++;
    }

    *end = start + pos;
    return number;
}

void bt_get_time(char *ptr, uint32_t len, uint8_t format)
{
#if UART0_PRINTF_SEL
    u16 year = 0;
    u8 month = 0;
    u8 day = 0;
    u8 hour = 0;
    u8 min = 0;
    u8 sec = 0;

    my_printf("-->set time\n");
    if (format == 0) {
        year = nibble_for_char(ptr[0])*1000 + nibble_for_char(ptr[1])*100 + nibble_for_char(ptr[2])*10 + nibble_for_char(ptr[3]);
        month = nibble_for_char(ptr[4])*10 + nibble_for_char(ptr[5]);
        day = nibble_for_char(ptr[6])*10 + nibble_for_char(ptr[7]);
        hour = nibble_for_char(ptr[9])*10 + nibble_for_char(ptr[10]);
        min = nibble_for_char(ptr[11])*10 + nibble_for_char(ptr[12]);
        sec = nibble_for_char(ptr[13])*10 + nibble_for_char(ptr[14]);
    } else if (format == 1) {
        uint8_t start_pos = 0;
        uint8_t end_pos;

        year = 2000 + get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        month = get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        day = get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        hour = get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        min = get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        sec = get_number((uint8_t *)ptr, start_pos, &end_pos);
    }

    if ((year >= 2000) && (year <= 3000) && (month <= 12) && (day <= 31) && (hour <= 24) && (min <= 60) && sec <= 60) {
        printf("get time:\n");
        printf("date:%04d.%02d.%02d time:%02d:%02d:%02d\n",year,month,day,hour,min,sec);
    }
#endif
}

#if BT_MAP_EN
void bt_map_data_callback(uint8_t *data, uint16_t data_len, uint8_t data_type)     //获取时间例程
{
    uint8_t tag_id;
    uint8_t tag_len;
    uint8_t *tag_data;
    uint16_t offset = 0;

    if ((data == NULL) || (data_len <= 2)) {
        return;
    }

    if (data_type != 0x4c) {  /* time info is contain in application parameter */
        return;
    }

    do {
        tag_id = data[offset];
        tag_len = data[offset + 1];
        tag_data = &data[offset + 2];

        if (tag_len + offset + 2 <= data_len) {
            if (tag_id == 0x19) {  /* MSE Time */
                bt_get_time((char *)tag_data, tag_len, 0);
            }
        }

        offset += 2 + tag_len;
    } while (data_len > (offset + 2));
}
#endif

