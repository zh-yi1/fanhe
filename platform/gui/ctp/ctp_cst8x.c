#include "include.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

//CTP升级相关
#define CST8X_UPDATE_EN			 0		            //打开TP升级功能

#define PER_LEN                  512
#define TANS_LIMT                (8L)               //To be divisible by 512  eg:4 8 16 32 64 ....
#define MAX_ADDR                 (0x3C00L)
//CST816只支持一个TOUCH NUM
#define CTP_READ_ADDR            0x03               //P1_XH, P1_XL, P1_YH, P1_YL
#define CTP_READ_SIZE            4

static u8 ctp_cst8x_buf[CTP_READ_SIZE];

#if CST8X_UPDATE_EN
bool ctp_iic_update_read(u8 dev_addr, void *rbuf, int rlen, u16 w_addr, u8 *w_cmd, int wlen);
bool ctp_iic_update_write(u8 dev_addr, u16 addr, u8 *cmd, int len);
void ctp_reset(void);

bool cst8x_enter_bootmode(void)
{
//    printf("%s\n", __func__);
    uint8_t retry_cnt = 50;

	//进入Boot要拉复位，拉低拉高后延时10ms，否则无法进入Boot
    delay_5ms(2);
    PORT_CTP_RST_L();
    delay_5ms(2);
    PORT_CTP_RST_H();
    delay_5ms(2);

    while (retry_cnt--) {
        u8 cmd[1];
        u8 res = 0;
        cmd[0] = 0xAB;
        ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA001, cmd, 2);
        cmd[0] = 0;
        ctp_iic_update_read(TP_IIC_UPDATE_ADDR, &res, 1, 0xA003, cmd, 1);

        // if (cmd[0] != 0x55) { // CST716, CST816S, CST826, CST830, CST836U
        if (res != 0xC1) { // CST816D, CST816T
            delay_ms(2);
            continue;
        } else {
            return true;
        }
        delay_ms(2);
    }
    printf("%s fail!\n", __func__);
    return false;
}

static uint16_t cst8x_read_checksum(void)
{
    union {
        uint32_t sum;
        uint8_t buf[4];
    } checksum;
    uint8_t cmd[3] = {0};
//    uint8_t readback[4] = {0};

    if (cst8x_enter_bootmode() == false) {
        return false;
    }

    cmd[0] = 0;
    ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA003, cmd, 2);
    delay_ms(500);

    checksum.sum = 0;
    ctp_iic_update_read(TP_IIC_UPDATE_ADDR, &checksum.buf, 4, 0xA008, 0, 1);

    return (uint16_t)checksum.sum;
}

static int ctp_cst8x_update_do(uint16_t start_addr, uint16_t len, u8 *src)
{
    uint16_t temp_addr;
    uint16_t total_len = len;
    uint8_t  cmd[2 + TANS_LIMT];
//    uint8_t  index;
    int ret = 0;

    if (cst8x_enter_bootmode() == false) {
        ret =  -1;
        goto update_fail;
    }
    printf("start updata !!\n");
    do {
        if (start_addr > (MAX_ADDR - PER_LEN)) { //protect bootloade
            ret =  -2;
            goto update_fail;
        }
        // send address
        cmd[0] = start_addr & 0xFF;
        cmd[1] = start_addr >> 8;
//        printf("---> start_addr:%lx\n", start_addr);
        ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA014, cmd, 3);
        delay_ms(2);

//////////////////write 512 byte to ram
        temp_addr = 0xA018;
        do {
            ctp_iic_update_write(TP_IIC_UPDATE_ADDR, temp_addr, src, TANS_LIMT + 1);
            temp_addr += TANS_LIMT;
            src += TANS_LIMT;
            delay_ms(2);
        } while (temp_addr < 0xA214);
///////////////////

        cmd[0] = 0xEE;
        ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA004, cmd, 2);
        delay_5ms(40);

        {
            uint8_t retrycnt = 50;
            while (--retrycnt) {
                cmd[0] = 0;
                ctp_iic_update_read(TP_IIC_UPDATE_ADDR, &cmd, 1, 0xA005, 0, 1);
                if (cmd[0] == 0x55) {
                    // success
                    break;
                }
                delay_ms(10);
            }
        }

        start_addr += PER_LEN;
        len = len > PER_LEN ? (len - PER_LEN) : 0;
		//预留升级百分比方便UI做显示
        printf("%d%%\n", (total_len - len) *100 / total_len);
    } while (len);
    ret  = 0;

update_fail:
    cmd[0] = 0x00;
    ctp_iic_update_write(TP_IIC_UPDATE_ADDR, 0xA003, cmd, 2); // exit program mode

