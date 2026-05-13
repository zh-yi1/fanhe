#include "include.h"
#include "bsp_i2c.h"
#include "axs5106.h"

#if (CTP_SELECT == CTP_AXS5106)

#define AXS_UPGRADE_ENABLE

#ifdef AXS_UPGRADE_ENABLE
#define AXS_UPGRADE_CHECK_VERSION    		1
#define AXS_UPGRADE_FAILED_RETRY_TIMES   	1
#define AXS_UPGRADE_FAST_MODE   			1

static const uint8_t fw_buf[] =
{
#include "GD9853_G2538B_AXS5106_V01_03_20231106_App.h"
};
#endif


#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define I2C_ADDR_AXS5106             (0x63)

#if 1
// RST PE7
#define AXS5106_RESET_FEN            GPIOEFEN
#define AXS5106_RESET_PDIR           GPIOEDIR
#define AXS5106_RESET_PORTSET        GPIOESET
#define AXS5106_RESET_PORTCLR        GPIOECLR
#define AXS5106_RESET_FEN            GPIOEFEN
#define AXS5106_RESET_DE             GPIOEDE
#define AXS5106_RESET                7
#elif 0
// RST PB7
#define AXS5106_RESET_FEN            GPIOBFEN
#define AXS5106_RESET_PDIR           GPIOBDIR
#define AXS5106_RESET_PORTSET        GPIOBSET
#define AXS5106_RESET_PORTCLR        GPIOBCLR
#define AXS5106_RESET_FEN            GPIOBFEN
#define AXS5106_RESET_DE             GPIOBDE
#define AXS5106_RESET                7
#else
	#error CHIP_PACKAGE_SELECT select error!!!
#endif


//// INT PF3
//
//#define AXS5106_INT_PDIR             GPIOFDIR
//#define AXS5106_INT_PORTSET          GPIOFSET
//#define AXS5106_INT_PORTCLR          GPIOFCLR
//#define AXS5106_INT_FEN              GPIOFFEN
//#define AXS5106_INT_DE               GPIOFDE
//#define AXS5106_INT                  3


#define AXS5106_WRITE_ADDR(ADDR)     ((ADDR) << 1)
#define AXS5106_READ_ADDR(ADDR)      ((ADDR) << 1 | 1)

#define AXS5106_FINGER_MAX           1
#define TOUCH_DATA_GROUP_LEN         4
#define TOUCH_DATA_LEN              (2 + TOUCH_DATA_GROUP_LEN * AXS5106_FINGER_MAX)

#define I2C_WRITE_ADDR(ADDR)     		 ((ADDR) << 1)				//CTP IIC写地址
#define I2C_READ_ADDR(ADDR)      		 ((ADDR) << 1 | 1)			//CTP IIC读地址

#if 1
    static i2c_t __AX5106_IIC1 = {
        .sfr         = (i2c_sfr_t *) &IIC1CON0,
        .map         = (i2c_map_t *) &FUNCMCON2,
    };

    i2c_t *AX5106_IICx = &__AX5106_IIC1;
#elif 0
    static i2c_t __AX5106_IIC0 = {
        .sfr         = (i2c_sfr_t *) &IIC0CON0,
        .map         = (i2c_map_t *) &FUNCMCON2,
    };

    i2c_t *AX5106_IICx = &__AX5106_IIC0;
#else
	#error CHIP_PACKAGE_SELECT select error!!!
#endif


