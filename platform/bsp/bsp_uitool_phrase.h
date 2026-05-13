#ifndef _BSP_UITOOL_PHRASE_H
#define _BSP_UITOOL_PHRASE_H


#define UITOOL_HEADER_FORMAT   0x4657
#define UITOOL_HEADER          sizeof(uitool_header_t)
#define UITOOL_RES_HEADER      sizeof(uitool_res_t)
#define UITOOL_BUF_WATCH_DIY   0xffffffff


enum {
    UITOOL_TYPE_NULL,
    UITOOL_TYPE_POINTER,
    UITOOL_TYPE_IMAGE,
    UITOOL_TYPE_TEXT,
    UITOOL_TYPE_NUM,
    UITOOL_TYPE_AREA,
    UITOOL_TYPE_REVERSE,
    UITOOL_TYPE_VIDEO,


};

//表盘资源结构体8bytes
typedef struct __attribute__((packed)) uitool_header_t_ {
    u16 sig;                                //表盘头              2bytes
    u16 ver;                                //版本号              2bytes
    u32 user_id;                            //用户ID              2bytes
    u8 user_extend[32];                     //用户自定义扩展      32bytes
    u16 index;                              //表盘编号            2bytes
    u16 wid;                                //宽                  2bytes
    u16 hei;                                //高                  2bytes
    u16 num;                                //控件个数            2bytes
    u32 size;                               //总大小              4bytes
} uitool_header_t;

//各部件结构体信息头16bytes
typedef struct uitool_res_t_ {
    u32 res_addr;                           //资源相对bin的偏移   4bytes
    u8 res_type;                            //资源类型            1byte
    u8 bond_type;                           //资源绑定类型        1byte
    u8 res_num;                             //资源图片数量        1byte
    u8 rsv;                                 //保留位              1bytes
    s16 x;                                  //x坐标               2bytes
    s16 y;                                  //y坐标               2bytes
    s32 param1;                             //参数                4bytes
    s32 param2;                             //参数                4bytes
} uitool_res_t;


u16 bsp_uitool_header_phrase(u32 base_addr);

//void bsp_uitool_create(compo_form_t *frm, u32 base_addr, u16 compo_num);
#endif
