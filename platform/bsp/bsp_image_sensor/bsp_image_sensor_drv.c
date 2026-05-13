#include "include.h"

#if AVI_DVP_USE_CAMERA


static gpio_t drv_i2c_scl, drv_i2c_sda;

img_sensor_drv_t *img_drv_t = NULL;
/**
 * i2c delay
 */
static void drv_i2c_delay (u32 us)
{
	delay_us(us*2);
}


/**
 * i2c start
 */
static void drv_i2c_start (void)
{
    drv_i2c_scl.sfr[GPIOxSET] = BIT(drv_i2c_scl.num);
    drv_i2c_sda.sfr[GPIOxSET] = BIT(drv_i2c_sda.num);
    drv_i2c_delay(1);
    drv_i2c_sda.sfr[GPIOxCLR] = BIT(drv_i2c_sda.num);
    drv_i2c_delay(1);
    drv_i2c_scl.sfr[GPIOxCLR] = BIT(drv_i2c_scl.num);
    drv_i2c_delay(1);
}


/**
 * i2c stop
 */
static void drv_i2c_stop (void)
{
    drv_i2c_scl.sfr[GPIOxCLR] = BIT(drv_i2c_scl.num);
    drv_i2c_sda.sfr[GPIOxCLR] = BIT(drv_i2c_sda.num);
    drv_i2c_delay(1);
    drv_i2c_scl.sfr[GPIOxSET] = BIT(drv_i2c_scl.num);
    drv_i2c_delay(1);
    drv_i2c_sda.sfr[GPIOxSET] = BIT(drv_i2c_sda.num);
    drv_i2c_delay(1);
}


/**
 * i2c write
 */
static int drv_i2c_write (u16 value, u8 ack)
{
	u8 i;
	int ret = -1;

	for(i=0;i<8;i++){
		drv_i2c_scl.sfr[GPIOxCLR] = BIT(drv_i2c_scl.num);
		drv_i2c_delay (1);
		if(value & 0x80) {
			drv_i2c_sda.sfr[GPIOxSET] = BIT(drv_i2c_sda.num);
		} else {
			drv_i2c_sda.sfr[GPIOxCLR] = BIT(drv_i2c_sda.num);
		}
		drv_i2c_delay(1);
		drv_i2c_scl.sfr[GPIOxSET] = BIT(drv_i2c_scl.num);
		drv_i2c_delay(1);
		value <<= 1;
	}

	// The 9th bit transmission
	drv_i2c_scl.sfr[GPIOxCLR] = BIT(drv_i2c_scl.num);
    drv_i2c_sda.sfr[GPIOxDIR] |= BIT(drv_i2c_sda.num);
	drv_i2c_delay(1);
	drv_i2c_scl.sfr[GPIOxSET] = BIT(drv_i2c_scl.num);

	// ckeck ack = low
	if(ack) {
        u32 ticks = tick_get();
        while(tick_check_expire(ticks, 10) == false){
            if((drv_i2c_sda.sfr[GPIOx] & BIT(drv_i2c_sda.num)) == 0) {
                ret = 0;
                break;
            }
        }
	}

	drv_i2c_delay(1);
	drv_i2c_scl.sfr[GPIOxCLR] = BIT(drv_i2c_scl.num);
    drv_i2c_sda.sfr[GPIOxDIR] &= ~BIT(drv_i2c_sda.num);
	return ret;
}


/**
 * i2c read
 */