#ifdef AXS_UPGRADE_ENABLE
static bool axs_fwupg_get_ver_in_bin(uint16_t *ver);
//static bool axs_fwupg_get_ver_in_tp(uint16_t *ver);
static bool axs_fwupg_get_pj_id_in_bin(uint16_t *ver);
//static bool axs_fwupg_get_pj_id_in_tp(uint16_t *ver);
static void axs_fwupg_enter_debug_mode(void);
static void axs_fwupg_exit_debug_mode(void);
static bool axs_fwupg_unlock_mtpc_wp(void);
static bool axs_fwupg_flash_erase(void);
static bool axs_fwupg_flash_write(const uint8_t *fw_buf, uint16_t fw_len);
static bool axs_fwupg_write_flash(void);
bool axs_upgrade_process(void);
#ifdef AXS_UPGRADE_CHECK_VERSION
static bool axs_fwupg_need_upgrade(void);
#endif
#define  MAX_TP_UPDATE_LEN_ONCE   (0x4000) // 加大buffer 一次传完测试Ok
u8 tp_ota_buf[ MAX_TP_UPDATE_LEN_ONCE] AT(.ctp_ota);
#endif

#if TRACE_EN
AT(.com_rodata.tp)
const char tp_read[] = "AX5106_IICx->sfr->IICxCMDA: %x len: %d \n";

//AT(.com_rodata.tp)
//const char AX5106_IICx->sfr->IICx_DATA[] = " read AX5106_IICx->sfr->IICxCON0: %x AX5106_IICx->sfr->IICxCON1:%x  AX5106_IICx->sfr->IICxCMDA: %x AX5106_IICx->sfr->IICxDMAADR: %x AX5106_IICx->sfr->IICxDMACNT: %x \n";
//
//AT(.com_rodata.tp)
//const char AX5106_IICx->sfr->IICx_DATA2[] = "read2 AX5106_IICx->sfr->IICxCON0: %x AX5106_IICx->sfr->IICxCON1:%x  AX5106_IICx->sfr->IICxCMDA: %x AX5106_IICx->sfr->IICxDMAADR: %x AX5106_IICx->sfr->IICxDMACNT: %x \n";
//
//AT(.com_rodata.tp)
//const char AX5106_IICx->sfr->IICx_wDATA[] = "writ AX5106_IICx->sfr->IICxCON0: %x AX5106_IICx->sfr->IICxCON1:%x  AX5106_IICx->sfr->IICxCMDA: %x AX5106_IICx->sfr->IICxDMAADR: %x AX5106_IICx->sfr->IICxDMACNT: %x \n";
//
//AT(.com_rodata.tp)
//const char AX5106_IICx->sfr->IICx_wDATA2[] = "writ2 AX5106_IICx->sfr->IICxCON0: %x AX5106_IICx->sfr->IICxCON1:%x  AX5106_IICx->sfr->IICxCMDA: %x AX5106_IICx->sfr->IICxDMAADR: %x AX5106_IICx->sfr->IICxDMACNT: %x \n";

AT(.com_rodata.tp)
const char tp_str[] = "%x %x \n";

AT(.com_rodata.tp)
const char tp_x_y[] = "x: %d  y: %d  \n";

AT(.com_rodata.tp)
const char h_str[] = "\n";

AT(.com_rodata.tp)
const char d_str[] = "down:%d,%d\n";

AT(.com_rodata.tp)
const char u_str[] = "up\n";
#endif
uint8_t tp_data[TOUCH_DATA_LEN];


AT(.com_text.ctp)
static void tp_i2c_read_kick(uint8_t subAddr, uint8_t *read_data, u16 len)
{
    AX5106_IICx->sfr->IICxCMDA = (I2C_READ_ADDR(I2C_ADDR_AXS5106) << 24) | (subAddr << 8) | I2C_WRITE_ADDR(I2C_ADDR_AXS5106);       //0x2A
    AX5106_IICx->sfr->IICxCON1 = BIT(12) | BIT(11) | BIT(9) | BIT(8) | BIT(7) | BIT(5) | BIT(4) | BIT(3) | len;
    AX5106_IICx->sfr->IICxDMAADR = DMA_ADR(read_data);
    AX5106_IICx->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(0);
    AX5106_IICx->sfr->IICxCON0 |= BIT(28);                //KICK
}


