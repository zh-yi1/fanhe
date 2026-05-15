#include "include.h"
#if CTP_SELECT != CTP_NO

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_K(...)            printk(__VA_ARGS__)
AT(.com_rodata)
const char str_xy[] = "%d: %d, %d\n";
#else
#define TRACE(...)
#define TRACE_K(...)
#endif

//TP相关配置
#define CTP_CNT_SHORT_MOV                (GUI_SCREEN_WIDTH / 30)    //CTP短划阈值(屏幕距离)
#define CTP_CNT_LONG_MOV                 (GUI_SCREEN_WIDTH / 3)     //CTP长划阈值(屏幕距离)
#define CTP_SHORT_DOWN_TOP_ZONE          (GUI_SCREEN_HEIGHT / 10)   //下短划仅当按下起点 Y 在此区域内（屏顶向下）
#define CTP_CNT_LONG                     40                         //CTP长按时间阈值(CTP INT时钟个数)
#define CTP_DOUBLE_CLICK_TIME            200                        //CTP双击响应时间（单位ms）
#define I2C_WRITE_ADDR(ADDR)     		 ((ADDR) << 1)				//CTP IIC写地址
#define I2C_READ_ADDR(ADDR)      		 ((ADDR) << 1 | 1)			//CTP IIC读地址

AT(.com_text.ctp_var)
bool ctp_kick_status;

enum {
    CTP_STA_IDLE,
    CTP_STA_PRESS,
    CTP_STA_PRESS_LONG,
    CTP_STA_MOV_SHORT,
    CTP_STA_MOV_LONG,
    CTP_STA_CLICK,
};


static struct ctp_cb_t {
    s32 x;          //当前坐标
    s32 y;
    s32 sx;         //起始坐标
    s32 sy;
    s32 last_dx;    //最后一次相对偏移
    s32 last_dy;
    u32 double_tick;
    u8 cnt;
    u8 sta;
    bool flag_last;
} ctp_cb;

i2c_t *CTP_IIC = NULL;

i2c_t *ctp_iic_reg_get(void)
{
    return CTP_IIC;
}

AT(.com_text.ctp)
void ctp_int_isr(void)
{
    if (WKUPEDG & BIT(16+PORT_CTP_INT_VECTOR)) {
        WKUPCPND = BIT(16+PORT_CTP_INT_VECTOR);
#if (CTP_SELECT == CTP_CST8X)
        ctp_cst8x_readkick();
#elif (CTP_SELECT == CTP_CHSC6X)
        ctp_chsc6x_readkick();
#elif (CTP_SELECT == CTP_AXS5106)
        AXS5106_read_readkick();
#elif (CTP_SELECT == CTP_AXS152x)
        AXS152x_read_readkick();
#endif
    }
}

