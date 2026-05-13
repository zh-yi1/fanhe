/**********************************************************************
*
*   strong_symbol.c
*   定义库里面部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

#if (UART0_PRINTF_SEL == PRINTF_NONE)
void wdt_irq_init(void) {}
AT(.sleep_text.lp.sleep.wdts)
void lp_wdt_irq_en(void) {}
#endif

#if !FUNC_USBDEV_EN
void usb_dev_isr(void){}
void ude_ep_reset(void){}
void ude_control_flow(void){}
void ude_isoc_rx_process(void){}
void lock_code_usbdev(void){}
#endif //FUNC_USBDEV_EN

#if !UDE_MIC_EN
void uda_mic_start(void *tcb) {}
void uda_mic_stop(void *tcb) {}
void ude_isoc_tx_process(void){}
bool uda_mic_tx_process(u8 *ptr, u32 samples, u32 nch, bool is_24bit) {return false;}
#endif

#if !REC_MP3_SUPPORT
void mpa_encode_kick_start(void){}
void mpa_encode_frame(void) {}
bool mpa_encode_init(u32 spr, u32 nchannel, u32 bitrate) {return false;}
void mpa_encode_exit(void) {}
#endif

#if (!REC_SBC_SUPPORT && !BT_EMIT_EN)
bool sbc_encode_init(u8 spr, u8 nch){return false;}
u16 sbc_encode_frame(u8 *buf, u16 len) {return 0;}
void sbc_encode_process(void){}
void sbc_encode_exit(void){}
AT(.com_text.bsp.emit)
bool sbcen_puts_buf(u8 *buf, u16 len){return false;}
AT(.com_text.bsp.emit)
bool sbcen_gets_buf(u8 *buf, u16 len){return false;}
AT(.com_text.bsp.emit)
u16 sbcen_get_data_size(void){return 0;}
AT(.com_text.bsp.emit)
bool emit_gets_pcm_obuf(u8 *buf, u16 len){return false;}
void emit_count_add(void){}
void emit_timer_reset(void){}

void au0_dma_start(void) {}
void au0_dma_stop(void) {}
void audma_out_process(u8 flag) {}
AT(.com_text.au0dma.isr)
void au0_dma_isr(void) {}
#endif

#if !REC_ADPCM_SUPPORT
void adpcm_encode_process(void){}
#endif

#if !MUSIC_WAV_SUPPORT
int wav_decode_init(void){return 0;}
int wav_dec_init(void){return 0;}
bool wav_dec_frame(void){return false;}
void lock_code_wavdec(void){}
#endif

#if !MUSIC_WMA_SUPPORT
int wma_decode_init(void){return 0;}
int wma_dec_init(void){return 0;}
bool wma_dec_frame(void){return false;}
void lock_code_wmadec(void){}
#endif

#if !MUSIC_APE_SUPPORT
int ape_decode_init(void){return 0;}
int ape_dec_init(void){return 0;}
bool ape_dec_frame(void){return false;}
void lock_code_apedec(void){}
#endif

#if !MUSIC_FLAC_SUPPORT
int flac_decode_init(void){return 0;}
int flac_dec_init(void){return 0;}
bool flac_dec_frame(void){return false;}
void lock_code_flacdec(void){}
#endif

#if !MUSIC_SBC_SUPPORT
int sbcio_decode_init(void){return 0;}
int sbcio_dec_init(void){return 0;}
bool sbcio_dec_frame(void){return false;}
AT(.sbcdec.code)
void codec_sbcdec_init(void *param) {}
AT(.sbcdec.code)
void codec_sbcdec_update(void) {}
#endif

#if !MUSIC_AAC_SUPPORT
int aacio_decode_init(void){return 0;}
int aacio_dec_init(void) {return 0;}
#endif

#if !MUSIC_M4A_SUPPORT
int m4a_decode_init(void){return 0;}
int m4a_dec_init(void){return 0;}
bool aacio_sub_process(void){return true;}
#endif

#if !MUSIC_AAC_SUPPORT && !MUSIC_M4A_SUPPORT
bool aacio_dec_frame(void) {return false;}
#endif

#if !MUSIC_SBR_SUPPORT
AT(.text.strong.sbr)
int SbrInitDecoder(void) {return 0;}
AT(.text.strong.sbr)
void sbr_audec_kick(void *tcb) {}
AT(.text.strong.sbr)
int SBRDecodePorcess(void *tcb, int *baseChanSBR) {return 0;}
AT(.text.strong.sbr)
void sbr_dec_pcm_out(void *tcb) {}
AT(.text.strong.sbr)
int sbr_windows_overlap_add(void *acb, int ch, int chOut) {return -1;}
AT(.text.strong.sbr)
void DecodeFillElementSbr(void *dec_cb, void *bs, unsigned int fillCount) {}
void sbr_pcm_out_cb(u8 *buf, u32 samples, u32 nch) {}
#endif

void bt_music_rec_start(void) {}
void bt_music_rec_stop(void) {}

#if !USB_SUPPORT_EN
void usb_isr(void){}
void usb_init(void){}
#endif

#if !MUSIC_UDISK_EN
bool udisk_read(void *buf, u32 lba) {return false;}
bool udisk_write(void* buf, u32 lba) {return false;}
bool usb_host_init(void) {return false;}
bool dev_udisk_activation_try(u8 mode) {return false;}
bool uhs_wait_pending(uint timeout){return false;}
void os_sem_uhs_post(void){}
void os_sem_uhs_trytake(void){}
#endif

#if ((!SD_SUPPORT_EN) && (!FUNC_USBDEV_EN))
void sd_disk_init(void){}
void sdctl_isr(void){}
void sd_disk_switch(u8 index){}
bool sd0_stop(bool type){return false;}

bool sd0_init(void){return false;}
bool sd0_read(void *buf, u32 lba){return false;}
bool sd0_write(void* buf, u32 lba){return false;}
#endif

#if !FUNC_MUSIC_EN
u32 fs_get_file_size(void){return 0;}
void fs_save_file_info(unsigned char *buf){}
void fs_load_file_info(unsigned char *buf){}
#endif

#if !FATFS_SUPPORT_EN
FRESULT fs_open (FIL* fp, const TCHAR* path, BYTE mode) {return 0;}
FRESULT fs_close (FIL* fp) {return 0;}
FRESULT fs_read (FIL* fp, void* buff, UINT btr, UINT* br) {return 0;}
FRESULT fs_write (FIL* fp, const void* buff, UINT btw, UINT* bw) {return 0;}
FRESULT fs_lseek (FIL* fp, FSIZE_t ofs) { return 0;}
void os_sem_usb_sd_init(void) {}
FRESULT f_circ_open(CIRCST* fcirc, BYTE free_per) {return 0;}
FRESULT f_circ_write(CIRCST* fcirc, const void* buff, UINT btw, bool is_en) {return 0;}
FRESULT f_circ_updata(CIRCST* fcirc) {return 0;}
FRESULT f_circ_process(CIRCST* fcirc) {return 0;}
FRESULT f_circ_err(CIRCST* fcirc) {return 0;}
FRESULT f_circ_close(CIRCST* fcirc) {return 0;}
FRESULT f_circ_lock(CIRCST* fcirc) {return 0;}
FRESULT f_circ_unlock(CIRCST* fcirc) {return 0;}
FRESULT f_circ_deltemp(CIRCST* fcirc) {return 0;}
void* fs_circ_container(void) {return 0;}
u32 fs_circ_container_size(void) {return 0;}
bool fs_circ_is_fill(CIRCST* fcirc) {return 0;}
FRESULT fs_circ_lseek2begin(CIRCST* fcirc) {return 0;}
u64 fs_circ_wr2tell(CIRCST* fcirc) {return 0;}
u64 fs_circ_temp2tell(CIRCST* fcirc) {return 0;}
FRESULT fs_circ_temp2write (
    CIRCST* fcirc,            /* Open file to be written */
    const void* buff,    /* Data to be written */
    UINT btw,            /* Number of bytes to write */
    UINT* bw            /* Number of bytes written */
) {return 0;}
FRESULT fs_circ_wr2write (
    CIRCST* fcirc,            /* Open file to be written */
    const void* buff,    /* Data to be written */
    UINT btw,            /* Number of bytes to write */
    UINT* bw            /* Number of bytes written */
) {return 0;}
FRESULT fs_circ_wr2lseek (
    CIRCST* fcirc,
    FSIZE_t ofs
) {return 0;}
FRESULT fs_circ_temp2lseek (
    CIRCST* fcirc,
    FSIZE_t ofs
) {return 0;}
FRESULT fs_circ_wr2close (
    CIRCST* fcirc
) {return 0;}
FRESULT fs_circ_temp2close (
    CIRCST* fcirc
) {return 0;}
void fs_circ_close (
    CIRCST* fcirc
) {return ;}
u64 fs_circ_temp2size (
    CIRCST* fcirc
) {return 0;}
FRESULT fs_circ_cat(CIRCST* fcirc) {return 0;}
u32 fs_circ_csize(CIRCST* fcirc) {return 0;}
u64 fs_circ_wr2objs(CIRCST* fcirc) {return 0;}
FIL* fs_circ_fp(CIRCST* fcirc) {return 0;}
u32 fs_circ_sclust(CIRCST* fcirc) {return 0;}
#endif