static u16 drv_i2c_read (u8 phase)
{
	u16 i;
	u16 data = 0x00;

    drv_i2c_sda.sfr[GPIOxDIR] |= BIT(drv_i2c_sda.num);
	for(i=0;i<8;i++) {
		drv_i2c_scl.sfr[GPIOxCLR] = BIT(drv_i2c_scl.num);
		drv_i2c_delay(1);
		drv_i2c_scl.sfr[GPIOxSET] = BIT(drv_i2c_scl.num);
		data <<= 1;
        if((drv_i2c_sda.sfr[GPIOx] & BIT(drv_i2c_sda.num))){
    		data |= 0x01;
        }
		drv_i2c_delay(1);
	}

	// The 9th bit transmission
	drv_i2c_scl.sfr[GPIOxCLR] = BIT(drv_i2c_scl.num);
    drv_i2c_sda.sfr[GPIOxDIR] &= ~BIT(drv_i2c_sda.num);
	if(phase == 2) {
		drv_i2c_sda.sfr[GPIOxCLR] = BIT(drv_i2c_sda.num);
	} else {
		drv_i2c_sda.sfr[GPIOxSET] = BIT(drv_i2c_sda.num);
	}

	drv_i2c_delay(1);
	drv_i2c_scl.sfr[GPIOxSET] = BIT(drv_i2c_scl.num);
	drv_i2c_delay(1);
	drv_i2c_scl.sfr[GPIOxCLR] = BIT(drv_i2c_scl.num);
	return data;
}



/**
 * i2c init
 */
static void drv_i2c_init (void){
    gpio_cfg_init(&drv_i2c_scl, PORT_IMAGE_SENSOR_CLK);
    gpio_cfg_init(&drv_i2c_sda, PORT_IMAGE_SENSOR_SDA);

	drv_i2c_scl.sfr[GPIOxFEN] &= ~BIT(drv_i2c_scl.num);
	drv_i2c_scl.sfr[GPIOxDE]  |= BIT(drv_i2c_scl.num);
	drv_i2c_scl.sfr[GPIOxDIR] &= ~BIT(drv_i2c_scl.num);
	drv_i2c_scl.sfr[GPIOxPU]  |= BIT(drv_i2c_scl.num);
	drv_i2c_scl.sfr[GPIOxSET] = BIT(drv_i2c_scl.num);

	drv_i2c_sda.sfr[GPIOxFEN] &= ~BIT(drv_i2c_sda.num);
	drv_i2c_sda.sfr[GPIOxDE]  |= BIT(drv_i2c_sda.num);
	drv_i2c_sda.sfr[GPIOxDIR] &= ~BIT(drv_i2c_sda.num);
	drv_i2c_sda.sfr[GPIOxPU]  |= BIT(drv_i2c_sda.num);
	drv_i2c_sda.sfr[GPIOxSET] = BIT(drv_i2c_sda.num);
}


static void drv_i2c_reuse_io_enter(void)
{
#if (PORT_IMAGE_SENSOR_CLK == PORT_CTP_SCL) ||  (PORT_IMAGE_SENSOR_CLK == PORT_CTP_SDA) || \
    (PORT_IMAGE_SENSOR_SDA == PORT_CTP_SCL) ||  (PORT_IMAGE_SENSOR_SDA == PORT_CTP_SDA)
    u32 ticks = tick_get();
    while (ctp_get_kick_status()) {
        if (tick_check_expire(ticks, 100)) {
            //printf("IIC_Master_Send time out ERROR\n");
            break;
        }
    }
    sys_ctp_irq_disable(IRQ_I2C_VECTOR);
    port_irq_free(PORT_CTP_INT_VECTOR);
#endif
}

static void drv_i2c_reuse_io_exit(void)
{
#if (PORT_IMAGE_SENSOR_CLK == PORT_CTP_SCL) ||  (PORT_IMAGE_SENSOR_CLK == PORT_CTP_SDA) || \
    (PORT_IMAGE_SENSOR_SDA == PORT_CTP_SCL) ||  (PORT_IMAGE_SENSOR_SDA == PORT_CTP_SDA)
    drv_i2c_scl.sfr[GPIOxDIR] |= BIT(drv_i2c_scl.num);
    drv_i2c_scl.sfr[GPIOxPU] |= BIT(drv_i2c_scl.num);
    drv_i2c_scl.sfr[GPIOxDE] |= BIT(drv_i2c_scl.num);
    drv_i2c_scl.sfr[GPIOxFEN] |= BIT(drv_i2c_scl.num);

    drv_i2c_sda.sfr[GPIOxDIR] |= BIT(drv_i2c_sda.num);
    drv_i2c_sda.sfr[GPIOxPU] |= BIT(drv_i2c_sda.num);
    drv_i2c_sda.sfr[GPIOxDE] |= BIT(drv_i2c_sda.num);
    drv_i2c_sda.sfr[GPIOxFEN] |= BIT(drv_i2c_sda.num);

    port_irq_register(PORT_CTP_INT_VECTOR, ctp_int_isr);
    sys_ctp_irq_enble(IRQ_I2C_VECTOR);
#endif
}

