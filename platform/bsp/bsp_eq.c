#include "include.h"
#include "bsp_eq.h"

#define EQ_CRC_SEED                             0xffff
#define EQ_BAND_NUM                             12
#define CAL_FIX(x)		                        ((int)(x * (1 << 27)))

uint calc_crc(void *buf, uint len, uint seed);
extern const eq_param music_eq_tbl[MUSIC_EQ_TBL_LEN];

AT(.text.music)
void music_set_eq_by_num(u8 num)
{
    if (num > (MUSIC_EQ_TBL_LEN - 1)) {
        return;
    }
    music_set_eq_by_res(*music_eq_tbl[num].addr, *music_eq_tbl[num].len);
}


#if EQ_MODE_EN

AT(.text.music)
void sys_set_eq(void)
{
    sys_cb.eq_mode++;
    if (sys_cb.eq_mode > 5) {
        sys_cb.eq_mode = 0;
    }
    music_set_eq_by_num(sys_cb.eq_mode);
//    gui_box_show_eq();
}
#endif // EQ_MODE_EN


#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
u8 eq_rx_buf[EQ_BUFFER_LEN];
eq_dbg_cb_t eq_dbg_cb;
static u8 eq_tx_buf[12];

//格式:[E/D, Q/R, 0, *, (u8)size, (size >> 8), band_cnt, param size, param cnt, rsvd]
const char tbl_eq_version[10] = {'E', 'Q', '1', '*', 6, 0, 12, 4, 5, 0};
const char tbl_drc_version[10] = {'D', 'R', '2', '*', 6, 0, 4, 0, 0, 0};

AT(.com_rodata.anc)
const char tbl_anc_header[3] = "ANC";

static void tx_ack(uint8_t *packet, uint16_t len)
{
    delay_5ms(1);   //延时一段时间再ack
    if (eq_dbg_cb.rx_type) {
#if EQ_DBG_IN_UART
        huart_tx(packet, len);
#endif // EQ_DBG_IN_UART
    } else {
#if EQ_DBG_IN_SPP && BT_SPP_EN
        if (xcfg_cb.eq_dgb_spp_en && (bt_get_status() >= BT_STA_CONNECTED)) {
            bt_spp_tx(SPP_SERVICE_CH0, packet, len);
        }
#endif // EQ_DBG_IN_SPP
    }
    eq_dbg_cb.rx_type = 0;
}

void eq_tx_ack(u8 bank_num, u8 ack)
{
    if (eq_dbg_cb.type) {
        eq_tx_buf[0] = 'D';
    } else {
        eq_tx_buf[0] = 'A';
    }
    eq_tx_buf[1] = bank_num;
    eq_tx_buf[2] = ack;
    eq_tx_buf[3] = -(eq_tx_buf[0] + eq_tx_buf[1] + eq_tx_buf[2]);
    tx_ack(eq_tx_buf, 4);
}

void eq_tx_version(void)
{
    if (eq_dbg_cb.type) {
        memcpy(eq_tx_buf, tbl_drc_version, 10);
    } else {
        memcpy(eq_tx_buf, tbl_eq_version, 10);
    }
    u16 crc = calc_crc(eq_tx_buf, 10, EQ_CRC_SEED);
    eq_tx_buf[10] = crc;
    eq_tx_buf[11] = crc >> 8;
    tx_ack(eq_tx_buf, 12);
}

void eq_parse_cmd(void)
{
    if (eq_rx_buf[0] == 'E' || eq_rx_buf[1] == 'Q') {
        eq_dbg_cb.type = 0;
    } else if (eq_rx_buf[0] == 'D' && eq_rx_buf[1] == 'R') {
        eq_dbg_cb.type = 1;
    } else {
        return;
    }
    if (eq_rx_buf[2] == '?' && eq_rx_buf[3] == '#') {
        eq_tx_version();
        return;
    }

//    print_r(eq_rx_buf, EQ_BUFFER_LEN);
    u8 band_num = eq_rx_buf[6];
    u32 size = little_endian_read_16(eq_rx_buf, 4);
    u32 crc = calc_crc(eq_rx_buf, size+4, EQ_CRC_SEED);
    if (crc == little_endian_read_16(eq_rx_buf, 4+size)) {
        eq_tx_ack(band_num, 0);
    } else {
        eq_tx_ack(band_num, 1);
        return;
    }

    if (crc == eq_dbg_cb.pre_crc) {
        return;
    }
    eq_dbg_cb.pre_crc = crc;
    u8 band_cnt = eq_rx_buf[7];

    if (eq_dbg_cb.type == 0) {
        printf("eq band_cnt = %d\n",band_cnt);
#if (UART0_PRINTF_SEL != PRINTF_NONE)
        printf("%08x\n", little_endian_read_32(eq_rx_buf, 14));
        u8 k;
        u32 *ptr = (u32 *)&eq_rx_buf[18];
        for (k = 0; k < band_cnt*5; k++) {
            printf("%08x", *ptr++);
            if (k % 5 == 4) {
                printf("\n");
            } else {
                printf(" ");
            }
        }
#endif
        music_set_eq(band_cnt, (u32 *)&eq_rx_buf[14]);
    } else {
        music_set_drc_by_online(band_cnt, (const u32 *)&eq_rx_buf[10]);

#if (UART0_PRINTF_SEL != PRINTF_NONE)
        printf("drc band_cnt = %d\n",band_cnt);
        u32 *ptr = (u32 *)&eq_rx_buf[10];
        for (int i = 0; i < (5+band_cnt*3); i++) {
            printf("%08x\n", ptr[i]);
        }
        printf("\n");
#endif
    }

    memset(eq_rx_buf, 0, EQ_BUFFER_LEN);
}

AT(.com_huart.text)
u8 bsp_eq_rx_done(u8* rx_buf)
{
    if (memcmp(rx_buf, tbl_anc_header, 3) == 0) {
        msg_enqueue(EVT_ONLINE_SET_ANC);
        eq_dbg_cb.rx_type = 1;
    } else if(((rx_buf[0]=='E')&&(rx_buf[1]=='Q')) || (rx_buf[0] == 'D' && rx_buf[1] == 'R')){
        msg_enqueue(EVT_ONLINE_SET_EQ);
        eq_dbg_cb.rx_type = 1;
    }
    return eq_dbg_cb.rx_type;
}

void eq_dbg_init(void)
{
    memset(&eq_dbg_cb, 0, sizeof(eq_dbg_cb_t));
}
#endif

void bsp_eq_init(void)
{


}
