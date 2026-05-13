#include "include.h"
#include "func_calculator_alg.h"

#define TRACE_EN    1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

void calc_num_clear(void);
int8_t sub_operation(char *num1, char *num2, char *result, int decimal_len1, int decimal_len2, bool div_sub, bool in_operation);

static gcal_cb_t *gcal_cb = NULL;

AT(.text.func.gcalc)
bool gcal_cb_init(void)
{
    if (gcal_cb == NULL) {
        gcal_cb = func_zalloc(sizeof(gcal_cb_t));
        if (gcal_cb) {
            calc_num_clear();
        } else {
            return false;
        }
    }
    return true;
 }

AT(.text.func.gcalc)
void gcal_cb_destroy(void)
{
    func_free(gcal_cb);
    gcal_cb = NULL;
}

//获取计算器当前显示字符串
char * gcal_get_show_str(void)
{
    if (gcal_cb) {
        return gcal_cb->num_text;
    } else {
        return "0";
    }
}

//获取当前操作符(+-*/null)
u16 gcal_get_holding_operator(void)
{
    if (gcal_cb->calc_operator >= BTN_DIV && gcal_cb->calc_operator <= BTN_ADD) {
        return gcal_cb->calc_operator;
    }
    return BTN_NULL;
}

//如果字符串str包含字符ch，则返回ch的下标，否则返回-1
AT(.text.func.gcalc)
int my_strchr(char *str, char ch)
{
    u8 len = strlen(str);
    int8_t i;
    for (i=0; i < len; i++) {
        if (str[i] == ch) {
            break;
        }
    }
    return i == len ? -1 : i;
}

//字符串整体前/向后移动nByte，越界部分截掉，返回输入指针（fill_chr：空位置填充字符）
AT(.text.func.gcalc)
char *str_move_right(char *str, s16 n, u16 buff_len, char fill_chr)
{
    char *cur, *aim;
    u16 len;
    len = strlen(str);
    if (len) {
        if (n > 0) {   //后移
            aim = str + len - 1;
            cur = aim + n;
            if (len + n >= buff_len) {
                aim -= len + n - (buff_len - 1);
                cur = str + buff_len - 2;
            }
            *(cur + 1) = '\0';
            for (; aim >= str  && cur >= str; aim--, cur--) {
                *cur = *aim;
            }
            for (; cur >= str; cur--) {
                *cur = fill_chr;
            }
        } else if (n < 0) {    //前移
            cur = str;
            if (-n < len) {
                aim = str - n;
                for (; *aim != '\0'; aim++, cur++) {
                    *cur = *aim;
                    *aim = fill_chr;
                }
                *aim = fill_chr;
            } else {
                aim = str + len;
                for (; aim > str; aim--) {
                    *aim = fill_chr;
                }
            }
            *cur = '\0';
        }
    }
    return str;
}

//UI上显示操作数和运算结果
AT(.text.func.gcalc)
void calc_num_set_content(char *num)
{
//	int x = LCD_WIDTH_SIZE - strlen(num) * 16 - 20;
//    TRACE("%s: x=%d, len=%d\n", __func__, x, strlen(num));
//    if (x < 0) {
//        x = 0;
//    }

//    if (strcmp(num, "Error") == 0) {
//        sprintf(gcal_cb->num_text, "%s", "-----");
//    } else {
        sprintf(gcal_cb->num_text, "%s", num);
//    }
}

//删除
AT(.text.func.gcalc)
void calc_num_del(u8 cur_num, bool btn_ce)
{
    TRACE("cur_num:%d\n",cur_num);
    switch (cur_num) {
        case NUM1:
            if (strlen(gcal_cb->num1) > 0) {
                memset(&gcal_cb->num1[strlen(gcal_cb->num1) - 1], '\0', sizeof(gcal_cb->num1[strlen(gcal_cb->num1) - 1]));
                if (strlen(gcal_cb->num1) == 0 || btn_ce) {
                    memset(gcal_cb->num1, '\0', sizeof(gcal_cb->num1));
                    gcal_cb->num1[0] = '0';
                }
                calc_num_set_content(gcal_cb->num1);
                // TRACE("1---> num_text:%s, num_len:%d\n", gcal_cb->num_text, strlen(gcal_cb->num1));
            }
            break;

        case NUM2:
            if (strlen(gcal_cb->num2) > 0) {
                memset(&gcal_cb->num2[strlen(gcal_cb->num2) - 1], '\0', sizeof(gcal_cb->num2[strlen(gcal_cb->num2) - 1]));
                if (strlen(gcal_cb->num2) == 0 || btn_ce) {
                    memset(gcal_cb->num2, '\0', sizeof(gcal_cb->num2));
                    gcal_cb->num2[0] = '0';
                }
                calc_num_set_content(gcal_cb->num2);
                // TRACE("2---> num_text:%s, num_len:%d\n", gcal_cb->num_text, strlen(gcal_cb->num2));
            }
            break;

        default:
            break;
    }
}

//取相反数
AT(.text.func.gcalc)
void calc_num_opposite(char *num_buff, u16 buff_len)
{
    TRACE("opposite:[%s]-->", num_buff);
    if (strcmp(num_buff, "0")) {
        if (my_strchr(num_buff, '-') != 0) { //正数，添加-号，超过buff_len部分截掉
            str_move_right(num_buff, 1, buff_len, '-');
        } else {    //负数，前移覆盖掉-号
            str_move_right(num_buff, -1, buff_len, 0);
        }
    }
    TRACE("[%s]\n", num_buff);
    calc_num_set_content(num_buff);
}

