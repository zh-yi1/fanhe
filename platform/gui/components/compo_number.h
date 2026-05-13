#ifndef _COMPO_NUMBER_H
#define _COMPO_NUMBER_H

//当图片资源只有0~9，暂不支持负数，需要自己手动拼图
//当图片资源有 0~9 -, 可以选择支持负数 
//默认设置数字组件都不显示符号位
#define MAX_NUMBER_CNT          8   //最大支持8位: 0 ~ 99999999

typedef struct compo_number_t_ {
    COMPO_STRUCT_COMMON;
    widget_image_t *img;
    int num_cnt;                //数
    int value;                  //值
    int max_val;                //最大值
    s16 num_wid;                //单数字宽
    s16 num_hei;                //单数字高
    s16 total_hei;              //总高
    s16 x;
    s16 y;
    s16 x_margin;
    bool visible;
    bool flag_zfill;            //高位自动填0
    bool flag_sym;              //是否带符号
    bool flag_str;              //是否字符串形式赋值
    char *val_str;              //字符串指针
    u8 num_align;               //0:默认按设置位数的中心点对齐  1:按实际val位数的中心点对齐  2:左对齐  3:右对齐
    u8 num_layout;              //0:横向排列  1:竖向排列
    u8 num_part;                //数字位分割, 个十百千万
    u8 radix;                   //裁剪个数 (默认为10，0~9数字裁剪，符号不算)，进制数，默认10进制
    widget_image_t *img_num[MAX_NUMBER_CNT];
    widget_image_t *img_sym;    //符号位
} compo_number_t;

/**
 * @brief 根据数字图像资源创建一个数字
 * @param[in] frm : 窗体指针
 * @param[in] res_addr : 数字图片资源地址
 * @param[in] num_cnt : 表示该数字最大位数
 * @return 返回数字指针
 **/
compo_number_t *compo_number_create(compo_form_t *frm, u32 res_addr, int num_cnt);

/**
 * @brief 根据数字图像资源创建一个数字
 * @param[in] frm : 窗体指针
 * @param[in] parent : 父页面
 * @param[in] res_addr : 数字图片资源地址
 * @param[in] num_cnt : 表示该数字最大位数
 * @return 返回数字指针
 **/
 compo_number_t *compo_number_create_for_page(compo_form_t *frm, widget_page_t* parent, u32 res_addr, int num_cnt);

/**
 * @brief 设置数字框裁剪个数
 * @param[in] num : 数字指针
 * @param[in] radix : 裁剪图片资源拼图个数
 * @param[in] type : 数字图片资源是否带符号
 **/
void compo_number_set_radix(compo_number_t *num, u8 radix, bool type);

/**
 * @brief 设置数字值
 * @param[in] num : 数字指针
 * @param[in] val : 设置的数值
 **/
void compo_number_set(compo_number_t *num, int val);

/**
 * @brief 设置数字值（字符串形式），先set_radix，字符串必须是全局或static变量
 * @param[in] num : 数字指针
 * @param[in] str : 字符串
 **/
void compo_number_set_str(compo_number_t *num, char *str);

/**
 * @brief 设置坐标
          注意：该设置默认的坐标是以中心点作为参考点
 * @param[in] num : 数字指针
 * @param[in] x : x轴坐标
 * @param[in] y : y轴坐标
 **/
void compo_number_set_pos(compo_number_t *num, s16 x, s16 y);

/**
 * @brief 设置透明度
 * @param[in] num : 数字指针
 * @param[in] alpha : 透明度
 **/
void compo_number_set_alpha(compo_number_t *num, u8 alpha);

/**
 * @brief 设置字间距
 * @param[in] num : 数字指针
 * @param[in] margin : 字间距
 **/
void compo_number_set_margin(compo_number_t *num, int margin);

/**
 * @brief 设置数字高位是否填0
 * @param[in] num : 数字指针
 * @param[in] flag : true  高位补0
                     false 高位不补0
 **/
void compo_number_set_zfill(compo_number_t *num, bool flag);

/**
 * @brief 设置数字对齐方式
 * @param[in] num : 数字指针
 * @param[in] num_align : 0  默认按设置位数的中心点对齐
                          1  按实际val位数的中心点对齐
                          2  左对齐
                          3  右对齐
 **/
void compo_number_set_align(compo_number_t *num, u8 num_align);

/**
 * @brief 设置数字框是否可见
 * @param[in] num : 数字指针
 * @param[in] visible : true  可见
                        false 不可见
 **/
void compo_number_set_visible(compo_number_t *num, bool visible);

/**
 * @brief 判断数字框是否可见
 * @param[in] num : 数字指针
 * @return true可见，false不可见
 **/
bool compo_number_get_visible(compo_number_t *num);

/**
 * @brief 获取数字框位置和大小
 * @param[in] num : 数字指针
 * @return 返回数字框位置和大小
 **/
rect_t compo_number_get_location(compo_number_t *num);

/**
 * @brief 设置数字排列方式
 * @param[in] num : 数字指针
 * @param[in] margin : 0  横向排列(默认); 1  竖向排列;
 **/
void compo_number_set_layout(compo_number_t *num, u8 margin);
#endif
