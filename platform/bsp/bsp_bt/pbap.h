#ifndef __BSP_PBAP_H
#define __BSP_PBAP_H

#define PBAP_MAX_NAME_LEN		20
#define PBAP_MAX_NUM_LEN		16
#define PBAP_MAX_DATE_LEN		16

typedef void (*pbap_sync_finish_cb_t)(void *info, u16 count);

enum {
    PBAP_SYNC_LOCAL,    //同步本机数据
    PBAP_SYNC_SIM,      //同步sim卡数据
};

enum {
    PBAP_OBJECT_PB,	    //通讯录
    PBAP_OBJECT_FAV,    //收藏的联系人
    PBAP_OBJECT_ICH,    //来电记录
    PBAP_OBJECT_OCH,    //去电记录
    PBAP_OBJECT_MCH,    //未接来电
    PBAP_OBJECT_CCH,    //所有的通话记录
};

//btstack跟app通讯用，用户不可修改
typedef struct {
    char name[160];
    char anum[20];
    char bnum[20];
} pbap_data_cb_t;

typedef struct {
    char name[PBAP_MAX_NAME_LEN];
    char num[PBAP_MAX_NUM_LEN];
    char date[PBAP_MAX_DATE_LEN];
} pbap_pb_buf_t;

typedef struct {
    u8 pb_size;
    u8 pb_idx;
    u16 pb_count;
    bool is_pb_syncing;
    bool is_pbap_connected;
    pbap_pb_buf_t *pb_buf;
    pbap_sync_finish_cb_t cb;
} pbap_param_t;

void bt_pbap_data_callback(u8 type, void *item);
void bt_pbap_event_handle(uint evt, u8 *params);
void bt_pbap_param_reset(void);

/** 判断当前是否在读取pbap数据 **/
bool bt_pbap_is_syncing(void);

/**
 * @brief 开始同步获取pbap的数据
 * @param[in] object : PBAP_OBJECT_ICH, 来电记录
                       PBAP_OBJECT_OCH, 去电记录
                       ...
 * @param[in] buf :    保存数据的buf
 * @param[in] count :  要获取的数据数量
 * @param[in] finish_cb : pbap数据同步完后的回调
 **/
void bt_pbap_sync_start(u8 object, void *buf, u16 count, pbap_sync_finish_cb_t finish_cb);

void bt_pbap_sync_stop(void);
#endif // __BSP_PBAP_H
