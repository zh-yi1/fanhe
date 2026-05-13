#include "include.h"

#if AVI_DVP_USE_CAMERA

#if IMG_SENSOR_BF03A2_EN
#define bf30a2_I2C_ADDR_W       0xDC
#define bf30a2_I2C_ADDR_R       0xDD

void bf30a2_config_vclk_delay(u8 delay1, u8 delay2)
{
    u8 reg_addr = 0xCA;
    u8 config_val = 0x03;

    if(delay1 == 1){
        config_val |= (1 << 7); //延迟6ns
    }
    else{
        config_val &= ~(1 << 7); //第一级延迟0ns
    }
    config_val &= ~(0x3 << 4);
    config_val |= (delay2 << 4);

    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, reg_addr, config_val);
}

static void sensor_drv_bf30a2_init(void)
{
    //240x320 init registers code.
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x12, 0x10); //software reset
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x15, 0x00); //power up
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x6b, 0x72|0);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x04, 0x00);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x06, 0x26);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x08, 0x07);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x1c, 0x12);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x1e, 0x26);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x1f, 0x01);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x20, 0x20);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x21, 0x20);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x34, 0x02);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x35, 0x02);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x36, 0x21);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x37, 0x13);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xca, 0x03);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xcb, 0x22);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xcc, 0x89);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xcd, 0x4c);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xce, 0x6b);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xcf, 0x90);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa0, 0x8e);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x01, 0x1b);

    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x02, 0x1d);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x13, 0x08);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x87, 0x13);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x8a, 0x33);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x8b, 0x08);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x70, 0x1f);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x71, 0x43);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x72, 0x0a);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x73, 0x62);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x74, 0xa2);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x75, 0xbf);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x76, 0x02);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x77, 0xcc);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x40, 0x32);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x41, 0x28);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x42, 0x26);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x43, 0x1d);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x44, 0x1a);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x45, 0x14);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x46, 0x11);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x47, 0x0f);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x48, 0x0e);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x49, 0x0d);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x4B, 0x0c);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x4C, 0x0b);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x4E, 0x0a);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x4F, 0x09);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x50, 0x09);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x24, 0x50);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x25, 0x36);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x80, 0x00); //00
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x81, 0x20);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x82, 0x40);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x83, 0x30); //30
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x84, 0x50); //50
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x85, 0x30);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x86, 0xD8);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x89, 0x45);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x8f, 0x81);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x91, 0xff);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x92, 0x08);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x94, 0x82);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x95, 0xfd);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x9a, 0x20);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x9e, 0xbc);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xf0, 0x8f);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x51, 0x06);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x52, 0x25);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x53, 0x2b);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x54, 0x0f);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x57, 0x2a);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x58, 0x22);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x59, 0x2c);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x23, 0x33);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa0, 0x8f);

    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa1, 0x93);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa2, 0x0f);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa3, 0x2a);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa4, 0x08);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa5, 0x26);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa7, 0x80);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa8, 0x80);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xa9, 0x1e);

    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xaa, 0x19);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xab, 0x18);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xae, 0x50);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xaf, 0x04);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xc8, 0x10);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xc9, 0x15);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xd3, 0x0c);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xd4, 0x16);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xee, 0x06);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xef, 0x04);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x55, 0x34);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x56, 0x9C);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xb1, 0x98);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xb2, 0x98);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xb3, 0xc4);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0xb4, 0x0C);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x00, 0x40);
    img_write_Reg8Data8(bf30a2_I2C_ADDR_W, 0x13, 0x07);

    bf30a2_config_vclk_delay(0,1);
}

static u32 bf30a2_drv_read_id(void)
{
    u32 id;
    u8 temp = 0;
    img_read_Reg8Data8(bf30a2_I2C_ADDR_W, 0xfc, &temp);
    id = temp;
    return id;
}


img_sensor_drv_t img_bf30a2_drv = {
    .speed_mhz            = 24,//Mhz
    .image_height         = 320,
    .image_width          = 240,
    .in_format            = IMG_IN_YUYV,
    .out_format           = IMG_OUT_RGB565,
    .imag_sensor_reg_init = sensor_drv_bf30a2_init,
    .imag_sensor_read_id  = bf30a2_drv_read_id,
    .vsync_is_high        = 0,     //0高有效，1低有效
    .hsync_is_high        = 0,     //0高有效，1低有效
    .dev_id               = 0x3b,  //有效ID值，为FF表示不判断ID
    .type                 = IMG_TYPE_SPICS,
};
#endif

#endif
