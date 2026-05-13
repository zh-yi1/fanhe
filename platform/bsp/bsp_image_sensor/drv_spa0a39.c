#include "include.h"


#if AVI_DVP_USE_CAMERA

#if IMG_SENSOR_SPA0A39_EN
#define SP0A39_ID   0x42

static void sensor_drv_spa0a39_init(void)
{
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x00);
    /*
    [7] ds_i2c
    [6:5] ds_data
    [4] pd_dig
    */
    img_write_Reg8Data8(SP0A39_ID, 0x1d, 0x25);
    img_write_Reg8Data8(SP0A39_ID, 0x31, 0x00); //comm_ctrl_reg   0111

    /*
    [2] ext_sync_sel
    [1:0] pvdd_sel
    */
    img_write_Reg8Data8(SP0A39_ID, 0x32, 0x01);
    img_write_Reg8Data8(SP0A39_ID, 0x30, 0x01); //[2:0] dclk_ctrl

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x00);  //
    img_write_Reg8Data8(SP0A39_ID, 0xf0, 0xff); //gb_suboffset
    img_write_Reg8Data8(SP0A39_ID, 0xf1, 0xff); //gr_suboffset
    img_write_Reg8Data8(SP0A39_ID, 0xf2, 0xff); //red_suboffset
    img_write_Reg8Data8(SP0A39_ID, 0xf3, 0xff); //blue_suboffset
    img_write_Reg8Data8(SP0A39_ID, 0xfc, 0x50); //blc_dpc_th_p_8lsb

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01);
    /*
    [5:4] lsc_position_set Raw order set for awb gain and lsc gain
    [3:2] awb_position_set
    [1:0] bayer order set for demosaic
    */
    img_write_Reg8Data8(SP0A39_ID, 0x5d, 0x01);
    /*
    [7] dpc_flt_en
    [6] dpc_dirtm_en
    [5] gamma_en2(raw gamma)
    [4] domu_en
    [3] sharp_mdy_en
    [2] dpc_low_en
    [1] gamma correction(y gamma)
    [0] color correction
    */
    img_write_Reg8Data8(SP0A39_ID, 0x34, 0xe3);
    /*
    [7] raw_mid8_en
    [6] raw_low8_en
    [5] add_row4_col4_en
    [4] swap byte
    0:u y v y
    1:y u y v
    [3] unpro_raw_out_en
    [2] y_out_en
    [0] yuv_order
    0: U Y V Y
    1: V Y U Y
    */
    img_write_Reg8Data8(SP0A39_ID, 0x35, 0x10);

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x00); //; ae setting
    img_write_Reg8Data8(SP0A39_ID, 0x03, 0x03); //Integration time high 3 bits
    img_write_Reg8Data8(SP0A39_ID, 0x04, 0x6c); //Integration time low 8 bits
    img_write_Reg8Data8(SP0A39_ID, 0x24, 0x10); //pga_gain_ctl
    img_write_Reg8Data8(SP0A39_ID, 0xef, 0x40);
    img_write_Reg8Data8(SP0A39_ID, 0x06, 0x00); //Vsync blank
    img_write_Reg8Data8(SP0A39_ID, 0x09, 0x01); //hblank_4msb
    img_write_Reg8Data8(SP0A39_ID, 0x0a, 0x80); //hblank_8lsb
    img_write_Reg8Data8(SP0A39_ID, 0x10, 0x0a);//;07  by wwj 0124
    img_write_Reg8Data8(SP0A39_ID, 0x11, 0x02);//;04  by wwj 0124
    /*
    [1] dac_mode
    [0] FPN_33ms_timing_sel
    */
    img_write_Reg8Data8(SP0A39_ID, 0x16, 0x01);
    /*
    [6:4] icomp1
    [1:0] icomp2
    */
    img_write_Reg8Data8(SP0A39_ID, 0x19, 0x22);
    img_write_Reg8Data8(SP0A39_ID, 0x1e, 0x60);//;58 by wwj 0124
    img_write_Reg8Data8(SP0A39_ID, 0x29, 0x60);//;48 by wwj 0124
    /*
    [5] cp_num
    [4:3] spi_clk_delay
    [2:0] vcp_sel_ctl
    */
    img_write_Reg8Data8(SP0A39_ID, 0x13, 0x37);
    /*
    [5:4] ds_hsync
    [3:2] ds_vsync
    [1:0] ds_pclk
    */
    img_write_Reg8Data8(SP0A39_ID, 0x14, 0x01);
    img_write_Reg8Data8(SP0A39_ID, 0x25, 0x01);
    img_write_Reg8Data8(SP0A39_ID, 0x2a, 0x06);
    img_write_Reg8Data8(SP0A39_ID, 0x27, 0x00);//   by wwj 0218
    img_write_Reg8Data8(SP0A39_ID, 0x54, 0x00);
    img_write_Reg8Data8(SP0A39_ID, 0x55, 0x10);
    img_write_Reg8Data8(SP0A39_ID, 0x58, 0x28);//;38 by wwj 0124
    img_write_Reg8Data8(SP0A39_ID, 0x5d, 0x12);
    img_write_Reg8Data8(SP0A39_ID, 0x63, 0x00);
    img_write_Reg8Data8(SP0A39_ID, 0x64, 0x00);
    img_write_Reg8Data8(SP0A39_ID, 0x66, 0x2a);//;28   by wwj 0218
    img_write_Reg8Data8(SP0A39_ID, 0x68, 0x28);//;2a   by wwj 0218
    img_write_Reg8Data8(SP0A39_ID, 0x72, 0x32);//;3a by wwj 0124
    img_write_Reg8Data8(SP0A39_ID, 0x73, 0x0a);
    img_write_Reg8Data8(SP0A39_ID, 0x75, 0x30);//;48 by wwj 0124
    img_write_Reg8Data8(SP0A39_ID, 0x76, 0x0a);
    img_write_Reg8Data8(SP0A39_ID, 0x1f, 0x99);//;77  by wwj 0215
    img_write_Reg8Data8(SP0A39_ID, 0x20, 0x09);//;/7 by wwj 0215
    img_write_Reg8Data8(SP0A39_ID, 0xfb, 0x16);

    //AEC
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01);
    /*
    [6] lum_down_en
    [5] exp_max_en
    [4] exp_accr_sel
    [3] outdoor_mode_en
    1: enable
    0: disable
    [2:0] mean_mode_reg
    The value must be bigger than 2
    */
    img_write_Reg8Data8(SP0A39_ID, 0xf2, 0x69);
    img_write_Reg8Data8(SP0A39_ID, 0xf7, 0x97); //;ABF exp base time is line number equal to 10 ms
    img_write_Reg8Data8(SP0A39_ID, 0x02, 0x04); //;Exp_max_indr   帧率限制
    img_write_Reg8Data8(SP0A39_ID, 0x03, 0x01); //;Exp_min_indr
    img_write_Reg8Data8(SP0A39_ID, 0x06, 0x8a); //;exp_max_outdr
    img_write_Reg8Data8(SP0A39_ID, 0x08, 0x04); //;Exp_min_outdr  帧率限制

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x02); //;ae gain &status
    img_write_Reg8Data8(SP0A39_ID, 0xb8, 0x50); //;Luminance Low threshold from normal mode to dummy mode
    img_write_Reg8Data8(SP0A39_ID, 0xb9, 0xff); //;Luminance Low threshold from dummy mode to normal mode
    img_write_Reg8Data8(SP0A39_ID, 0xba, 0x40); //;Luminance Low threshold from dummy mode to low light mod
    img_write_Reg8Data8(SP0A39_ID, 0xbb, 0x45); //;Luminance high threshold from low light mode to dummy mod
    img_write_Reg8Data8(SP0A39_ID, 0xbc, 0xc0); //;RPC low threshold from dummy mode to low light mod
    img_write_Reg8Data8(SP0A39_ID, 0xbd, 0x50); //;
    img_write_Reg8Data8(SP0A39_ID, 0xbe, 0xb8); //;exp_heq_dummy 8 LSM Exp_heq_dummy: exposure time low threshold from normal mode to dummy mode
    img_write_Reg8Data8(SP0A39_ID, 0xbf, 0x04); //Exp_heq_dummy_5hsm
    img_write_Reg8Data8(SP0A39_ID, 0xd0, 0xb8); //Exp 8lsb threshold from dummy mode to low mode
    img_write_Reg8Data8(SP0A39_ID, 0xd1, 0x04); //Exp 3msb threshold from dummy mode to low mode

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01); //;rpc
    img_write_Reg8Data8(SP0A39_ID, 0xc0, 0x1f); //;rpc_1base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc1, 0x18); //;rpc_2base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc2, 0x15); //;rpc_3base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc3, 0x13); //;rpc_4base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc4, 0x13); //;rpc_5base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc5, 0x12); //;rpc_6base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc6, 0x12); //;rpc_7base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc7, 0x11); //;rpc_8base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc8, 0x11); //;rpc_9base_max
    img_write_Reg8Data8(SP0A39_ID, 0xc9, 0x11); //;rpc_10base_max
    img_write_Reg8Data8(SP0A39_ID, 0xca, 0x10); //;rpc_11base_max
    img_write_Reg8Data8(SP0A39_ID, 0xf3, 0x10); //;rpc_12base_max
    img_write_Reg8Data8(SP0A39_ID, 0xf4, 0x10); //;rpc_13base_max

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01); //;ae min gain
    img_write_Reg8Data8(SP0A39_ID, 0x04, 0xd0); //;rpc_max_indr      ff
    img_write_Reg8Data8(SP0A39_ID, 0x05, 0x10); //;rpc_min_indr
    img_write_Reg8Data8(SP0A39_ID, 0x0a, 0x30); //;rpc_max_outdr
    img_write_Reg8Data8(SP0A39_ID, 0x0b, 0x10); //;rpc_min_outdr

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01); //;target
    img_write_Reg8Data8(SP0A39_ID, 0xcb, 0x38); //;Hold threshold of dark pixel value at indoor       38
    img_write_Reg8Data8(SP0A39_ID, 0xcc, 0x35); //;Hold threshold of bright pixel value at indoor      35
    img_write_Reg8Data8(SP0A39_ID, 0xcd, 0x03); //;Hold threshold of dark pixel value at outdoor
    img_write_Reg8Data8(SP0A39_ID, 0xce, 0x05); //;Hold threshold of bright pixel value at outdoor

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x00); //;
    /*
    [5] bl_gain_en Should be set to 1'b0
    [4] bl_dpc_en
        1’b1: enable dark row dpc
        1’b0: disable
    [2] exp_update_en
        1’b1: blc update when exp change
        1’b0: disable
    [1] rpc_update_en
        1’b1: blc update when rpc change
        1’b0: disable
    [0] free_update_en
        1’b1: blc update every frame
        1’b0: disable
    */
    img_write_Reg8Data8(SP0A39_ID, 0xfb, 0x16); //;
    img_write_Reg8Data8(SP0A39_ID, 0x35, 0xaa); //; Glb_gain[7:0]


    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01); //;lsc
    img_write_Reg8Data8(SP0A39_ID, 0x1e, 0x00); //Lsc_sig_ru[6:4] Lsc_sig_lu[2:0]
    img_write_Reg8Data8(SP0A39_ID, 0x20, 0x00); //Lsc_sig_rd[6:4] Lsc_sig_ld[2:0]
    img_write_Reg8Data8(SP0A39_ID, 0x84, 0x25); //Lens shading parameter, left side, R channel
    img_write_Reg8Data8(SP0A39_ID, 0x85, 0x25); //Lens shading parameter, right side, R channel
    img_write_Reg8Data8(SP0A39_ID, 0x86, 0x1f); //Lens shading parameter, upper side, R channel
    img_write_Reg8Data8(SP0A39_ID, 0x87, 0x23);
    img_write_Reg8Data8(SP0A39_ID, 0x88, 0x1a);  //1c   g left
    img_write_Reg8Data8(SP0A39_ID, 0x89, 0x1c);   //20
    img_write_Reg8Data8(SP0A39_ID, 0x8a, 0x15); //g down       1a
    img_write_Reg8Data8(SP0A39_ID, 0x8b, 0x15);
    img_write_Reg8Data8(SP0A39_ID, 0x8c, 0x15);
    img_write_Reg8Data8(SP0A39_ID, 0x8d, 0x1a);
    img_write_Reg8Data8(SP0A39_ID, 0x8e, 0x0a);
    img_write_Reg8Data8(SP0A39_ID, 0x8f, 0x13);
    img_write_Reg8Data8(SP0A39_ID, 0x90, 0x13);
    img_write_Reg8Data8(SP0A39_ID, 0x91, 0x00);
    img_write_Reg8Data8(SP0A39_ID, 0x92, 0x0a);
    img_write_Reg8Data8(SP0A39_ID, 0x93, 0x02);//08 red down
    img_write_Reg8Data8(SP0A39_ID, 0x94, 0x12);
    img_write_Reg8Data8(SP0A39_ID, 0x95, 0x00);
    img_write_Reg8Data8(SP0A39_ID, 0x96, 0x0a);
    img_write_Reg8Data8(SP0A39_ID, 0x97, 0x08);
    img_write_Reg8Data8(SP0A39_ID, 0x98, 0x15);
    img_write_Reg8Data8(SP0A39_ID, 0x99, 0x05); //00 up blue
    img_write_Reg8Data8(SP0A39_ID, 0x9a, 0x0a);
    img_write_Reg8Data8(SP0A39_ID, 0x9b, 0x09);//Lens shading parameter, right-down side, B channel   05
    img_write_Reg8Data8(SP0A39_ID, 0xe8, 0x20);//Ae_thr_low
    img_write_Reg8Data8(SP0A39_ID, 0xe9, 0x0f);//Slope_k
    img_write_Reg8Data8(SP0A39_ID, 0xea, 0x00);//Ae_gain_min_ratio
    img_write_Reg8Data8(SP0A39_ID, 0xbd, 0x1e);
    img_write_Reg8Data8(SP0A39_ID, 0xbe, 0x00);

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01);
    img_write_Reg8Data8(SP0A39_ID, 0xa4, 0x00); //;lum_limit
    img_write_Reg8Data8(SP0A39_ID, 0x0e, 0x80); //;heq_mean  outdoor,indoor contrast     80
    img_write_Reg8Data8(SP0A39_ID, 0x18, 0x40); //;heq_mean  dummy,lowlight      80
    img_write_Reg8Data8(SP0A39_ID, 0x0f, 0x20); //;k_max
    img_write_Reg8Data8(SP0A39_ID, 0x10, 0x80); //;ku_outdoor     90
    img_write_Reg8Data8(SP0A39_ID, 0x11, 0x80); //;ku_nr
    img_write_Reg8Data8(SP0A39_ID, 0x12, 0x80); //;ku_dummy
    img_write_Reg8Data8(SP0A39_ID, 0x13, 0x80); //;ku_low         a0
    img_write_Reg8Data8(SP0A39_ID, 0x14, 0x80); //;kl_outdoor
    img_write_Reg8Data8(SP0A39_ID, 0x15, 0x80); //;kl_nr          90
    img_write_Reg8Data8(SP0A39_ID, 0x16, 0x80); //;kl_dummy       85
    img_write_Reg8Data8(SP0A39_ID, 0x17, 0x80); //;kl_low        85
    //;gamma
    img_write_Reg8Data8(SP0A39_ID, 0x6e, 0x00);
    img_write_Reg8Data8(SP0A39_ID, 0x6f, 0x03);
    img_write_Reg8Data8(SP0A39_ID, 0x70, 0x07);
    img_write_Reg8Data8(SP0A39_ID, 0x71, 0x0d);
    img_write_Reg8Data8(SP0A39_ID, 0x72, 0x17);
    img_write_Reg8Data8(SP0A39_ID, 0x73, 0x29);
    img_write_Reg8Data8(SP0A39_ID, 0x74, 0x3d);
    img_write_Reg8Data8(SP0A39_ID, 0x75, 0x4f);
    img_write_Reg8Data8(SP0A39_ID, 0x76, 0x5f);
    img_write_Reg8Data8(SP0A39_ID, 0x77, 0x79);
    img_write_Reg8Data8(SP0A39_ID, 0x78, 0x8c);
    img_write_Reg8Data8(SP0A39_ID, 0x79, 0x9d);
    img_write_Reg8Data8(SP0A39_ID, 0x7a, 0xa9);
    img_write_Reg8Data8(SP0A39_ID, 0x7b, 0xb3);
    img_write_Reg8Data8(SP0A39_ID, 0x7c, 0xbe);
    img_write_Reg8Data8(SP0A39_ID, 0x7d, 0xc7);
    img_write_Reg8Data8(SP0A39_ID, 0x7e, 0xd0);
    img_write_Reg8Data8(SP0A39_ID, 0x7f, 0xd6);
    img_write_Reg8Data8(SP0A39_ID, 0x80, 0xde);
    img_write_Reg8Data8(SP0A39_ID, 0x81, 0xe4);
    img_write_Reg8Data8(SP0A39_ID, 0x82, 0xe9);
    img_write_Reg8Data8(SP0A39_ID, 0x83, 0xee);

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x02); //;skin detect
    img_write_Reg8Data8(SP0A39_ID, 0x09, 0x06);
    img_write_Reg8Data8(SP0A39_ID, 0x0d, 0x1a);
    img_write_Reg8Data8(SP0A39_ID, 0x1c, 0x09);
    img_write_Reg8Data8(SP0A39_ID, 0x1d, 0x03);
    img_write_Reg8Data8(SP0A39_ID, 0x1e, 0x10); //;awb
    img_write_Reg8Data8(SP0A39_ID, 0x1f, 0x06);

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01);
    img_write_Reg8Data8(SP0A39_ID, 0x32, 0x00);

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x02);
    img_write_Reg8Data8(SP0A39_ID, 0x26, 0xcb);
    img_write_Reg8Data8(SP0A39_ID, 0x27, 0xc2);
    img_write_Reg8Data8(SP0A39_ID, 0x10, 0x00); //;br offset
    img_write_Reg8Data8(SP0A39_ID, 0x11, 0x00); //;br offset_f
    img_write_Reg8Data8(SP0A39_ID, 0x18, 0x17);
    img_write_Reg8Data8(SP0A39_ID, 0x19, 0x36);
    img_write_Reg8Data8(SP0A39_ID, 0x2a, 0x01);
    img_write_Reg8Data8(SP0A39_ID, 0x2b, 0x10);
    img_write_Reg8Data8(SP0A39_ID, 0x28, 0xf8);
    img_write_Reg8Data8(SP0A39_ID, 0x29, 0x08);
    img_write_Reg8Data8(SP0A39_ID, 0x66, 0x5F); //;d65 10
    img_write_Reg8Data8(SP0A39_ID, 0x67, 0x7f);
    img_write_Reg8Data8(SP0A39_ID, 0x68, 0xE0);
    img_write_Reg8Data8(SP0A39_ID, 0x69, 0x10);
    img_write_Reg8Data8(SP0A39_ID, 0x6a, 0xa6);
    img_write_Reg8Data8(SP0A39_ID, 0x7c, 0x4A); //;indoor 11
    img_write_Reg8Data8(SP0A39_ID, 0x7d, 0x80);
    img_write_Reg8Data8(SP0A39_ID, 0x7e, 0x00);
    img_write_Reg8Data8(SP0A39_ID, 0x7f, 0x30);
    img_write_Reg8Data8(SP0A39_ID, 0x80, 0xaa);
    img_write_Reg8Data8(SP0A39_ID, 0x70, 0x32); //;cwf 12
    img_write_Reg8Data8(SP0A39_ID, 0x71, 0x60);
    img_write_Reg8Data8(SP0A39_ID, 0x72, 0x30);
    img_write_Reg8Data8(SP0A39_ID, 0x73, 0x5a);
    img_write_Reg8Data8(SP0A39_ID, 0x74, 0xaa);
    img_write_Reg8Data8(SP0A39_ID, 0x6b, 0xff); //;tl84 13
    img_write_Reg8Data8(SP0A39_ID, 0x6c, 0x50);
    img_write_Reg8Data8(SP0A39_ID, 0x6d, 0x40);
    img_write_Reg8Data8(SP0A39_ID, 0x6e, 0x60);
    img_write_Reg8Data8(SP0A39_ID, 0x6f, 0x6a);
    img_write_Reg8Data8(SP0A39_ID, 0x61, 0xff); //;f 14
    img_write_Reg8Data8(SP0A39_ID, 0x62, 0x27);
    img_write_Reg8Data8(SP0A39_ID, 0x63, 0x51);
    img_write_Reg8Data8(SP0A39_ID, 0x64, 0x7f);
    img_write_Reg8Data8(SP0A39_ID, 0x65, 0x6a);
    img_write_Reg8Data8(SP0A39_ID, 0x75, 0x80);
    img_write_Reg8Data8(SP0A39_ID, 0x76, 0x09);
    img_write_Reg8Data8(SP0A39_ID, 0x77, 0x02);
    img_write_Reg8Data8(SP0A39_ID, 0x0e, 0x12);
    img_write_Reg8Data8(SP0A39_ID, 0x3b, 0x09);
    img_write_Reg8Data8(SP0A39_ID, 0x48, 0xea); //;green u center            ea
    img_write_Reg8Data8(SP0A39_ID, 0x49, 0xfc); //;green v center
    img_write_Reg8Data8(SP0A39_ID, 0x4a, 0x05); //;green range
    img_write_Reg8Data8(SP0A39_ID, 0x02, 0x00); //;outdoor awb exp 5msb
    img_write_Reg8Data8(SP0A39_ID, 0x03, 0x88); //;outdoor awb exp 8lsb
    img_write_Reg8Data8(SP0A39_ID, 0xf5, 0xfe); //;a8 ;outdoor awb rgain top
    img_write_Reg8Data8(SP0A39_ID, 0x22, 0xfe); //blue top fe
    img_write_Reg8Data8(SP0A39_ID, 0x20, 0xd8);   //red top fe
    img_write_Reg8Data8(SP0A39_ID, 0x23, 0xa8);  //  blue bottom 70
    img_write_Reg8Data8(SP0A39_ID, 0xf7, 0xfe);


    /*sharpen*/
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x02);
    /*
    [3] raw sharpen enable, in outdoor
    [2] raw sharpen enable, in normal
    [1] raw sharpen enable, in dummy
    [0] raw sharpen enable, in low light
    0x0f
    */
    img_write_Reg8Data8(SP0A39_ID, 0xde, 0x0f);
    img_write_Reg8Data8(SP0A39_ID, 0xcf, 0x0a); //;Sharp_flat_thr0
    /*
    P2:0xd7 sharp_flat_thr1 7~0 Edge threshold in analog gain is 2* 0x10
    P2:0xd8 sharp_flat_thr2 7~0 Edge threshold in analog gain is 4* 0x16
    P2:0xd9 sharp_flat_thr3 7~0 Edge threshold in analog gain is 8* 0x18
    P2:0xda sharp_flat_thr4 7~0 Edge threshold in analog gain is 16* 0x20
    */
    img_write_Reg8Data8(SP0A39_ID, 0xd7, 0x0a);
    img_write_Reg8Data8(SP0A39_ID, 0xd8, 0x12);
    img_write_Reg8Data8(SP0A39_ID, 0xd9, 0x14);
    img_write_Reg8Data8(SP0A39_ID, 0xda, 0x1a);

    img_write_Reg8Data8(SP0A39_ID, 0xdc, 0x07); //Sharpness gain in skin area
    img_write_Reg8Data8(SP0A39_ID, 0xe8, 0x60); //;正值范围
    img_write_Reg8Data8(SP0A39_ID, 0xe9, 0x40);
    img_write_Reg8Data8(SP0A39_ID, 0xea, 0x40);
    img_write_Reg8Data8(SP0A39_ID, 0xeb, 0x30);
    img_write_Reg8Data8(SP0A39_ID, 0xec, 0x60); //;负值范围
    img_write_Reg8Data8(SP0A39_ID, 0xed, 0x50);
    img_write_Reg8Data8(SP0A39_ID, 0xee, 0x40);
    img_write_Reg8Data8(SP0A39_ID, 0xef, 0x30);
    img_write_Reg8Data8(SP0A39_ID, 0xd3, 0x30); //Sharp_ofst_pos;正边缘锐化范围，限制正边缘的锐化量； 该值越大，锐化范围越大
    img_write_Reg8Data8(SP0A39_ID, 0xd4, 0x30); //Sharp_ofst_neg;负   c0
    img_write_Reg8Data8(SP0A39_ID, 0xd5, 0x50); //Sharp_ofst_min_nr;限制锐化的最小值范围
    img_write_Reg8Data8(SP0A39_ID, 0xd6, 0x0b);
    img_write_Reg8Data8(SP0A39_ID, 0xf0, 0x7f);

    //;skin sharpen
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01); //
    img_write_Reg8Data8(SP0A39_ID, 0xb1, 0xf0); //Skin_sharp_delta
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x02);
    img_write_Reg8Data8(SP0A39_ID, 0xdc, 0x07); //skin_sharp_sel
    img_write_Reg8Data8(SP0A39_ID, 0x05, 0x08); //Skin_num_th2

    //;bpc
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01);
    img_write_Reg8Data8(SP0A39_ID, 0x26, 0x33); //[7:4] dpc_range_ratio_outdoor[3:0] dpc_range_ratio_nr
    img_write_Reg8Data8(SP0A39_ID, 0x27, 0x99); //[7:4] dpc_range_ratio_dummy [3:0] dpc_range_ratio_low
    img_write_Reg8Data8(SP0A39_ID, 0x62, 0xf0); //Dpc_grad_thr_outdoor
    img_write_Reg8Data8(SP0A39_ID, 0x63, 0x80); //Dpc_grad_thr_nr
    img_write_Reg8Data8(SP0A39_ID, 0x64, 0x80); //Dpc_grad_thr_dummy
    img_write_Reg8Data8(SP0A39_ID, 0x65, 0x20); //Dpc_grad_thr_low

    //;dns
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x02);
    /*
    [7] raw_gflt_en_outdoor
    [6] raw_gflt_en_nr
    [5] raw_gflt_en_dummy
    [4] raw_gflt_en_low
    [3] raw_denoise_en_outdoor
    [2] raw_denoise_en_nr
    [1] raw_denoise_en_dummy
    [0] raw_denoise_en_low
    */
    img_write_Reg8Data8(SP0A39_ID, 0xdd, 0xff);

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01);
    /*
    P1:0xa8 raw_dif_thr_outdoor 7~0 Raw_dif_thr_outdoor 0x04
    P1:0xa9 raw_dif_thr_normal 7~0 Raw_dif_thr_normal 0x04
    P1:0xaa raw_dif_thr_dummy 7~0 Raw_dif_thr_dummy 0x04
    P1:0xab raw_dif_thr_low_light 7~0 Raw_dif_thr_low_light 0x04
    */
    img_write_Reg8Data8(SP0A39_ID, 0xa8, 0x09); //;单通道间平滑阈值       00
    img_write_Reg8Data8(SP0A39_ID, 0xa9, 0x09);
    img_write_Reg8Data8(SP0A39_ID, 0xaa, 0x09);
    img_write_Reg8Data8(SP0A39_ID, 0xab, 0x0c);

    /*
    P1:0xcf raw_gflt_fac_outdoor 7~0 Raw_gflt_fac_outdoor 0x00
    P1:0xd0 raw_gflt_fac_normal 7~0 Raw_gflt_fac_normal 0x00
    P1:0xd1 raw_gflt_fac_dummy 7~0 Raw_gflt_fac_dummy 0x00
    P1:0xd2 raw_gflt_fac_low 7~0 Raw_gflt_fac_low 0x00
    P1:0xd3 raw_grgb_thr_outdoor 7~0 Raw_grgb_thr_outdoor 0x08
    P1:0xd4 raw_grgb_thr_normal 7~0 Raw_grgb_thr_normal 0x08
    P1:0xd5 raw_grgb_thr_dummy 7~0 Raw_grgb_thr_dummy 0x08
    P1:0xd6 raw_grgb_thr_low 7~0 Raw_grgb_thr_low 0x08
    P1:0xdf raw_gf_fac_outdoor 7~0 Raw_gf_fac_outdoor 0x00
    P1:0xe0 raw_gf_fac_normal 7~0 Raw_gf_fac_normal 0x00
    P1:0xe1 raw_gf_fac_dummy 7~0 Raw_gf_fac_dummy 0x00
    P1:0xe2 raw_gf_fac_low 7~0 Raw_gf_fac_low 0x00
    P1:0xe3 raw_rb_fac_outdoor 7~0 Raw_rb_fac_outdoor 0x00
    P1:0xe4 raw_rb_fac_normal 7~0 Raw_rb_fac_normal 0x00
    P1:0xe5 raw_rb_fac_dummy 7~0 Raw_rb_fac_dummy 0x00
    P1:0xe6 raw_rb_fac_low 7~0 Raw_rb_fac_low 0x00
    */
    img_write_Reg8Data8(SP0A39_ID, 0xd3, 0x09); //;GrGb平滑阈值       00
    img_write_Reg8Data8(SP0A39_ID, 0xd4, 0x09);
    img_write_Reg8Data8(SP0A39_ID, 0xd5, 0x09);
    img_write_Reg8Data8(SP0A39_ID, 0xd6, 0x0c);
    img_write_Reg8Data8(SP0A39_ID, 0xcf, 0xff); //;Gr\Gb之间平滑强度
    img_write_Reg8Data8(SP0A39_ID, 0xd0, 0xf0);
    img_write_Reg8Data8(SP0A39_ID, 0xd1, 0x80);
    img_write_Reg8Data8(SP0A39_ID, 0xd2, 0x80);
    img_write_Reg8Data8(SP0A39_ID, 0xdf, 0xff); //;Gr、Gb单通道内平滑强度
    img_write_Reg8Data8(SP0A39_ID, 0xe0, 0xf0);
    img_write_Reg8Data8(SP0A39_ID, 0xe1, 0xd0);
    img_write_Reg8Data8(SP0A39_ID, 0xe2, 0x80);
    img_write_Reg8Data8(SP0A39_ID, 0xe3, 0xff); //;R、B平滑强度
    img_write_Reg8Data8(SP0A39_ID, 0xe4, 0xf0);
    img_write_Reg8Data8(SP0A39_ID, 0xe5, 0xd0);
    img_write_Reg8Data8(SP0A39_ID, 0xe6, 0x80);


    /*
    ;CCM
    P2:0xa0 c00_eff1_8lsb 7~0 C00_eff1_8lsb for color correction 0x8c
    P2:0xa1 c01_eff1_8lsb 7~0 C01_eff1_8lsb for color correction 0x00
    P2:0xa2 c02_eff1_8lsb 7~0 C02_eff1_8lsb for color correction 0xf4
    P2:0xa3 c10_eff1_8lsb 7~0 C10_eff1_8lsb for color correction 0xfa
    P2:0xa4 c11_eff1_8lsb 7~0 C11_eff1_8lsb for color correction 0xa0
    P2:0xa5 c12_eff1_8lsb 7~0 C12_eff1_8lsb for color correction 0xe7
    P2:0xa6 c20_eff1_8lsb 7~0 C20_eff1_8lsb for color correction 0x0c
    P2:0xa7 c21_eff1_8lsb 7~0 C21_eff1_8lsb for color correction 0xcd
    P2:0xa8 c22_eff1_8lsb 7~0 C22_eff1_8lsb for color correction 0xa6
    P2:0xac c00_eff2_8lsb 7~0 C00_eff2_8lsb for color correction 0xa2
    P2:0xad c01_eff2_8lsb 7~0 C01_eff2_8lsb for color correction 0x04
    P2:0xae c02_eff2_8lsb 7~0 C02_eff2_8lsb for color correction 0xda
    P2:0xaf c10_eff2_8lsb 7~0 C10_eff2_8lsb for color correction 0xcd
    P2:0xb0 c11_eff2_8lsb 7~0 C11_eff2_8lsb for color correction 0xd9
    P2:0xb1 c12_eff2_8lsb 7~0 C12_eff2_8lsb for color correction 0xda
    P2:0xb2 c20_eff2_8lsb 7~0 C20_eff2_8lsb for color correction 0xf6
    P2:0xb3 c21_eff2_8lsb 7~0 C21_eff2_8lsb for color correction 0x98
    P2:0xb4 c22_eff2_8lsb 7~0 C22_eff2_8lsb for color correction 0xf3
    */
    //ccm
    // 0xfd, 0x02,
    // 0x15, 0xe0, //;b>th For f light judge
    // 0x16, 0x95, //;r<th For f light judge
    // 0xa0, 0x9b, //;C00_eff1_8lsb for color correction
    // 0xa1, 0xe4, //C01_eff1_8lsb for color correction
    // 0xa2, 0x01, //
    // 0xa3, 0xf2,
    // 0xa4, 0x8f,
    // 0xa5, 0xff,
    // 0xa6, 0x01,
    // 0xa7, 0xdb,
    // 0xa8, 0xa4,
    // 0xac, 0x80, //;F
    // 0xad, 0x21,
    // 0xae, 0xdf,
    // 0xaf, 0xf2,
    // 0xb0, 0xa0,
    // 0xb1, 0xee,
    // 0xb2, 0xea,
    // 0xb3, 0xd9,
    // 0xb4, 0xbd,//C22_eff2_8lsb for color correction


    // 0xfd, 0x02,
    // 0x15, 0xe0, //;b>th For f light judge
    // 0x16, 0x95, //;r<th For f light judge
    // 0xa0, 0x8c, //;C00_eff1_8lsb for color correction
    // 0xa1, 0x00, //C01_eff1_8lsb for color correction
    // 0xa2, 0xf4, //
    // 0xa3, 0xfa,
    // 0xa4, 0xa0,
    // 0xa5, 0xe7,
    // 0xa6, 0x0c,
    // 0xa7, 0xcd,
    // 0xa8, 0xa6,
    // 0xac, 0xa2, //;F
    // 0xad, 0x04,
    // 0xae, 0xda,
    // 0xaf, 0xcd,
    // 0xb0, 0xd9,
    // 0xb1, 0xda,
    // 0xb2, 0xf6,
    // 0xb3, 0x98,
    // 0xb4, 0xf3,//C22_eff2_8lsb for color correction




    /*
    Saturation
    P1:0xb3 sat_u_s1 7~0
    Saturation U at Auto Saturation Control segment 1 when in the outdoor mode
    0x98
    P1:0xb4 sat_u_s2 7~0
    Saturation U at Auto Saturation Control segment 2 when in the normal mode
    0x98
    P1:0xb5 sat_u_s3 7~0
    Saturation U at Auto Saturation Control segment 3 when in the dummy mode
    0x88
    P1:0xb6 sat_u_s4 7~0
    Saturation U at Auto Saturation Control segment 4 when in the low mode
    0x7f
    P1:0xb7 sat_v_s1 7~0
    Saturation V at Auto Saturation Control segment 1 when in the outdoor mode
    0x92
    P1:0xb8 sat_v_s2 7~0
    Saturation V at Auto Saturation Control segment 2 when in the normal mode
    0x92
    P1:0xb9 sat_v_s3 7~0
    Saturation V at Auto Saturation Control segment 3 when in the dummy mode
    0x82
    P1:0xba sat_v_s4 7~0
    Saturation V at Auto Saturation Control segment 4 when in the low mode
    */
    //--------satur
    // 0xfd, 0x01, //;sat u B
    // 0xb3, 0xb0,
    // 0xb4, 0x90, //;
    // 0xb5, 0x70,
    // 0xb6, 0x55,
    // 0xb7, 0xb0, //;sat v R
    // 0xb8, 0x90,
    // 0xb9, 0x70,
    // 0xba, 0x55,

    // 0xfd, 0x01, //;auto_sat
    // 0xbf, 0xff, //;The brightness thread of the ymean
    // 0x00, 0x00, //;[4] fix_state_en [2:0] fix_state_mode

    // 0xfd, 0x01, //;low_lum_offset
    // 0xa4, 0x00, //;Lum_limit
    // 0xa5, 0x1f, //;lum_set
    // 0xa6, 0x50, //;black vt
    // 0xa7, 0x65, //;If current luminance is bigger than it, the luminance of the frame will decrease.

    //;gw
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x02);
    img_write_Reg8Data8(SP0A39_ID, 0x30, 0x38); //Gw_mean_th
    img_write_Reg8Data8(SP0A39_ID, 0x31, 0x40); //Write gw offset for calculate offset, and read  out is real offset
    img_write_Reg8Data8(SP0A39_ID, 0x32, 0x40); //Gw_y_bot
    img_write_Reg8Data8(SP0A39_ID, 0x33, 0xd0); //Gw_y_top
    img_write_Reg8Data8(SP0A39_ID, 0x34, 0x10); //Gw_uv_radius
    img_write_Reg8Data8(SP0A39_ID, 0x35, 0x60); //Swap uv pix value when it is not gray pix
    img_write_Reg8Data8(SP0A39_ID, 0x36, 0x28); //Gw offset max limit
    img_write_Reg8Data8(SP0A39_ID, 0x37, 0x07); //[6] gw_upt_fr_en [5] gw_en_sel [2:0] Gw offset adjust step
    img_write_Reg8Data8(SP0A39_ID, 0x38, 0x08); //Gw_jdg_th;(read 0x31_gw_offset>0x10,is gray image)
    img_write_Reg8Data8(SP0A39_ID, 0xe6, 0x8F); //;(bit7:4 white edge 3:0 dark edg zhiyuedafanweiyueda )

    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01); //;
    img_write_Reg8Data8(SP0A39_ID, 0x1b, 0x15); //Ku_offset;(baibanshi：ku&kljiaduoshao)
    img_write_Reg8Data8(SP0A39_ID, 0x1c, 0x1A); //Kl_offset;(Read 0x31*0x1c的值/16为最终加的值)
    img_write_Reg8Data8(SP0A39_ID, 0x1d, 0x0c); //Auto_contrast_cttl; auto contrast enable ,bit3 outdoor  bit2 indoor bit1 dummy  bit0 lowlight



    //function enable
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x01);
    img_write_Reg8Data8(SP0A39_ID, 0x32, 0x15);// [7] test_en [6] ft_test_en [5] fix_awb [4] awb2_en [2] auto_gain_en [0] ae_en
    /*
    [7] Lens shading in outdoor enable
        0: disable
        1: enable
    [6] Lens shading in normal enable
        0: disable
        1: enable
    [5] Lens shading in dummy enable
        0: disable
        1: enable
    [4] Lens shading in low light enable
        0: disable
        1: enable
    [3] DPC in dummy enable
        0: disable
        1: enable
    [2] DPC in outdoor enable
        0: disable
        1: enable
    [1] DPC in normal enable
        0: disable
        1: enable
    [0] DPC in low light enable
        0: disable
        1: enable
    */
    img_write_Reg8Data8(SP0A39_ID, 0x33, 0xef);
    /*
    [6] raw window
        0: enable raw window function
        1: disable
    [5] binning_mode1
        1: col averge mode
        0: col sum mode
    [4] ace_test
        1: aec_win select raw data
        0: aec_win select y data
    [3] VSYNC Inversion
    [2] HSYNC Inversion
    [1] Disable HSYNC & VSYNC
    [0] scale_en
    */
    img_write_Reg8Data8(SP0A39_ID, 0x36, 0x10); //;AE统计在GAmma之前，00是AE在Gamma之后
    img_write_Reg8Data8(SP0A39_ID, 0xf6, 0xb0); //y_top_ae;亮态，防过暗机制，值越大过曝场景对其他影响小
    img_write_Reg8Data8(SP0A39_ID, 0xf5, 0x10); //y_bot_ae;暗态，防止黑色物体进入导致过曝， 10


    /*
    AE WINDOWN
    P1:0xd7 sat_ypix_thr1 7~0 Sat_ypix_thr1 0x04
    P1:0xd8 sat_ypix_thr2 7~0 Sat_ypix_thr2 0x04
    P1:0xd9 sat_ypix_thr3 7~0 Sat_ypix_thr3 0x10
    P1:0xda sat_ypix_thr4 7~0 Sat_ypix_thr4 0x20
    P1:0xdb sat_yadt_fac1 7~0 Sat_yadt_fac1 0x50
    P1:0xdc sat_yadt_fac2 7~0 Sat_yadt_fac2 0x30
    P1:0xdd sat_yadt_fac3 7~0 Sat_yadt_fac3 0x10
    P1:0xde sat_yadt_fac4 7~0 Sat_yadt_fac4 0x09
    */
    //0xd7, 0x3a, //;tbh
    //0xd8, 0x10,
    //0xd9, 0x20,
    //0xda, 0x10,
    //0xdb, 0x7a,
    //0xdc, 0x3a,
    //0xdd, 0x30,
    //0xde, 0x30,
    //0xe7, 0x3a, //Sat_bot
    //0x9c, 0xaa, //;u_v_th_outdoor
    //0x9d, 0xaa, //u_v_th_nr
    //0x9e, 0x55, //u_v_th_dummy
    //0x9f, 0x55,   //u_v_th_low
    img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x00);
    img_write_Reg8Data8(SP0A39_ID, 0x1c, 0x00);
}



static u32 spa0a39_drv_read_id(void)
{
    u32 id;
    u8 temp = 0;
    //img_write_Reg8Data8(SP0A39_ID, 0xfd, 0x00);
    //img_write_Reg8Data8(SP0A39_ID, 0x01, 0x00);
    //img_read_Reg8Data8(0x43, 0x00, &temp);
    //id = temp;
    img_read_Reg8Data8(SP0A39_ID, 0x01, &temp);
    id = temp;
    return id;
}


img_sensor_drv_t img_spa0a39_drv = {
    .speed_mhz            = 24,     //Mhz
    .image_height         = 480,
    .image_width          = 640,
    .in_format            = IMG_IN_YUYV,
    .out_format           = IMG_OUT_RGB565,
    .imag_sensor_reg_init = sensor_drv_spa0a39_init,
    .imag_sensor_read_id  = spa0a39_drv_read_id,
    .vsync_is_high        = 0,     //0高有效，1低有效
    .hsync_is_high        = 0,     //0高有效，1低有效
    .dev_id               = 0x39,  //有效ID值，为FF表示不判断ID
    .type                 = IMG_TYPE_DVP,
};
#endif

#endif
