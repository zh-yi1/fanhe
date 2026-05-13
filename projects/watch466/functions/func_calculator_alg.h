#ifndef _FUNC_CALCULATOR_ALG_H
#define _FUNC_CALCULATOR_ALG_H

#define INPUT_NUM_LEN                   10                                              //输入最大字符数
#define DECIMAL_DIGITS_RESERVED         6                                               //保留的小数位
#define RESULT_NUM_LEN                  (INPUT_NUM_LEN + DECIMAL_DIGITS_RESERVED + 5)   //计算结果长度（5：小数点+负号+安全余量；长度过长转换为科学记数法）
#define MEDIAN_NUM_LEN                  (INPUT_NUM_LEN * 10)                            //计算中间值长度

enum {
    BTN_NULL,
    BTN_0,
    BTN_1,
    BTN_2,
    BTN_3,
    BTN_4,
    BTN_5,
    BTN_6,
    BTN_7,
    BTN_8,
    BTN_9,
    BTN_POINT,
    BTN_OPPOSITE,
    BTN_CE,
    BTN_C,
    BTN_DEL,
    BTN_DIV,
    BTN_MUL,
    BTN_SUB,
    BTN_ADD,
    BTN_EQUAL,
};

enum {
    NUM1,
    NUM2
};

typedef struct {
    char num1[RESULT_NUM_LEN];          //第一个操作数
    char num2[RESULT_NUM_LEN];          //第二个操作数
    char result[RESULT_NUM_LEN];        //计算结果
    u8 calc_operator;                   //操作符
    u8 cur_num;                         //当前操作数
    bool css_change_flag, num2_first_press, one_operation_process, operator_first_press, calc_error;
    int decimal_len1, decimal_len2;     //小数部分的长度
    char num_text[RESULT_NUM_LEN];    //当前显示内容
//    void *calc_show;
} gcal_cb_t;

bool gcal_cb_init(void);
void gcal_cb_destroy(void);
void gcalc_btn_click_handler(u16 btn_id);
char *gcal_get_show_str(void);
u16 gcal_get_holding_operator(void);

#endif // _FUNC_CALCULATOR_ALG_H