/**
 * drv write
 */
static int img_drv_write(u8  id, u32 addr_bits,
	                        u32 addr, u32 data_bits, u32 data)
{
	u8 temp, ack = 1;
	int ret;

	drv_i2c_start();
	ret = drv_i2c_write(id, 1);
	if(ret < 0) {
        drv_i2c_stop();
        return ret;
	}

	while(addr_bits >= 8) {
		addr_bits -= 8;
		temp = addr >> addr_bits;
		ret = drv_i2c_write(temp, ack);
		if(ret < 0) {
            drv_i2c_stop();
            return ret;
		}
	}

	while(data_bits >= 8) {
		data_bits -= 8;
		if(data_bits) {
			ack = 1;
		} else {
			ack = 0;
		}

		temp = data >> data_bits;
		ret = drv_i2c_write(temp, ack);
		if(ret < 0) {
            drv_i2c_stop();
            return ret;
		}
	}

	drv_i2c_stop();
	return ret;
}

/**
 * drv read
 */

static int img_drv_read(u8 id, u32 addr_bits,
	                       u32 addr, u32 data_bits, u32 *data)
{
	u8 temp, ack = 1;
	u32 read_data;
	int ret;

	drv_i2c_start();

	ret = drv_i2c_write(id, 1);
	if(ret < 0) {
        drv_i2c_stop();
        return ret;
    }

	while(addr_bits >= 8) {
		addr_bits -= 8;
		temp = addr >> addr_bits;
		ret = drv_i2c_write(temp, ack);
		if(ret < 0) {
            drv_i2c_stop();
            return ret;
		}
	}

	drv_i2c_stop();
	drv_i2c_start();

	ret = drv_i2c_write(id | 0x01, 1);
	if(ret < 0) {
        drv_i2c_stop();
        return ret;
	}

	read_data = 0;
	while(data_bits >= 8) {
		data_bits -= 8;
		if(data_bits) {
			ack = 1;
		} else {
			ack = 0;
		}
		temp = drv_i2c_read(ack);
		read_data <<= 8;
		read_data |= temp;
	}
	*data = read_data;

	drv_i2c_stop();
	return ret;
}


/**
 * write Reg8Data8
 */
int img_write_Reg8Data8(u8 id, u8 addr, u8 data)
{
    return img_drv_write(id, 8, addr, 8, data);
}


/**
 * write Reg8Data16
 */
int img_write_Reg8Data16(u8 id, u8 addr, u16 data)
{
    return img_drv_write(id, 8, addr, 16, data);
}

/**
 * write Reg16Data8
 */
int img_write_Reg16Data8(u8 id, u16 addr, u8 data)
{
    return img_drv_write(id, 16, addr, 8, data);
}

/**
 * write Reg16Data16
 */
int img_write_Reg16Data16(u8 id, u16 addr, u16 data)
{
    return img_drv_write(id, 16, addr, 16, data);
}


/**
 * raed Reg8Data8
 */
int img_read_Reg8Data8(u8 id, u8 addr, u8 *data)
{
    u32 rb;

    int ret = img_drv_read(id, 8, addr, 8, &rb);
    if(ret >= 0){
        *data = rb & 0xFF;
    }
    return ret;
}

/**
 * raed Reg8Data16
 */
int img_read_Reg8Data16(u8 id, u8 addr, u16 *data)
{
    u32 rb;

    int ret = img_drv_read(id, 8, addr, 16, &rb);
    if(ret >= 0){
        *data = rb & 0xFFFF;
    }
    return ret;

}

/**
 * raed Reg16Data8
 */
int img_read_Reg16Data8(u8 id, u16 addr, u8 *data)
{
    u32 rb;

    int ret = img_drv_read(id, 16, addr, 8, &rb);
    if(ret >= 0){
        *data = rb & 0xFF;
    }
    return ret;
}

/**
 * raed Reg16Data8
 */