AT(.text.sensor.AXS5106) __attribute__((unused))
static void tp_i2c_write(uint16_t subAddr, const uint8_t *write_data, u16 len)
{
    //tp_i2c_write_do(false, I2C_ADDR_AXS5106, subAddr, false, write_data, len);
    u32 ticks;
    AX5106_IICx->sfr->IICxCMDA =  (subAddr << 8) | I2C_WRITE_ADDR(I2C_ADDR_AXS5106);       //0x2A

    AX5106_IICx->sfr->IICxCON1 = BIT(12) | BIT(11)  | BIT(10) | BIT(5) | BIT(4) | BIT(3) ;
    AX5106_IICx->sfr->IICxDMAADR = DMA_ADR(write_data);
    AX5106_IICx->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(1) |BIT(0);
    AX5106_IICx->sfr->IICxCON0 |= BIT(28);                //KICK
    delay_ms(2);

    ticks = tick_get();
    while (!(AX5106_IICx->sfr->IICxCON0 & BIT(31))) {
        if (tick_check_expire(ticks, 20)) {
            TRACE("tp_i2c_write time out ERROR\n");
            return;
        }
    }
    AX5106_IICx->sfr->IICxCON0 |= BIT(29);                //Clear Pending
}

AT(.text.sensor.AXS5106) __attribute__((unused))
static void tp_i2c_read(uint8_t subAddr, uint8_t *read_data, u16 len)
{
   // tp_i2c_read_do(false, I2C_ADDR_AXS5106, subAddr, false, read_data, len);
    u32 ticks;

    AX5106_IICx->sfr->IICxCMDA = (I2C_READ_ADDR(I2C_ADDR_AXS5106) << 24) | (subAddr << 8) | I2C_WRITE_ADDR(I2C_ADDR_AXS5106);       //0x2A
    AX5106_IICx->sfr->IICxCON1 = BIT(12) | BIT(11) | BIT(9) | BIT(8) | BIT(7) | BIT(5) | BIT(4) | BIT(3) | len;
    AX5106_IICx->sfr->IICxDMAADR = DMA_ADR(read_data);
    AX5106_IICx->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(0);
    AX5106_IICx->sfr->IICxCON0 |= BIT(28);                //KICK

    ticks = tick_get();
    while (!(AX5106_IICx->sfr->IICxCON0 & BIT(31))) {
        if (tick_check_expire(ticks, 20)) {
            TRACE("tp_i2c_read time out ERROR\n");
            return;
        }
    }
    AX5106_IICx->sfr->IICxCON0 |= BIT(29);                //Clear Pending

}



AT(.text.sensor.AXS5106)
void AXS5106_io_init(void)
{
    // Reset
    AXS5106_RESET_FEN &= ~BIT(AXS5106_RESET);
    AXS5106_RESET_PORTSET  =  BIT(AXS5106_RESET);
    AXS5106_RESET_PDIR    &= ~BIT(AXS5106_RESET);
    AXS5106_RESET_DE      |=  BIT(AXS5106_RESET);
}

AT(.text.sensor.AXS5106)
void AXS5106_reset(void)
{

    u8 cmd = 0xff;
    TRACE("AXS5106_reset data:");

    AXS5106_RESET_PORTCLR |= BIT(AXS5106_RESET);
    delay_us(50);
    tp_i2c_write(0xff, &cmd, 1);
    delay_ms(15);
    AXS5106_RESET_PORTSET |= BIT(AXS5106_RESET);
}


void tp_sleep_en(void)
{
    u8 cmd = 0x03;
    tp_i2c_write(0x19, &cmd, 1);
}
#ifdef AXS_UPGRADE_ENABLE
AT(.text.sensor.AXS5106)
static uint8_t axs_upgrade_enter(void)
{
	u8 i=0;
#if AXS_UPGRADE_CHECK_VERSION
	AXS5106_reset();
	//delay > 5ms
	delay_ms(10);
    if (!axs_fwupg_need_upgrade())
    {
        TRACE("o not need upgrade:\n");
        return -1; /*do not need upgrade*/
    }
#endif

    for (i=0; i<(AXS_UPGRADE_FAILED_RETRY_TIMES+1); i++)
    {
        TRACE("axs_upgrade_enter:\n");
        if (axs_upgrade_process())
        {
            break; // success
        }
    }

    if (i==(AXS_UPGRADE_FAILED_RETRY_TIMES+1))
    {
        return -2;/*upgrade failed*/
    }
    return 0;/*upgrade success*/
}
#endif