//清零
AT(.text.func.gcalc)
void calc_num_clear(void)
{
    memset(gcal_cb->num_text, '\0', sizeof(gcal_cb->num_text));
    gcal_cb->num_text[0] = '0';
    memset(gcal_cb->num1, '\0', sizeof(gcal_cb->num1));
    gcal_cb->num1[0] = '0';
    memset(gcal_cb->num2, '\0', sizeof(gcal_cb->num2));
    gcal_cb->num2[0] = '0';
    gcal_cb->calc_operator = BTN_EQUAL;
    gcal_cb->cur_num = NUM1;
    gcal_cb->decimal_len1 = gcal_cb->decimal_len2 = 0;
    gcal_cb->css_change_flag = gcal_cb->num2_first_press = gcal_cb->one_operation_process = gcal_cb->operator_first_press = gcal_cb->calc_error = 0;
}

//去掉整数字符串头部的0
AT(.text.func.gcalc)
void remove_head_zero(char *int_num)
{
    if (int_num[0] == '-') {    //负数
        int_num++;
    }
    for (u8 i = 0; i < strlen(int_num); i++) {
        if (int_num[i] != '0') {
            str_move_right(int_num, -i, strlen(int_num), 0);
            return;
        }
    }
}

//去掉小数末尾的0
AT(.text.func.gcalc)
void remove_end_zero(char *num)
{
//    TRACE("%s:%s\n", __func__, num);
    for (u8 i = strlen(num) - 1; i >= 0; i--) {
        if (num[i] == '0') {
            num[i] = '\0';
        } else if (num[i] != '0' && num[i] != '.') {
            return;
        }
        if (num[i] == '.') {
            num[i] = '\0';
            return;
        }
    }
}

//判断是否是小数，是则返回小数点的下标
AT(.text.func.gcalc)
int is_decimal(char *num)
{
    int8_t index = my_strchr(num, '.');
    u8 len = strlen(num);
    if (index != -1) {                //如果有包含小数点，且小数点后面不全是0，则是小数
        for (int8_t i = index + 1; i < len; i++) {
            if (num[i] != '0') {
                return index;
            }
        }
        remove_end_zero(num);   //如果有包含小数点，但小数点后面全是0，则是整数，并去掉小数点和后面的0
    }
    return -1;
}

//判断是否是科学计数法，是则返回指数，否则返回0
AT(.text.func.gcalc)
int is_scientific_notation(char *num)
{
    int power = 0;
    int index = my_strchr(num, 'E');
    char sign = num[index + 1];
    if (index != -1 && (sign == '+' || sign == '-')) {
        index += 2;
        while (num[index] >= '0' && num[index] <= '9') {
            power = power * 10 + (num[index++] - '0');
        }
//        if (num[index] != '\0') {
//            power = 0;
//        }
    }
    if (sign == '-') {
        power = -power;
    }
    return power;
}

//不断输入num的值
AT(.text.func.gcalc)
void append_num_char(char *num, int para)
{
    char ch = '\0';
    if (para == BTN_POINT) {
        ch = '.';
    } else {
        para -= BTN_0;
        if (para >= 0 && para <= 9) {
            ch = para + 0x30;
        } else {
            return;
        }
    }
    if (strcmp(num, "0") == 0 && ch != '.') {
        sprintf(num, "%c", ch);
    } else if((strcmp(num, "0") == 0 && ch == '.') || \
              ((strcmp(num, "0") != 0) && (my_strchr(num, '.') == -1)) || \
              ((strcmp(num, "0") != 0) && my_strchr(num, '.') != -1 && ('0' <= ch  && ch <= '9'))) {
        sprintf(num + strlen(num), "%c", ch);
    }
}

//获取小数部分的长度
AT(.text.func.gcalc)
int get_decimal_len(char *str, int index)
{
    remove_end_zero(str);
    return strlen(str) - index - 1;
}

//判断小数绝对值是否小于零 index:小数点下标
AT(.text.func.gcalc)
bool decimal_abs_less_than_zero(const char *str, int index)
{
    bool ret = true;
    bool negative = str[0] == '-';
    if (str[index] == '.') {
        for (index--; index >= negative; index--) {
            if (str[index] > '0') {
                ret = false;
            }
        }
    } else {
        ret = false;
    }
//    TRACE("%s:%d\n", __func__, ret);
    return ret;
}

//小数点后连续零的个数 index:小数点下标
AT(.text.func.gcalc)
u8 decimal_zero_count(const char *str, int index)
{
    u8 cnt = 0;
    if (str[index] == '.') {
        index++;
        while (str[index + cnt]) {
            if (str[index + cnt] == '0') {
                cnt++;
            } else {
                break;
            }
        }
    }
//    TRACE("%s:%d\n", __func__, cnt);
    return cnt;
}

//保留n位小数，返回是否进位（四舍五入）
AT(.text.func.gcalc)
bool keep_n_decimals(char *str, u8 n)
{
    u8 i;
    char chr;
    s8 idx_pt = my_strchr(str, '.');
    s8 idx_e = my_strchr(str, 'E');
    if (idx_pt != -1) {
        for (i = 1; i <= n; i++) {
            if (str[idx_pt + i] == '\0' || str[idx_pt + i] == 'E') {  //串尾/科学记数法
                return false;
            }
        }
        chr = str[idx_pt + i];
        if (idx_e != -1) {
            str_move_right(str + idx_pt + i, -(idx_e - (idx_pt + i)), RESULT_NUM_LEN, 0);
        } else {
            str[idx_pt + i] = '\0';
        }
//        TRACE("%s:%d\n", __func__, n);
        if (chr >= '5' && chr <= '9') {
//            TRACE("carry!\n");
            return true;
        }
    }
    return false;
}

//小数字符串转换为整数字符串
AT(.text.func.gcalc)
void decimalStr_2_intStr(char *str, int index)
{
    str_move_right(str + index, -1, RESULT_NUM_LEN, '\0');
}

