#include "config.h"

/*
    SRAM0: 0x10000          32K
    SRAM1: 0x18000          32K
    SRAM3: 0x30000          64K
    SRAM4: 0x40000          64K
    SRAM5: 0x50000          64K

    GPU SRAM0: 0x310000     32K
    GPU SRAM1: 0x318000     32K
    GPU SRAM6: 0x60000      64K
    GPU SRAM7: 0x70000      64K
    GPU SRAM8: 0x80000      64K
*/

ENTRY(_start)

/* Define the flash max size */
__max_flash_size = (FLASH_CODE_SIZE);

__base = 0x10000000;

/* Define the area */
__stack_vma     = 0x10800;
__bram_vma      = 0x10C00;
__cram_vma      = 0x1AC00;
__share_vma     = 0x20000;
__sram_vma      = 0x20400;
__disp_vma      = 0x6A000;
__lpram_vma     = 0x90000;

__aram_vma      = 0xE0000;
__eram_vma      = 0xE5000;
__rram_vma      = 0xE6000;  //unuseable
__dram_vma      = 0xE8000;
__gram_vma      = 0x3F0000;	//GIF
__jram_vma      = 0x3F6000;	//JPEG
__maxram_vma    = 0x3F6900;
__comm_vma      = __sram_vma;

__stack_vma_size    = __bram_vma            - __stack_vma;  //1K
__bram_vma_size     = __cram_vma            - __bram_vma;   //40K;
__cram_vma_size     = __share_vma           - __cram_vma;   //21K;
__share_vma_size    = __sram_vma            - __share_vma;  // 1K
__sram_vma_size     = __disp_vma            - __sram_vma;   //351K;
__disp_vma_size     = __lpram_vma           - __disp_vma;   //96K;
__lpram_vma_size    = 0xA0000               - __lpram_vma;  //64K
__aram_vma_size     = __eram_vma            - __aram_vma;   //20K
__eram_vma_size     = __rram_vma-0x600      - __eram_vma;   //2.5K
__dram_vma_size     = (__gram_vma&0xfffff)  - __dram_vma;   //17.5K
__gam_vma_size      = __jram_vma-0x1000     - __gram_vma;   //20K
__jram_vma_size     = __maxram_vma          - __jram_vma;   //2.25K

#if (PHOTO_VIEW_EN || VIDEO_RECODE_TAKE_PHOTO_EN || VIDEO_PLAY_EN || FUNC_REC_OPUS_EN)
__camera_heap_size = 0x1f000;
__camera_heap_start = __disp_vma - __camera_heap_size;
#else
__camera_heap_start = __disp_vma;
#endif


//复用cram后13K空间用于跑通话算法程序
__dnn_vma_size = 9k;
__dnn_vma = __share_vma - __dnn_vma_size;
__sleep_vma = __dnn_vma;

__psram_vma     = 0x20000000;
__psram_size    = PSRAM_SIZE;

MEMORY
{
    init            : org = __base,             len = 512
    flash(rx)       : org = __base + 512,       len = __max_flash_size
    dnn_comm(rx)    : org = __dnn_vma,          len = __dnn_vma_size
    sleep_comm(rx)  : org = __sleep_vma,        len = 11k

    stack           : org = __stack_vma,        len = __stack_vma_size
    bram            : org = __bram_vma,         len = __bram_vma_size
    cram            : org = __cram_vma,         len = __cram_vma_size
    share           : org = __share_vma,        len = __share_vma_size
    sram            : org = __sram_vma,         len = __sram_vma_size
    disp            : org = __disp_vma,         len = __disp_vma_size
    lpram           : org = __lpram_vma,        len = __lpram_vma_size

    aram            : org = __aram_vma,         len = __aram_vma_size
    eram            : org = __eram_vma,         len = __eram_vma_size
    dram            : org = __dram_vma,         len = __dram_vma_size
    gram            : org = __gram_vma,         len = __gam_vma_size
    jram            : org = __jram_vma,         len = __jram_vma_size
#if PSRAM_SIZE
    psram           : org = __psram_vma,        len = __psram_size
#endif // PSRAM_SIZE
}

