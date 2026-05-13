#include "include.h"
// 360 * 360

//#define Delay(ms) bsp_spide_cs(1);delay_ms(ms)
#if (GUI_SELECT == GUI_TFT_170_560_AXS15231B)


#define WriteData(v) tft_write_data(v)
#define CommEnd()    tft_write_end()
// cmd 12h 1:cmd      4:addr/data
// cmd 32h 1:cmd/addr 4:data
// cmd 02h 1:cmd      1:addr/data

#define WriteComm12(v) \
({                   \
bsp_spide_cs(1);         \
bsp_spide_cs(0);         \
bsp_spide_bus_mode(SPIDE_1IO);   \
bsp_spide_snd_byte(0x12);\
bsp_spide_bus_mode(SPIDE_4IO);   \
bsp_spide_snd_byte(0x00);\
bsp_spide_snd_byte(v);   \
bsp_spide_snd_byte(0x00);\
})
#define WriteComm32(v) \
({                   \
bsp_spide_cs(1);         \
bsp_spide_cs(0);         \
bsp_spide_bus_mode(SPIDE_1IO);   \
bsp_spide_snd_byte(0x32);\
bsp_spide_snd_byte(0x00);\
bsp_spide_snd_byte(v);   \
bsp_spide_snd_byte(0x00);\
bsp_spide_bus_mode(SPIDE_4IO);   \
})
#define WriteComm02(v) \
({                   \
bsp_spide_cs(1);         \
bsp_spide_cs(0);         \
bsp_spide_bus_mode(SPIDE_1IO);   \
bsp_spide_snd_byte(0x02);\
bsp_spide_snd_byte(0x00);\
bsp_spide_snd_byte(v);   \
bsp_spide_snd_byte(0x00);\
})
#define ReadComm03(v) \
({                   \
bsp_spide_cs(1);         \
bsp_spide_cs(0);         \
bsp_spide_bus_mode(SPIDE_1IO);   \
bsp_spide_snd_byte(0x03);\
bsp_spide_snd_byte(0x00);\
bsp_spide_snd_byte(v);   \
bsp_spide_snd_byte(0x00);\
})

//0x02: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_write_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    DESPICON &= ~BIT(3);                        //1BIT
    tft_spi_sendbyte(0x02);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