//正整数字符串转换为小数字符串，小数部分长度动态修改
AT(.text.func.gcalc)
void intStr_2_decimalStr(char *str_result, int *decimal_len)
{
//    TRACE("%s: %s, %d\n", __func__, str_result, *decimal_len);
    if (*decimal_len == 0) {     //如果结果为整数，则不用处理小数点
        return;
    }

    if (*decimal_len < 0) {
        while (*decimal_len) {
            sprintf(str_result + strlen(str_result), "%c", '0');
            (*decimal_len)++;
        }
//        TRACE("decimal_str_result: %s\n", str_result);
        return;
    }

    u8 len = strlen(str_result);
    if (len > *decimal_len){
        str_move_right(str_result + len - *decimal_len, 1, MEDIAN_NUM_LEN - (len - *decimal_len), '.');  //小数部分后移一位并添加小数点
    } else {
        str_move_right(str_result, *decimal_len - len + 2, MEDIAN_NUM_LEN, '0');  //整体后移n位
        str_result[1] = '.';
    }

    remove_end_zero(str_result);    //去掉小数部分末尾的0

    int idx = my_strchr(str_result, '.');
    if (idx != -1) { //重新计算小数部分长度
        *decimal_len = strlen(str_result) - idx - 1;
    } else {
        *decimal_len = 0;
    }

//    TRACE("decimal_str_result: %s\n", str_result);
}

//数字串转换为科学计数法显示 str_result: 数字串; result: 转换结果buffer; index: 小数点下标; negative: 是否是负数
AT(.text.func.gcalc)
void digitalStr_2_scientific_notation(char *str_result, char *result, int8_t index, int8_t negative)
{
    u8 i = 0, j = 0, len, num_idx_start = 0, num_idx_end = 0;
    if (negative) {   //负数
        result[0] = str_result[0];  //负号
        i = 1;
        j = 1;
    }
    len = strlen(str_result);
    for (; i < len; i++) {
        if (str_result[i] != '0' && str_result[i] != '.') {
            num_idx_start = i;    //第一个非0数字的下标
            break;
        }
    }
    if (i == len && num_idx_start == 0) {
        sprintf(result, "%s", "0");
        return;
    }
    if (num_idx_start == index - 1) {
        for (u8 z = 0; z < 2+DECIMAL_DIGITS_RESERVED; z++) {
            result[j++] = str_result[i++];
        }
        result[j] = '\0';
        return;
    }
    for (int8_t w = len - 1; w >= 0; w--) {
        if (str_result[w] != '0' && str_result[w] != '.') {
            num_idx_end = w;    //最后一个非0数字的下标
            break;
        }
    }

//    TRACE("[1] num_idx_start: %d, num_idx_end: %d\n", num_idx_start, num_idx_end);
    result[j++] = str_result[i++];
    if (num_idx_end > num_idx_start) {
        if (num_idx_end - num_idx_start > DECIMAL_DIGITS_RESERVED) {
            num_idx_end = num_idx_start + DECIMAL_DIGITS_RESERVED;
            if (index <= num_idx_end) {
                if (num_idx_end+1 < len && str_result[num_idx_end + 1] != '0') {
                    num_idx_end++;
                }
            }
        }
        result[j++] = '.';
    }
//    TRACE("[2] num_idx_start: %d, num_idx_end: %d\n", num_idx_start, num_idx_end);
    for (u8 k = 0; k < (num_idx_end - num_idx_start); k++) {
        if (str_result[i] == '.') {
            i++;
            continue;
        }
        result[j++] = str_result[i++];
    }
    result[j++] = 'E';
    result[j] = '\0';
    if (index > num_idx_start) {
        sprintf(result + strlen(result), "%c%d", '+', index - num_idx_start - 1);
    } else {
        sprintf(result + strlen(result), "%d", index - num_idx_start);
    }
//    TRACE("%s:%s\n", __func__, result);
}

//科学记数法指数转换（移动基数小数点，"E"之后的字符直接删除，指数需要存起来）
AT(.text.func.gcalc)
void scientific_notation_power_convert(char *str, s8 from, s8 to)
{
    bool negative = false;
    u8 i;
    int move_bits = from - to;

    int idx = my_strchr(str, 'E');
    if (idx != -1) {
        str[idx] = '\0';
    }

    if (move_bits == 0) {
        return;
    }

    if (str[0] == '-') {
        str_move_right(str, -1, RESULT_NUM_LEN, 0);
        negative = true;
    }

    idx = my_strchr(str, '.');
    if (idx == -1) {
        idx = strlen(str);
        if (idx >= RESULT_NUM_LEN - 1) {
            idx = RESULT_NUM_LEN - 2;
        }
        str[idx + 1] = '\0';
    }

    if (move_bits > 0) {
        bool tag = false;
        for (i = 0; i < move_bits && idx + i < RESULT_NUM_LEN - 1; i++) {
            if (tag) {
                str[idx + i] = '0';
            } else {
                str[idx + i] = str[idx + i + 1];
            }
            if (str[idx + i] == '\0') {
                str[idx + i] = '0';
                tag = true;
            }
        }
        if (tag) {
            str[idx + i] = '\0';
        } else {
            str[idx + i] = '.';
        }
    } else if (move_bits < 0) {
        if (idx <= -move_bits) {
            str_move_right(str, -move_bits - idx + 1, RESULT_NUM_LEN,  '0');
            idx += -move_bits - idx + 1;
            idx = MIN(idx, RESULT_NUM_LEN - 2);
        }
        for (i = 0; i < -move_bits && idx > 0; i++, idx--) {
            str[idx] = str[idx - 1];
        }
        str[idx] = '.';
    }

    if (negative) {
        str_move_right(str, 1, RESULT_NUM_LEN, '-');
    }
    TRACE("e_convert[%d->%d]:%s\n", from, to, str);
}

//标准基数恢复为科学记数法（末尾补E+-power，若|power|过小则恢复为正常数字）
AT(.text.func.gcalc)
void scientific_notation_recover(char *str, int power)
{
    if (strcmp(str, "0") == 0) {
        return;
    }
    if (power >= -DECIMAL_DIGITS_RESERVED && power < INPUT_NUM_LEN) {
        scientific_notation_power_convert(str, power, 0);
        keep_n_decimals(str, DECIMAL_DIGITS_RESERVED);
    } else if (power) {
        sprintf(str + strlen(str), "E%c%d", power > 0 ? '+' : '-', power > 0 ? power : -power);
    }
    TRACE("e_recorver[%d]:%s\n", power, str);
}

