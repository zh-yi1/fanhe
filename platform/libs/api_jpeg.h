#ifndef __API_JPEG_H
#define __API_JPEG_H

//异常类型宏
#define JPGERR_OUTBUF                           -1
#define JPGERR_DQT                              -2
#define JPGERR_PRECISION                        -3
#define JPGERR_COMPONENT                        -4
#define JPGERR_DRI                              -5
#define JPGERR_TAG                              -6
#define JPGERR_READBITSTREAM                    -7
#define JPGERR_APP                              -8
#define JPGERR_SOF                              -9
#define JPGERR_SOS                              -10
#define JPGERR_DHT                              -11
#define JPGERR_HUFFMAN                          -12
#define JPGERR_BUILD_TREE                       -13


/**
 * 初始化
 */
void jpeg_hw_init(void);

/*
    解码函数：
    说明：该函数可以用于一次解码所有数据，也可以用于第一次解码设置解码输入数据；
            每次调用解码输出的数据大小为16行的整数倍 out_size_max >= 16*width*sizeof(u16)*n + 8，width:图片宽，n:整数倍系数 8:为信息头；
            输出图片格式为RGB565。

    out_buf:        输出buffer地址, [0~3] byte是 0x24150, [4~5] byte是 图片的宽度, [6~7] byte是 图片的高度。[8~] 以后是解码数据
    out_size_max:   输出buffer大小
    in_buf:         输入buffer地址
    in_size:        输入buffer大小
    返回值[>0]:     解码输出的长度(byte)。
    返回值[==0]:    解码输出为空，可能是 out_size_max 太小，解码buffer小于16行，n为0。
    返回值[<0]:     解码异常，异常类型参考宏。
*/
int jpeg_decode(void *out_buf, int out_size_max, void *in_buf, int in_size);

/*
    持续解码函数：
    说明：必须先调用一次 jpeg_decode 进行第一次解码，并且设置输入数据；
            后面可以多次调用 jpeg_decode_continue 函数获取解码输出，实现节省解码输出ram的效果；
            每次调用解码输出的数据大小为16行的整数倍，具体解码多少行根据 out_size_max 的值进行计算得到；
            输出图片格式为RGB565。

    out_buf:        输出buffer地址，[0~] 以后是解码数据
    out_size_max:   输出buffer大小
    返回值[>0]:     解码输出的长度(byte)。
    返回值[==0]:    解码已完成，已经没有数据可以继续解码。
    返回值[<0]:     解码异常，异常类型参考宏。
*/
int jpeg_decode_continue(void *out_buf, int out_size_max);

//示例
/*
    //解码一张360*360的图片
    AT(.jpeg_buf.sta)
    u8 dec_rbuf[360*16];                                        //输入buf 大于等于plen

    AT(.jpeg_buf.sta)
    u8 draw_buf[360*16*2+8];                                    //输出buf 16行的整数倍+8个信息头

    u8 *obuf = draw_buf;                                        //输出buf
    u8 *ptr  = RES_BUF_WATCH_JPG;                               //输入buf
    u32 plen = RES_LEN_WATCH_JPG;                               //输入buf大小
    memcpy(dec_rbuf, ptr, plen);                                //待解码数据

    int res = jpeg_decode(obuf, 360*16*2+8, dec_rbuf, plen);    //开始首次解码
    if (res < 0) {
        printf("0### JPEG DECODE err:%d\n", res);               //报错 解码异常，异常类型参考宏
    }
    w = *(u16*)&draw_buf[4];                                    //jpeg图片 宽
    h = *(u16*)&draw_buf[6];                                    //jpeg图片 高
    obuf += res;                                                //输出buf偏移

    while (1) {                                                 //解完剩下的
        res = jpeg_decode_continue(obuf, 360*16*2+8);           //继续解码
        if (res < 0) {                                          //报错 解码异常，异常类型参考宏
            printf("### JPEG DECODE err:%d\n", res);
            break;
        } else if (res == 0) {                                  //解码完毕退出
            res = 1;
            break;
        }
        obuf += res;                                            //输出buf偏移
    }

    printf("[w:%d, h:%d]\n", w, h);
 */

#endif
