#ifndef _BSP_KEY_H
#define _BSP_KEY_H

typedef u16 size_msg_t;                      //定义MSG的SIZE

#define KEY_SCAN_TIMES          6           //按键防抖的扫描次数
#define KEY_UP_TIMES            6           //抬键防抖的扫描次数
#define KEY_LONG_TIMES          150         //长按键的次数
#define KEY_HOLD_TIMES          35          //连按的频率次数
#define KEY_LONG_HOLD_TIMES     (KEY_LONG_TIMES + KEY_HOLD_TIMES)

#define KEY_SHORT               0x00
#define KEY_SHORT_UP            0x10
#define KEY_LONG                0x20
#define KEY_LONG_UP             0x30
#define KEY_HOLD                0x40
#define KEY_LHOLD               0x50
#define KEY_DOUBLE              0x60        //2击
#define KEY_THREE               0x70        //3击
#define KEY_SHORT_UP_DELAY      0x80        //短按延时抬起
#define KEY_TYPE_MASK           0xF0
#define KEY_USAGE_MASK          0x0F

//CTP Message, 范围: 0xA0 ~ 0xAF
#define MSG_CTP_TOUCH           0xA0        //接触
#define MSG_CTP_CLICK           0xA1        //单击
#define MSG_CTP_SHORT_LEFT      0xA2        //短划
#define MSG_CTP_SHORT_UP        0xA3
#define MSG_CTP_SHORT_RIGHT     0xA4
#define MSG_CTP_SHORT_DOWN      0xA5
#define MSG_CTP_LONG            0xA6        //长划
#define MSG_CTP_LONG_LEFT       0xA7
#define MSG_CTP_LONG_UP         0xA8
#define MSG_CTP_LONG_RIGHT      0xA9
#define MSG_CTP_LONG_DOWN       0xAA
#define MSG_CTP_LONG_LIFT       0xAB        //长按抬起
#define MSG_CTP_COVER           0xAC
#define MSG_CTP_DOUBLE_CLICK    0xAD        //双击

//编码器
#define MSG_QDEC_FORWARD        0xBF
#define MSG_QDEC_BACKWARD        0xBE

//Event Message, 范围：0xC0 ~ 0xFF
#define MSG_SYS_1S              0xff
#define MSG_SYS_500MS           0xfe
#define EVT_SD_INSERT           0xfd
#define EVT_SD_REMOVE           0xfc
#define EVT_SD1_INSERT          0xfb
#define EVT_SD1_REMOVE          0xfa
#define EVT_UDISK_INSERT        0xf9
#define EVT_UDISK_REMOVE        0xf8
#define EVT_PC_INSERT           0xf7
#define EVT_PC_REMOVE           0xf6
#define EVT_HFP_SET_VOL         0xf5
#define EVT_A2DP_SET_VOL        0xf4
#define EVT_A2DP_MUSIC_PLAY     0xf3
#define EVT_A2DP_MUSIC_STOP     0xf2
#define EVT_BT_SCAN_START       0xf1
#define EVT_ONLINE_SET_EQ       0xf0
#define EVT_ONLINE_SET_ANC      0xef
#define EVT_BT_SET_LANG_ID      0xee
#define EVT_BT_SET_EQ           0xed
#define EVT_ECHO_LEVEL          0xec
#define EVT_MIC_VOL             0xeb
#define EVT_MUSIC_VOL           0xea
#define EVT_UDE_SET_VOL         0xe9
#define EVT_CALL_NUMBER_UPDATE  0xe8
#define EVT_READY_EXCHANGE_MTU  0xe7
#define EVT_BT_CONNECT_ONCE     0xe5
#define EVT_ID3_TITLE_UPDATE    0xe4
#define EVT_ID3_ARTIST_UPDATE   0xe3
#define EVT_HALT                0xe2
#define EVT_4G_INIT_DONE        0xe1
#define EVT_JPEG_DECODE         0xe0
#if BT_SCO_APP_DBG_EN
#define EVT_SCO_DBG_COMM_RSP    0xdf
#define EVT_SCO_DBG_TLV_RSP     0xde
#define EVT_SCO_DBG_NOTIFY      0xdd
#endif // BT_SCO_APP_DBG_EN

#define EVT_BT_CONNECT         0xdc
#define EVT_BT_DISCONNECT      0xdb
#define EVT_BT_SEARCH_FAIL     0xda
#define EVT_BT_SEARCH_FINISH   0xd9
#define EVT_EMIT_LIST_REFRESH  0xd8
#define EVT_EMIT_RECONNECT     0xd7
#if BT_EMIT_EN
#define EVT_EMIT_START         0xd6
#define EVT_EMIT_STOP          0xd5
#endif

#define EVT_UART_RX_DONE       0xd4
#define EVT_SD_CARD_INTER      0xd3
#define EVT_SD_CARD_OUT        0xd2


#define EVT_VOX_RECORD_EN      0x200
#define EVT_VOX_RECORD_DIS     0x201

//普通按键定义，可以支持短按，长按，双击等。范围：0 ~ 0xf
#define NO_KEY                  0xFF
#define KEY_1                   0x04
#define KEY_2                   0x00
#define KEY_3                   0x05
#define KEY_4                   0x06
#define KEY_5                   0x02
#define KEY_6                   0x07
#define KEY_7                   0x01
#define KEY_8                   0x03

