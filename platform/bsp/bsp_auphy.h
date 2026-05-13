#ifndef BSP_AUPHY_H
#define BSP_AUPHY_H

void bsp_auphy_mic_setup(u16 channel);
void bsp_set_auphy_spr(u8 auphy, u8 spr);
void bsp_auphy_adc_mode(u8 auphy, u8 mode);
void bsp_auphy_vcmbuf_en(void);
void bsp_auphy_set_dac_volume(u8 vol);
void bsp_auphy_set_mic_analog_gain(u16 channel, u8 gain);

void fpga_uart_putchar(u8 auphy, char ch);
void fpga_uart_putcs(u8 auphy, const void *buf, uint len);
void fpga_uart_init(void);
void fpga_uart_reinit(void);
#endif