//获取计算结果
AT(.text.func.gcalc)
static void get_result(char *str_result, char *result, int decimal_len, bool in_operation)
{
//    TRACE("%s: str_result[%s]  decimal_len[%d]  in_operation[%d]\n", __func__, str_result, decimal_len, in_operation);
    if (in_operation) { //乘除操作内调用加减操作，中间值不转科学记数法
        if (strlen(str_result) >= MEDIAN_NUM_LEN - 1) { //溢出
            gcal_cb->calc_error = 1;
            return;
        }
        sprintf(result, "%s", str_result);
    } else {
        int8_t index;   //小数点下标
        int8_t flag;    //正数或负数
        int8_t len = strlen(str_result);
        index = len;
        flag = str_result[0] == '-';
        if (flag) {
            len--;
        }
        if (decimal_len) {
            len = len - decimal_len - 1;    //小数的整数部分长度
            index = my_strchr(str_result, '.');
        }
        if (len > INPUT_NUM_LEN) {
            //如果整数部分溢出(长度超过10位)，则用科学计数法显示结果
            digitalStr_2_scientific_notation(str_result, result, index, flag);
        } else if (decimal_abs_less_than_zero(str_result, index) && decimal_zero_count(str_result, index) >= DECIMAL_DIGITS_RESERVED) {
            //如果小数绝对值小于零且小数点后连续零的个数大于保留位数，则用科学计数法显示结果
            digitalStr_2_scientific_notation(str_result, result, index, flag);
        } else {
            keep_n_decimals(str_result, DECIMAL_DIGITS_RESERVED);   //保留n位小数（去尾法）
            sprintf(result, "%s", str_result);
        }
    }
    TRACE("%s:%s\n", __func__, result);
}

//补齐小数部分，使两个操作数的小数部分一样长
AT(.text.func.gcalc)
void completion_decimal_part(char *num1, char *num2, int len1, int len2)
{
    int len = 0;
    if (len1 > len2) {
        len = len1 - len2;
        while(len-- && strlen(num2) < RESULT_NUM_LEN - 1) {
            sprintf(num2 + strlen(num2), "%c", '0');
        }
    } else if (len2 > len1) {
        len = len2 - len1;
        while(len-- && strlen(num1) < RESULT_NUM_LEN - 1) {
            sprintf(num1 + strlen(num1), "%c", '0');
        }
    }
}

//对比两个整数(数字字符串形式)的大小: num1>num2:返回1; num1<num2:返回-1; num1=num2:返回0;
AT(.text.func.gcalc)
int8_t comparison(char *num1, char *num2)
{
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    if (len1 > len2 || (len1 == len2 && memcmp(num1, num2, len1) > 0)) {
        return 1;
    } else if (len1 < len2 || (len1 == len2 && memcmp(num1, num2, len1) < 0)) {
        return -1;
    } else if (len1 == len2 && memcmp(num1, num2, len1) == 0) {
        return 0;
    } else {
        return 0xff;
    }
}

//申请空间，申请失败打印提示信息并设置标志位
AT(.text.func.gcalc)
void* gcal_zalloc(u32 size, char *result)
{
    void *mem = func_zalloc(size);
    if (mem == NULL) {
        gcal_cb->calc_error = 1;
        printf("###malloc failed!\n");
    }
    return mem;
 }

//加法运算
AT(.text.func.gcalc)
void add_operation(char *num1, char *num2, char *result, int decimal_len1, int decimal_len2, bool in_operation)
{
    if (gcal_cb->calc_error) {
        return;
    }

    TRACE("--->[add]:num1[%s], num2[%s]\n", num1, num2);
    if (my_strchr(num1, '-') != -1) {       //num1(负数)+num2(正数) => num2(正数)-num1(正数)
        calc_num_opposite(num1, RESULT_NUM_LEN);
        sub_operation(num2, num1, result, decimal_len2, decimal_len1, 0, 0);
        return;
    } else if (my_strchr(num2, '-') != -1){   //num1(正/负数)+num2(负数) => num1(正/负数)-num2(正数)
        calc_num_opposite(num2, RESULT_NUM_LEN);
        sub_operation(num1, num2, result, decimal_len1, decimal_len2, 0, 0);
        return;
    }

    u8 *inum1 = (u8*)gcal_zalloc(MEDIAN_NUM_LEN, result);             //被加数
    u8 *inum2 = (u8*)gcal_zalloc(MEDIAN_NUM_LEN, result);             //加数
    u8 *iresult = (u8*)gcal_zalloc(MEDIAN_NUM_LEN, result);           //和
    char *str_result = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);
    if (!(inum1 && inum2 && iresult && str_result)) {
        return;
    }

    u8 carry = 0;   //进位
    u8 len1, len2, len, temp;
    int8_t i, j, k;
    int decimal_len;

    completion_decimal_part(num1, num2, decimal_len1, decimal_len2);
    len1 = strlen(num1);
    len2 = strlen(num2);
    len = len1 > len2 ? len2 : len1;

    for (i = 0; i < len1; i++) {     //字符串逆序放入数组中
        inum1[i] = num1[len1 - 1 - i] - '0';
    }

    for (i = 0; i < len2; i++) {
        inum2[i] = num2[len2 - 1 - i] - '0';
    }

    for (i = 0; i < len; i++) {      //计算每一位的和
        temp = inum1[i] + inum2[i] + carry;
        iresult[i] = temp % 10;
        carry = temp / 10;
    }

    while (i < len1) {                //加上剩余部分
        temp = inum1[i] + carry;
        iresult[i] = temp % 10;
        carry = temp / 10;
        i++;
    }

    while (i < len2) {
        temp = inum2[i] + carry;
        iresult[i] = temp % 10;
        carry = temp / 10;
        i++;
    }

    if (carry > 0) {        //最高位有进位
        iresult[i] = carry;
        i++;
    }

    for (j = 0, k = i - 1; k >= 0; j++, k--) {   //将逆序的整数结果，正序放入字符串数组中
        str_result[j] = iresult[k] + '0';
    }
    str_result[j] = '\0';
    TRACE("[add]str_result:%s\n", str_result);

    decimal_len = decimal_len1 > decimal_len2 ? decimal_len1 : decimal_len2;
    intStr_2_decimalStr(str_result, &decimal_len);
    get_result(str_result, result, decimal_len, in_operation);

    func_free(inum1);
    func_free(inum2);
    func_free(iresult);
    func_free(str_result);
}

