#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if BT_ID3_TAG_EN

#define BT_ID3_TITLE        1
#define BT_ID3_ARTIST       2
#define BT_ID3_ALBUM        3
#define BT_ID3_NUMBER       4
#define BT_ID3_TOTAL        5
#define BT_ID3_GENRE        6
#define BT_ID3_TIME         7
#define ID3_BUF_LEN         512

typedef void (*bt_id3_tag_update_cb_t)(u8 type, char *str);

uint8_t cfg_bt_a2dp_id3_en = BT_ID3_TAG_EN;

//u8 id3_buf[ID3_BUF_LEN] AT(.bt_id3_buf);
static bt_id3_tag_update_cb_t bt_id3_tag_update_cb = NULL;

void bt_id3_tag_update_cb_reg(void *func_cb)
{
    if (func_cb) {
        bt_id3_tag_update_cb = (bt_id3_tag_update_cb_t)func_cb;
    }
}

void bt_id3_tag_update_cb_unreg(void)
{
    bt_id3_tag_update_cb = NULL;
}


//packet_type 0:single, 1:start, 2:continue, 3:end（处type0，其他需要拼包）
void bt_id3_tag_callback(u8 packet_type, u8 *buf, u16 size)
{
    //拼包
    u8 *id3_buf = ab_malloc(ID3_BUF_LEN);
    assert_malloc_buf_is_valid(id3_buf);
    static u16 ptr = 0;
    if (packet_type != 0) {
        if(packet_type == 3){
            memcpy(&id3_buf[ptr-1], buf, size);
            ptr += (size-1);
        } else {
            memcpy(&id3_buf[ptr], buf, size);
            ptr += size;
        }
        if (packet_type != 3) {
            ab_free(id3_buf);
            return;
        }
        buf = id3_buf;
    }
    ptr = 0;

    //格式
    //byte[0]   : num
    //byte[4:1] : index n
    //byte[6:5] : 编码格式（0x6a:UTF8）
    //byte[8:7] : lenght
    //byte[n:9] : value
    //...
    u16 offset = 0;
    u8 att_num = buf[offset++];

    while (att_num) {
        u32 index = ((u32)buf[offset + 3]) | (((u32)buf[offset + 2]) << 8) | (((u32)buf[offset + 1]) << 16) | (((u32)buf[offset + 0]) << 24);
        offset += 4;
        u16 character = ((u32)buf[offset + 1]) | (((u32)buf[offset + 0]) << 8);
        offset += 2;
        u16 length = ((u32)buf[offset + 1]) | (((u32)buf[offset + 0]) << 8);
        offset += 2;
        TRACE("bt id3:%x %x %x\n", index, character, length);
        switch (index) {
            case BT_ID3_TITLE:
                 TRACE("name: %s\n", &buf[offset]);
                 if (bt_id3_tag_update_cb) {
                    bt_id3_tag_update_cb(BT_ID3_TAG_TITLE, (char *)&buf[offset]);
                 }
                break;

            case BT_ID3_ARTIST:
                TRACE("artist: %s\n", &buf[offset]);
                if (bt_id3_tag_update_cb) {
                    bt_id3_tag_update_cb(BT_ID3_TAG_ARTIST, (char *)&buf[offset]);
                }
                break;

            default:
                break;
        }
        offset += length;
        att_num--;
        if((offset >= size) && (packet_type == 0)){
            break;
        }
        character = character;
    }
    ab_free(id3_buf);
}

//bt_music_paly_status_info() 调用此函数，就会返回下面的信息。
//buf[0:3]  歌曲时长单位MS，buf[4:7] -> 当前播放位置， buf[8] -> 播放状态0x00:stopped, 0x01:playing, 0x02:paused, 0x03:fwd_seek, 0x04:rev_seek, 0xff:error
void bt_get_paly_status_info_callback(u8 *buf, u16 size)
{
    uint32_t tmp = 0;

    tmp = big_endian_read_32(buf, 0);
    TRACE("song time[%2d:%2d]", (tmp / 1000 / 60), (tmp / 1000 % 60));

    tmp = big_endian_read_32(buf, 4);
    TRACE("--->[%2d:%2d]", (tmp / 1000 / 60), (tmp / 1000 % 60));

    tmp = buf[8];
    bt_set_music_sta(tmp);
    TRACE(" sta[%d]\n", tmp);
}

#endif