//处理TP的消息
AT(.com_text.ctp)
void ctp_msg_deal(bool press)
{
    if (press) {
        reset_sleep_delay_all();

        int mx, my;
        mx = abs_s(ctp_cb.x - ctp_cb.sx);
        my = abs_s(ctp_cb.y - ctp_cb.sy);

        switch (ctp_cb.sta) {
        case CTP_STA_IDLE:
        case CTP_STA_CLICK:
            ctp_cb.sx = ctp_cb.x;
            ctp_cb.sy = ctp_cb.y;
            ctp_cb.cnt++;
            if (ctp_cb.sta == CTP_STA_CLICK) {
                if (tick_check_expire(ctp_cb.double_tick, CTP_DOUBLE_CLICK_TIME)) {
                    msg_enqueue(MSG_CTP_TOUCH);                      //触摸事件
                    ctp_cb.double_tick = 0;
                }
            } else {
                msg_enqueue(MSG_CTP_TOUCH);                          //触摸事件
            }
            ctp_cb.sta = CTP_STA_PRESS;                              //按下
            break;

        case CTP_STA_PRESS:
        case CTP_STA_PRESS_LONG:
        case CTP_STA_MOV_SHORT:
            if (ctp_cb.sta == CTP_STA_PRESS) {
                ctp_cb.cnt++;
                if (ctp_cb.cnt >= CTP_CNT_LONG) {
                    msg_enqueue(MSG_CTP_LONG);                       //长按
                    ctp_cb.sta = CTP_STA_PRESS_LONG;
                    return;
                }
            }
            if (mx >= my) {
                if (mx >= CTP_CNT_LONG_MOV) {
                    if (ctp_cb.x < ctp_cb.sx) {
                        if (ctp_cb.sta != CTP_STA_MOV_SHORT) {
                            msg_enqueue(MSG_CTP_SHORT_LEFT);        //补发左短划
                        }
                        msg_enqueue(MSG_CTP_LONG_LEFT);             //左长划
                        ctp_cb.sta = CTP_STA_MOV_LONG;
                    } else {
                        if (ctp_cb.sta != CTP_STA_MOV_SHORT) {
                            msg_enqueue(MSG_CTP_SHORT_RIGHT);       //补发右短划
                        }
                        msg_enqueue(MSG_CTP_LONG_RIGHT);            //右长划
                        ctp_cb.sta = CTP_STA_MOV_LONG;
                    }
                    return;
                }
                if (ctp_cb.sta != CTP_STA_MOV_SHORT && mx >= CTP_CNT_SHORT_MOV) {
                    if (ctp_cb.x < ctp_cb.sx) {
                        msg_enqueue(MSG_CTP_SHORT_LEFT);            //左短划
                        ctp_cb.sta = CTP_STA_MOV_SHORT;
                    } else {
                        msg_enqueue(MSG_CTP_SHORT_RIGHT);           //右短划
                        ctp_cb.sta = CTP_STA_MOV_SHORT;
                    }
                    return;
                }
            } else {
                if (my >= CTP_CNT_LONG_MOV) {
                    if (ctp_cb.y < ctp_cb.sy) {
                        if (ctp_cb.sta != CTP_STA_MOV_SHORT) {
                            msg_enqueue(MSG_CTP_SHORT_UP);          //补发上短划
                        }
                        msg_enqueue(MSG_CTP_LONG_UP);               //上长划
                        ctp_cb.sta = CTP_STA_MOV_LONG;
                    } else {
                        if (ctp_cb.sta != CTP_STA_MOV_SHORT) {
                            if (ctp_cb.sy < CTP_SHORT_DOWN_TOP_ZONE) {
                                msg_enqueue(MSG_CTP_SHORT_DOWN);    //补发下短划（仅屏顶起始）
                            }
                        }
                        msg_enqueue(MSG_CTP_LONG_DOWN);             //下长划
                        ctp_cb.sta = CTP_STA_MOV_LONG;
                    }
                    return;
                }
                if (ctp_cb.sta != CTP_STA_MOV_SHORT && my >= CTP_CNT_SHORT_MOV) {
                    if (ctp_cb.y < ctp_cb.sy) {
                        msg_enqueue(MSG_CTP_SHORT_UP);              //上短划
                        ctp_cb.sta = CTP_STA_MOV_SHORT;
                    } else {
                        if (ctp_cb.sy < CTP_SHORT_DOWN_TOP_ZONE) {
                            msg_enqueue(MSG_CTP_SHORT_DOWN);        //下短划（仅屏顶起始）
                        }
                        ctp_cb.sta = CTP_STA_MOV_SHORT;
                    }
                    return;
                }
            }
            break;

        case CTP_STA_MOV_LONG:
        default:
            break;
        }
    } else {
        if (ctp_cb.sta == CTP_STA_PRESS) {
#if CTP_DOUBLE_CLICK_EN
            if (ctp_cb.double_tick) {
                msg_enqueue(MSG_CTP_DOUBLE_CLICK);
                ctp_cb.double_tick = 0;
                ctp_cb.sta = CTP_STA_IDLE;
            } else {
                ctp_cb.double_tick = tick_get();
                ctp_cb.sta = CTP_STA_CLICK;
            }
#else
            msg_enqueue(MSG_CTP_CLICK);                             //单击
            ctp_cb.sta = CTP_STA_IDLE;
#endif
        } else if (ctp_cb.sta != CTP_STA_CLICK) {
            if (ctp_cb.sta == CTP_STA_PRESS_LONG) {
                msg_enqueue(MSG_CTP_LONG_LIFT);                     //长按抬起
            }
            ctp_cb.double_tick = 0;
            ctp_cb.sta = CTP_STA_IDLE;
        }
        ctp_cb.cnt = 0;
    }
}

