/**********************************************************************
*
*   strong_bt.c
*   定义库里面bt部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

#if !BT_FCC_TEST_EN
void bt_uart_init(void) {}

AT(.com_text.stack.uart_isr)
bool bt_uart_isr(void) {return false;}
#endif

#if !BT_PBAP_EN
void pbap_client_init(void) {}
#endif

#if !(BT_PBAP_EN || BT_MAP_EN)
void goep_client_init(void) {}
#endif

#if !BT_PANU_EN
void btstack_panu_connect(void) {}
void btstack_panu_disconnect(void) {}

void bnep_network_init(void) {}
void bnep_network_up(bd_addr_t addr) {}
void bnep_network_down(void) {}
void bnep_network_process_packet(const uint8_t *packet, uint16_t size) {}
AT(.com_text.stack.run_loop)
void bnep_network_outgoing_process(void) {}
void bnep_network_packet_sent(uint8_t *buf) {}
u8 bnep_get_psm(void) {return 0;}

void bnep_audio_init(void) {}
void bnep_audio_exit(void) {}
void bnep_audio_write(void *buf, unsigned int size) {}
void bnep_audio_start(void) {}
int web_stream_read(void *buf, unsigned int size) {return 0;}
bool web_stream_seek(unsigned int ofs, int whence) {return false;}

// errno
AT(.com_text.debug)
void mutex_take_save_errno(va_list param) {}
AT(.com_text.debug)
void mutex_release_restore_errno(va_list param) {}

static int my_errno;
AT(.com_text)
int *_os_errno(void) {
    return &my_errno;
}
#endif

#if !BT_HCI_DUMP
void hci_dump_open(void) {}
void hci_dump_close(void) {}
void hci_dump_packet(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len) {}
#endif

//是否支持BT AAC音频
#if !BT_TWS_EN || !BT_A2DP_AAC_AUDIO_EN
bool aac_tws_dec_frame(void) {return false;};
void aac_cache_free_do(void) {}
size_t aac_cache_read_do(uint8_t *buf, uint max_size) {return 0;}
AT(.com_text.weak.aac.obuf)
void aac_fill_tws_obuf(void) {}
AT(.aacdec.text)
void aac_obuf_tws_cpy(void) {}
AT(.aacdec.text)
void aac_proc_tws_pcm(bool aac_first){}
void aac_cpy_tws_obuf(void) {}
void aac_kick_copy_tws_obuf(void) {}
void aac_gpdma_done(void) {}
#endif

#if !BT_A2DP_AAC_AUDIO_EN
void aac_dec_init(void) {}
void aac_decode_init(void) {}
bool aac_dec_frame(void) {return false;}
bool aac_nor_dec_frame(void) {return false;};
AT(.aacdec.text)
bool aac_decode(void) { return false; }
#else
void aac_decode_init_do(void);
bool aac_dec_frame_do(void);
void aac_cache_free_do(void);
size_t aac_cache_read_do(uint8_t *buf, uint max_size);
AT(.text.music.init.aac)
void aac_decode_init(void) {
    aac_decode_init_do();
}
AT(.aacdec.text)
bool aac_dec_frame(void) {
    return aac_dec_frame_do();
}
AT(.aacdec.text)
void aac_cache_free(void) {
#if BT_TWS_EN
    aac_cache_free_do();
#endif
}
AT(.aacdec.text)
size_t aac_cache_read(uint8_t *buf, uint max_size) {
#if BT_TWS_EN
    return aac_cache_read_do(buf, max_size);
#else
    return 0;
#endif
}
#endif