AT(.text.sensor.AXS5106)
int axs_read_fw_version(uint16_t *ver)
{

    uint8_t firmware_ver[2];
    delay_ms(50);
    tp_i2c_read(0x05, firmware_ver, 2);
    *ver  = ((firmware_ver[0]<<8)|firmware_ver[1]);

    TRACE("axs_read_fw_version: %x %x \n", firmware_ver[0], firmware_ver[1]);
    return *ver;
}

AT(.text.sensor.AXS5106)
int axs_read_fw_chipid(uint16_t *chipid)
{

    uint8_t firmware_chipid[3];
    tp_i2c_read(0x08, firmware_chipid, 3);
	*chipid  = ((firmware_chipid[1]<<8)|firmware_chipid[2]);

    return *chipid;
}

AT(.text.sensor.AXS5106)
int axs_read_fw_pj_id(uint16_t *pj_id)
{

    uint8_t firmware_pj_id[2];
    tp_i2c_read(0x0A, firmware_pj_id, 2);
	*pj_id  = ((firmware_pj_id[0]<<8)|firmware_pj_id[1]);

    return *pj_id;
}



AT(.text.sensor.AXS5106)
bool  AXS5106_init(void)
{
    uint16_t TP_verison = 0;
    uint16_t chip_ID = 0;

    AXS5106_io_init();

    axs_read_fw_version(&TP_verison);

    delay_5ms(2);
    AXS5106_reset();
    delay_5ms(10);


    // uint8_t cmd[1];
    // cmd[0] = 0x60;
    // tp_i2c_write(0xFA, cmd,1);        //报点模式
    // cmd[0] = 0x00;
    // tp_i2c_write(0xFC, cmd,1);        //关闭手势自动复位
    // tp_i2c_write(0xFD, cmd,1);        //关闭长按自动复位


    //tp_i2c_read(0xFA, &tmp,1);
    //TRACE("read 0xFA :%x\n",tmp);
    axs_read_fw_version(&TP_verison);
    axs_read_fw_chipid(&chip_ID);
    TRACE("TP_verison:%x ,chip_ID:%x \n", TP_verison, chip_ID);

    u8 info[4];

    delay_ms(200);
    ctp_iic_read(info, 0x08, 1);
    TRACE("info: %x :%02x %02x %02x \n", info[0], info[1], info[2], info[3]);
#ifdef AXS_UPGRADE_ENABLE
    axs_upgrade_enter();
#endif
    if (TP_verison == 0 || chip_ID != 0x0601) {
        TRACE("Touch error! TP_verison = %x, true is 1, chip_ID = %x, true is b6\r\n", TP_verison, chip_ID);
        return true;
    } else {
        return true;
    }

}



AT(.com_text.ctp)
//void AXS5106_get_touch_point(void)
void AXS5106_read_readkick(void)
{
    tp_i2c_read_kick(0x01, tp_data, TOUCH_DATA_LEN);
}

// AT(.com_rodata.tp)
// const char err_str[] = "----> return:%x, %x\n";
// AT(.com_rodata.tp)
// const char test_str[] = "----> AXS5106d:%x, %x\n";

