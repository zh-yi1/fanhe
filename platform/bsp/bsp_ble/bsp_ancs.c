#include "include.h"
#include "bsp_ancs.h"

#if (LE_ANCS_CLIENT_EN && LE_EN)

void ble_ancs_client_notifiy_callback(u8 id, const char *att_name, const char *att_content)
{
    printf("id:%d\n", id);
    printf("Notification:%s - %s\n", att_name, att_content);

    ble_app_ancs_client_notifiy_callback(id, att_name, att_content);
}

void ble_ancs_notifiy_info_callback(uint32_t uid, uint8_t event_id, uint8_t event_flags, uint8_t category_id, uint8_t category_count)
{
    printf("uid:%x, event_id:%x, category_id:%x\n", uid, event_id, category_id);
}

#endif // (LE_ANCS_CLIENT_EN && LE_EN)

