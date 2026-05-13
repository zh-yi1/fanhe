#include "include.h"

#if BT_PBAP_EN
//static char qp[] = {"=12=34=56=78=9A=BC=DE=F0=12=34=56=78"};  //for test

//获取IOS的电话本，需要在bt_get_class_of_device()中把COD改为0x240408

#define PBAP_DEBUG_EN    0

static pbap_param_t pbap_param;

static u8 qp_convert_do(char *in)
{
    u8 utf8_cod = 0;
    for(u8 i = 0;i < 2;i++){
        if(*in > '@' && *in < 'G'){
            utf8_cod |= (0x0A + *in - 'A') << 4*(1-i);
        }else if(*in > '/' && *in < ':'){
            utf8_cod |= (*in - '0') << 4*(1-i);
        }
        in++;
    }
    return utf8_cod;
}

static void qp_convert(char *out)
{
    u8 cnt = strlen(out);
    u8 utf8_len = 0;
    for(u8 i = 0;i < cnt;i ++){
       if(out[i] != '='){
         out[utf8_len++] = qp_convert_do(&out[i]);
         i++;
       }
    }
    out[utf8_len] = '\0';
}

//type:本地号码[0], 来电号码[1], 去电号码[2], 未接号码[3]
void bt_pbap_data_callback(u8 type, void *item)
{
    //注意函数内不要进行耗时大的操作，会影响电话本获取的速度
    pbap_data_cb_t *p = (pbap_data_cb_t *)item;
    //memcpy(p->name,qp,sizeof(qp));
    if (p->name[0] == '=') {
        qp_convert(p->name);
    }

#if PBAP_DEBUG_EN
    printf("[%d] [name:%s]  ", type, p->name);
    printf("[tele:%s]  ", p->anum);
    if (type) {
        printf("[date:%s]\n", p->bnum);
    } else {
        if (p->bnum[0]) {
            printf("[%s]\n", p->bnum);
        } else {
            printf("\n");
        }
    }
#endif

    if (!pbap_param.is_pb_syncing) return;

#if FLASHDB_EN
    // 本地号码存 kvdb
    if ((type == 0) && (strlen(p->anum) > 0)) {
        contacts_kvdb_set(p->name, p->anum);
        return;
    }
#endif
    if (pbap_param.pb_idx < pbap_param.pb_count) {
        if (strlen(p->name) > PBAP_MAX_NAME_LEN || strlen(p->anum) > PBAP_MAX_NUM_LEN) {
            printf("[pbap]name len or num len exceeds the max!");
            return;
        }
        pbap_pb_buf_t *pb_buf = pbap_param.pb_buf;
        memcpy(&pb_buf[pbap_param.pb_idx].name[0], p->name, PBAP_MAX_NAME_LEN);
        memcpy(&pb_buf[pbap_param.pb_idx].num[0], p->anum, PBAP_MAX_NUM_LEN);
        memcpy(&pb_buf[pbap_param.pb_idx].date[0], p->bnum, PBAP_MAX_DATE_LEN);
        pbap_param.pb_idx++;
    }
}

void bt_pbap_param_reset(void)
{
    memset(&pbap_param, 0, sizeof(pbap_param_t));
}

bool bt_pbap_is_syncing(void)
{
    return pbap_param.is_pb_syncing;
}

void bt_pbap_event_handle(uint evt, u8 *params)
{
    switch(evt) {
        case BT_NOTICE_PBAP_CONNECTED:
            printf("===>>> PBAP: Connected\n");
            //若sync之前pbap没连上,连上后再读取通话记录
            if (pbap_param.is_pb_syncing) {
                bt_pbap_get_phonebook_size();
            }
            pbap_param.is_pbap_connected = true;
            break;
        case BT_NOTICE_PBAP_DISCONNECT:
            printf("===>>> PBAP: Disconnect\n");
            bt_pbap_param_reset();
            pbap_param.is_pbap_connected = false;
            break;
        case BT_NOTICE_PBAP_GET_PHONEBOOK_SIZE_COMPLETE:
            printf("===>>> PBAP: Phonebook size: %u\n", GET_LE32(params));
            pbap_param.pb_size = GET_LE32(params);
            // 设备保存的通话记录少于用户要读取的数量时将其全部导出
            pbap_param.pb_count = MIN(pbap_param.pb_count, pbap_param.pb_size);
            bt_pbap_pull_phonebook_whole();
            break;
        case BT_NOTICE_PBAP_PULL_PHONEBOOK_COMPLETE:
            printf("===>>> PBAP: Pull phonebook complete\n");
            pbap_param.is_pb_syncing = false;
            if (pbap_param.cb) {
                pbap_param.cb(pbap_param.pb_buf, pbap_param.pb_count);
            }
            break;
        default:
            break;
    }
}

void bt_pbap_sync_start(u8 object, void *buf, u16 count, pbap_sync_finish_cb_t finish_cb)
{
    printf("[%s]", __func__);
    if (object > PBAP_OBJECT_CCH || count == 0) {
        printf("[%s]:object or count error!", __func__);
        return;
    }
    if (buf == NULL || finish_cb == NULL) {
        printf("[%s]:buf or cb is null!", __func__);
        return;
    }
    if (bt_get_status() < BT_STA_CONNECTED) {
        printf("[%s]:bt is disconnected!", __func__);
        return;
    }

    memset(&pbap_param, 0, sizeof(pbap_param_t));
    pbap_param.pb_count = count;
    pbap_param.pb_buf = (pbap_pb_buf_t *)buf;
    pbap_param.cb = finish_cb;
    pbap_param.is_pb_syncing = true;
    //默认同步本机的数据
    bt_pbap_select_phonebook(object, PBAP_SYNC_LOCAL);
    bt_pbap_connect();
    bt_pbap_get_phonebook_size();
}

void bt_pbap_sync_stop(void)
{
    pbap_param.is_pb_syncing = false;
    pbap_param.cb = NULL;
}

#endif //BT_PBAP_EN