#if (!(MUSIC_WMA_SUPPORT | MUSIC_APE_SUPPORT | MUSIC_FLAC_SUPPORT | MUSIC_WAV_SUPPORT))
void msc_stream_start(u8 *ptr) {}
void msc_stream_end(void) {}
int msc_stream_read(void *buf, unsigned int size) {return 0;}
bool msc_stream_seek(unsigned int ofs, int whence) {return false;}
void msc_stream_fill(void) {}
void os_stream_fill(void) {}
void os_stream_read(void) {}
void os_stream_seek(void) {}
void os_stream_end(void) {}
#endif

#if (!USB_SUPPORT_EN && !SD_SUPPORT_EN)
void dev_init(void)
{
    sd_disable();
    usb_disable();
}

AT(.com_text.strong)
bool dev_is_online(u16 dev_num)
{
    return false;
}
#endif

#if !USB_SD_UPDATE_EN
int updatefile_init(const char *file){return -1;}
void updateproc(void){}
#endif

#if !WARNING_WAVRES_PLAY
bool wav_res_stop(void){return false;}
void wav_res_play_kick(u32 addr, u32 len){}
void wav_res_dec_process(void){}
bool wav_res_is_play(void){return false;}
AT(.rodata.wavres.buf)
const int wavres_cb = 0;
#endif

