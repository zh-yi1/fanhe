#include "include.h"
#include "bsp_i2c.h"
#include "axs152x.h"

#if (CTP_SELECT == CTP_AXS152x)

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if 1
    static i2c_t __AX152X_IIC0 = {
        .sfr         = (i2c_sfr_t *) &IIC0CON0,
        .map         = (i2c_map_t *) &FUNCMCON2,
    };

    i2c_t *AX152X_IICx = &__AX152X_IIC0;
#elif 0
    static i2c_t __AX152X_IIC1 = {
        .sfr         = (i2c_sfr_t *) &IIC1CON0,
        .map         = (i2c_map_t *) &FUNCMCON2,
    };

    i2c_t *AX152X_IICx = &__AX152X_IIC1;
#else
	#error CHIP_PACKAGE_SELECT select error!!!
#endif

static u8 touch_buff[9];
static u8 axs5106_read_flag = 0;

AT(.com_text.ctp)
static void tp_i2c_read_kick(uint8_t *read_data, u16 len)
{
    AX152X_IICx->sfr->IICxCMDA =  I2C_AXS152X_READ_ADDR(I2C_ADDR_AXS152X)<<24;
    AX152X_IICx->sfr->IICxCON1 = BIT(12) | BIT(11) | BIT(9) | BIT(8) | BIT(7);
    AX152X_IICx->sfr->IICxDMAADR = DMA_ADR(read_data);
    AX152X_IICx->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(0);
    AX152X_IICx->sfr->IICxCON0 |= BIT(28);                //KICK
}

AT(.com_text.ctp)
static void tp_i2c_write_kick(uint8_t *write_data, u16 len)
{
    AX152X_IICx->sfr->IICxCMDA =  (write_data[0]<<8) | I2C_AXS152X_WRITE_ADDR(I2C_ADDR_AXS152X);
    AX152X_IICx->sfr->IICxCON1 = BIT(12) | BIT(11)  | BIT(10) | BIT(5) | BIT(4) | BIT(3);
    AX152X_IICx->sfr->IICxDMAADR = DMA_ADR(&write_data[1]);
    AX152X_IICx->sfr->IICxDMACNT = ((len - 2) << 16) | BIT(1) |BIT(0);
    AX152X_IICx->sfr->IICxCON0 |= BIT(28);                //KICK
}


/**
  Write
*/
static bool tp_i2c_write_buff(const uint8_t *write_data, u16 len)
{

    u32 ticks;
    if(len < 2){
        return false;
    }

    AX152X_IICx->sfr->IICxCMDA =  (write_data[0]<<8) | I2C_AXS152X_WRITE_ADDR(I2C_ADDR_AXS152X);

    AX152X_IICx->sfr->IICxCON1 = BIT(12) | BIT(11)  | BIT(10) | BIT(5) | BIT(4) | BIT(3);
    AX152X_IICx->sfr->IICxDMAADR = DMA_ADR(&write_data[1]);
    AX152X_IICx->sfr->IICxDMACNT = ((len - 2) << 16) | BIT(1) |BIT(0);
    AX152X_IICx->sfr->IICxCON0 |= BIT(28);                //KICK
//    printf("IICxCMDA: 0x%x, IICxDMAADR: 0x%x, IICxDMACNT: 0x%x, IICxCON0: 0x%x, IICxCON1: 0x%x\n",
//           HS_IIC->sfr->IICxCMDA, HS_IIC->sfr->IICxDMAADR, HS_IIC->sfr->IICxDMACNT, HS_IIC->sfr->IICxCON0, HS_IIC->sfr->IICxCON1);
    ticks = tick_get();
    while (!(AX152X_IICx->sfr->IICxCON0 & BIT(31))) {
        if (tick_check_expire(ticks, 1000)) {
            printf("i2c_write time out ERROR\n");
            return false;
        }
    }
    AX152X_IICx->sfr->IICxCON0 |= BIT(29);                //Clear Pending


    return true;
}