//减法运算
AT(.text.func.gcalc)
int8_t sub_operation(char *num1, char *num2, char *result, int decimal_len1, int decimal_len2, bool div_sub, bool in_operation)
{
    if (gcal_cb->calc_error) {
        return -1;
    }

    TRACE("--->[sub]:num1[%s], num2[%s]\n", num1, num2);
    if (div_sub && comparison(num1, num2) < 0) {    //除法中使用的减法,不够减时直接返回-1
        return -1;
    }

    if (strcmp(num1, "0") == 0) {
        char *inum2 = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);
        if (inum2) {
            strcpy(inum2, num2);
        } else {
            return -1;
        }
        intStr_2_decimalStr(inum2, &decimal_len2);
        calc_num_opposite(inum2, MEDIAN_NUM_LEN);
        get_result(inum2, result, decimal_len2, in_operation);
        func_free(inum2);
        return 1;
    }

    if (comparison(num1, num2) == 0 && decimal_len1 == decimal_len2) {
        sprintf(result, "%s", "0");
        return 0;
    }
    if (my_strchr(num1, '-') != -1 && my_strchr(num2, '-') == -1) {   //num1(负数)-num2(正数) => -[num1(正数)+num2(正数)]
        calc_num_opposite(num1, RESULT_NUM_LEN);
        add_operation(num1, num2, result, decimal_len1, decimal_len2, 0);
        calc_num_opposite(result, RESULT_NUM_LEN);
        return 1;
    } else if (my_strchr(num2, '-') != -1){   //num1(正/负数)-num2(负数) => num1(正/负数)+num2(正数)
        calc_num_opposite(num2, RESULT_NUM_LEN);
        add_operation(num1, num2, result, decimal_len1, decimal_len2, 0);
        return 1;
    }

    int8_t *inum1 = (int8_t*)gcal_zalloc(MEDIAN_NUM_LEN, result);                //把较大的数放于被减数
    int8_t *inum2 = (int8_t*)gcal_zalloc(MEDIAN_NUM_LEN, result);                //把较小的数放于减数
    int8_t *iresult = (int8_t*)gcal_zalloc(MEDIAN_NUM_LEN, result);              //差
    char *str_result = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);
    if (!(inum1 && inum2 && iresult && str_result)) {
        return -1;
    }

    int8_t i, j, k;     //i用来循环,两个数中较大数的长度放到j中,两个数中较小数的长度放到k中
    u8 len1, len2;
    u8 flag = 0;        //标记结果为正或负
    int decimal_len;
    completion_decimal_part(num1, num2, decimal_len1, decimal_len2);
    len1 = strlen(num1);
    len2 = strlen(num2);
    k = len1 > len2 ? len2 : len1;
    j = len1 < len2 ? len2 : len1;

    if (comparison(num1, num2) > 0) {      //被减数必须大于减数,方便计算;如果小于,则调换位置
        flag = 0;
        for (i = 0; i < len1; i++) {     //字符串逆序放入数组中
            inum1[i] = num1[len1 - 1 - i] - '0';
        }
        for (i = 0; i < len2; i++) {
            inum2[i] = num2[len2 - 1 - i] - '0';
        }
    } else if (comparison(num1, num2) < 0) {
        flag = 1;
        for (i = 0; i < len2; i++) {
            inum1[i] = num2[len2 - 1 - i] - '0';
        }
        for (i = 0; i < len1; i++) {
            inum2[i] = num1[len1 - 1 - i] - '0';
        }
    }
    for (i = 0; i < k; i++) {
        if (inum1[i] >= inum2[i]) {
            iresult[i] = inum1[i] - inum2[i];
        } else {
            iresult[i] = inum1[i] + 10 - inum2[i];
            --inum1[i + 1];
        }
    }
    while (i < j) {
        if (inum1[i] >= 0) {
            iresult[i] = inum1[i];
        } else {
            iresult[i] = inum1[i] + 10;
            --inum1[i + 1];
        }
        i++;
    }
    while (iresult[i - 1] == 0) {
        i--;
    }
    j = 0;
    for (k = i - 1; k >= 0; k--, j++) {   //将逆序的整数结果，正序放入字符串数组中
        str_result[j] = iresult[k] + '0';
    }
    str_result[j] = '\0';
    TRACE("[sub]str_result:%s\n", str_result);

    decimal_len = decimal_len1 > decimal_len2 ? decimal_len1 : decimal_len2;
    intStr_2_decimalStr(str_result, &decimal_len);

    if (flag) {     //如果是负数，添加负号
        calc_num_opposite(str_result, MEDIAN_NUM_LEN);
    }

    get_result(str_result, result, decimal_len, in_operation);

    func_free(inum1);
    func_free(inum2);
    func_free(iresult);
    func_free(str_result);

    return 1;
}