#define KEY_BACK                KEY_1
#define KEY_LEFT                KEY_2
#define KEY_RIGHT               KEY_3
#define KEY_VOL_UP              KEY_4
#define KEY_VOL_DOWN            KEY_5
#define KEY_NEXT                KEY_6
#define KEY_PREV                KEY_7
#define KEY_MODE                KEY_8

#define K_BACK                  (KEY_BACK | KEY_SHORT)
#define KU_BACK                 (KEY_BACK | KEY_SHORT_UP)
#define KU_DELAY_BACK           (KEY_BACK | KEY_SHORT_UP_DELAY)
#define KL_BACK                 (KEY_BACK | KEY_LONG)
#define KLU_BACK                (KEY_BACK | KEY_LONG_UP)
#define KH_BACK                 (KEY_BACK | KEY_HOLD)
#define KD_BACK                 (KEY_BACK | KEY_DOUBLE)
#define KTH_BACK                (KEY_BACK | KEY_THREE)
#define KLH_BACK                (KEY_BACK | KEY_LHOLD)

#define K_LEFT                  (KEY_LEFT | KEY_SHORT)
#define KU_LEFT                 (KEY_LEFT | KEY_SHORT_UP)
#define KU_DELAY_LEFT           (KEY_LEFT | KEY_SHORT_UP_DELAY)
#define KL_LEFT                 (KEY_LEFT | KEY_LONG)
#define KLU_LEFT                (KEY_LEFT | KEY_LONG_UP)
#define KH_LEFT                 (KEY_LEFT | KEY_HOLD)
#define KD_LEFT                 (KEY_LEFT | KEY_DOUBLE)
#define KLH_LEFT                (KEY_LEFT | KEY_LHOLD)

#define K_RIGHT                 (KEY_RIGHT | KEY_SHORT)
#define KU_RIGHT                (KEY_RIGHT | KEY_SHORT_UP)
#define KU_DELAY_RIGHT          (KEY_RIGHT | KEY_SHORT_UP_DELAY)
#define KL_RIGHT                (KEY_RIGHT | KEY_LONG)
#define KLU_RIGHT               (KEY_RIGHT | KEY_LONG_UP)
#define KH_RIGHT                (KEY_RIGHT | KEY_HOLD)
#define KD_RIGHT                (KEY_RIGHT | KEY_DOUBLE)
#define KTH_RIGHT               (KEY_RIGHT | KEY_THREE)
#define KLH_RIGHT               (KEY_RIGHT | KEY_LHOLD)

#define K_VOL_UP                (KEY_VOL_UP | KEY_SHORT)
#define KU_VOL_UP               (KEY_VOL_UP | KEY_SHORT_UP)
#define KL_VOL_UP               (KEY_VOL_UP | KEY_LONG)
#define KLU_VOL_UP              (KEY_VOL_UP | KEY_LONG_UP)
#define KH_VOL_UP               (KEY_VOL_UP | KEY_HOLD)
#define KD_VOL_UP               (KEY_VOL_UP | KEY_DOUBLE)

#define K_VOL_DOWN              (KEY_VOL_DOWN | KEY_SHORT)
#define KU_VOL_DOWN             (KEY_VOL_DOWN | KEY_SHORT_UP)
#define KL_VOL_DOWN             (KEY_VOL_DOWN | KEY_LONG)
#define KLU_VOL_DOWN            (KEY_VOL_DOWN | KEY_LONG_UP)
#define KH_VOL_DOWN             (KEY_VOL_DOWN | KEY_HOLD)
#define KD_VOL_DOWN             (KEY_VOL_DOWN | KEY_DOUBLE)

#define K_MODE                  (KEY_MODE | KEY_SHORT)
#define KU_MODE                 (KEY_MODE | KEY_SHORT_UP)
#define KL_MODE                 (KEY_MODE | KEY_LONG)
#define KLU_MODE                (KEY_MODE | KEY_LONG_UP)
#define KH_MODE                 (KEY_MODE | KEY_HOLD)
#define KD_MODE                 (KEY_MODE | KEY_DOUBLE)


#define K_PREV                  (KEY_PREV | KEY_SHORT)
#define KU_PREV                 (KEY_PREV | KEY_SHORT_UP)
#define KL_PREV                 (KEY_PREV | KEY_LONG)
#define KLU_PREV                (KEY_PREV | KEY_LONG_UP)

#define K_NEXT                  (KEY_NEXT | KEY_SHORT)
#define KU_NEXT                 (KEY_NEXT | KEY_SHORT_UP)
#define KL_NEXT                 (KEY_NEXT | KEY_LONG)
#define KLU_NEXT                (KEY_NEXT | KEY_LONG_UP)


//按键消抖参数
typedef struct {
    u16 scan_cnt;
    u16 up_cnt;
    u16 long_cnt;
    u16 hold_cnt;
} key_shake_tbl_t;

typedef struct {
    u8 adc_val;
    u8 usage_id;
} adkey_tbl_t;

extern const adkey_tbl_t adkey_table[];
extern const adkey_tbl_t adkey2_table[];
extern const adkey_tbl_t pwrkey_table[6];
extern const u8 key_config_table[];

void io_key_init(void);
u8 get_iokey(void);
void key_var_init(void);
void key_init(void);
u8 bsp_key_scan(void);
u8 bsp_pwrkey_get_usage_id(void);

u8 get_double_key_time(void);

void bsp_qdec_init(void);
void bsp_qdec_exit(void);

#endif // _BSP_KEY_H