/**
  Read
*/
static bool tp_i2c_read_buff(uint8_t *read_data, u16 len)
{
    u32 ticks;
    if(len < 1){
        return false;
    }
    AX152X_IICx->sfr->IICxCMDA =  I2C_AXS152X_READ_ADDR(I2C_ADDR_AXS152X)<<24;
    AX152X_IICx->sfr->IICxCON1 = BIT(12) | BIT(11) | BIT(9) | BIT(8) | BIT(7);
    AX152X_IICx->sfr->IICxDMAADR = DMA_ADR(read_data);
    AX152X_IICx->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(0);
    AX152X_IICx->sfr->IICxCON0 |= BIT(28);                //KICK

    ticks = tick_get();
    while (!(AX152X_IICx->sfr->IICxCON0 & BIT(31))) {
        if (tick_check_expire(ticks, 2000)) {
            printf("alipay_i2c_read time out ERROR\n");
            return false;
        }
    }
    AX152X_IICx->sfr->IICxCON0 |= BIT(29);                //Clear Pending

    return true;
}


/**
 * write_bytes_read_bytes
 */
bool axs_write_bytes_read_bytes(u8 *cmd, u16 cmdlen, u8 *data, u16 datalen)
{
    bool ret;
    if (!cmd || !data
        || (cmdlen >= AXS_CMD_MAX_WRITE_BUF_LEN) || (datalen >= AXS_CMD_MAX_READ_BUF_LEN))
    {
        return false;
    }

    if (cmd && (cmdlen > 0))
    {
        ret = tp_i2c_write_buff(cmd, cmdlen);
        if (ret == false)
        {
            return false;
        }
    }

    delay_us(50);
    ret = tp_i2c_read_buff(data, datalen);
    return ret;
}


/**
 * axs_read_bytes
 */
bool axs_read_bytes(u8 *rdbuf, u16 rdlen)
{
    bool ret;

    if ((rdbuf == NULL) || (rdlen >= AXS_CMD_MAX_READ_BUF_LEN))
    {
        return false;
    }
    ret = tp_i2c_read_buff(rdbuf, rdlen);

    return ret;
}

/**
 * axs_write_bytes
 */
int axs_write_bytes(u8 *writebuf, u16 writelen)
{
    bool ret;

    if ((writebuf == NULL) || (writelen >= AXS_CMD_MAX_WRITE_BUF_LEN))
    {
        return false;
    }

    ret = tp_i2c_write_buff(writebuf, writelen);
    return ret;
}

/**
 * regs
*/
static u8 write_cmd[22] = {0xb5,0xab,0x5a,0xa5,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0xff,0x00};  // max write len: 22-12=10
static u8 read_cmd[13]  = {0xb5,0xab,0xa5,0x5a,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00};
static u8 read_sfr_cmd[11] = {0x5a,0xa5,0xab,0xb5,0x00,0x00,0x00,0x01,0x00,0x80,0x00};


int axs_read_regs(u8 *reg,u16 reg_len,u8 *rd_data,u16 rd_len)
{
    int ret = 0;
    if (reg_len>10)
    {
        reg_len = 10;
    }
    write_cmd[4] = (u8)((reg_len+1)>>8);
    write_cmd[5] = (u8)((reg_len+1)&0xff);
    memcpy(&write_cmd[11],reg,reg_len);
    write_cmd[11+reg_len] = 0;
    ret = axs_write_bytes(write_cmd,12+reg_len);
    if (ret == false)
    {
        TRACE("axs_write_bytes fail");
        return ret;
    }
    if (rd_len > 0)
    {
        read_cmd[6] = (u8)(rd_len>>8);
        read_cmd[7] = (u8)(rd_len&0xff);
        ret = axs_write_bytes_read_bytes(read_cmd,sizeof(read_cmd), rd_data, rd_len);
        if (ret == false)
        {
            TRACE("axs_write_bytes_read_bytes fail");
            return ret;
        }
    }
    return 0;
}