#if CTP_DOUBLE_CLICK_EN
AT(.com_text.ctp)
void ctp_double_click_process()
{
    if (ctp_cb.double_tick && tick_check_expire(ctp_cb.double_tick, CTP_DOUBLE_CLICK_TIME)) {
        msg_enqueue(MSG_CTP_CLICK);                         //单击
        ctp_cb.double_tick = 0;
        ctp_cb.sta = CTP_STA_IDLE;
    }
}
#endif

bool ctp_get_kick_status(void)
{
    return ctp_kick_status;
}

void sys_ctp_irq_enble(int vector)
{
    PICEN |= BIT(vector);
}

void sys_ctp_irq_disable(int vector)
{
    PICEN &= ~BIT(vector);
}

AT(.com_text.ctp)
void ctp_isr(void)
{
    u8 press;
    if (CTP_IIC->sfr->IICxCON0 & BIT(31)) {
        CTP_IIC->sfr->IICxCON0 |= BIT(29);

        s32 last_x = ctp_cb.x;
        s32 last_y = ctp_cb.y;
#if (CTP_SELECT == CTP_CST8X)
        press = ctp_cst8x_get_point(&ctp_cb.x, &ctp_cb.y);
#elif (CTP_SELECT == CTP_CHSC6X)
        press = ctp_chsc6x_get_point(&ctp_cb.x, &ctp_cb.y);
#elif (CTP_SELECT == CTP_AXS5106)
       press = AXS5106_get_touch_point(&ctp_cb.x, &ctp_cb.y);
#elif (CTP_SELECT == CTP_AXS152x)
       press = AXS152x_get_touch_point(&ctp_cb.x, &ctp_cb.y);
       if(press == 0x5A){
            return;
       }
#endif
        if (press) {
            if (ctp_cb.flag_last) {
                ctp_cb.last_dx = ctp_cb.x - last_x;
                ctp_cb.last_dy = ctp_cb.y - last_y;
            } else {
                ctp_cb.flag_last = true;
                ctp_cb.last_dx = 0;
                ctp_cb.last_dy = 0;
            }
        } else {
            ctp_cb.flag_last = false;
        }
#if CTP_SUPPORT_COVER
        if ((ctp_cb.x == -1)) {
            msg_enqueue(MSG_CTP_COVER);                             //盖手
        }
#endif
        TRACE_K(str_xy, press, ctp_cb.x, ctp_cb.y);
        ctp_msg_deal(press);
        ctp_kick_status = false;
    }
}

//获取屏幕是否在触摸
bool ctp_is_touch(void)
{
    return (ctp_cb.sta != CTP_STA_IDLE);
}

//获取坐标，返回触屏点击状态
AT(.com_text.ctp)
bool ctp_get_cur_point(s32 *sx, s32 *sy, s32 *x, s32 *y)
{
    bool press;
    GLOBAL_INT_DISABLE();
    *sx = ctp_cb.sx;
    *sy = ctp_cb.sy;
    *x = ctp_cb.x;
    *y = ctp_cb.y;
    press = (ctp_cb.sta != CTP_STA_IDLE);
    GLOBAL_INT_RESTORE();
    return press;
}

//获取坐标差量，返回触屏点击状态
AT(.com_text.ctp)
bool ctp_get_dxy(s32 *dx, s32 *dy)
{
    bool press;
    GLOBAL_INT_DISABLE();
    press = (ctp_cb.sta != CTP_STA_IDLE);
    if (press) {
        *dx = ctp_cb.x - ctp_cb.sx;
        *dy = ctp_cb.y - ctp_cb.sy;
    }
    GLOBAL_INT_RESTORE();
    return press;
}