//乘法运算
AT(.text.func.gcalc)
void multiplication_operation(char *num1, char *num2, char *result, int decimal_len1, int decimal_len2)
{
    if (gcal_cb->calc_error) {
        return;
    }

    TRACE("--->[mul]:num1[%s], num2[%s]\n", num1, num2);
    if (memcmp(num1, "0", strlen(num1)) == 0 || memcmp(num2, "0", strlen(num2)) == 0) {
        sprintf(result, "%s", "0");
        return;
    }

    u8 *inum1 = (u8*)gcal_zalloc(MEDIAN_NUM_LEN, result);                        //被乘数
    u8 *inum2 = (u8*)gcal_zalloc(MEDIAN_NUM_LEN, result);                        //乘数
    u8 *iresult = (u8*)gcal_zalloc(MEDIAN_NUM_LEN, result);
    char *temp_result = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);
    char *str_result = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);
    if (!(inum1 && inum2 && iresult && temp_result && str_result)) {
        return;
    }

    u8 carry = 0;   //进位
    u8 len1, len2, temp;
    int8_t i, j, k;
    u8 flag = 0;        //标记结果为正或负
    int decimal_len = decimal_len1 + decimal_len2;

    if (my_strchr(num1, '-') != -1) {
        flag = 1;
        str_move_right(num1, -1, RESULT_NUM_LEN, 0);      //把负号去掉
    }
    if (my_strchr(num2, '-') != -1) {
        flag = !flag;
        str_move_right(num2, -1, RESULT_NUM_LEN, 0);      //把负号去掉
    }
    len1 = strlen(num1);
    len2 = strlen(num2);

    for (i = 0; i < len1; i++) {     //字符串逆序放入数组中
        inum1[i] = num1[len1 - 1 - i] - '0';
    }

    for (i = 0; i < len2; i++) {
        inum2[i] = num2[len2 - 1 - i] - '0';
    }

    for (i = 0; i < len2; i++) {    //竖式乘法: num2的每一位乘以num1，再将所有结果相加
        memset(iresult, 0, MEDIAN_NUM_LEN);
        memset(temp_result, '\0', MEDIAN_NUM_LEN);
        carry = 0;
        for (j = 0; j < len1; j++) {
            temp = inum2[i] * inum1[j] + carry;
            iresult[j] = temp % 10;
            carry = temp / 10;
        }
        if (carry > 0) {
            iresult[j] = carry;
            j++;
        }
        while (iresult[j - 1] == 0){
            j--;
        };
        for (k = 0; k < j; k++) {
            temp_result[k] = iresult[j - 1 - k] + '0';
        }
        for (j = 0; j < i; j++) {       //每一位乘以对应的倍数,如十位*10, 百位*100
            temp_result[k++] = '0';
        }
        temp_result[k] = '\0';
        add_operation(str_result, temp_result, str_result, 0, 0, 1);
        TRACE("[mul:%d]str_result:%s\n", i, str_result);
    }

    intStr_2_decimalStr(str_result, &decimal_len);
    if (flag) {     //如果是负数，添加负号
        calc_num_opposite(str_result, MEDIAN_NUM_LEN);
    }

    get_result(str_result, result, decimal_len, 0);

    func_free(inum1);
    func_free(inum2);
    func_free(iresult);
    func_free(temp_result);
    func_free(str_result);
}

//除法运算
AT(.text.func.gcalc)
void division_operation(char *num1, char *num2, char *result, int decimal_len1, int decimal_len2)
{
    TRACE("--->[div]:num1[%s], num2[%s]\n", num1, num2);

    if (memcmp(num2, "0", strlen(num2)) == 0) {
        gcal_cb->calc_error = 1;
        return;
    }

//    if (memcmp(num2, "1", strlen(num2)) == 0) {
//        decimal_len1 -= decimal_len2;
//        intStr_2_decimalStr(num1, &decimal_len1);
//        get_result(num1, result, decimal_len1, 0);
//        return;
//    }

    if (memcmp(num1, "0", strlen(num1)) == 0) {     //被除数为0，则结果为0
        sprintf(result, "%s", "0");
        return;
    }

    char *tnum1 = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);                    //被除数
    char *tnum2 = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);                    //除数
    char *str_result = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);               //商
    char *temp_result = (char*)gcal_zalloc(MEDIAN_NUM_LEN, result);
    if (!(tnum1 && tnum2 && temp_result && str_result)) {
        return;
    }

    int quotient = 0;
    int decimal_len = 0, dlen = 0;
    int8_t len1, len2, len, i;          //len: 被除数和除数相差的位数
    u8 flag = 0;                        //标记结果为正或负
    bool len_flag = false;
    u8 cnt = 0;

    memcpy(tnum1, num1, strlen(num1));
    memcpy(tnum2, num2, strlen(num2));

    if (DECIMAL_DIGITS_RESERVED <= decimal_len1 - decimal_len2) {    //小数部分很长的被除数 / 小数部分很短的除数的情况
        len_flag = true;
    }
    if (my_strchr(tnum1, '-') != -1) {
        flag = 1;
        str_move_right(tnum1, -1, MEDIAN_NUM_LEN, 0);      //把负号去掉
        strcpy(num1, tnum1);
    }
    if (my_strchr(tnum2, '-') != -1) {
        flag = !flag;
        str_move_right(tnum2, -1, MEDIAN_NUM_LEN, 0);      //去掉负号
        strcpy(num2, tnum2);
    }

    if (comparison(tnum1, tnum2) == 0) {    //两数相等
        sprintf(str_result, "%s", "1");
        dlen = decimal_len1 - decimal_len2;
    } else {
        while (comparison(tnum1, tnum2) < 0) {          //如果被除数小于除数，则先把被除数不断扩大10倍，直到大于除数
            sprintf(tnum1 + strlen(tnum1), "%c", '0');
            decimal_len++;                              //被除数每扩大10倍，商的小数位加1
        }
        if (decimal_len + decimal_len1 > DECIMAL_DIGITS_RESERVED) {    //很小的被除数(传进来的被除数已经扩大了decimal_len1倍) / 很大的除数 的情况
            len_flag = true;
        }

        while (decimal_len <= DECIMAL_DIGITS_RESERVED - (decimal_len1 - decimal_len2) || (len_flag && cnt <= 6)) {  //除不尽则保留6位小数
            memcpy(tnum2, num2, strlen(num2));
            len1 = strlen(tnum1);
            len2 = strlen(tnum2);
            len = len1 - len2;              //相差位数
            for (i = 0; i < len; i++) {     //将除数扩大,使得除数和被除数位数相等
                sprintf(tnum2 + strlen(tnum2), "%c", '0');
            }

            len2 = len1;
            TRACE("tnum1:%s[len:%d], tnum2:%s[len:%d],len:%d\n", tnum1, len1, tnum2, len2, len);
            for (i = 0; i <= len; i++) {
                tnum2[len2 - i] = '\0';
                quotient = 0;
                while (sub_operation(tnum1, tnum2, temp_result, 0, 0, 1, 1) != -1) {
                    sprintf(tnum1, "%s", temp_result);
                    quotient++;
                }
                if (i == 0 && quotient == 0 && len) {
                    continue;
                }
                sprintf(str_result + strlen(str_result), "%d", quotient);
            }
            if (len < 0) {      //如果余数*10后不够除，则商的这一位为0
                sprintf(str_result + strlen(str_result), "%d", 0);
            }
            decimal_len++;
            if (memcmp(tnum1, "0", 1) == 0) {        //余数为0，可以整除或除的尽
                break;
            } else {
                sprintf(tnum1 + strlen(tnum1), "%c", '0');   //有余数则将被除数扩大10倍，继续除
            }
            if (len_flag) {
                cnt++;
            }
        }
        dlen = decimal_len1 - decimal_len2 + decimal_len - 1;
    }

    remove_head_zero(str_result);
    intStr_2_decimalStr(str_result, &dlen);

    if (flag) {     //如果是负数，添加负号
        calc_num_opposite(str_result, MEDIAN_NUM_LEN);
    }

    TRACE("[div]str_result:%s\n", str_result);
    get_result(str_result, result, dlen, 0);

    func_free(tnum1);
    func_free(tnum2);
    func_free(str_result);
    func_free(temp_result);
}

