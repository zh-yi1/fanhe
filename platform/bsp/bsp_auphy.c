#include "include.h"

#if FPGA_EN

static const char str_phy_cmd_mic_ch[] = "#AUAMM\x00\x00*";
static const char str_phy_cmd_mic_gain[] = "#AUAMV\x00\x00*";

AT(.text.fpga)
void fpga_uart_putchar(u8 auphy, char ch)
{
    if (auphy & BIT(0)) {
        while (!(FPGAUTCON & BIT(8)));
        FPGAUTDATA = ch;
    }
//    if (auphy & BIT(1)) {
//        while (!(FPGAUTCON2 & BIT(8)));
//        FPGAUTDAT2 = ch;
//    }
}

AT(.text.fpga)
void fpga_uart_putcs(u8 auphy, const void *buf, uint len)
{
    const char *txbuf = buf;
    while (len--) {
        fpga_uart_putchar(auphy, *txbuf++);
    }
}

////vol：0~16
//void bsp_auphy_set_dac_volume(u8 vol)
//{
//    u8 auphy = 1;
//    fpga_uart_putchar(auphy, '#');
//    fpga_uart_putchar(auphy, 'A');
//    fpga_uart_putchar(auphy, 'U');
//    fpga_uart_putchar(auphy, 'V');
//    fpga_uart_putchar(auphy, 'V');  //多空了1byte
//    fpga_uart_putchar(auphy, 'V');
//    fpga_uart_putchar(auphy, vol);
//    fpga_uart_putchar(auphy, '*');
//}

void bsp_auphy_set_mic_analog_gain(u16 channel, u8 gain)
{
    u16 phy_ch = 0;
    u8 micn = channel & 0xf;
    u8 micm = (channel >> 8) & 0xf;
    u8 cmd_len = sizeof(str_phy_cmd_mic_gain) - 1;
    u8 fuart_txbuf[16];

    phy_ch |= micn > 0? ((0x0c << ((micn - 1) % 2) * 4)) : 0;   //对应queen phy的mic channel
    phy_ch |= micm > 0? ((0x0c << ((micm - 1) % 2) * 4)) : 0;

    memcpy(fuart_txbuf, str_phy_cmd_mic_gain, cmd_len);
    fuart_txbuf[6] = (u8)phy_ch >> 8;
    fuart_txbuf[7] = (u8)phy_ch;
    fuart_txbuf[8] = gain;
    fpga_uart_putcs(3, fuart_txbuf, cmd_len);
}

void bsp_auphy_mic_setup(u16 channel)
{
    u8 cmd_len = sizeof(str_phy_cmd_mic_ch) - 1;
    u8 fuart_txbuf[16];

    memcpy(fuart_txbuf, str_phy_cmd_mic_ch, cmd_len);
    fuart_txbuf[6] = 0x0c;
    fuart_txbuf[7] = 0xcc;

    fpga_uart_putcs(3, fuart_txbuf, cmd_len);
    delay_ms(30);           //等待AUDIOPHY的MIC上电, 约30ms
}

AT(.text.fpga)
void fpga_uart_init(void)
{
    u32 uart_baud = 0;
    u32 sys_clk = get_sysclk_nhz();

    uart_baud = (((sys_clk + (115200 / 2)) / 115200) - 1);

    //audio phy 1
    FPGAUTBAUD = (uart_baud << 16) | uart_baud;
    FPGAUTCON |= BIT(7) | BIT(0);

//    //audio phy 2
//    FPGAUTBAUD2 = (uart_baud << 16) | uart_baud;
//    FPGAUTCON2 |= BIT(7) | BIT(0);

    //printf("FPGAUTBAUD = %x, FPGAUTBAUD2 = %x\n",FPGAUTBAUD, FPGAUTBAUD2);

    fpga_uart_putcs(3, "#RESET*", 7);
    delay_5ms(100);
}

void fpga_uart_reinit(void)
{
    u32 uart_baud = 0;
    u32 sys_clk = get_sysclk_nhz();

    uart_baud = (((sys_clk + (115200 / 2)) / 115200) - 1);

    //audio phy 1
    FPGAUTBAUD = (uart_baud << 16) | uart_baud;
    FPGAUTCON |= BIT(7) | BIT(0);

//    //audio phy 2
//    FPGAUTBAUD2 = (uart_baud << 16) | uart_baud;
//    FPGAUTCON2 |= BIT(7) | BIT(0);
}

#endif