#if !BT_HFP_MSBC_EN
AT(.sbcdec.code)
bool btmsbc_fill_callback(u8 *eptr_in, u8 *eptr_out, u8 *dptr_in, u8 *dptr_out)
{
    return false;
}
void msbc_kick_start(void){}
void bt_msbc_process(void){}
void bt_msbc_pcm_output(u16 *output){}
void msbc_init(void){}
void msbc_exit(void){}
bool msbc_encode_init(void){return false;}
void msbc_encode_exit(void){}
u16 bt_msbc_encode_frame(u8 *buf, u16 len){return 0;}
#endif

#if (UART0_PRINTF_SEL == PRINTF_NONE)
AT(.com_text.err)
void sys_error_hook(u8 err_no)
{
    WDT_RST();
    while(1);
}

AT(.com_text.exception) WEAK
void exception_isr(void)
{
    sys_error_hook(2);
}
#endif

#if (!BT_SCO_DUMP_EN && !BT_SCO_EQ_DUMP_EN && !BT_SCO_FAR_DUMP_EN)
void sco_dump_init(void) {}
AT(.com_text.sco_dump)
void bt_sco_huart_tx_done(void){}
AT(.bt_voice.sco_dump)
bool sco_huart_putcs(u8 type, u8 frame_num, void *buf, uint len){return false;}
AT(.bt_voice.sco_dump)
void bt_sco_dump(u32 index, s16 *ptr, u32 len){}
#endif

#if !BT_SCO_MAV_EN
AT(.com_text.mav)
void mav_kick_start(void) {}
void magic_voice_process(void){}
void magic_voice_init(void *cfg){}
#endif

#if !BT_SCO_FAR_NR_EN
void nr_far_init(u16 noise_thr, u16 nr_level, s16 value_ns) {}
void nr_far_process(s16 *data) {}
void bt_sco_far_nr_process(u8 *buf, u32 samples){}
#endif