int img_read_Reg16Data16(u8 id, u16 addr, u16 *data)
{
    u32 rb;

    int ret = img_drv_read(id, 16, addr, 16, &rb);
    if(ret >= 0){
        *data = rb & 0xFFFF;
    }
    return ret;
}


/**
 * 获取Sensor的数据范围
 */
void bsp_dvp_set_crop(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h)
{
    DVPCROPH = (DVPCROPH & ~(0
        | BIT(DVP_CROP_H_OFF_SHF) * DVP_CROP_H_OFF_MSK
        | BIT(DVP_CROP_H_END_SHF) * DVP_CROP_H_END_MSK
        ))
        | BIT(DVP_CROP_H_OFF_SHF) * x0
        | BIT(DVP_CROP_H_END_SHF) * (x0 + w - 1)
        ;
    DVPCROPV = (DVPCROPV & ~(0
        | BIT(DVP_CROP_V_OFF_SHF) * DVP_CROP_V_OFF_MSK
        | BIT(DVP_CROP_V_END_SHF) * DVP_CROP_V_END_MSK
        ))
        | BIT(DVP_CROP_V_OFF_SHF) * y0
        | BIT(DVP_CROP_V_END_SHF) * (y0 + h - 1)
        ;
}

/**
 * 设置DMA stride
 */
void bsp_dvp_set_outsize(uint16_t stride)
{
    DVPSTRIDE = (DVPSTRIDE & ~(BIT(DVP_STRIDE_SHF)*DVP_STRIDE_MSK))
        | BIT(DVP_STRIDE_SHF) * stride;
}

/**
 * 设置输出格式
 * 仅支持 1:RGB565 0:YUYV
 */
void bsp_dvp_set_ofmt(uint32_t ofmt)
{
    if (ofmt > 1) printf ("Error DVP output fmt\n");
    DVPCON = (DVPCON & ~BIT(DVP_OUTFMT_SHF))
        | BIT(DVP_OUTFMT_SHF) * ofmt;
}

/**
 * 设置输入大小
 */
void bsp_dvp_set_insize(uint16_t width, uint16_t height)
{
    DVPSIZE = 0
        | BIT(DVP_IMG_WIDTH_SHF)  * (width - 1)
        | BIT(DVP_IMG_HEIGHT_SHF) * (height - 1)
        ;
}

/**
 * 设置DMA buff
 */
extern u32 __psram_vma;

AT(.com_text.dvp)
void bsp_dvp_set_dbuf(void *buf, uint32_t stride, uint32_t lines)
{
    uint32_t adr = (uint32_t) buf;
    bool ext = 0;   //0:sram 1:psram

    if (adr >= (u32)&__psram_vma) {
        ext = 1;
    }

    DVPADR = DMA_ADR(buf);
    DVPCON = (DVPCON & ~BIT(DVP_OUTSRAM_SHF)) | BIT(DVP_OUTSRAM_SHF) * !ext;
    DVPSTRIDE = 0
        | BIT(DVP_HEIGHT_SEG_SHF) * lines
        | BIT(DVP_STRIDE_SHF)     * stride
        ;
}

/**
 * DMA start
 */
void bsp_dvp_dma_start(void)
{
    DVPPEND = 0xff;
    DVPCON |= BIT(DVP_DMA_EN_SHF) | BIT(DVP_EN_SHF);
}

/**
 * DMA start
 */
AT(.com_text.dvp)
void bsp_dvp_dma_stop(void)
{
    DVPPEND = 0xff;
    DVPCON &= ~(BIT(DVP_DMA_EN_SHF) | BIT(DVP_EN_SHF));
}

/**
 * GPIO initt
 */
static void image_sensor_ctrl_init(u8 io_num)
{
    if(io_num == IO_NONE){
        return;
    }
    gpio_t ctrl_gpio;
    gpio_cfg_init(&ctrl_gpio, io_num);
    ctrl_gpio.sfr[GPIOxFEN] &= ~BIT(ctrl_gpio.num);
    ctrl_gpio.sfr[GPIOxDE]  |= BIT(ctrl_gpio.num);
    ctrl_gpio.sfr[GPIOxDIR] &= ~BIT(ctrl_gpio.num);
    ctrl_gpio.sfr[GPIOxSET] = BIT(ctrl_gpio.num);
}

