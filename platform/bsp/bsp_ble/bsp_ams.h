#ifndef __BSP_AMS_H
#define __BSP_AMS_H

/*************************************************************************
 *                  AMS define
**************************************************************************/

enum
{
    BLE_AMS_STA_UPDATE_PAUSE,       //暂停
    BLE_AMS_STA_UPDATE_PLAYING,     //播放
    BLE_AMS_STA_UPDATE_REWINDING,   //回退
    BLE_AMS_STA_UPDATE_FFORWARDING, //快进
    BLE_AMS_STA_UPDATE_VOLUME,      //音量
    BLE_AMS_STA_UPDATE_TITLE,       //歌曲名
    BLE_AMS_STA_UPDATE_ARTIST,      //艺术家
    BLE_AMS_STA_UPDATE_ALBUM,       //专辑
    BLE_AMS_STA_UPDATE_APP_NAME,    //播放器名称
};

enum
{
    AMS_REMOTE_CMD_PLAY,            //播放
    AMS_REMOTE_CMD_PAUSE,           //暂停
    AMS_REMOTE_CMD_PLAY_PAUSE,      //播放/暂停
    AMS_REMOTE_CMD_NEXT_TRACK,      //下一曲
    AMS_REMOTE_CMD_PREV_TRACK,      //上一曲
    AMS_REMOTE_CMD_VOL_UP,          //音量加
    AMS_REMOTE_CMD_VOL_DOWN,        //音量减
    AMS_REMOTE_CMD_REPEAT_MODE,     //重复模式
    AMS_REMOTE_CMD_SHUFFLE_MODE,    //顺序模式
    AMS_REMOTE_CMD_SKIP_FORWARD,    //快进
    AMS_REMOTE_CMD_SKIP_BACKWARD,   //快退
    AMS_REMOTE_CMD_LIKE_TRACK,
    AMS_REMOTE_CMD_DISLIKE_TRACK,
    AMS_REMOTE_CMD_BOOKMARK_TRACK,
};

#if LE_AMS_CLIENT_EN

typedef struct {
    u8 play_state;
    u8 vol;
    char app_name[52];
} ble_ams_cb_t;
extern ble_ams_cb_t ble_ams_cb;

typedef void (*ble_ams_sta_update_cb_t)(u8 sta, void *p_data, u16 len);

/**
 * @brief AMS-音乐播放器状态更新回调注册
 * @param[in] func_cb : 实体函数
 * @return 无
 **/
void ble_ams_sta_update_cb_reg(ble_ams_sta_update_cb_t func_cb);

/**
 * @brief AMS-音乐播放器状态更新回调反注册
 * @param[in] 无
 * @return 无
 **/
void ble_ams_sta_update_cb_unreg(void);

/**
 * @brief AMS 变量初始化
 * @param[in] 无
 * @return 无
 **/
void ble_ams_var_init(void);

#else
#define ble_ams_sta_update_cb_reg(x)
#define ble_ams_sta_update_cb_unreg()
#define ble_ams_var_init()
#endif // LE_AMS_CLIENT_EN

#endif  //__BSP_AMS_H