//获取起始X,Y坐标
AT(.com_text.ctp)
point_t ctp_get_sxy(void)
{
    point_t pt;
    GLOBAL_INT_DISABLE();
    pt.x = ctp_cb.sx;
    pt.y = ctp_cb.sy;
    GLOBAL_INT_RESTORE();
    return pt;
}

//获取最后一次偏移
AT(.com_text.ctp)
point_t ctp_get_last_dxy(void)
{
    point_t pt;
    GLOBAL_INT_DISABLE();
    pt.x = ctp_cb.last_dx;
    pt.y = ctp_cb.last_dy;
    GLOBAL_INT_RESTORE();
    return pt;
}

//获取点击XY的相对于控件的坐标
point_t ctp_get_rxy(widget_t *widget)
{
    point_t pt;
    rect_t rect = widget_get_absolute(widget);
    GLOBAL_INT_DISABLE();
    pt.x = (ctp_cb.sx - (rect.x - (rect.wid >> 1)) + (rect.wid >> 1)) >> 1;
    pt.y = (ctp_cb.sy - (rect.y - (rect.hei >> 1)) + (rect.hei >> 1)) >> 1;
    GLOBAL_INT_RESTORE();
    return pt;
}

//读数据KICK
AT(.com_text.ctp)
void ctp_iic_readkick(void *buf, u8 addr, int len)
{
    CTP_IIC->sfr->IICxCMDA = (I2C_READ_ADDR(TP_IIC_ADDR) << 24) | (addr << 8) | I2C_WRITE_ADDR(TP_IIC_ADDR);       //0x2A
    CTP_IIC->sfr->IICxCON1 = BIT(12) | BIT(11) | BIT(9) | BIT(8) | BIT(7) | BIT(5) | BIT(4) | BIT(3);
    CTP_IIC->sfr->IICxDMAADR = DMA_ADR(buf);
    CTP_IIC->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(0);
    CTP_IIC->sfr->IICxCON0 |= BIT(28);                //KICK
    ctp_kick_status = true;
}

//读数据接口(开中断前)
bool ctp_iic_read(void *buf, u8 addr, int len)
{
    u32 ticks;
    ctp_iic_readkick(buf, addr, len);
    ticks = tick_get();
    while (!(CTP_IIC->sfr->IICxCON0 & BIT(31))) {
        if (tick_check_expire(ticks, 20)) {
            return false;
        }
    }
    CTP_IIC->sfr->IICxCON0 |= BIT(29);                //Clear Pending
    ctp_kick_status = false;
    return true;
}


//复位
void ctp_reset(void)
{
    PORT_CTP_RST_L();
    delay_5ms(2);
    PORT_CTP_RST_H();
    delay_5ms(21);
}

//初始化
void ctp_init(void)
{
    bool res;
    memset(&ctp_cb, 0, sizeof(ctp_cb));
    port_gpio_set_out(PORT_CTP_RST, 1);
    bsp_i2c_init(PORT_CTP_IIC_HW, PORT_CTP_SCL, PORT_CTP_SDA, PORT_CTP_MAP_GPIO);
    i2c_t* bsp_i2c_get_register(u8 i2cx_idx);
    CTP_IIC = bsp_i2c_get_register(PORT_CTP_IIC_HW);

    //复位TP/等待模块使能
    ctp_reset();
#if (CTP_SELECT == CTP_CST8X)
    res = ctp_cst8x_init();
#elif (CTP_SELECT == CTP_CHSC6X)
    res = ctp_chsc6x_init();
#elif (CTP_SELECT == CTP_AXS5106)
    res = AXS5106_init();
#elif (CTP_SELECT == CTP_AXS152x)
    res = AXS152x_init();
#endif
    if (!res) {
        TRACE("CTP ERROR!\n");
        return;
    }
    sys_irq_init(IRQ_I2C_VECTOR, 0, ctp_isr);
    CTP_IIC->sfr->IICxCON0 |= BIT(1);                             //IIC INT EN

    //INT信号中断
    port_irq_register(PORT_CTP_INT_VECTOR, ctp_int_isr);
    port_wakeup_init(PORT_CTP_INT, 1, 1);           //开内部上拉, 下降沿唤醒
}