static void image_sensor_ctrl_rest_do(u8 io_num)
{
    if(io_num == IO_NONE){
        return;
    }
    gpio_t ctrl_gpio;
    gpio_cfg_init(&ctrl_gpio, io_num);
    ctrl_gpio.sfr[GPIOxCLR] = BIT(ctrl_gpio.num);
    delay_ms(20);
    ctrl_gpio.sfr[GPIOxSET] = BIT(ctrl_gpio.num);
    delay_ms(20);
}

static void image_sensor_ctrl_pwdn_do(u8 io_num)
{
    if(io_num == IO_NONE){
        return;
    }
    gpio_t ctrl_gpio;
    gpio_cfg_init(&ctrl_gpio, io_num);
    ctrl_gpio.sfr[GPIOxSET] = BIT(ctrl_gpio.num);
    delay_ms(20);
    ctrl_gpio.sfr[GPIOxCLR] = BIT(ctrl_gpio.num);
    delay_ms(20);
}


/**
 * GPIO ctrl
 */
static u8 drv_ctrl_io[IMG_SENSOR_NUM][2] =
{
    {PORT_IMAGE_SENSOR_RST1, PORT_IMAGE_SENSOR_PWDN1},
#if IMG_SENSOR_NUM > 1
    {PORT_IMAGE_SENSOR_RST2, PORT_IMAGE_SENSOR_PWDN2},
#endif
};

static void image_sensor_ctrl(u8 idx)
{
    if(idx >= IMG_SENSOR_NUM){
        return;
    }

    for(u8 i=0; i<IMG_SENSOR_NUM; i++){
        image_sensor_ctrl_init(drv_ctrl_io[i][0]);
        image_sensor_ctrl_init(drv_ctrl_io[i][1]);
    }
    delay_ms(5);
    image_sensor_ctrl_rest_do(drv_ctrl_io[idx][0]);
    image_sensor_ctrl_pwdn_do(drv_ctrl_io[idx][1]);
}

static void image_sensor_ctrl_unregister(void)
{
    for(u8 i=0; i<IMG_SENSOR_NUM; i++){
        image_sensor_ctrl_init(drv_ctrl_io[i][0]);
        image_sensor_ctrl_init(drv_ctrl_io[i][1]);
    }
}

/**
 * lcd register
 */