SECTIONS
{
    .init : {
        KEEP(*(.reset))
        . = ALIGN(512);
    } > init

    .comm : {
        __comm_start = .;
        KEEP(*(.vector))
        *(.plt)
        *(.com_text*)
        *(.com_text.bb.leisr)
        *(.com_text.stack.handler)
        *(.com_rodata*)
        *(.srodata*)
        *bsp_halt.o(.rodata*)
        *debug.o(.rodata*)
        *ele_image.o(.rodata*)
        *(.data*)
        *(.sdata*)
        *\ext_scan.o(.rodata*)
        *\ble_evt.o(.rodata*)
        *\hci_transport_h3.o(.rodata*)
        *gui.o(.rodata*)

        *(.text.src*)
        *(.rodata.src*)

        //dnn,dmdnn,aiaec及dmdnn_aiaec模型数据需要放在RAM
        *(.rodata.dnn.nn)
        *(.rodata.aiaec.ai)
        *(.rodata.dmdnn.nn)
        *(.rodata.dmdnn_aiaec.nn)

#if SECURITY_PAY_EN
        . = ALIGN(4);
        __alipay_start = .;
        *(.alipay.hs_HSI2CBuf)
        *hs_private.o(.bss*)
        *(.hs.pTestBuf)
        *(.alipay.hed_HEDI2CBuf)
        *hed_private.o(.bss*)
        *vendor_api.o(.bss*)
        *alipay_misc.c.obj(.bss*)
        *alipay*.c.obj(.sbss*)
        *vendor_os.o(.bss*)
        *se_bind.c.obj(.bss*)
        *alipay_commu_route.c.obj(.bss*)
        *unbind_proto.c.obj(.bss*)
        *ble_bind.c.obj(.bss*)
        *cJSON.c.obj(.bss*)
        *se_sst.c.obj(.bss*)
        *dev_comm.c.obj(.bss*)
        *common.o(.sbss*)
        *hs_private.o(.sbss*)
        *func_alipay.o(.sbss*)
        *ble_bind.c.obj(.sbss*)
        *cJSON.c.obj(.sbss*)
        *se_sst.c.obj(.sbss*)
        *dev_comm.c.obj(.sbss*)
        *unbind_proto.c.obj(.sbss*)
        __alipay_end = .;
        #endif // SECURITY_PAY_EN

        __comm_end = .;
        . = ALIGN(512);
    } > sram AT > flash

#if SECURITY_PAY_EN
    __alipay_size = __alipay_end - __alipay_start;
#endif // SECURITY_PAY_EN

    //DNN/DMDNN/AIAEC/DMDNN_AIAEC/AINS4代码, 算法五选一
    .dnn_comm : {
        *(.text.dnn*)
        *(.text.aiaec*)
        *(.text.dmdnn*)
        *(.text.dmdnn_aiaec*)
        *(.ains_text*)
        *(.text.ains4*)
        *(.rodata.ains4*)
        *(.ains_rodata*)
        *(.rodata.dnn*)
        *(.rodata.aiaec*)
        *(.rodata.dmdnn*)
        *(.rodata.dmdnn_aiaec*)
        . = ALIGN(512);
    } > dnn_comm AT > flash

    //仅sleep运行的代码或BUF放retention ram
    .sleep_comm : {
        *(.sleep_text*)
        *(.lp_buf*)
        . = ALIGN(512);
    } > sleep_comm AT > flash


    .flash : {
        __code_start_stream = .;
        *(.com_stream.text*)
        . = ALIGN(512);
        __code_end_stream = .;

        __code_start_record = .;
        *(.com_rec*)
        . = ALIGN(512);
        __code_end_record = .;

        __code_start_btdec_com = .;
        *(.rodata.btdec*)
        *(.btdec*)
        . = ALIGN(512);
        __code_end_btdec_com = .;

        __code_start_mp3dec = .;
        *(.mp3dec*)
        . = ALIGN(512);
        __code_end_mp3dec = .;

        __code_start_sbcdec = .;
        *(.sbcdec*)
        *(.sbc_com*)
        . = ALIGN(512);
        __code_end_sbcdec = .;

         __code_start_aacdec = .;
        *(.aacdec*)
        . = ALIGN(512);
        __code_end_aacdec = .;

        __code_start_sbcenc = .;
        *(.rodata.sbcenc*)
        *(.text.sbcenc*)
        . = ALIGN(512);
        __code_end_sbcenc = .;

        __code_start_mp3enc = .;
        *(.text.mp3enc*)
        *(.rodata.mp3enc*)
        . = ALIGN(512);
        __code_end_mp3enc = .;

        __code_start_mp2enc = .;
        *(.text.mp2enc*)
        *(.rodata.mp2enc*)
        . = ALIGN(512);
        __code_end_mp2enc = .;

        __code_start_wavdec = .;
        *(.wavdec*)
        . = ALIGN(512);
        __code_end_wavdec = .;

        __code_start_wmadec = .;
        *(.wmadec.bitstream)
        *(.wmadec.windows)
        *(.wmadec*)
        . = ALIGN(512);
        __code_end_wmadec = .;

        __code_start_apedec = .;
        *(.apedec*)
        . = ALIGN(512);
        __code_end_apedec = .;

        __code_start_flacdec = .;
        *(.flacdec*)
        . = ALIGN(512);
        __code_end_flacdec = .;

        __code_start_usbdev = .;
        *(.usbdev*)
        . = ALIGN(512);
        __code_end_usbdev = .;

        __code_start_wma_rlc44q = .;
        *(.wma_huff_rlc44q_table)
        . = ALIGN(512);
        __code_end_wma_rlc44q = .;

        __code_start_wma_rlc44o = .;
        *(.wma_huff_rlc44o_table)
        . = ALIGN(512);
        __code_end_wma_rlc44o = .;

        __code_start_wma_rlc16 = .;
        *(.wma_huff_rlc16_table)
        . = ALIGN(512);
        __code_end_wma_rlc16 = .;

        //此处放sleep与非sleep都要运行的代码 (sleep时会lock到cache ram)
        __code_start_sleep = .;
        *(.text.sleep.rf*)
        *(.text.charge_com*)
        *(.saradc_text*)
        *(.text.bt_sleep*)              //部分btstack代码lock起来，减少唤醒窗内的缺页
        *(.text.stack.link_key*)
        *(.text.stack.util)
        *(.rodata.bt_sleep*)
        . = ALIGN(512);
        __code_end_sleep = .;

        __code_start_pwrsave = .;
        *(.text.pwroff*)
        . = ALIGN(512);
        __code_end_pwrsave = .;

        __code_start_bb_test = .;
        *(.bb_test*)
        *(.bb_test_rodata*)
        . = ALIGN(512);
        __code_end_bb_test = .;

        __code_start_bt_voice = .;
        *(.bt_voice*)
        . = ALIGN(512);
        __code_end_bt_voice = .;

        __code_start_bt_aec = .;
        *(.text.aec.*)
        . = ALIGN(512);
        __code_end_bt_aec = .;

        __code_start_bt_alc = .;
        *(.text.alc.*)
        . = ALIGN(512);
        __code_end_bt_alc = .;

        __code_start_nr_far = .;
        *(.text.ains_far.*)
        *(.rodata.ains_far.*)
        . = ALIGN(512);
        __code_end_nr_far  = .;

        __code_start_wsola = .;
        *(.text.mav.*)
        *(.rodata.mav.*)
        . = ALIGN(512);
        __code_end_wsola   = .;

        __code_start_huart = .;
        *(.com_huart*)
        . = ALIGN(512);
        __code_end_huart = .;

         __code_start_piano = .;
        *(.piano_com*)
        . = ALIGN(512);
        __code_end_piano = .;

        __code_start_fota = .;
        *(.text.fot.cache*)
        . = ALIGN(512);
        __code_end_fota = .;

        __code_start_opus = .;
        *(.text.opus.asm)
        *(.text.opus.enc.asm)
        *(.text.opus.proc)
        *(.text.opus.enc.proc)
        *(.text.opus.ec_tell)
        *(.text.opus.enc.entenc)
        *(.text.opus.mathops)
        *(.rodata.opus*)
        . = ALIGN(512);
        __code_end_opus = .;

        __code_start_update = .;
        *(.text.update*)
        . = ALIGN(512);
        __code_end_update = .;

        __code_start_rf = .;
        *(.text.rf_init*)
        . = ALIGN(512);
        __code_end_rf = .;

        __code_start_sbr = .;
        *(.text.sbr*)
        *(.rodata.sbr*)
         . = ALIGN(512);
        __code_end_sbr = .;


#if (ASR_SELECT == ASR_WS_AIR)
        __code_start_ir = .;
        *(.kws_ir*)
        . = ALIGN(512);
        __code_end_ir = .;

        *(.ws_text.tdnn_weight_1)
        *(.ws_text.tdnn_weight_2)
        *(.ws_text.tdnn_weight_2)
        *(.ws_text.tdnn_weight_3)
        *(.ws_text.tdnn_weight_4)
        *(.ws_text.tdnn_weight_5)
        *(.ws_text.tdnn_weight_6)
        *(.ws_text.tdnn_weight_7)
#endif
        *(.irq_init.aligned)
        *(.text*)
        __code_end_all = .;
        *(.rodata*)
        *(.rela*)
        LONG(0)
        . = ALIGN(512);

    } > flash

    .stack (NOLOAD) : {
        __irq_stack_start = .;
        . = 0x400;
        __irq_stack = .;
    } > stack
    __irq_stack_size = __irq_stack - __irq_stack_start;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    .alipay_disp_ram __aram_vma (NOLOAD) : {
        *(.alipay_disp.*)
        . = 0x4000;
    } > aram
#endif // SECURITY_PAY_EN

    .disp (NOLOAD) : {
        __disp_start = .;
        *(.disp.temp_buf2)
        *(.disp.buf*)
        __disp_restore_start = .;
        *(.disp.ele_buf*)
        *(.disp.wdg_buf*)
        *(.disp.graphic*)
        *(.disp.widget*)
        *(.disp.compo*)
        __disp_restore_end = .;
        *(.disp.temp_buf)
        *(.disp.home_ram*)
        __disp_end = .;
    } > disp
    __disp_stack_size = __disp_end - __disp_start;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    __ali_trans_reuse_size = 0x3000 + COMPO_BUF_SIZE + 0x400;   //0x4C00

    .alipay_tmpbuf_ram __disp_vma (NOLOAD) : {
        . = __disp_vma_size - __ali_trans_reuse_size;
        *(.alipay_tmpbuf.*)
    } > disp
#endif // SECURITY_TRANSITCODE_EN

    .share (NOLOAD) : {
        __share_start = .;
        KEEP(*(.share.sys))
        . = 0x40;
        KEEP(*(.share.gpu))
        . = 0x100;
        KEEP(*(.share.*))
        . = 0x200;
        KEEP(*(.share_dvp.*))
        . = 0x280;
        KEEP(*(.share_video_jpeg.*))
        . = 0x2C0;
        KEEP(*(.share_photo_jpeg.*))
        . = 0x300;
        KEEP(*(.share_gui.*))
        . = 0x340;
        __share_end = .;
    } > share
    __share_size = __share_end - __share_start;

    .bss (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        *(.sbss*)
        *(.buf*)
        *(.dac_obuf*)
        *(.sdadc_buf*)
        *(.mavsco_buf*)
        *(.src_buf*)
        *(.heap.os)
        *(.heap.func)
        *(.sco_dump*)
        *(.ttf_buf*)
        *(.fatfs*)

        //如果不需要和音频共存可以复用到aram/dram
        *(.video_play.*)
        *(.photo_view.*)
        __bss_end = .;

#if (USE_APP_TYPE == APP_AB_LINK)
        *(.ble_cache*)
#endif

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
        __dram_ali_start = .;
        *js_api.c.obj(.bss*)
        *lib_sm2.c.obj(.bss*)
        *alipay_network_util.c.obj(.bss*)
        *secure_cert.c.obj(.bss*)
        __dram_ali_end = .;
        *(.alipay_data)
        *(.alipay_file)
#endif // SECURITY_TRANSITCODE_EN

        __hr_start = .;

        __hr_3300_3605_start = .;
        *algo.o(.bss*)
        *modle5_13.o(.bss*)
        *modle5_17.o(.bss*)
        *modle5_18.o(.bss*)
        *modle5_2.o(.bss*)
        *modle5_5.o(.bss*)
        *modle5_6.o(.bss*)
        *spo2Algo.o(.bss*)
        *spo2_cal.o(.bss*)
        *acPower_add_doublefre.o(.bss*)
        *(.hrs3300_buf)
        *(.hr_buf)
        *(.hr_data*)
        *hx3605_hrs_agc.o(.bss*)
        *hx3605_spo2_agc.o(.bss*)
        __hr_3300_3605_stop = .;
        __hr_stop = .;
        . = ALIGN(512);

        *(.sbr_buf.psinfo)
#if BT_EMIT_EN
        *(.custom_buf.*)
        *(.sbcenc.buf)
        *(.sbcbuf.enc)
#endif

        __asr_buf_start = .;
#if (ASR_SELECT == ASR_WS)
            *(.ws_asr.buf)
            *(.ws_asr.sum)
            *(.ws_asr.feat)
            *(.ws_asr*)
            *(.asr_dump*)
            *(.ws_asr.test)
            *(.asr_record)
            *(.asr_sco)
            *(.ws_text.strong)
#elif (ASR_SELECT == ASR_YJ)
            *(.asr.matrix)
            *(.asr.buff)
            *(.asr.pieces)
            *(.asr.save)
            *(.asr*)
            *(.asr_record)
	    	*(.asr_sco)
#elif (ASR_SELECT == ASR_WS_AIR)
            *(.ws_asr.buf)
            *(.ws_asr.sum)
            *(.ws_asr.feat)
            *(.ws_asr*)
            *(.asr_dump*)
            *(.ws_asr.test)
            *(.asr_record)
	    	*(.asr_sco)
            *(.ws_text.strong)

            *(.ir_prd.rcv)
            *(.ir_rmt.dat)
            *(.ir_pwm.out)
#endif
        __asr_buf_end = .;

#if ((PHOTO_VIEW_EN || VIDEO_RECODE_TAKE_PHOTO_EN || VIDEO_PLAY_EN || FUNC_REC_OPUS_EN) == 0)
        *(.music_play.*)
#endif

        *(.dynamic_pool.*)          //自动适配ab_malloc大小,剩下多少ram用多少
        __dynamic_pool_start = .;
    } > sram
    __bss_size = __bss_end - __bss_start;

#if (PHOTO_VIEW_EN || VIDEO_RECODE_TAKE_PHOTO_EN || VIDEO_PLAY_EN || FUNC_REC_OPUS_EN)
    .gpu_ram __camera_heap_start (NOLOAD) : {
        *(.gpu_buf.*)
        . = __camera_heap_size;
    } > sram

    .avi_ram __camera_heap_start (NOLOAD) : {
        *(.avi_play_fifo.*)
        . = __camera_heap_size;
    } > sram

    .photo_ram __camera_heap_start (NOLOAD) : {
        *(.photo_view_fifo.*)
        . = __camera_heap_size;
    } > sram

    .music_ram __camera_heap_start (NOLOAD) : {
        *(.music_play.*)
        . = __camera_heap_size;
    } > sram

    .opus_ram __camera_heap_start (NOLOAD) : {
        *(.opus_buf.dec*)
        *(.opus_buf.stack)
        *(.opus.buf.bsp)
        *(.opus_buf.input)
        *(.opus_buf.enc*)
        . = __camera_heap_size;
    } > sram

#endif

    /* 堆上限锚点：供 C 用 &__dynamic_pool_end - &__dynamic_pool_start 计算大小，避免 ABS 符号 GPREL 重定位失败 */
    .dynamic_pool_limit __camera_heap_start (NOLOAD) : {
        __dynamic_pool_end = .;
    } > sram

    __dynamic_pool_size = __camera_heap_start - __dynamic_pool_start;
    __asr_buf_size = __asr_buf_end - __asr_buf_start;

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
    __dram_ali_size = __dram_ali_end - __dram_ali_start;
#endif // SECURITY_PAY_EN

    .bram __bram_vma (NOLOAD) : {
        __bram_start = .;
        *(.rtc_cache)
        *(.btmem.bthw)
        *(.btmem*)
        *(.btstack_hci.buf)
        *(.ble_buf*)
#if (USE_APP_TYPE == APP_BLUE_FIT)
        *(.ble_cache*)
#endif
        *(.ancs_client*)
        *(.ams_client*)
        __bram_end = .;
    } > bram
    __bram_size = __bram_end - __bram_start;


    .a2dp_cram __cram_vma (NOLOAD) : {
        *(.aac_buf*)
        *(.a2dp_play*)
        *(.a2dp_cache1)
        *(.bt_id3_buf)
        . = 0x1A00;
    } > cram

    .sco_cram __cram_vma (NOLOAD) : {
        *(.sco_alg_buf*)
        *(.nr_buf*)
        *(.sco_data.pcm.aec*)
        *(.nr_far_buf.*)
        . = 0x1A00;
    } > cram

    .music_cram __cram_vma (NOLOAD) : {
        . = 0x1A00;
        *(.avio_buf)
        *(.udev_buf*)
        *(.msc_stm_buf)
        *(.sbcio_buf.dec)
        *(.rec_buf.wav)
        *(.adpcm_buf*)
        *(.codecs_buf*)
        . = (0x5400 - __dnn_vma_size);     //dnn use
    } > cram

    .sticker_cram __cram_vma (NOLOAD) : {
        . = 0x1A00;
        *(.img_sticker_buff.*)
    } > cram

    .aec_ram __aram_vma (NOLOAD) : {
        *(.aec_buf)
        . = 0x5000;
    } > aram

    .aram_sbc __aram_vma (NOLOAD) : {
        *(.sbc_cache_buf)
        . = 0x5000;
    } > aram

    .aram_ctp __aram_vma (NOLOAD) : {
        *(.ctp_ota*)
        . = 0x5000;
    } > aram

    .aram_music __aram_vma (NOLOAD) : {
        *(.mp3buf.dec)
        *(.mp3_id3_buf)
        *(.wavres.buf)
        *(.lrc_buf*)
        . = 0x5000;
    } > aram

    .aram_video_play __aram_vma (NOLOAD) : {
        . = 0x600;
        /**(.video_play.*)*/
        . = 0x5000;
    } > aram

    .aram_video_recode __aram_vma (NOLOAD) : {
        *(.video_recode.*)
        . = 0x5000;
    } > aram

    .aram_take_photo __aram_vma (NOLOAD) : {
        *(.take_photo.*)
        . = 0x5000;
    } > aram

    .aram_photo_view __aram_vma (NOLOAD) : {
        /**(.photo_view.*)*/
        . = 0x5000;
    } > aram

    .aram_wma __aram_vma (NOLOAD) : {
        *(.wma_coefs_buf0)
        *(.wma_coefs_buf)
    } > aram

    .aram_flac __aram_vma (NOLOAD) : {
        *(.flac_decoded_buf)
    } > aram

    .aram_m4a __aram_vma (NOLOAD) : {
        *(.aacio_buf*)
        *(.m4a_buf*)
        *(.sbr_buf*)
    } > aram

    .aram_mp2enc __aram_vma (NOLOAD) : {
        *(.mp2enc_buf.*)
        . = 0x4000;
    } > aram

    .aram_mp3enc __aram_vma (NOLOAD) : {
        *(.mp3enc_buf.*)
        . = 0x4000;
    } > aram

    .aram_fcc __aram_vma (NOLOAD) : {
        . = 0x1000;
        *(.fcc_buf.huart)
        *(.fcc_buf*)
    } > aram

    .aram_contacts __aram_vma (NOLOAD) : {
        *(.kvdb_buf*)
    } > aram

    .aram_opus __aram_vma (NOLOAD) : {
#if !FUNC_REC_OPUS_EN
        *(.opus_buf.stack)
#endif
    } > aram

    .dram_upd __dram_vma (NOLOAD) : {
        . = 0x2800;
        *(.upd_buf*)
        *(.ota_buf*)
        . = 0x3200;
    } > dram

    .dram_mp3 __dram_vma (NOLOAD) : {
        . = 0x1000;
        *(.mp3_decram_buf)
        . = 0x4500;
    } > dram

    .dram_ape __dram_vma (NOLOAD) : {
        *(.apebuf.cb)
        *(.apebuf.filter)
        *(.apebuf.decode)
        *(.apebuf.seektbl)
        . = 0x4500;
    } > dram

    .dram_wma __dram_vma (NOLOAD) : {
        *(.wmabuf*)
        *(.wma_ctx_buf)
        *(.wma_wincb_buf)
        *(.wma_id3_buf)
        . = 0x4500;
    } > dram

    .dram_flac __dram_vma (NOLOAD) : {
        *(.flacbuf*)
        *(.flac_decoded_buf1)
        *(.wavbuf.dec)
        . = 0x4500;
    } > dram

    .dram_aac __dram_vma (NOLOAD) : {
        *(.aac_decram_buf)
        *(.aac_cache*)
    } > dram

    .dram_sbc __dram_vma (NOLOAD) : {
        *(.sbc_decram_buf)
        . = 0xD38;
        *(.sbcbuf.dec)
        *(.tsco_cache)
        *(.sco_cache*)
        *(.sco_data*)
        *(.sco_hci)
        *(.plc_buf)
        *(.plc_data*)
        *(.sbcenc.buf)
        . = 0x4500;
     }  > dram

    .dram_rec __dram_vma (NOLOAD) : {
        *(.wavenc.sco)
        . = 0xD38;
        *(.rec_buf.*)
        . = 0x4500;
    } > dram

    .dram_opus_enc __dram_vma (NOLOAD) : {
#if !FUNC_REC_OPUS_EN
        *(.opus.buf.bsp)
        *(.opus_buf.input)
        *(.opus_buf.enc*)
        . = 0x3000;
#endif
    } > dram

    .dram_opus_dec __dram_vma (NOLOAD) : {
#if !FUNC_REC_OPUS_EN
        *(.opus_buf.dec*)
#endif
    } > dram

    .dram_usb __dram_vma (NOLOAD) : {
        *(.usb.device.desc.buf)
        *(.ude.aubuf)
        . = 0x4500;
    } > dram

    .dram_fot __dram_vma (NOLOAD) : {
        *(.fot_data.*)
    } > dram

    .eram_mp3enc __eram_vma (NOLOAD) : {
        *(.mp3_enc_hwbuf)
    } > eram

    .heap_ram __lpram_vma (NOLOAD) : {
        *(.agc_buf*)
        *(.ains_buf*)
        *(.dnn_buf*)
        *(.aiaec_buf*)
        *(.dmdnn_data*)
        *(.dmdnn_aiaec_buf*)
    } > lpram

    .sleep_ram __lpram_vma (NOLOAD) : {
        *(.sleep_backup.*)      //系统休眠备份(sys+gui)
        *(.sensorhub.*)         //1024
    } > lpram

    .jpeg_ram __lpram_vma (NOLOAD) : {
        *(.jpeg_buf.*)
    } > lpram

    .take_ram __lpram_vma (NOLOAD) : {
        *(.take_photo_buf.*)
    } > lpram

    .play_ram __lpram_vma (NOLOAD) : {
        *(.video_play_buf.*)
    } > lpram

    .photo_ram __lpram_vma (NOLOAD) : {
        *(.photo_view_buf.*)
    } > lpram

    .noise_alg_ram __lpram_vma (NOLOAD) : {
        *(.dnn_data.*)
    } > lpram

    .qr_ram __lpram_vma (NOLOAD) : {
        *(.qr_buff)
    } > lpram

    .qr_exram __lpram_vma (NOLOAD) : {
        *(.qr_ex_buff)
    } > lpram


    .gram_gif __gram_vma (NOLOAD) : {
        KEEP(*(.gif_buf.*))
    } > gram

    .gram_jpg __gram_vma (NOLOAD) : {
        KEEP(*(.jpg_gbuf.*))
#if WATER_MARK_EN
        *(.watermark.*)
#endif // WATER_MARK_EN
#if FLASH_DISK_EN
        *(.spiflash_disk*)
#endif
    } > gram


#if PSRAM_SIZE
#if (PHOTO_VIEW_EN || VIDEO_RECODE_TAKE_PHOTO_EN || VIDEO_PLAY_EN)
    .psram_dvp __psram_vma (NOLOAD) : {
        *(.psram_imgage_sensor.*)
        *(.psram_imgage_sensor_disp.*)
        __psram_dvp_end = .;
    } > psram

    .psram_video_play __psram_vma (NOLOAD) : {
        *(.psram_video_play.*)
        *(.psram_video_play_disp.*)
        . = 0x1EB000;
    } > psram

    .psram_photo_view __psram_vma (NOLOAD) : {
        *(.psram_photo_view.*)
        *(.psram_photo_view_disp.*)
        . = 0x1EB000;
    } > psram

    .psram_video_recode __psram_dvp_end (NOLOAD) : {
        *(.psram_video_recode.*)
        *(.psram_video_recode_disp.*)
        . = 0x1EB000;
    } > psram

    .psram_take_photo __psram_dvp_end (NOLOAD) : {
        *(.psram_take_photo.*)
        *(.psram_take_photo_disp.*)
        . = 0x1EB000;
    } > psram

#endif

    .psram_gif __psram_vma (NOLOAD) : {
        *(.gif.*)
        . = 0x1EB000;
    } > psram

    .psram_other __psram_vma (NOLOAD) : {
#if (PHOTO_VIEW_EN || VIDEO_RECODE_TAKE_PHOTO_EN || VIDEO_PLAY_EN)
        . = 0x1EB000;
#endif
        *(.img_test.*)
        *(.psram_gui.*)
        *(.psram_buf*)
        *(.screenshot_buf*)
        __amap_buf_start = .;
#if AWK_MAP_EN
        *(.amap_buf)
#endif
        __amap_buf_end = .;
    } > psram
    __amap_buf_size = __amap_buf_end - __amap_buf_start;
#endif // PSRAM_SIZE

}

/* Calc the lma */
__bank_size = SIZEOF(.flash);
__comm_lma = LOADADDR(.comm);
__comm_size = SIZEOF(.comm);
__dnn_lma = LOADADDR(.dnn_comm);
__dnn_size = SIZEOF(.dnn_comm);
__sleep_lma = LOADADDR(.sleep_comm);
__sleep_size = SIZEOF(.sleep_comm);