AT(.com_text.ctp)
bool AXS5106_get_touch_point(s32 *x, s32 *y)
{
    uint8_t touch_num = tp_data[1];
    u8 touch_type = tp_data[2] >> 4;

    *x = ((u16)(tp_data[2] & 0x0f) << 8) + (u16)tp_data[3];
    *y = ((u16)(tp_data[4] & 0x0f) << 8) + (u16)tp_data[5];
#if CTP_SUPPORT_COVER
    if ( tp_data[0] == 0x80) {
        *x = -1;
        *y = -1;
        return false;
    }
#endif
    if (touch_type == 0x00 && touch_num) {       //第一次按下
        return true;
    } else if (touch_type == 0x04 && !touch_num) { //抬起
        return false;
    } else if (touch_type == 0x08 && touch_num) { //按住
        return true;
    }
    return true;


}

#ifdef AXS_UPGRADE_ENABLE
AT(.text.sensor.AXS5106)
static void tp_i2c_write_regs(const uint8_t *subAddr_data, u16 reg_len, const uint8_t *write_data, u16 len)
{
    TRACE("%s \n", __func__);

    u32 ticks;
    u16 transfer_len = 0;
    u32 regen_addr1 = 0;
    u32 regen_addr2 = 0;

    u32 iicxcon0 = AX5106_IICx->sfr->IICxCON0;
    u32 iicxcon1 = AX5106_IICx->sfr->IICxCON1;

    //50KHZ 低速配置IIC模块
    AX5106_IICx->sfr->IICxCON0 = BIT(10) | (63 << 4) | BIT(0) | (0 << 2);         //WSCL_OPT, POSDIV, IIC EN

    regen_addr1 = I2C_WRITE_ADDR(I2C_ADDR_AXS5106);
    regen_addr2 = subAddr_data[0];

    memcpy(tp_ota_buf, &subAddr_data[1], reg_len -1);

    memcpy(&tp_ota_buf[reg_len -1], write_data, len);
    transfer_len = len + reg_len -1 ;
    TRACE("transfer_len: %d \n",  transfer_len);
    AX5106_IICx->sfr->IICxCMDA =   (regen_addr2 << 8) | regen_addr1;       //0x2A

    AX5106_IICx->sfr->IICxCON1 = BIT(12) | BIT(11)  | BIT(10) | BIT(5) | BIT(4) | BIT(3) ;
    AX5106_IICx->sfr->IICxDMAADR = DMA_ADR(tp_ota_buf);
    AX5106_IICx->sfr->IICxDMACNT = ((transfer_len-1) << 16) | BIT(1) |BIT(0);
    AX5106_IICx->sfr->IICxCON0 |= BIT(28);                //KICK
    delay_ms(2);

    ticks = tick_get();
    while (!(AX5106_IICx->sfr->IICxCON0 & BIT(31))) {
        WDT_CLR();
        if (tick_check_expire(ticks, 10000)) {
            TRACE("tp_i2c_write_regs ERROR timerout \n");
            return ;
        }
    }
    AX5106_IICx->sfr->IICxCON0 |= BIT(29);                //Clear Pending

    AX5106_IICx->sfr->IICxCON0 = iicxcon0;
    AX5106_IICx->sfr->IICxCON1 = iicxcon1;

}

AT(.text.sensor.AXS5106)
static bool axs_fwupg_get_ver_in_bin(uint16_t *ver)
{
    uint16_t ver_offset = 0x400;
    if (!ver)
    {
        TRACE("axs_data/upgrade/func/fw/ver is NULL");
        return FALSE;
    }

    if (sizeof(fw_buf) < ver_offset)
    {
        TRACE("fw len(0x%0x) < fw ver offset(0x%x)",
                  sizeof(fw_buf), ver_offset);
        return FALSE;
    }
    *ver = ((fw_buf[ver_offset]<<8)|(fw_buf[ver_offset+1]));
    TRACE("current version :%x \n", *ver);
    return TRUE;

}

AT(.text.sensor.AXS5106)
static bool axs_fwupg_get_pj_id_in_bin(uint16_t *ver)
{
    uint16_t ver_offset = 0x402;

    if (!ver)
    {
        TRACE("axs_data/upgrade/func/fw/ver is NULL");
        return FALSE;
    }

    if (sizeof(fw_buf) < ver_offset)
    {
        TRACE("fw len(0x%0x) < fw ver offset(0x%x)",
                  sizeof(fw_buf), ver_offset);
        return FALSE;
    }
    *ver = ((fw_buf[ver_offset]<<8)|(fw_buf[ver_offset+1]));

    return TRUE;
}