static void image_sensor_drv_register_do(u8 idx, img_sensor_drv_t *drv)
{
    if(drv == NULL){
        return;
    }
    img_drv_t = drv;
    //rest io
    image_sensor_ctrl(idx);

    if (img_drv_t->type == IMG_TYPE_DVP) {
        //DVP io init
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D7,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D6,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D5,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D4,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D3,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D2,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D1,0);

        port_gpio_set_func(PORT_IMAGE_SENSOR_HSYNC,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_VSYNC,0);
    }
    port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D0,0);
    port_gpio_set_func(PORT_IMAGE_SENSOR_VCLK,0);
    port_gpio_set_func(PORT_IMAGE_SENSOR_MCLK,0);
    gpio_t gpio;
    gpio_cfg_init(&gpio, PORT_IMAGE_SENSOR_MCLK);
    gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);

    if (img_drv_t->type == IMG_TYPE_SPICS) {
        //CLK init
        RSTCON0 &= ~(BIT(18)|BIT(19));                          // dvp/spisen
        CLKDIVCON0 = (CLKDIVCON0 & ~(BIT(8) * 0x3)) | (BIT(8) * 0);     // clkdiv for dvp_clk          // 00:XOSC 01:XOSCx2 10:PLL0DIV2 11:PLL1DIV2
        CLKCON2 = (CLKCON2 & ~(BIT(17)*0x3)) | (BIT(17) * 0);
        CLKDIVCON2 = (CLKDIVCON2 & ~(BIT(24) * 0xF)) | (0 * BIT(24));   // clkdiv for dvp_mclk
        CLKCON2 = (CLKCON2 & ~(BIT(19)* 0xF)) | (0b0000 * BIT(19));       // inv1&dly3
        CLKGAT3 |=  BIT(4) | BIT(5);                              // dvp/spisen
        RSTCON0 |=  BIT(18) | BIT(19);                            // dvp/spisen
        FUNCMCON3 = (1 << 4);  //SPI_CS
        //init senspi
        SENSPIBAUD = 20;
        SENSPIFSIZE =  0 | ((img_drv_t->image_height-1) << 16 | (img_drv_t->image_width-1));
        SENSPIFHEAD = 0xff0000ab;
        SENSPILHEAD = 0xff000080;
        SENSPICON = BIT(15) | BIT(10)| BIT(5) | BIT(4) |BIT(6)| (1<<2) | BIT(1) |  BIT(0) ;
        SENSPIDMACNT = 0;
    } else if (img_drv_t->type == IMG_TYPE_DVP) {
        //CLK init
        RSTCON0 &= ~(BIT(18)|BIT(19));                          // dvp/spisen
        CLKDIVCON0 = (CLKDIVCON0 & ~(0x3 << 8)) | (0 << 8);     // clkdiv for dvp_clk
        CLKCON2 = (CLKCON2 & ~(0x3 << 7)) | (0 << 7);           // 00:XOSC 01:XOSCx2 10:PLL0DIV2 11:PLL1DIV2
        CLKDIVCON2 = (CLKDIVCON2 & ~(0xF << 24)) | (0 << 24);   // clkdiv for dvp_mclk
        CLKCON2 = (CLKCON2 & ~(0xF << 19)) | (0x0 << 19);       // inv1&dly3
        CLKGAT3 |=  BIT(4) | BIT(5);                              // dvp/spisen
        RSTCON0 |=  BIT(18) | BIT(19);                            // dvp/spisen
        FUNCMCON3 = (1 << 8); //DVP_CS
    }

    //init dvp
    DVPCON =
         BIT(16) |
         BIT(15) |
         BIT(13) |
         BIT(12) |
         BIT(18) |              //DMA HALF LINE DONE
         BIT(17) |              //DMA LINE DONE
         BIT(14) |              //DMA done
         (BIT(6) * img_drv_t->vsync_is_high) |              //VSYNC LOW
         (BIT(5) * img_drv_t->hsync_is_high) |            //HSYNC LOW
         BIT(2)  |              //DMA
         BIT(1);                //CROP EN

    DVPCON |= (img_drv_t->in_format) << 3; //Input format
    bsp_dvp_set_crop(0, 0, img_drv_t->image_width, img_drv_t->image_height);
    bsp_dvp_set_outsize(img_drv_t->image_width * 2);         //2bytes/pix
    bsp_dvp_set_ofmt(img_drv_t->out_format - IMG_OUT_YUV422);
    bsp_dvp_set_insize(img_drv_t->image_width, img_drv_t->image_height);

    //I2c
    drv_i2c_reuse_io_enter();
    drv_i2c_init();
    delay_ms(10);

    if(img_drv_t->imag_sensor_read_id != NULL){
        printf("image sensor id:0x%x\n", img_drv_t->imag_sensor_read_id());
     }
     img_drv_t->imag_sensor_reg_init();

     drv_i2c_reuse_io_exit();
}

/**
 * sensor active
 */