//0x12: 1CMD 4ADDR 4DATA
AT(.com_text.tft_spi)
void tft_write_cmd12(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    DESPICON &= ~BIT(3);                        //1BIT
    tft_spi_sendbyte(0x12);
    DESPICON |= BIT(3);                         //4BIT
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

//0x03: 1CMD 1ADDR 1DATA
AT(.com_text.tft_spi)
void tft_read_cmd(u8 cmd)
{
    TFT_SPI_CS_DIS();
    delay_us(1);
    TFT_SPI_CS_EN();
    DESPICON &= ~BIT(3);                        //1BIT
    tft_spi_sendbyte(0x03);
    tft_spi_sendbyte(0x00);
    tft_spi_sendbyte(cmd);
    tft_spi_sendbyte(0x00);
}

AT(.com_text.tft_spi)
void tft_write_data(u8 data)
{
    tft_spi_sendbyte(data);
}

AT(.com_text.tft_spi)
void tft_write_end(void)
{
    TFT_SPI_CS_DIS();
}

AT(.com_text.tft_spi)
void tft_set_window(u16 x0, u16 y0, u16 x1, u16 y1)
{
    x0 += GUI_SCREEN_OFS_X;
    x1 += GUI_SCREEN_OFS_X;
    y0 += GUI_SCREEN_OFS_Y;
    y1 += GUI_SCREEN_OFS_Y;

    tft_write_cmd(0x2A);        //TFT_CASET
    tft_write_data(BYTE1(x0));
    tft_write_data(BYTE0(x0));
    tft_write_data(BYTE1(x1));
    tft_write_data(BYTE0(x1));

    tft_write_cmd(0x2B);        //TFT_PASET
    tft_write_data(BYTE1(y0));
    tft_write_data(BYTE0(y0));
    tft_write_data(BYTE1(y1));
    tft_write_data(BYTE0(y1));
    tft_write_end();
}

AT(.com_text.tft_spi)
void tft_write_data_start(void)
{
    tft_write_cmd12(0x2C);      //TFT_RAMWR
}

void tft_write_cmd(u8 cmd);
void tft_write_data(u8 data);
void tft_write_end(void);

#define WriteComm(v) tft_write_cmd(v)
#define ReadComm(v)  ReadComm03(v)

uint32_t tft_read_id(void)
{
    uint32_t id = 0;
    tft_read_cmd(0xDA);
    id = tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    id = (id << 8) + tft_spi_getbyte();
    tft_write_end();
    return id;
}

void tft_170_560_axs15231B_full(void)
{
    delay_ms(100);
    printf("tft_170_560_axs15231B_full\n");
    tft_set_window(0, 0, 169, 559);
    tft_write_data_start();
    for (int x = 0;x <= 170; x++){
        for (int y = 0; y <= 560; y++){
            WDT_CLR();
            tft_write_data(0x00);
            tft_write_data(0x00);
        }
    }
    tft_write_end();
}

void tft_170_560_axs15231B_init(void)
{
    printf("tft_170_560_axs15231B_init\n");

    ////////// LCD init code ///////////////////

    WriteComm(0xBB);
    WriteData(0x00);//0
    WriteData(0x00);//1
    WriteData(0x00);//2
    WriteData(0x00);//3
    WriteData(0x00);//4
    WriteData(0x00);//5
    WriteData(0x5A);//6
    WriteData(0xA5);//7

    WriteComm(0xA0);
    WriteData(0x00);//0
    WriteData(0x10);//1 D[2]cr_spi_rd888
    WriteData(0x00);//2
    WriteData(0x02);//3
    WriteData(0x00);//4
    WriteData(0x00);//5
    WriteData(0x00);//6 D[6-4]cr_mipi_tsm,D[3-0]cr_tsm
    WriteData(0x00);//7 D[5-0]cr_ts_en
    WriteData(0x20);//8
    WriteData(0x05);//9
    WriteData(0x3F);//10
    WriteData(0x3F);//11
    WriteData(0x00);//12
    WriteData(0x00);//13
    WriteData(0x00);//14
    WriteData(0x00);//15
    WriteData(0x00);//16

    WriteComm(0xA2);
    WriteData(0x30);//0
    WriteData(0x04);//1 cr_vsa        4
    WriteData(0x0A);//2 cr_hsa
    WriteData(0x3C);//3 cr_vbp[7:0]
    WriteData(0xa0);//4 cr_hbp[7:0]   160
    WriteData(0x50);//5 cr_vfp[7:0]
    WriteData(0xB4);//6 cr_hfp[7:0]
    WriteData(0x30);//7 cr_vact[7:0]
    WriteData(0xAA);//8 cr_hact[7:0]
    WriteData(0x28);//9 D[7-4]VACT,yuy565_en,rgb_db_mode,D[1-0]HACT
    WriteData(0x7F);//10 cr_gray_r
    WriteData(0x7F);//11 cr_gray_g
    WriteData(0x7F);//12 cr_gray_b
    WriteData(0x20);//13 col_inc:chess pixel
    WriteData(0xF8);//14 cr_cmd_yuv_en  cr_data_mode cr_rot90_en cr_frame_pat_num[1:0]
    WriteData(0x10);//15 cr_dsprim_col_str,cr_dsprim_col_end
    WriteData(0x02);//16 cr_dsprim_row_str,cr_dsprim_row_end
    WriteData(0xFF);//17 cr_pat_sel[15:8]
    WriteData(0xFF);//18 cr_pat_sel[7:0]
    WriteData(0xF0);//19 cr_color_row_start[7:0]
    WriteData(0x90);//20 cr_color_row_end[7:0]
    WriteData(0x01);//21 cr_color_row_start[11:8]  cr_color_row_end[11:8]
    WriteData(0x32);//22 cr_color_col_start[7:0]
    WriteData(0xA0);//23 cr_color_col_end[7:0]
    WriteData(0x91);//24 cmd_onethird_en,cr_color_row_end[7:0],cr_format_cmd3a_en,cr_color_col_end[10:8]
    WriteData(0xC0);//25 cr_qspi_diomode,cr_corr_dyn_en,cr_corr_col_en,cr_corr_valueD[4-2],cr_grayscal_sel
    WriteData(0x20);//26 cr_qspis_datamode cr_spi_format cr_dbi_format cr_ext_format
    WriteData(0x7F);//27 ref
    WriteData(0xFF);//28 high
    WriteData(0x00);//29 low
    WriteData(0x04);//30 cr_cmdmode_hsneg_sel


    WriteComm(0xD0);
    WriteData(0x30);//0 cr_v_reso[7:0]
    WriteData(0xAA);//1 cr_h_reso[7:0]
    WriteData(0x21);//2  cr_inv_fst_lines
    WriteData(0x24);//3  cr_fm_odd_half cr_fm_odd_period
    WriteData(0x08);//4  cr_lchdly_adj
    WriteData(0x09);//5 cr_le_width
    WriteData(0x10);//6  cr_sd_t9 cr_sd_lnstr_adj
    WriteData(0x01);//7  cr_hbp_dsp
    WriteData(0x80);//8 cr_hend_dsp[7:0]
    WriteData(0x12);//9 cr_n_ln cr_hend_dsp[11:8]
    WriteData(0xC2);//10  cr_sd_en_sel cr_chop_en cr_fm_odd_inv cr_fm_odd_en cr_le_dly
    WriteData(0x00);//11 cr_dual_type cr_oe_str_adj
    WriteData(0x22);//12  cr_oe_end_adj
    WriteData(0x22);//13  cr_sd_t0 cr_sd_t1
    WriteData(0xAA);//14  cr_sd_dum
    WriteData(0x03);//15  cr_sd_t4[7:0]
    WriteData(0x10);//16  cr_tp_scan_en
    WriteData(0x12);//17
    WriteData(0x40);//18 cr_dsh_shr cr_data_pol_inv cr_inv_sel cr_half_mode
    WriteData(0x14);//19 cr_byptime cr_dual_pattern
    WriteData(0x1E);//20 cr_rd_sta
    WriteData(0x51);//21 cr_sd_igzo_opt cr_p15l_mode cr_line_en_opt cr_p14l_mode
    WriteData(0x15);//22 cr_p15h_mode cr_p14h_mode
    WriteData(0x00);//23  cr_p2_mode cr_p1_mode
    WriteData(0x40);//24 cr_line_f_half[7:0]
    WriteData(0x10);//25  cr_line_f_half[9:8] cr_pixnum_offset
    WriteData(0x00);//26  cr_sd_lnend_adj
    WriteData(0x03);//27
    WriteData(0x04);//28 cr_sd_bp
    WriteData(0x12);//29  cr_rgb_cnt_num cr_sd_oe_test

    WriteComm(0xA3);
    WriteData(0xA0);//0
    WriteData(0x06);//1
    WriteData(0xAA);//2 cr_svee_en cr_svee_en cr_vgh_en cr_vgl_en
    WriteData(0x00);//3
    WriteData(0x08);//4
    WriteData(0x02);//5
    WriteData(0x0A);//6
    WriteData(0x04);//7 cr_normal_dspon_svddclk_sel
    WriteData(0x04);//8 cr_normal_dspoff_svddclk_sel
    WriteData(0x04);//9  cr_normal_tpen_svddclk_sel
    WriteData(0x04);//10 cr_color2_dspon_svddclk_sel
    WriteData(0x04);//11 cr_color2_dspon_svddclk_sel
    WriteData(0x04);//12 cr_color2_dspoff_svddclk_sel
    WriteData(0x04);//13 cr_color2_tpen_svddclk_sel
    WriteData(0x04);//14 cr_normal_dspoff_sveeclk_sel
    WriteData(0x04);//15 cr_normal_tpen_sveeclk_sel
    WriteData(0x04);//16 cr_color2_dspon_sveeclk_sel
    WriteData(0x04);//17 cr_color2_dspoff_sveeclk_sel
    WriteData(0x04);//18 cr_color2_tpen_sveeclk_sel
    WriteData(0x00);//19
    WriteData(0x55);//20 cr_vgh_eq_ctrl
    WriteData(0x55);//21 cr_vgl_eq_ctrl


    WriteComm(0xC1);
    WriteData(0x33);//0 cr_src_opt cr_bgr_opt cr_win_en cr_ln_sel
    WriteData(0x04);//1 cr_settle_d_sel
    WriteData(0x02);//2
    WriteData(0x02);//3
    WriteData(0x71);//4
    WriteData(0x05);//5
    WriteData(0x24);//6 cr_pcdiv
    WriteData(0x55);//7 cr_ram_clk_opt cr_led_pwm_oe cr_miso_edge_opt cr_te_oe
    WriteData(0x02);//8 cr_pix_format cr_cmddsp_linemode cr_rx_cmddsp_dis
    WriteData(0x00);//9 cr_de_qspi_delay_adj cr_vsync_qspi_delay_adj cr_din_sda_delay_adj cr_dual_delay_adj
    WriteData(0x41);//10 cr_bl_en
    WriteData(0x01);//11 cr_cmd_rgb565_en cr_dsp_sel cr_divi
    WriteData(0x53);//12 cr_cmd_rgb666_en cr_vs_opt cr_hs_opt
    WriteData(0xFF);//13
    WriteData(0xFF);//14
    WriteData(0xFF);//15 cr_qspi_schmitt_en
    WriteData(0x4F);//16 cr_vs_neg[7:0]
    WriteData(0x52);//17 cr_vs_pos[7:0]
    WriteData(0x00);//18 cr_vs_neg[11:8] cr_vs_pos[11:8]
    WriteData(0x4F);//19
    WriteData(0x52);//20
    WriteData(0x00);//21
    WriteData(0x45);//22 cr_byte_data_sel cr_tx_sel
    WriteData(0x3B);//23
    WriteData(0x0B);//24
    WriteData(0x02);//25 cr_term_sel_ana cr_term_sel_ana_pre
    WriteData(0x0d);//26 cr_term_sel_dig cr_term_sel_dig_pre 8’
    WriteData(0x00);//27
    WriteData(0xFF);//28
    WriteData(0x40);//29 cr_qspi_trig cr_36_opt


    WriteComm(0xC3);
    WriteData(0x00);//0
    WriteData(0x00);//1 cr_inv_en cr_bgr_en cr_sre_en cr_sre_mode
    WriteData(0x00);//2
    WriteData(0x50);//3
    WriteData(0x03);//4
    WriteData(0x00);//5
    WriteData(0x00);//6
    WriteData(0x00);//7
    WriteData(0x01);//8
    WriteData(0x80);//9
    WriteData(0x01);//10 cr_usr_pix_en


    WriteComm(0xC4);
    WriteData(0x00);//0
    WriteData(0x24);//1
    WriteData(0x33);//2 cr_vgh_off_en cr_vgl_off_en cr_vghclk_off cr_svddclk_off_en cr_sveeclk_en
    WriteData(0x80);//3 cr_vcom_en cr_vcom_en_mode cr_vcom_olim_sw cr_vcom_cap_sel
    WriteData(0x4F);//4 cr_vcom_vsel
    WriteData(0xEA);//5 cr_vcom_drv
    WriteData(0x64);//6 cr_svdd_on
    WriteData(0x32);//7 cr_svee_on
    WriteData(0xC8);//8 cr_vgh_on
    WriteData(0x64);//9 cr_svdd_off
    WriteData(0xC8);//10 cr_svee_off
    WriteData(0x32);//11 cr_vgh_off
    WriteData(0x90);//12 cr_vgl_on[7:0]
    WriteData(0x90);//13 cr_vgl_off[7:0]
    WriteData(0x11);//14 cr_vgl_on[11:8] cr_vgl_off[11:8]
    WriteData(0x06);//15 cr_chp_on
    WriteData(0xDC);//16 cr_chp_off
    WriteData(0xFA);//17
    WriteData(0x00);//18 cr_vsp_clk_mode_at_tp cr_vsn_clk_mode_at_tp cr_vgh_clk_mode_at_tp cr_vgl_clk_mode_at_tp
    WriteData(0x00);//19 cr_svdddly_boost_sel cr_sveedly_boost_sel cr_vcom_en_mode_at_tp cr_gamma_en_mode_at_tp
    WriteData(0x80);//20 cr_vcom_en_opt cr_vcom_offset
    WriteData(0xFE);//21
    WriteData(0x10);//22
    WriteData(0x10);//23
    WriteData(0x00);//24
    WriteData(0x0A);//25 cr_vcomon_pre_opt
    WriteData(0x0A);//26 cr_vcomon_opt
    WriteData(0x44);//27 cr_svddclk_boost_dly cr_svddclk_boost_sel cr_sveeclk_boost_dly cr_sveeclk_boost_sel
    WriteData(0x50);//28 cr_svdd_dly_adj cr_svee_dly_adj cr_svddclk_boost_en cr_sveeclk_boost_en


    WriteComm(0xC5);
    WriteData(0x18);//0
    WriteData(0x00);//1
    WriteData(0x00);//2
    WriteData(0x03);//3
    WriteData(0xFE);//4
    WriteData(0x78);//5  cr_te_str[7:0]
    WriteData(0x33);//6  cr_te_end[7:0]
    WriteData(0x20);//7 cr_te_str[11:8],cr_te_end[11:8]
    WriteData(0x30);//8
    WriteData(0x10);//9
    WriteData(0x88);//10
    WriteData(0xDE);//11
    WriteData(0x0D);//12
    WriteData(0x08);//13
    WriteData(0x0F);//14
    WriteData(0x0F);//15
    WriteData(0x01);//16
    WriteData(0x78);//17  cr_tp_str[7:0]
    WriteData(0x33);//18  cr_tp_end[7:0]
    WriteData(0x20);//19 cr_tp_str[11:8],cr_tp_end[11:8]
    WriteData(0x10);//20
    WriteData(0x10);//21
    WriteData(0x00);//22

    WriteComm(0xC6);
    WriteData(0x05);//0 cr_pwron_opt
    WriteData(0x0A);//1 cr_dspon_opt
    WriteData(0x05);//2 cr_dspclkon_opt
    WriteData(0x0A);//3 cr_gipon_opt
    WriteData(0x00);//4 cr_fmloss_opt cr_gip_igzo_en cr_gip_hiz_en cr_tm_unit[7:0]
    WriteData(0xE0);//5 cr_tm_unit[7:0]
    WriteData(0x2E);//6 cr_tm_unit[15:8]
    WriteData(0x0B);//7 cr_otp_flh_dly
    WriteData(0x12);//8 cr_otp_11_dly
    WriteData(0x22);//9 cr_otp_29_dly
    WriteData(0x12);//10 cr_pwron_dly1
    WriteData(0x22);//11 cr_pwron_dly2
    WriteData(0x01);//12 cr_fmdly_opt cr_frame_loss_num
    WriteData(0x00);//13 cr_gipon_dly_num
    WriteData(0x00);//14 cr_pwron_dly_num
    WriteData(0x02);//15 cr_ga_fm_mode cr_ga_fm_opt cr_ga_fm_rpt cr_ga_fm_num 8
    WriteData(0x6A);//16 cr_dim_tm_unit[7:0]
    WriteData(0x18);//17 cr_dim_tm_unit[15:8]
    WriteData(0xC8);//18 cr_dim_dly[7:0]
    WriteData(0x22);//19 cr_dim_step_num cr_dim_step_ind



    WriteComm(0xC7);
    WriteData(0x50);//0
    WriteData(0x36);//1 cr_lvd18_en cr_lvd30_en
    WriteData(0x28);//2 cr_lvd_wait_num[7:0]
    WriteData(0x00);//3
    WriteData(0xa2);//4 cr_lvd18_adj cr_lvd30_adj
    WriteData(0x80);//5
    WriteData(0x8f);//6
    WriteData(0x00);//7
    WriteData(0x80);//8 cr_lvd_gip_dly
    WriteData(0xff);//9
    WriteData(0x07);//10
    WriteData(0x11);//11
    WriteData(0x9c);//12
    WriteData(0x6f);//13 cr_lvd_gip_vlu[11:8]
    WriteData(0xff);//14 cr_lvd_gip_vlu[7:0]
    WriteData(0x26);//15 cr_igzo_timing_en
    WriteData(0x0c);//16
    WriteData(0x0d);//17
    WriteData(0x0e);//18
    WriteData(0x0f);//19


    WriteComm(0xC9);
    WriteData(0x33);//0 cr_ga_le_width cr_ga_tx_freq
    WriteData(0x44);//1 cr_ib_adjp cr_ib_adjn
    WriteData(0x44);//2 cr_vb_adjp cr_vb_adjn
    WriteData(0x01);//3 cr_ga_curve_manu

    WriteComm(0xCF);
    WriteData(0x2C);//0 cr_vgho_gbase cr_vrefp_en cr_vrefn_en_mode cr_io_rgb_trig
    WriteData(0x1E);//1 cr_bgr_base
    WriteData(0x88);//2 cr_vrefp_base cr_vrefn_base
    WriteData(0x58);//3 cr_sdp_boost_dly cr_sdn_boost_dly cr_vdd_stb_adj
    WriteData(0x13);//4 cr_mvdda_stb_adj cr_mvdd_i_adj cr_mvdd_adj
    WriteData(0x18);//5 cr_vgho1_base
    WriteData(0x56);//6 cr_vdn_i_adj cr_vgho1_i_adj cr_vgho_disc
    WriteData(0x18);//7 cr_vglo1_base
    WriteData(0x1E);//8 cr_vdn_ifb_en cr_vglo_disc
    WriteData(0x60);//9 cr_vdn_adj cr_osc_base
    WriteData(0xF7);//10 cr_svee_adj cr_svdd_adj
    WriteData(0x00);//11
    WriteData(0x65);//12 cr_vgh_adj1 cr_vgh_adj
    WriteData(0x0C);//13 cr_vgl_adj1 cr_vgl_adj
    WriteData(0x22);//14
    WriteData(0xC4);//15 cr_vgh_comp_en
    WriteData(0x0C);//16 cr_osc32k_base
    WriteData(0x77);//17 cr_vgh_eq_en cr_vgl_eq_en
    WriteData(0x22);//18
    WriteData(0x44);//19
    WriteData(0xAA);//20 cr_ana_utl1_reg
    WriteData(0x55);//21 cr_ana_utl2_reg
    WriteData(0x04);//22 cr_vglclk_sel
    WriteData(0x04);//23 cr_vghclk_sel
    WriteData(0x12);//24 cr_bgr1_base
    WriteData(0xA0);//25 cr_bgr2_base
    WriteData(0x08);//26


    WriteComm(0xD5);
    WriteData(0x3E);//0 cr_gspf2
    WriteData(0x3E);//1 cr_gsps2
    WriteData(0x88);//2 cr_vst_pre_adj
    WriteData(0x00);//3 cr_vst_post_adj
    WriteData(0x44);//4 cr_hig_wdt cr_low_wdt
    WriteData(0x04);//5 cr_rst_wdt
    WriteData(0x9A);//6 cr_gpwr1_star[7:0]
    WriteData(0x11);//7 cr_gpwr1_end[7:0]
    WriteData(0x20);//8 cr_gpwr1_star[11:8] cr_gpwr1_end[11:8]
    WriteData(0x9A);//9 cr_gpwr2_star[7:0]
    WriteData(0x11);//10 cr_gpwr2_end[7:0]
    WriteData(0x20);//11 cr_gpwr2_star[11:8] cr_gpwr2_end[11:8]
    WriteData(0x04);//12 cr_dum_num cr_rst_star[7:0]
    WriteData(0x28);//13 cr_rst_star[7:0]
    WriteData(0xD3);//14 cr_fv cr_frm_sel cr_rst_star[11:8]
    WriteData(0x47);//15 cr_fl cr_gck_bgn
    WriteData(0x03);//16
    WriteData(0x03);//17
    WriteData(0x03);//18
    WriteData(0x03);//19
    WriteData(0x86);//20 cr_gck_en_adj
    WriteData(0x00);//21 cr_vg_fix_high[7:0]
    WriteData(0x00);//22 cr_vg_fix_low[7:0]
    WriteData(0x00);//23 cr_vg_fix_high[11:8] cr_vg_fix_low[11:8]
    WriteData(0x30);//24 cr_nl[7:0]
    WriteData(0x52);//25 cr_gip_igzo_opt cr_nl[11:8]
    WriteData(0x3F);//26 cr_bp
    WriteData(0x40);//27 cr_gspf1
    WriteData(0x40);//28 cr_gsps1
    WriteData(0x96);//29 cr_gto_max


    WriteComm(0xD6);
    WriteData(0x10);//0
    WriteData(0x32);//1
    WriteData(0x54);//2
    WriteData(0x76);//3
    WriteData(0x98);//4
    WriteData(0xBA);//5
    WriteData(0xDC);//6
    WriteData(0xFE);//7
    WriteData(0x95);//8 cr_rst_stal[7:0]
    WriteData(0x00);//9 cr_rst_stal[11:8] 8??h0
    WriteData(0x01);//10 cr_vst_pre_step
    WriteData(0x83);//11 cr_vst_post_step
    WriteData(0x75);//12 cr_gpwr3_star[7:0]
    WriteData(0x36);//13 cr_gpwr3_end[7:0]
    WriteData(0x20);//14 cr_gpwr3_star[11:8] cr_gpwr3_end[11:8]
    WriteData(0x75);//15 cr_gpwr4_star[7:0]
    WriteData(0x36);//16 cr_gpwr4_end[7:0]
    WriteData(0x20);//17 cr_gpwr4_star[11:8] cr_gpwr4_end[11:8]
    WriteData(0x3F);//18 cr_gp3r
    WriteData(0x03);//19 cr_gp3f
    WriteData(0x03);//20 cr_gp4r
    WriteData(0x03);//21 cr_gp4f
    WriteData(0x10);//22 cr_rst_gspf
    WriteData(0x10);//23 cr_rst_gsps
    WriteData(0x00);//24 cr_gto_min
    WriteData(0x04);//25 cr_hs_dsp_half_sel cr_gpwr_opt cr_rst_sel cr_vst_wdt
    WriteData(0x51);//26
    WriteData(0x20);//27 cr_gpwr3_sel cr_gpwr4_sel
    WriteData(0x01);//28 cr_gip_t1
    WriteData(0x00);//29 Cr_gck_sw_num cr_rst_stal_dly


    WriteComm(0xD7);
    WriteData(0x0a);//0 CLK7
    WriteData(0x08);//1 CLK5
    WriteData(0x0e);//2 CLK3
    WriteData(0x0c);//3 CLK1
    WriteData(0x1E);//4 VDS
    WriteData(0x18);//5 GCH
    WriteData(0x19);//6 GCL
    WriteData(0x1F);//7 VGL
    WriteData(0x00);//8 STV1
    WriteData(0x1F);//9 VSD
    WriteData(0x1A);//10 STV0
    WriteData(0x1F);//11
    WriteData(0x3E);//12
    WriteData(0x3E);//13
    WriteData(0x04);//14 cr_gip_bp_add cr_gip_hs_keep cr_gip_delay_en
    WriteData(0x00);//15 cr_gip_hs_dual1[11:8] cr_gip_hs_dual2[11:8]
    WriteData(0x1F);//16 cr_gip_hs_dual1[7:0]
    WriteData(0x1F);//17 cr_gip_hs_dual2[7:0]
    WriteData(0x1F);//18 cr_gip_delay


    WriteComm(0xD8);
    WriteData(0x0B);//0 CLK8
    WriteData(0x09);//1 CLK6
    WriteData(0x0F);//2 CLK4
    WriteData(0x0D);//3 CLK2
    WriteData(0x1E);//4 VDS
    WriteData(0x18);//5 GCH
    WriteData(0x19);//6 GCL
    WriteData(0x1F);//7 VGL
    WriteData(0x01);//8 STV2
    WriteData(0x1F);//9 VSD
    WriteData(0x1A);//10 STV0
    WriteData(0x1F);//11


    WriteComm(0xD9);
    WriteData(0x0D);//0 CLK7
    WriteData(0x0F);//1 CLK5
    WriteData(0x09);//2 CLK3
    WriteData(0x0B);//3 CLK1
    WriteData(0x1F);//4 VDS
    WriteData(0x18);//5 GCH
    WriteData(0x19);//6 GCL
    WriteData(0x1F);//7 VGL
    WriteData(0x01);//8 STV1
    WriteData(0x1E);//9 VSD
    WriteData(0x1A);//10 STV0
    WriteData(0x1F);//11

    WriteComm(0xDD);
    WriteData(0x0C);//0 CLK8
    WriteData(0x0E);//1 CLK6
    WriteData(0x08);//2 CLK4
    WriteData(0x0A);//3 CLK2
    WriteData(0x1F);//4 VDS
    WriteData(0x18);//5 GCH
    WriteData(0x19);//6 GCL
    WriteData(0x1F);//7 VGL
    WriteData(0x00);//8 STV2
    WriteData(0x1E);//9 VSD
    WriteData(0x1A);//10 STV0
    WriteData(0x1F);//11

    WriteComm(0xDF);
    WriteData(0x44);//0 cr_hs_lag_en_c cripost_adj cr_hs_lag_en_d cripre_adj
    WriteData(0x33);//1 cr_vdd_adj cr_tres_adj_c
    WriteData(0x4B);//2 cr_vdd_i_adj cr_lp_tx_sel cr_lp_rx_smt cr_vdd_ifb_en cr_tres_adj_d0
    WriteData(0x69);//3 cr_vih_sel cr_vil_sel
    WriteData(0x00);//4 cr_hsd_pre_dly_d0 cr_hsd_pre_dly_d1
    WriteData(0x0A);//5 cr_hs_lag_adj_c cr_hs_lag_adj_d cr_mipi_lp_tx_sr cr_mipi_pre_en cr_mipi_vref_en
    WriteData(0x02);//6 cr_iref_sel
    WriteData(0x90);//7 cr_mipi_lp_mode_en cr_vcd_sel cr_hs_clk_dly_sel cr_hs_clk_inv cr_hs_data_dly_sel cr_hs_data_inv

    WriteComm(0xE0);
    WriteData(0x21);//0  vgmp
    WriteData(0x20);//1  j0p V0P      255
    WriteData(0x08);//2  v1p          251
    WriteData(0x0E);//3  v2p          247
    WriteData(0x0B);//4  v4p          239
    WriteData(0x08);//5  v6p          231
    WriteData(0x12);//6  v13p         203
    WriteData(0x20);//7  v20p         179
    WriteData(0x44);//8  v27p v36p    147   111
    WriteData(0x1B);//9  v43p         83
    WriteData(0x0B);//10 v50p         55
    WriteData(0x34);//11 v57p         27
    WriteData(0x12);//12 v59p         19
    WriteData(0x2A);//13 v61p         11
    WriteData(0x30);//14 v62p         7
    WriteData(0x2f);//15 j1p v63p     3
    WriteData(0x02);//16 vgsp

    WriteComm(0xE1);
    WriteData(0x3f);//0  vgmn
    WriteData(0x20);//1  j0n V0n      255
    WriteData(0x08);//2  v1n          251
    WriteData(0x0E);//3  v2n          247
    WriteData(0x0B);//4  v4n          239
    WriteData(0x08);//5  v6n          231
    WriteData(0x11);//6  v13n         203
    WriteData(0x20);//7  v20n         179
    WriteData(0x43);//8  v27n v36n    147   111
    WriteData(0x1A);//9  v43n         83
    WriteData(0x0B);//10 v50n         55
    WriteData(0x34);//11 v57n         27
    WriteData(0x12);//12 v59n         19
    WriteData(0x2A);//13 v61n         11
    WriteData(0x2F);//14 v62n         7
    WriteData(0x2f);//15 j1n v63n     3
    WriteData(0x27);//16 vgsn



    WriteComm(0xE2);
    WriteData(0x3B);//0
    WriteData(0x07);//1
    WriteData(0x12);//2
    WriteData(0x18);//3
    WriteData(0x0E);//4
    WriteData(0x0D);//5
    WriteData(0x17);//6
    WriteData(0x35);//7
    WriteData(0x44);//8
    WriteData(0x32);//9
    WriteData(0x0C);//10
    WriteData(0x14);//11
    WriteData(0x14);//12
    WriteData(0x36);//13
    WriteData(0x3A);//14
    WriteData(0x2F);//15
    WriteData(0x0D);//16


    WriteComm(0xE3);
    WriteData(0x37);//0
    WriteData(0x07);//1
    WriteData(0x12);//2
    WriteData(0x18);//3
    WriteData(0x0E);//4
    WriteData(0x0D);//5
    WriteData(0x17);//6
    WriteData(0x35);//7
    WriteData(0x44);//8
    WriteData(0x32);//9
    WriteData(0x0C);//10
    WriteData(0x14);//11
    WriteData(0x14);//12
    WriteData(0x36);//13
    WriteData(0x32);//14
    WriteData(0x2F);//15
    WriteData(0x0F);//16


    WriteComm(0xE4);
    WriteData(0x3B);//0
    WriteData(0x07);//1
    WriteData(0x12);//2
    WriteData(0x18);//3
    WriteData(0x0E);//4
    WriteData(0x0D);//5
    WriteData(0x17);//6
    WriteData(0x39);//7
    WriteData(0x44);//8
    WriteData(0x2E);//9
    WriteData(0x0C);//10
    WriteData(0x14);//11
    WriteData(0x14);//12
    WriteData(0x36);//13
    WriteData(0x3A);//14
    WriteData(0x2F);//15
    WriteData(0x0D);//16

    WriteComm(0xE5);
    WriteData(0x37);//0
    WriteData(0x07);//1
    WriteData(0x12);//2
    WriteData(0x18);//3
    WriteData(0x0E);//4
    WriteData(0x0D);//5
    WriteData(0x17);//6
    WriteData(0x39);//7
    WriteData(0x44);//8
    WriteData(0x2E);//9
    WriteData(0x0C);//10
    WriteData(0x14);//11
    WriteData(0x14);//12
    WriteData(0x36);//13
    WriteData(0x3A);//14
    WriteData(0x2F);//15
    WriteData(0x0F);//16


    WriteComm(0xBB);
    WriteData(0x00);//0
    WriteData(0x00);//1
    WriteData(0x00);//2
    WriteData(0x00);//3
    WriteData(0x00);//4
    WriteData(0x00);//5
    WriteData(0x00);//6
    WriteData(0x00);//7

    WriteComm(0x35);        //TE On
    WriteData(0x00);

    WriteComm(0x11);//REG ADDR

    CommEnd();
    delay_ms(120);
    WriteComm(0x29);
    //  	WriteComm(0x2C);

    CommEnd();

    tft_170_560_axs15231B_full();
}

#endif
