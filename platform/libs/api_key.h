#ifndef _API_KEY_H
#define _API_KEY_H

typedef struct {
    u16 val;
    u16 cnt;
    u16 up;

    u16 val_prev;
    u16 dcnt;
    u16 hcnt;                    //long hold times

    u8 pwroff_time;
    u8 pwr_usage_id;
    u8 resv[2];
} key_cb_t;
extern key_cb_t key_cb;

void key_var_init(void);
u16 key_process(u16 key_val);
u16 key_multi_press_process(u16 key_val);

#endif // _API_KEY_H