static void image_sensor_drv_active(u8 idx)
{
    //reset io
    image_sensor_ctrl(idx);

    //DVP io init
    if (img_drv_t->type == IMG_TYPE_DVP) {
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D7,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D6,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D5,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D4,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D3,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D2,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D1,0);

        port_gpio_set_func(PORT_IMAGE_SENSOR_HSYNC,0);
        port_gpio_set_func(PORT_IMAGE_SENSOR_VSYNC,0);
    }
    port_gpio_set_func(PORT_IMAGE_SENSOR_DVP_D0,0);
    port_gpio_set_func(PORT_IMAGE_SENSOR_VCLK,0);
    port_gpio_set_func(PORT_IMAGE_SENSOR_MCLK,0);
    gpio_t gpio;
    gpio_cfg_init(&gpio, PORT_IMAGE_SENSOR_MCLK);
    gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);

    if (img_drv_t->type == IMG_TYPE_SPICS) {
        //CLK init
        RSTCON0 &= ~(BIT(18)|BIT(19));                          // dvp/spisen
        CLKDIVCON0 = (CLKDIVCON0 & ~(BIT(8) * 0x3)) | (BIT(8) * 0);     // clkdiv for dvp_clk          // 00:XOSC 01:XOSCx2 10:PLL0DIV2 11:PLL1DIV2
        CLKCON2 = (CLKCON2 & ~(BIT(17)*0x3)) | (BIT(17) * 0);
        CLKDIVCON2 = (CLKDIVCON2 & ~(BIT(24) * 0xF)) | (0 * BIT(24));   // clkdiv for dvp_mclk
        CLKCON2 = (CLKCON2 & ~(BIT(19)* 0xF)) | (0b0000 * BIT(19));       // inv1&dly3
        CLKGAT3 |=  BIT(4) | BIT(5);                              // dvp/spisen
        RSTCON0 |=  BIT(18) | BIT(19);                            // dvp/spisen
        FUNCMCON3 = (1 << 4);  //SPI_CS
//        init senspi
//        SENSPIBAUD = 0;
//        SENSPIFSIZE =  (SENSPIFSIZE &~0xffffffff) | (319 << 16 | 239);
//        SENSPIFHEAD = 0xff0000ab;
//        SENSPILHEAD = 0xff000080;
        SENSPICON = BIT(15) | BIT(10)| BIT(5) | BIT(4) |BIT(6)| (1<<2) | BIT(1) |  BIT(0) ;
//        SENSPIDMACNT = 0;
    } else if (img_drv_t->type == IMG_TYPE_DVP) {
        //CLK init
        RSTCON0 &= ~(BIT(18)|BIT(19));                          // dvp/spisen
        CLKDIVCON0 = (CLKDIVCON0 & ~(0x3 << 8)) | (0 << 8);     // clkdiv for dvp_clk
        CLKCON2 = (CLKCON2 & ~(0x3 << 7)) | (0 << 7);           // 00:XOSC 01:XOSCx2 10:PLL0DIV2 11:PLL1DIV2
        CLKDIVCON2 = (CLKDIVCON2 & ~(0xF << 24)) | (0 << 24);   // clkdiv for dvp_mclk
        CLKCON2 = (CLKCON2 & ~(0xF << 19)) | (0x0 << 19);       // inv1&dly3
        CLKGAT3 |=  BIT(4) | BIT(5);                              // dvp/spisen
        RSTCON0 |=  BIT(18) | BIT(19);                            // dvp/spisen
        FUNCMCON3 = (1 << 8);
	}
    //init dvp
    DVPCON =
         BIT(16) |
         BIT(15) |
         BIT(13) |
         BIT(12) |
         BIT(18) |              //DMA HALF LINE DONE
         BIT(17) |              //DMA LINE DONE
         BIT(14) |              //DMA done
         BIT(2)  |              //DMA
         BIT(1);                //CROP EN
}


/**
 * lcd unregister
 */
void image_sensor_drv_unregister(void)
{
    DVPCON = 0;
    RSTCON0 |= (BIT(18)|BIT(19));
    CLKGAT3 &= ~(BIT(4) | BIT(5));

    image_sensor_ctrl_unregister();
}


/**
 * image sensor lowpower
 */