//=运算
AT(.text.func.gcalc)
void equal_operation(char *num1, char *num2, char *result)
{
    TRACE("%s: %s,%s\n", __func__, num1, num2);
    char *temp_num1 = (char*)gcal_zalloc(RESULT_NUM_LEN, result);
    char *temp_num2 = (char*)gcal_zalloc(RESULT_NUM_LEN, result);
    if (!(temp_num1 && temp_num2)) {
        return;
    }
    if (my_strchr(num2, '.') != -1) {
        remove_end_zero(num2);
    }
    memcpy(temp_num1, num1, strlen(num1) + 1);
    memcpy(temp_num2, num2, strlen(num2) + 1);

    //判断一下科学记数法，转整数/小数
    int result_is_numx = 0;  //科学记数法加减法，num2远大于/小于num1时取较大值，0:否 1:取num1 2:取num2 -1:取-num1 -2:取-num2
    int e_num1 = 0, e_num2 = 0, e_min = 0, e_result = 0, idx = 0;
    e_num1 = is_scientific_notation(temp_num1);
    if (e_num1) {
        idx = my_strchr(num2, '.');
        if (idx == -1) {
            idx = strlen(num2);
        }
        digitalStr_2_scientific_notation(num2, temp_num2, idx, num2[0] == '-');
        e_num2 = is_scientific_notation(temp_num2);
        switch (gcal_cb->calc_operator) {
        case BTN_ADD:
        case BTN_SUB:
            if (abs_s(e_num1 - e_num2) <= DECIMAL_DIGITS_RESERVED) {
                e_min = (e_num1 > e_num2 ? e_num2 : e_num1);  //取较小的指数转换
                scientific_notation_power_convert(temp_num1, e_num1, e_min);
                scientific_notation_power_convert(temp_num2, e_num2, e_min);
            } else {
                if (e_num1 > e_num2 || strcmp(temp_num2, "0") == 0 || strcmp(temp_num2, "-0") == 0) {
                    result_is_numx = 1;
                } else if (gcal_cb->calc_operator == BTN_ADD) {
                    result_is_numx = 2;
                } else if (gcal_cb->calc_operator == BTN_SUB) {
                    result_is_numx = -2;
                }
                e_num1 = e_num2 = 0;
            }
            break;
        case BTN_MUL:
        case BTN_DIV:
            scientific_notation_power_convert(temp_num1, e_num1, e_num1);
            scientific_notation_power_convert(temp_num2, e_num2, e_num2);
            break;
        }
    }

    if (!(result_is_numx)) {
        idx = is_decimal(temp_num1);   //判断num1是否是小数
        if (idx != -1) {
            gcal_cb->decimal_len1 = get_decimal_len(temp_num1, idx);
            decimalStr_2_intStr(temp_num1, idx);
            remove_head_zero(temp_num1);
        }
        idx = is_decimal(temp_num2);       //判断num2是否是小数
        if (idx != -1) {
            gcal_cb->decimal_len2 = get_decimal_len(temp_num2, idx);
            decimalStr_2_intStr(temp_num2, idx);
            remove_head_zero(temp_num2);
        }

        switch (gcal_cb->calc_operator) {
        case BTN_ADD:
            add_operation(temp_num1, temp_num2, result, gcal_cb->decimal_len1, gcal_cb->decimal_len2, 0);
            break;
        case BTN_SUB:
            sub_operation(temp_num1, temp_num2, result, gcal_cb->decimal_len1, gcal_cb->decimal_len2, 0, 0);
            break;
        case BTN_MUL:
            multiplication_operation(temp_num1, temp_num2, result, gcal_cb->decimal_len1, gcal_cb->decimal_len2);
            break;
        case BTN_DIV:
            division_operation(temp_num1, temp_num2, result, gcal_cb->decimal_len1, gcal_cb->decimal_len2);
            break;
        case BTN_EQUAL:
            result_is_numx = 1;  //如果没有点击+-*/，也没有输入num2，则结果等于num1
            break;
        default:
            break;
        }
    }

    //科学记数法恢复
    if (e_num1 || e_num2) {
        e_result = is_scientific_notation(result);
        if (!e_result) {
            strcpy(temp_num1, result);
            idx = my_strchr(temp_num1, '.');
            if (idx == -1) {
                idx = strlen(temp_num1);
            }
            digitalStr_2_scientific_notation(temp_num1, result, idx, temp_num1[0] == '-');
            e_result = is_scientific_notation(result);
        }
        TRACE("e_result:%d\n", e_result);
        scientific_notation_power_convert(result, e_result, e_result);
        switch (gcal_cb->calc_operator) {
        case BTN_ADD:
        case BTN_SUB:
            scientific_notation_recover(result, e_result + e_min);
            break;
        case BTN_MUL:
            scientific_notation_recover(result, e_result + e_num1 + e_num2);
            break;
        case BTN_DIV:
            scientific_notation_recover(result, e_result + e_num1 - e_num2);
            break;
        }
    }

    if (result_is_numx) {
        if (result_is_numx < 0) {
            calc_num_opposite(result_is_numx == -1 ? num1 : num2, RESULT_NUM_LEN);
            sprintf(result, "%s", result_is_numx == -1 ? num1 : num2);
        } else {
            sprintf(result, "%s", result_is_numx == 1 ? num1 : num2);
        }
        if (!is_scientific_notation(result) && ((idx = is_decimal(result)) != -1)) {
            memcpy(temp_num1, result, strlen(result) + 1);
            get_result(temp_num1, result, get_decimal_len(temp_num1, idx), 0);
        }
    }


    if (gcal_cb->calc_error) {
        sprintf(result, "%s", "Error");
    } else {
        calc_num_clear();
        sprintf(gcal_cb->num1, "%s", result);    //将当前结果作为第一个操作数
        TRACE("---> num1:%s\n", gcal_cb->num1);
    }

    TRACE("result = %s\n", result);
    calc_num_set_content(result);


    func_free(temp_num1);
    func_free(temp_num2);
}