#if !BT_SCO_AGC_EN
void bt_sco_agc_proc(s16 *ptr, int samples) {}
#endif

#if !DAC_DRC_EN
AT(.com_text.dac.dnr)
void dac_dnr_process(void) {}
#endif

#if !DAC_AUTO_ONOFF_EN
AT(.com_text.dac)
void dac_set_power_on_off(u32 status) {}
#endif

#if !GUI_USE_ARC
typedef void graphics_t;
typedef void dc_t;
typedef void draw_param_t;
typedef void ele_arc_t;
typedef void ele_circle_t;
typedef void widget_circle_t;

//画圆弧接口
AT(.text.element)
bool draw_arc(graphics_t *g, const draw_param_t *param) {return true;}
AT(.com_text.element)
bool ele_arc_dcout(dc_t *dc, const ele_arc_t *element) {return true;}
AT(.text.element)
bool draw_circle(graphics_t *g, const draw_param_t *param) {return true;}
AT(.com_text.element)
bool ele_circle_dcout(dc_t *dc, const ele_circle_t *element) {return true;}
widget_arc_t *widget_arc_create(widget_page_t *parent) {return NULL;}
void widget_arc_set_angles(widget_arc_t *arc, u16 start_angle, u16 end_angle) {}
void widget_arc_set_color(widget_arc_t *arc, u16 color_intra, u16 color_outre) {}
void widget_arc_set_alpha(widget_arc_t *arc, u8 alpha_intra, u8 alpha_outre) {}
void widget_arc_set_width(widget_arc_t *arc, u16 arc_width) {}
void widget_arc_set_edge_circle(widget_arc_t *arc, bool start_onoff, bool end_onoff) {}
widget_circle_t *widget_circle_create(widget_page_t *parent) {return NULL;}
void widget_circle_set_acolor(widget_circle_t *circle, u16 color_intra, u8 alpha_intra) {}
#endif

//线程总堆栈
#if MEM_HEAP_SIZE
u8 mem_heap[MEM_HEAP_SIZE] AT(.heap.os);
AT(.text.startup.init)
u32 system_get_mem_heap_size(void)
{
    return MEM_HEAP_SIZE;
}
#endif // MEM_HEAP_SIZE

//MAIN线程栈大小设置
#if OS_THREAD_MAIN_STACK
AT(.text.startup.init)
u32 system_get_main_stack_size(void)
{
    return OS_THREAD_MAIN_STACK;
}
#endif // OS_THREAD_MAIN_STACK

//MUSIC线程栈大小设置
#if OS_THREAD_MUSIC_STACK
AT(.text.startup.init)
u32 system_get_music_stack_size(void)
{
    return OS_THREAD_MUSIC_STACK;
}
#endif // OS_THREAD_MUSIC_STACK

#if !OPUS_ENC_EN
void opus_enc_process(void) {};
#endif

#if FLASHDB_EN
u32 onc_addr = FLASH_AB_PARAM_ADDR - 16 * 1024;
size_t onc_size = 16 * 1024;
#endif

#if BT_HID_ONLY_FOR_IOS_EN
bool bt_hid_disable_for_andriod(void){
    return true;
}
#endif

//是否复用双flash
AT(.com_text.spiflash)
bool mul_spiflash_is_support(void)
{
    return FLASH_MUL_EN;
}

//32k-64k擦除
#if !FLASH_ERASE_32K_64K
AT(.com_text.spiflash)
void spiflash_erase_select(u32 addr, u8 cmd){}
AT(.com_text.spiflash)
void os_spiflash_erase_32k(u32 addr){}
AT(.com_text.spiflash)
void os_spiflash_erase_64k(u32 addr){}
void spiflash_erase_32k(u32 addr, u8 spi_baud_w, u8 spi_baud) {}
void spiflash_erase_64k(u32 addr, u8 spi_baud_w, u8 spi_baud) {}
#endif