void ctp_exit(void)
{
    bsp_i2c_deinit(PORT_CTP_IIC_HW, PORT_CTP_SCL, PORT_CTP_SDA, PORT_CTP_MAP_GPIO);
    port_gpio_disable(PORT_CTP_RST);
    port_irq_free(PORT_CTP_INT_VECTOR);
    port_wakeup_exit(PORT_CTP_INT);
}

//升级写KICK
void ctp_iic_write_update_kick(u8 dev_addr, u16 addr, u8 *cmd, int len)
{
    CTP_IIC->sfr->IICxCMDA = ((addr >> 8) << 8) | I2C_WRITE_ADDR(dev_addr);       //2Bytes的reg addr放高位到addr0

    u8 buf[8];
    buf[0] = addr & 0xff;                                           //2Bytes的reg addr低位放到数据第一字节
    if(len > 1) {
        memcpy(&buf[1], cmd, len - 1);
    }

    CTP_IIC->sfr->IICxDMAADR = DMA_ADR(buf);
    CTP_IIC->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(1) | BIT(0);
    CTP_IIC->sfr->IICxCON0 |= BIT(28);                //KICK
    delay_ms(2);

}

//升级写
bool ctp_iic_update_write(u8 dev_addr, u16 addr, u8 *cmd, int len)
{
    CTP_IIC->sfr->IICxCON1 = BIT(11) | BIT(10) | BIT(5) | BIT(4) | BIT(3) | len;
    u32 ticks;
    ctp_iic_write_update_kick(dev_addr, addr, cmd, len);
    ticks = tick_get();
    while (!(CTP_IIC->sfr->IICxCON0 & BIT(31))) {
        if (tick_check_expire(ticks, 20)) {
            printf("WRITE UPDATE IIC TOUT\n");
            return false;
        }
        WDT_CLR();
    }
    CTP_IIC->sfr->IICxCON0 |= BIT(29);                //Clear Pending

//    delay_5ms(2);
    return true;
}

//升级读KICK
void ctp_iic_update_readkick(u8 dev_addr, void *buf, int len)
{
    CTP_IIC->sfr->IICxCMDA = (I2C_READ_ADDR(dev_addr)) << 24;

    CTP_IIC->sfr->IICxDMAADR = DMA_ADR(buf);
    CTP_IIC->sfr->IICxDMACNT = ((len - 1) << 16) | BIT(0);
    CTP_IIC->sfr->IICxCON0 |= BIT(28);                //KICK
}

//升级读
bool ctp_iic_update_read(u8 dev_addr, void *rbuf, int rlen, u16 w_addr, u8 *w_cmd, int wlen)
{
    ctp_iic_update_write(dev_addr, w_addr, w_cmd, wlen);

    CTP_IIC->sfr->IICxCON1 = BIT(12) | BIT(11) | BIT(9) | BIT(8) | BIT(7) | rlen;
    u32 ticks;
    ctp_iic_update_readkick(dev_addr, rbuf, rlen);
    ticks = tick_get();
    while (!(CTP_IIC->sfr->IICxCON0 & BIT(31))) {
        if (tick_check_expire(ticks, 20)) {
            printf("READ UPDATE IIC TOUT\n");
            return false;
        }
    }
    CTP_IIC->sfr->IICxCON0 |= BIT(29);                //Clear Pending

    delay_ms(2);
    return true;
}
#else
AT(.com_text.ctp)
void ctp_int_isr(void) {}
bool ctp_get_kick_status(void) {return false;}
void sys_ctp_irq_enble(int vector) {}
void sys_ctp_irq_disable(int vector) {}
AT(.com_text.ctp)
bool ctp_get_dxy(s32 *dx, s32 *dy) {return false;}
AT(.com_text.ctp)
point_t ctp_get_sxy(void) {point_t pt = {0}; return pt;}
AT(.com_text.ctp)
point_t ctp_get_last_dxy(void) {point_t pt = {0}; return pt;}
AT(.com_text.ctp)
bool ctp_get_cur_point(s32 *sx, s32 *sy, s32 *x, s32 *y) {return false;}
point_t ctp_get_rxy(widget_t *widget) {point_t pt = {0}; return pt;}
#endif