/******************************************************************************
 *@Description :  ɽ¼¶׼±¸£¬½øȫdebugģʽ
 *@Function    :  axs_fwupg_enter_debug_mode
 *@Note        :
*******************************************************************************/
AT(.text.sensor.AXS5106)
static void axs_fwupg_enter_debug_mode(void)
{
    uint8_t debug_mode_cmd[1] = {0x55};
    AXS5106_reset();
    //make sure: 500us < delay time < 4ms
    delay_ms(1);
    tp_i2c_write(0xaa,debug_mode_cmd,1);
    //delay time >= 50us
    delay_us(100);
}

AT(.text.sensor.AXS5106)
static void axs_fwupg_exit_debug_mode(void)
{
    uint8_t debug_mode_cmd[1] = {0x5f};
    tp_i2c_write(0xa0,debug_mode_cmd,1);
}
/******************************************************************************
 *@Description :  ɽ¼¶׼±¸£¬½⋸±£»¤
 *@Function    :  axs_fwupg_unlock_mtpc_wp
 *@Note        :
*******************************************************************************/
AT(.text.sensor.AXS5106)
static bool axs_fwupg_unlock_mtpc_wp(void)
{
    uint8_t unlock_mtpc_cmd[3] = {0x6f,0xff,0xff};
    tp_i2c_write(0x90,unlock_mtpc_cmd,3);
    unlock_mtpc_cmd[1]=0xda;
    unlock_mtpc_cmd[2]=0x18;
    tp_i2c_write(0x90,unlock_mtpc_cmd,3);
    return TRUE;
}
/******************************************************************************
 *@Description :  ɽ¼¶׼±¸£¬ǥ³ýflashĚȝ
 *@Function    :  axs_fwupg_flash_erase
 *@Note        :
*******************************************************************************/
AT(.text.sensor.AXS5106)
static bool axs_fwupg_flash_erase(void)
{
    uint8_t erase_cmd[3] = {0x6f,0xd6,0x77};
    tp_i2c_write(0x90,erase_cmd,3);
    //delay time >= 240ms
    delay_ms(260);
    erase_cmd[2]=0x00;
    tp_i2c_write(0x90,erase_cmd,3);
    return TRUE;
}

/******************************************************************************
 *@Description :  ɽ¼¶¶¯׷£¬½«¹̼þдȫflash
 *@Function    :  axs_fwupg_flash_write
 *@Note        :
*******************************************************************************/
AT(.text.sensor.AXS5106)
static bool axs_fwupg_flash_write(const uint8_t *fw_buf, uint16_t fw_len)
{
    uint8_t write_flash_cmd[3] = {0x6f,0xd4,0x00};
#if AXS_UPGRADE_FAST_MODE
	uint8_t write_flash_long_reg[6] = {0x70,0x8f,0xd7,0x00,0x00,0x00};
#else
	uint16_t i;
#endif
    tp_i2c_write(0x90,write_flash_cmd,3);
    write_flash_cmd[1]=0xd5;
    tp_i2c_write(0x90,write_flash_cmd,3);
    write_flash_cmd[1]=0xd2;
    write_flash_cmd[2]=((fw_len-1)&0xff);
    tp_i2c_write(0x90,write_flash_cmd,3);
    write_flash_cmd[1]=0xd3;
    write_flash_cmd[2]=(((fw_len-1)>>8)&0xff);
    tp_i2c_write(0x90,write_flash_cmd,3);
    write_flash_cmd[1]=0xd6;
    write_flash_cmd[2]=0xf4;
    tp_i2c_write(0x90,write_flash_cmd,3);
#if AXS_UPGRADE_FAST_MODE
	write_flash_long_reg[4]=((fw_len-1)>>8)&0xff;
	write_flash_long_reg[5]=(fw_len-1)&0xff;
    // 第一次写的有c6 这个命令，后续的 fw_buf 没有c6 这个命令
    TRACE("%s AXS_UPGRADE_FAST_MODE \n");
	tp_i2c_write_regs(write_flash_long_reg,6,fw_buf,fw_len);
#else
    write_flash_cmd[1]=0xd7;
    for (i = 0; i < (fw_len); i++)
    {
        write_flash_cmd[2]=fw_buf[i];
        tp_i2c_write(0x90,write_flash_cmd,3);
    }
#endif
    write_flash_cmd[1]=0xd6;
    write_flash_cmd[2]=0x00;
    tp_i2c_write(0x90,write_flash_cmd,3);
    return TRUE;
}