#if !FLASH_EXTERNAL_EN
AT(.com_text.spiflash)
bool gui_is_use_ext_flash(void) {return false;}
AT(.com_text.spiflash)
void gui_set_use_ext_flash(bool en) {}
AT(.com_text.spiflash)
bool spi1flash_read(void *buf, u32 addr, uint len) {return true;}
AT(.com_text.gui)
bool lnf_read_ex(u32 *lnf_cb, u32 *ln_info, int ln, int cnt) {return false;}
AT(.com_text.gui)
bool spiflash_read_lninfo_ex(u32 *ln_info, u32 par_idx_start, int data_start, int avg_size, int idx_size, uint ln, uint cnt, uint max_size) {return false;}
AT(.com_text.gui)
bool de_line_spiflash_depar_kick_ex(void *raw_buf, int raw_len, void *par_buf, int par_size, int first_klen, u32 par_mode, bool flag_par_kick) {return false;}
#endif

#if !SENSOR_HUB_EN
void bsp_sensor_hub_set_hold(bool en) {}
#endif

#if !PSRC_HW_EN
void psrc_var_init(void) {}
void psrc_out_process(uint idx) {}
#endif

#if !PSRC_CH1_EN
void psrc1_init(u32 spr_in, u32 spr_out, u32 channel) {}
void psrc1_stop(void) {}
int psrc1_audio_input(u8 *buf, u32 in_samples, bool is24b) {return -1;}
void psrc1_var_init(void) {}
#endif

#if !SECURITY_PAY_EN && !BT_PANU_EN
typedef void (*out_fct_type)(char character, void * buffer, size_t idx, size_t maxlen);

bool extern_sprintf_en(void) {return false;}
void _out_buffer(char character, void * buffer, size_t idx, size_t maxlen) {}
inline void _out_null(char character, void * buffer, size_t idx, size_t maxlen) {}
inline unsigned int _strnlen_s(const char * str, size_t maxsize) {return 0;}
inline bool _is_digit(char ch) {return 0;}
unsigned int _atoi(const char ** str) {return 0;}
size_t _out_rev(out_fct_type out, char * buffer, size_t idx, size_t maxlen, const char * buf, size_t len,
                       unsigned int width, unsigned int flags) {return 0;}
size_t _ntoa_format(out_fct_type out, char * buffer, size_t idx, size_t maxlen, char * buf, size_t len,
                           bool negative, unsigned int base, unsigned int prec, unsigned int width, unsigned int flags) {return 0;}
size_t _ntoa_long(out_fct_type out, char * buffer, size_t idx, size_t maxlen, unsigned long value, bool negative,
                         unsigned long base, unsigned int prec, unsigned int width, unsigned int flags) {return 0;}
size_t _ntoa_long_long(out_fct_type out, char * buffer, size_t idx, size_t maxlen, unsigned long long value,
                              bool negative, unsigned long long base, unsigned int prec, unsigned int width, unsigned int flags) {return 0;}
size_t _ftoa(out_fct_type out, char * buffer, size_t idx, size_t maxlen, double value, unsigned int prec,
                    unsigned int width, unsigned int flags) {return 0;}
size_t _etoa(out_fct_type out, char * buffer, size_t idx, size_t maxlen, double value, unsigned int prec,
                    unsigned int width, unsigned int flags) {return 0;}
int _extern_vsnprintf(out_fct_type out, char * buffer, const size_t maxlen, const char * format, va_list va) {return 0;}
int extern_snprintf_builtin(char * buffer, size_t count, const char * format, ...) {return 0;}
#endif

bool bt_turn_on_off_quickly(void){return true;}
//根据CIEV上报通话状态不走三方管理那套,三方管理那套上报状态兼容性很差;需要时候打开该强定义。
//bool bt_call_status_according_to_ciev(void) {return true;}
//只有来电active状态才向手机发送AT+CLCC,三方管理那套太多地方发送,导致手机不回复,需要时候打开该强定义。
//bool send_clcc_cmd_only_for_call_active(void) {return true;}
//不使能端操作哪端出声机制
//void hfp_speak_role_check(void){};

