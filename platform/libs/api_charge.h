#ifndef _API_CHARGE_H
#define _API_CHARGE_H

#define vusb_delay_rst_en(nsec)                     {VBRSTCON = ((nsec * 33) << 20) | BIT(30);}         //vusb delay n second reset enable
#define vusb_delay_rst_dis()                        {RTCCPND = 3 << 28;}                                //clr counter and pending, disable
#define vusb_delay_rst_kick()                       {RTCCPND = BIT(30);}                                //kick start vusb reset delay counter


//充电状态
enum {
    CHAG_STA_UNINIT = 0,        //未初始化
    CHAG_STA_OFF,               //充电关闭
    CHAG_STA_OFF_VBUS_PATH,     //充电关闭，但是还是插着5V电源
    CHAG_STA_ON_TRICKLE,        //正在涓流充电
    CHAG_STA_ON_CONSTANT,       //正在恒流充电
};

//充电配置
typedef struct {
    u8 chag_sta         : 3,    //充电状态
       dcin_reset       : 1,    //DC插入复位
       trick_curr_en    : 1,    //是否使能涓流充电
       buck_mode_en     : 1,
       vio_chg_en       : 1;    //5V->3.3V LDO控制
    u8 trick_stop_volt  : 2,
       stop_volt        : 2,    //充电截止电压
       follow_volt      : 2,    //跟随快充恒压差值档位
       follow_volt_en   : 1;    //跟随快充使能位

    u8 const_curr;              //恒流充电电流
    u8 trick_curr;              //涓流充电电流
    u8 stop_curr;               //充电截止电流
    u8 stop_curr_thd;           //满足充满截止电流占比阈值
    u8 bled_on_pr;
    u8 detect_cnt;              // 充电防抖次数
    volatile u16 stop_time;     //仅电压充满的超时时间
} charge_cfg_t;

void charge_stop(u8 mode);
void charge_init(charge_cfg_t *p);
#endif // _API_CHARGE_H