void image_sensor_drv_enter_pwdn(void)
{
    CAMERA_POWER_DISABLE();

    image_sensor_ctrl_unregister();

    gpio_cfg_init(&drv_i2c_scl, PORT_IMAGE_SENSOR_CLK);
    gpio_cfg_init(&drv_i2c_sda, PORT_IMAGE_SENSOR_SDA);

	drv_i2c_scl.sfr[GPIOxDE]  &= ~BIT(drv_i2c_scl.num);
	drv_i2c_scl.sfr[GPIOxPU]  &= ~BIT(drv_i2c_scl.num);
	drv_i2c_sda.sfr[GPIOxDE]  &= ~BIT(drv_i2c_sda.num);
	drv_i2c_sda.sfr[GPIOxPU]  &= ~BIT(drv_i2c_sda.num);

    port_gpio_disable(PORT_IMAGE_SENSOR_DVP_D7);
    port_gpio_disable(PORT_IMAGE_SENSOR_DVP_D6);
    port_gpio_disable(PORT_IMAGE_SENSOR_DVP_D5);
    port_gpio_disable(PORT_IMAGE_SENSOR_DVP_D4);
    port_gpio_disable(PORT_IMAGE_SENSOR_DVP_D3);
    port_gpio_disable(PORT_IMAGE_SENSOR_DVP_D2);
    port_gpio_disable(PORT_IMAGE_SENSOR_DVP_D1);
    port_gpio_disable(PORT_IMAGE_SENSOR_DVP_D0);

    port_gpio_disable(PORT_IMAGE_SENSOR_HSYNC);
    port_gpio_disable(PORT_IMAGE_SENSOR_VSYNC);
    port_gpio_disable(PORT_IMAGE_SENSOR_VCLK);
    port_gpio_disable(PORT_IMAGE_SENSOR_MCLK);
}


static img_sensor_drv_t *drv_img_sensor_reg[] =
{
#if IMG_SENSOR_SPA0A39_EN
    &img_spa0a39_drv,
#endif // IMG_SENSOR_SPA0A39_EN

#if IMG_SENSOR_GC0308_EN
    &img_gc0308_drv,
#endif
#if IMG_SENSOR_BF03A2_EN
    &img_bf30a2_drv,
#endif
};

static img_sensor_drv_t *drv_img_sensor_atuo_reg[IMG_SENSOR_NUM];

void image_sensor_drv_auto_reg(void)
{
    CAMERA_POWER_ENABLE();
//    printf("%s\n", __func__);
    u8 idx_num = sizeof(drv_img_sensor_reg) / sizeof(img_sensor_drv_t *);
    u32 id;

    for(u8 i=0; i<IMG_SENSOR_NUM; i++){
        drv_img_sensor_atuo_reg[i] = NULL;
        image_sensor_drv_active(i);
        //I2c
        drv_i2c_reuse_io_enter();
        drv_i2c_init();
        delay_ms(5);

        for(u8 j=0; j<idx_num; j++){
            img_sensor_drv_t *img_drv = drv_img_sensor_reg[j];
            id = 0xffffffff;
            if(img_drv->imag_sensor_read_id != NULL){
                id = img_drv->imag_sensor_read_id();
                printf("sensor id:0x%x\n", id);
            }
            if((img_drv->dev_id == 0xffffffff) || (img_drv->dev_id == id)){
                drv_img_sensor_atuo_reg[i] = img_drv;
                break;
            }
        }
        drv_i2c_reuse_io_exit();
    }
}

AT(.com_text.image_sensor)
u8 image_sensor_get_drv_type(void)
{
    if (drv_img_sensor_atuo_reg[0]) {
        return drv_img_sensor_atuo_reg[0]->type;
    }

    return 0;
}

bool image_sensor_drv_register(u8 idx)
{
    printf("idx:%d\n", idx);
    if(idx >= IMG_SENSOR_NUM){
        idx = 0;
    }

    if(drv_img_sensor_atuo_reg[idx] != NULL){
        image_sensor_drv_register_do(idx, drv_img_sensor_atuo_reg[idx]);
        return true;
    }else{
        for(u8 i=0; i<IMG_SENSOR_NUM; i++){
            if(drv_img_sensor_atuo_reg[i] != NULL){
                image_sensor_drv_register_do(i, drv_img_sensor_atuo_reg[i]);
                return true;
            }
        }
    }

    return false;
}


/**
 * image sensor change
 */
void image_sensor_drv_change(void)
{
    sys_cb.image_sensor_idx++;
    if(sys_cb.image_sensor_idx >= IMG_SENSOR_NUM){
        sys_cb.image_sensor_idx = 0;
    }
}

void ldo1_enable(u8 io_num)
{
    port_gpio_set_out(io_num, 1);
}

void ldo1_disable(u8 io_num)
{
    port_gpio_set_out(io_num, 0);
}

#endif // AVI_DVP_USE_CAMERA