//按钮点击处理
void gcalc_btn_click_handler(u16 btn_id)
{
    if (btn_id == BTN_NULL) {
        return;
    }

    if (btn_id <= BTN_POINT) {  //数字
        if (gcal_cb->one_operation_process || gcal_cb->calc_error) {    //一次运算结束或者出错后，下一次运算前先清零各种变量
            calc_num_clear();
        }
        if (gcal_cb->cur_num == NUM1 && strlen(gcal_cb->num1) < INPUT_NUM_LEN) {
            append_num_char(gcal_cb->num1, btn_id);  //不断输入num1的值
            TRACE("num1 = %s\n", gcal_cb->num1);
            calc_num_set_content(gcal_cb->num1);
        } else if (gcal_cb->cur_num == NUM2 && strlen(gcal_cb->num2) < INPUT_NUM_LEN) {
            append_num_char(gcal_cb->num2, btn_id);  //不断输入num2的值
            TRACE("num2 = %s\n", gcal_cb->num2);
            calc_num_set_content(gcal_cb->num2);
            if (gcal_cb->num2_first_press == 0) {    //如果是第一次输入num2的值
                gcal_cb->num2_first_press = 1;
                gcal_cb->css_change_flag = 1;        //将操作符按钮的样式变回来
            }
        }
//        TRACE("[%s, %s, %d, %d]\n", gcal_cb->num1, gcal_cb->num2, strlen(gcal_cb->num1), strlen(gcal_cb->num2));
    } else {    //操作符
        if (btn_id == BTN_C || gcal_cb->calc_error) {          //'C'按钮
            calc_num_clear();
        } else {
            if (btn_id == BTN_DEL || btn_id == BTN_CE) {   //'DEL'或'CE'按钮
                calc_num_del(gcal_cb->cur_num, btn_id == BTN_CE);
            } else if (btn_id == BTN_OPPOSITE) {    //"+/-"按钮
                calc_num_opposite(gcal_cb->cur_num == NUM1 ? gcal_cb->num1 : gcal_cb->num2, RESULT_NUM_LEN);
//                gcal_cb->one_operation_process = 0;   一轮结束后取相反数，继续向尾部添加数字
            } else if (btn_id > BTN_DEL && btn_id < BTN_EQUAL) {      //+-*/按钮
                if (gcal_cb->cur_num == NUM2 && gcal_cb->num2_first_press && !gcal_cb->one_operation_process) {
                    equal_operation(gcal_cb->num1, gcal_cb->num2, gcal_cb->result);
                    if (gcal_cb->calc_error) {
                        gcal_cb->calc_operator = BTN_EQUAL;
                        return;
                    }
                }

                gcal_cb->one_operation_process = 0;
                gcal_cb->cur_num = NUM2;

                gcal_cb->calc_operator = btn_id;     //记录当前操作符
                if (gcal_cb->operator_first_press == 0) {
                    gcal_cb->operator_first_press = 1;
                }
            } else if (btn_id == BTN_EQUAL) {            // =按钮
                if (gcal_cb->operator_first_press && gcal_cb->num2_first_press == 0) {    //如果按下了+-*/，但没有输入num2的值，则result = num1
                    gcal_cb->calc_operator = BTN_EQUAL;
                }

                if (strlen(gcal_cb->num1) && (!gcal_cb->one_operation_process)) {
                    equal_operation(gcal_cb->num1, gcal_cb->num2, gcal_cb->result);
                    gcal_cb->one_operation_process = 1;      //结束一轮运算
                    gcal_cb->calc_operator = BTN_EQUAL;
                }
            }
            TRACE("1--->calc_operator:%d\n", gcal_cb->calc_operator);
        }
    }
}