/******************************************************************************
 *@Description :  ɽ¼¶º¯ʽ
 *@Function    :  axs_fwupg_write_flash
 *@Note        :
*******************************************************************************/
AT(.text.sensor.AXS5106)
static bool axs_fwupg_write_flash(void)
{
    axs_fwupg_enter_debug_mode();

// sfr
    if (!axs_fwupg_unlock_mtpc_wp())
    {
        TRACE("axs_fwupg_flash_init failed");
        return FALSE;
    }
    if (!axs_fwupg_flash_erase())
    {
        TRACE("axs_fwupg_flash_erase failed");
        return FALSE;
    }

    // write firmware
    if (!axs_fwupg_flash_write(fw_buf, sizeof(fw_buf)))
    {
        TRACE("axs_fwupg_flash_write failed");
        return FALSE;
    }
	axs_fwupg_exit_debug_mode();

    delay_ms(1);
    return TRUE;
}

/******************************************************************************
 *@Description :  ɽ¼¶¼첩
 *@Function    :  axs_fwupg_need_upgrade
 *@Note        :
*******************************************************************************/
#ifdef AXS_UPGRADE_CHECK_VERSION

AT(.text.sensor.AXS5106)
static bool axs_fwupg_need_upgrade(void)
{
    uint16_t fw_ver_in_bin = 0;
    uint16_t fw_ver_in_tp = 0;
	uint16_t fw_pj_id_in_bin = 0;
    uint16_t fw_pj_id_in_tp = 0;

    if (!axs_fwupg_get_ver_in_bin(&fw_ver_in_bin))
    {
        return FALSE;
    }

    if (!axs_read_fw_version(&fw_ver_in_tp))
    {
        return FALSE; /*get firmware ver failed, do not upgrade;*/
    }
	if (!axs_fwupg_get_pj_id_in_bin(&fw_pj_id_in_bin))
    {
        return FALSE;
    }

    if (!axs_read_fw_pj_id(&fw_pj_id_in_tp))
    {
        return FALSE; /*get firmware ver failed, do not upgrade;*/
    }


    TRACE("fw version in tp:%x, host:%x \n", fw_ver_in_tp, fw_ver_in_bin);
    TRACE("fw project id in tp:%x, host:%x \n", fw_pj_id_in_tp, fw_pj_id_in_bin);
    if ((fw_ver_in_tp != fw_ver_in_bin) || (fw_pj_id_in_tp != fw_pj_id_in_bin))
    {
        return TRUE;
        //return false;
    }
    else
    {
        TRACE("fw version is latest! \n");
    }

    return FALSE;
}
#endif
/******************************************************************************
 *@Description :  ͪջɽ¼¶Á÷³̍
 *@Function    :  axs_upgrade_process
 *@Note        :
*******************************************************************************/
AT(.text.sensor.AXS5106)
bool axs_upgrade_process(void)
{
    if (!axs_fwupg_write_flash())
    {
        TRACE("writing firmware Failed\r\n");
        return FALSE;
    }
    return TRUE;
}

#endif


#endif //TP_AXS5106
