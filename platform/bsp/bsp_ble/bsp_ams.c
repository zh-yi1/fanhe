#include "include.h"
#include "bsp_ams.h"

#define TRACE_EN    0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if (LE_AMS_CLIENT_EN && LE_EN)

enum  //实体对象
{
    AMS_ENTITY_ID_PLAYER = 0,
    AMS_ENTITY_ID_QUEUE,
    AMS_ENTITY_ID_TRACK,
};

enum //player对象的属性
{
    AMS_PLAYER_ATT_ID_NAME = 0,
    AMS_PLAYER_ATT_ID_PLAYBACKINFO,
    AMS_PLAYER_ATT_ID_VOLUME,
};

enum //PLAYBACKINFO属性的值
{
    AMS_PLAYBACKINFO_VALUE_STA_PAUSED = '0',
    AMS_PLAYBACKINFO_VALUE_STA_PLAYING,
    AMS_PLAYBACKINFO_VALUE_STA_REWINDING,
    AMS_PLAYBACKINFO_VALUE_STA_FFORWARDING,
};

enum //queue对象的属性
{
    AMS_QUEUE_ATT_ID_INDEX = 0,
    AMS_QUEUE_ATT_ID_COUNT,
    AMS_QUEUE_ATT_ID_SHUFFLEMODE,
    AMS_QUEUE_ATT_ID_REPEATMODE,
};

enum //track对象的属性
{
    AMS_TRACK_ATT_ID_ARTIST = 0,
    AMS_TRACK_ATT_ID_ALBUM,
    AMS_TRACK_ATT_ID_TITLE,
    AMS_TRACK_ATT_ID_DURATION,
};

ble_ams_cb_t ble_ams_cb;
static ble_ams_sta_update_cb_t ble_ams_sta_update_cb = NULL;

void ble_ams_var_init(void)
{
    ble_ams_sta_update_cb = 0;
    memset(&ble_ams_cb, 0, sizeof(ble_ams_cb));
}

void ble_ams_sta_update_cb_reg(ble_ams_sta_update_cb_t func_cb)
{
    if (func_cb) {
        ble_ams_sta_update_cb = func_cb;
    }
}

void ble_ams_sta_update_cb_unreg(void)
{
    if (ble_ams_sta_update_cb) {
        ble_ams_sta_update_cb = NULL;
    }
}

//字符串转数字(针对音量), 0.0625为音量步进
static u32 str_to_num(char *str, u32 size)
{
    int i;
    u32 result = 0;
    u32 coef[4] = {1000, 100, 10, 1};

    if (size == 2) {
        if (str[0] == '0') {
            result = 0;
        } else if (str[0] == '1') {
            result = 10000;
        }
    } else if (size > 3) {
        size = size > 7 ? 7 : size;
        for (i = 2; i < size - 1; i++) {
            result += (str[i] - '0') * coef[i-2];
        }
    }
    TRACE("[%s, %d] --> %d\n", str, size, result);
    return result;
}

void ble_ams_entity_update_callback(uint8_t event_id, uint8_t attribute_id, uint8_t entity_update_flags, uint8_t *value, uint16_t size)
{
    TRACE("%s %x %x %x\n", __func__, event_id, attribute_id, entity_update_flags);
    TRACE("%s %d\n", value, size);

    //app close
    if (!strcmp("0,0.0,0.000", (char *)value)) {
        event_id = AMS_ENTITY_ID_PLAYER;
        attribute_id = AMS_PLAYER_ATT_ID_PLAYBACKINFO;
        value[0] = AMS_PLAYBACKINFO_VALUE_STA_PAUSED;
    }

    switch (event_id) {

    case AMS_ENTITY_ID_PLAYER:
        switch (attribute_id) {

        case AMS_PLAYER_ATT_ID_NAME:
            memset(ble_ams_cb.app_name, 0, sizeof(ble_ams_cb.app_name));
            memcpy(ble_ams_cb.app_name, value, (size > sizeof(ble_ams_cb.app_name)) ? sizeof(ble_ams_cb.app_name) : size);
            if (ble_ams_sta_update_cb) {
                ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_APP_NAME, (char *)value, size);
            }
            break;

        case AMS_PLAYER_ATT_ID_PLAYBACKINFO:

            switch (value[0]) {

            case AMS_PLAYBACKINFO_VALUE_STA_PAUSED:
                ble_ams_cb.play_state = 0;
                if (ble_ams_sta_update_cb) {
                    ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_PAUSE, NULL, 0);
                }
                break;

            case AMS_PLAYBACKINFO_VALUE_STA_PLAYING:
                ble_ams_cb.play_state = 1;
                if (ble_ams_sta_update_cb) {
                    ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_PLAYING, NULL, 0);
                }
                break;

            case AMS_PLAYBACKINFO_VALUE_STA_REWINDING:
                if (ble_ams_sta_update_cb) {
                    ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_REWINDING, NULL, 0);
                }
                break;

            case AMS_PLAYBACKINFO_VALUE_STA_FFORWARDING:
                if (ble_ams_sta_update_cb) {
                    ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_FFORWARDING, NULL, 0);
                }
                break;

            default:
                break;
            }
            // value[2:4] PlaybackRate(float)
            // value[6:12] ElapsedTime(float)
            break;
        case AMS_PLAYER_ATT_ID_VOLUME:              //float, from 0 to 1, step: 0.0625
            {
                u32 volume = str_to_num((char *)value, size);
                ble_ams_cb.vol = volume / 625;
                if (ble_ams_sta_update_cb) {
                    ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_VOLUME, &volume, sizeof(volume));
                }
            }
            break;

        default:
            break;
        }
        break;

    case AMS_ENTITY_ID_QUEUE:
        switch (attribute_id) {

        case AMS_QUEUE_ATT_ID_INDEX:
            break;

        case AMS_QUEUE_ATT_ID_COUNT:
            break;

        case AMS_QUEUE_ATT_ID_SHUFFLEMODE:
            break;

        case AMS_QUEUE_ATT_ID_REPEATMODE:
            break;
        default:
            break;
        }
        break;

    case AMS_ENTITY_ID_TRACK:
        switch (attribute_id) {

        case AMS_TRACK_ATT_ID_ARTIST:
            if (ble_ams_sta_update_cb) {
                ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_ARTIST, (char *)value, size);
            }
            break;

        case AMS_TRACK_ATT_ID_ALBUM:
            if (ble_ams_sta_update_cb) {
                ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_ALBUM, (char *)value, size);
            }
            break;

        case AMS_TRACK_ATT_ID_TITLE:
            if (ble_ams_sta_update_cb) {
                ble_ams_sta_update_cb(BLE_AMS_STA_UPDATE_TITLE, (char *)value, size);
            }
            break;

        case AMS_TRACK_ATT_ID_DURATION:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
#endif // (LE_AMS_CLIENT_EN && LE_EN)