int axs_write_buf(u8 *wt_buf,u16 wt_len)
{
    int ret = 0;
    if (wt_len>10)
    {
        wt_len = 10;
    }
    write_cmd[4] = (u8)((wt_len+1)>>8);
    write_cmd[5] = (u8)((wt_len+1)&0xff);
    memcpy(&write_cmd[11],wt_buf,wt_len);
    write_cmd[11+wt_len] = 0;
    ret = axs_write_bytes(write_cmd,12+wt_len);
    if (ret == false)
    {
        TRACE("axs_write_bytes fail");
        return ret;
    }
    return 0;
}

int axs_read_buf(u8 *rd_data,u16 rd_len)
{
    int ret = 0;
    if (rd_len > 0)
    {
        read_cmd[6] = (u8)(rd_len>>8);
        read_cmd[7] = (u8)(rd_len&0xff);
        ret = axs_write_bytes_read_bytes(read_cmd,sizeof(read_cmd), rd_data, rd_len);
        if (ret == false)
        {
            TRACE("axs_write_bytes_read_bytes fail");
            return ret;
        }
    }
    return 0;

}

int axs_read_sfr_reg(u8 sfr_reg,u8 *rd_data,u16 rd_len)
{
    int ret = 0;
    if (rd_len > 0)
    {
        read_sfr_cmd[6] = (u8)(rd_len>>8);
        read_sfr_cmd[7] = (u8)(rd_len&0xff);
        read_sfr_cmd[10] = sfr_reg;
        ret = axs_write_bytes_read_bytes(read_sfr_cmd,sizeof(read_sfr_cmd), rd_data, rd_len);
        if (ret == false)
        {
            TRACE("axs_read_esd_reg fail");
            return ret;
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

int axs_read_fw_version(u8 *ver)
{
    u8 sfr_value;
    u8 sfr_value2;
    //read firmware version from sfr reg
    if (axs_read_sfr_reg(0x89,&sfr_value,1))
    {
        return -1;
    }
    delay_ms(1);
    if (axs_read_sfr_reg(0x89,&sfr_value2,1))
    {
        return -1;
    }
    TRACE("axs_read_fw_version:0x%x 0x%x\n", sfr_value, sfr_value2);
    if (sfr_value != sfr_value2)
    {
        return  -1;
    }
    *ver = sfr_value2;
    return 0;
}


bool AXS152x_init(void)
{
    u8 TP_verison = 0;
    printf("AXS5106_init\n");

    axs_read_fw_version(&TP_verison);
    TRACE("TP_verison:0x%x\n", TP_verison);
    axs5106_read_flag = 0;

    return true;
}


// AT(.com_rodata.tp)
// const char err_str[] = "----> return:%x, %x\n";
// AT(.com_rodata.tp)
// const char test_str[] = "----> AXS5106d:%x, %x\n";

AT(.com_text.ctp)
u8 AXS152x_get_touch_point(s32 *x, s32 *y)
{
    if(axs5106_read_flag == 1){
        //delay_us(50);
        tp_i2c_read_kick(touch_buff, 8);
        axs5106_read_flag = 2;
        return 0x5A;
    }else if(axs5106_read_flag == 2){
        uint8_t touch_num = AXS_GET_POINT_NUM(touch_buff);
        u8 touch_type = AXS_GET_POINT_EVENT(touch_buff);
        axs5106_read_flag = 0;

        *x = AXS_GET_POINT_X(touch_buff);
        *y = AXS_GET_POINT_Y(touch_buff);

        if (touch_type == 0x00 && touch_num) {       //第一次按下
            return true;
        } else if (touch_type == 0x01 && !touch_num) { //抬起
            return false;
        } else if (touch_type == 0x02 && touch_num) { //按住
            return true;
        }
    }
    return false;
}


AT(.com_text.ctp)
void AXS152x_read_readkick(void)
{
    static u32 ticks = 0;

    if(tick_check_expire(ticks, 50)){
        axs5106_read_flag = 0;
    }

    ticks = tick_get();
    if(axs5106_read_flag == 0){
        read_cmd[6] = 0;
        read_cmd[7] = 0x08;
        tp_i2c_write_kick(read_cmd, 8);
        axs5106_read_flag = 1;
    }
}

#endif //TP_AXS5106