#if NOC_PSRAM_EN
u8 fs_wr_buff[512];
bool sd0_read_ls(
    BYTE *buff,        /* Data buffer to store read data */
    uint32_t sector,    /* Start sector in LBA */
    UINT count        /* Number of sectors to read */
){
    bool ret;

    if(buff == NULL){
        return false;
    }

    if(count == 0){
        memset(buff, 0, 512);
        return true;
    }

    for (UINT i = 0; i < count; i++) {
        ret = sd0_read(fs_wr_buff, sector);
        if(ret == false){
            return false;
        }
        sector++;
        memcpy(buff, fs_wr_buff, 512);
        buff += 512;
    }

    return true;
}

bool sd0_write_ls(
    BYTE *buff,        /* Data buffer to store read data */
    uint32_t sector,    /* Start sector in LBA */
    UINT count        /* Number of sectors to read */
){
    bool ret;

    if(buff == NULL){
        return false;
    }

    if(count == 0){
        return true;
    }

    for(UINT i=0; i < count; i++) {
        memcpy(fs_wr_buff, buff, 512);
        ret = sd0_write(fs_wr_buff, sector);
        if(ret == false){
            return false;
        }
        sector++;
        buff += 512;
    }

    return true;
}
#endif

#if (GUI_SELECT == GUI_NO)
AT(.com_text.tft_spi)
void tft_bglight_en(void) {}
AT(.com_text.tft_spi)
void tft_frame_end(void) {}
AT(.com_text.tft_spi)
void tft_frame_start(void){}
AT(.com_text.tft_spi)
int tft_te_getnorm(void){return 0;}
void tft_set_temode(u8 mode) {}
void tft_set_baud(u8 baud1, u8 baud2) {}
//void tft_bglight_set_level(uint8_t level, bool stepless_en){}
void lcd_drv_set_brightness(u8 brightness) {}
#endif

#if ((!VIDEO_PLAY_EN) && (!PHOTO_VIEW_EN) && (!VIDEO_RECODE_TAKE_PHOTO_EN))
void thread_avi_create(void) {}
#endif

#if !NOC_FLASH_EN
typedef void NOC_Controller_t;
void noc_spifls_init(uint32_t quadenter, const uint8_t param[]) {}
void noc_sdr_init(const NOC_Controller_t *controller, uint32_t busmod){}
void noc_fls_clk_sel(const NOC_Controller_t *controller, uint32_t src, uint32_t div){}
void noc_spifls_dcache1_init(u32 base_addr){}
void noc1_clk_tune(uint32_t clko, uint32_t clki, uint32_t dqs){}
uint32_t noc1_clk_sel(uint32_t src, uint32_t div){return 0;}
void noc_spiflash_dcache1_init(u32 base_addr){}
void noc_spiflash_exit(void){}
#endif

#if !NOC_PSRAM_EN
typedef void nocspi_ce_spec_t;
uint32_t noc0_clk_sel(uint32_t src, uint32_t div){return 0;}
void noc0_clk_tune(uint32_t clko, uint32_t clki, uint32_t dqs){}
void noc_ddr_reset(void) {}
void noc_ddr_init(const NOC_Controller_t *controller, uint32_t busmod){}
void noc_set_ddr_delay(const NOC_Controller_t *controller, uint32_t dly){}
void noc_ddr_clk_sel(const NOC_Controller_t *controller, uint32_t src, uint32_t div){}
int32_t noc_dqs_tune(const NOC_Controller_t *controller, int (*verify)(const NOC_Controller_t *, uint32_t), uint32_t val){return 0;}
void bsp_nocspi_clk_sel (const NOC_Controller_t *controller, uint32_t src, uint32_t div, nocspi_ce_spec_t (*dev_ce)(uint32_t KHz)){}
void bsp_nocspi_clki_tune_do (const NOC_Controller_t *controller, uint32_t clki, uint32_t dqs){}
uint32_t random_check (uint32_t adr, unsigned len, uint32_t cache){return 0;}
uint32_t fix_check (uint32_t adr, unsigned len, uint32_t val, uint32_t cache){return 0;}
void bsp_nocspi_clki_tune_ddr (const NOC_Controller_t *controller, uint32_t adr, uint32_t dqs){}
void noc_set_devid(const NOC_Controller_t *controller, uint32_t id){}
void noc_set_adr_width(const NOC_Controller_t *controller, uint32_t num){}
void noc_clkn_enable(const NOC_Controller_t *controller, int en){}
void noc_ddrram_init(const uint8_t param[]){}
#endif