//    printf("exit boot mode !!\n");
    return ret;
}

bool ctp_cst8x_update(void)
{
    u8 *update_ptr = (u8 *)(RES_BUF_CTP_CTP_UPDATE_BIN);
    u32 update_len = RES_LEN_CTP_CTP_UPDATE_BIN;

//    printf("update_len:%x\n", update_len);

    if (cst8x_enter_bootmode() == true) {
        if (update_len > 10) {
            uint16_t start_addr = (update_ptr[1] << 8) | update_ptr[0];
            uint16_t length = (update_ptr[3] << 8) | update_ptr[2];
            uint16_t checksum = (update_ptr[5] << 8) | update_ptr[4];
            uint16_t checksum_old = cst8x_read_checksum();
            //enter bootmode fail while checking sum
            if (checksum_old == 0) {
                printf("update cecksum enter bootmode err:0\n");
            }

            if (checksum_old == checksum) {
                printf("CTP fw check sum:0x%04x, it's already the latest version!\n");
            } else{
                printf("CTP check sum old:0x%x new:checksum=0x%x\n",checksum_old, checksum);
                ctp_cst8x_update_do(start_addr, length, &update_ptr[6]);
                checksum_old = cst8x_read_checksum();
                if (checksum_old == checksum) {
                    printf("update cecksum sucess!\n");
                } else {
                    printf("update cecksum error:0x%04x, correct:0x%04x\n", checksum_old, checksum);
                }
            }
        }
        return true;
    }
    return false;
}

extern i2c_t *CTP_IIC;

void ctp_cst8x_update_check(void)
{
    u32 iic1con0 = CTP_IIC->sfr->IICxCON0;
    u32 iic1con1 = CTP_IIC->sfr->IICxCON1;

    //50KHZ 低速配置IIC模块
    //IIC1CON0 = BIT(10) | (63 << 4) | BIT(0) | (0 << 2);         //WSCL_OPT, POSDIV, IIC EN

    CTP_IIC->sfr->IICxCON0 = BIT(10) | (63 << 4) | BIT(0) | (0 << 2);         //WSCL_OPT, POSDIV, IIC EN

    bool ret = ctp_cst8x_update();
    if (ret == false) {
        printf("CTP update failed!\n");
    }

    CTP_IIC->sfr->IICxCON0 = iic1con0;
    CTP_IIC->sfr->IICxCON1 = iic1con1;

    ctp_reset();
}

#endif

bool ctp_cst8x_init(void)
{
#if CST8X_UPDATE_EN
    //Only check once after power on
    static bool check_flag = false;
    if (!check_flag) {
        ctp_cst8x_update_check();
        check_flag = true;
    }
#endif

    u8 info[4];

//    //buf mode
//	uint32_t i2c_cfg = START_FLAG0 | DEV_ADDR0 | REG_ADDR_0 | START_FLAG1 | DEV_ADDR1;
//#define I2C_WRITE_ADDR(ADDR)     		 ((ADDR) << 1)				//CTP IIC写地址
//#define I2C_READ_ADDR(ADDR)      		 ((ADDR) << 1 | 1)			//CTP IIC读地址
//	bsp_hw_i2c_rx_buf(i2c_cfg, I2C_WRITE_ADDR(TP_IIC_ADDR) | (I2C_READ_ADDR(TP_IIC_ADDR) << 8), 0xA7, info, 4);
//	print_r(info, 4);

    if (!ctp_iic_read(info, 0xA7, 4)) {
        return false;
    }
    TRACE("CTP ID: %02x, VER: %02x\n", info[0], info[2]);
    if ((info[0] != 0xB5 && info[0] != 0xB6 && info[0] != 0xB7 && info[0] != 0xB9) || info[2] == 0) {     //0xB5:cst816T, 0xB6:cst816, 0xB7:cst820 0xB9:cst08C
        return false;
    }
    return true;
}

AT(.com_text.ctp)
void ctp_cst8x_readkick(void)
{
    ctp_iic_readkick(ctp_cst8x_buf, CTP_READ_ADDR, CTP_READ_SIZE);
//    print_r(ctp_cst8x_buf, CTP_READ_SIZE);
}

AT(.com_text.ctp)
bool ctp_cst8x_get_point(s32 *x, s32 *y)
{
    *x = ((ctp_cst8x_buf[0] & 0xf) << 8) + ctp_cst8x_buf[1];
    *y = ((ctp_cst8x_buf[2] & 0xf) << 8) + ctp_cst8x_buf[3];
    return ((ctp_cst8x_buf[0] & 0xC0) == 0x80);
}

