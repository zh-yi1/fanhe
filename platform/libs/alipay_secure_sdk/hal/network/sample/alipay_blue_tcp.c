#include "alipay_blue_tcp.h"
#include "data.pb-c.h"
#include "alipay_ble_common.h"
#include "alipay_net_kal.h"

#define printf(...)                     a_printf(__VA_ARGS__)
#define print_r(...)                    a_print_r(__VA_ARGS__)

void a_printf(const char *format, ...);
void a_print_r(const void *buf, uint cnt);
int alipay_iot_socket_tx(uint8_t *buf, uint8_t len);
void delay_5ms(uint n);
void func_process(void);
unsigned long tick_get(void);
bool tick_check_expire(unsigned long tick, unsigned long expire_val);
void alipay_iot_socket_rx_sta_set(bool sta);
bool alipay_iot_socket_rx_sta_get(void);

// 全局变量 需要锁维护
static uint64_t ORDER = 0;

//int alipay_iot_socket_tx(u8 *buf, u8 len);

#define ALIPAY_PACKET_LEN   100

//void alipay_send_to_mobile(uint8_t *buf, uint8_t len)
//{
//
//    uint8_t *tbuf = alipay_iot_ble_malloc(ALIPAY_PACKET_LEN);
//    memset(tbuf, 0, ALIPAY_PACKET_LEN);
//    uint16_t ramain = 0;
//
//    int res = 0;
//    if (len > ALIPAY_PACKET_LEN) {
//        ramain = len - ALIPAY_PACKET_LEN;
//        do {
//                memcpy(tbuf, buf, ALIPAY_PACKET_LEN)
//                buf[ALIPAY_PACKET_LEN] = 1;
//                res = alipay_iot_socket_tx(tbuf, ALIPAY_PACKET_LEN + 1);
//                printf("@@@ pack!\n");
//            } while(remain > ALIPAY_PACKET_LEN);
//
//    } else {
//        buf[len] = 0;
//        res = alipay_iot_socket_tx(buf, len + 1);
//    }
//    printf("### %s, len:%d, res:%x\n", __func__, len + 1, len);
//
//    alipay_iot_socket_rx_sta = false;
//    while(!alipay_iot_socket_rx_sta) {
//        func_process();
////        printf(".");
//    }
//
//}


int alipay_send_to_mobile(uint8_t *buf, uint16_t len)
{
//    printf("%s total:%d\n", __func__, len);
    int ret = 0;
    uint16_t mtu = ALIPAY_PACKET_LEN;
    const uint16_t max_data_len = mtu - 1;

    uint8_t *data = alipay_iot_ble_malloc(mtu);
//    uint8_t data[100];
//    memset(data, 0x55, 100);

    uint16_t left_len = len;
    do {
        bool left = left_len > max_data_len;
        uint16_t data_len = left ? max_data_len : left_len;

        memcpy(data, buf + (len - left_len), data_len);
        data[data_len] = left ? 1 : 0;
        ret = alipay_iot_socket_tx(data, data_len + 1);
        left_len -= data_len;
        delay_5ms(10);

//        printf("### left_len:%d, data_len:%d, left:%x, %x\n", left_len, data_len, data[data_len], data[data_len - 2]);
//        printf("### ret:%x\n", ret);
        while(ret == 0x57) {
            ret = alipay_iot_socket_tx(data, data_len + 1);
            delay_5ms(50);
            printf("### retry ret:%x\n", ret);
        }

//        print_r(data, 100);
    } while (left_len);

    alipay_iot_std_free(data);
    alipay_iot_socket_rx_sta_set(false);;
    unsigned long ticks = tick_get();
    while(!alipay_iot_socket_rx_sta_get()) {
        func_process();
        if (tick_check_expire(ticks, 3000)) {
            printf("alipay iot socket rx timeout\n");
            break;
        }
    }

    return ret;
}

/*
func： alipay_ble_send_to_mobile
统一的手表端发送接口，维护order

@param      packSend:        待发送的数据（API参数信息）
@param      retBuffer:       手机端调用API后，通过蓝牙传输过来的结果信息
@param      retLen:          信息的长度

*/
int32_t alipay_ble_send_to_mobile(Alipay__TCPPackageIotSend * packSend, uint8_t *retBuffer, uint16_t *retLen){


    // alipay_iot_mutex_lock(get_order_mtx());
    packSend->order = ORDER++;
    // alipay_iot_mutex_unlock(get_order_mtx());
    size_t len = alipay__tcppackage_iot_send__get_packed_size(packSend);
//    printf("@@@ packet:");
//    print_r(packSend, len);
    uint8_t * buffer = alipay_iot_ble_malloc(len + 1);
    alipay__tcppackage_iot_send__pack(packSend, buffer);


//    printf("### len:%d\n", len);
    alipay_send_to_mobile(buffer, len);
    alipay_iot_std_free(buffer);
    // 此处手表端对buffer进行分包发送
    // send_buffer_by_package(buffer, len);
    // 手机端组装手表端发送过来的包后，得到buffer
    // 手机端组装buffer、处理后得到retBuffer
    // clock_t start, end;
    // start = clock();
//    alipay_mobile_tcp_solve(buffer, len, retBuffer, retLen);
    // end = clock();
    // printf("--- mobile whole process  time=%f   ----- %d \n  ", (double)(end - start) / CLOCKS_PER_SEC, packSend->body_case);

    // printf("\n    *    alipay_ble_send_to_mobile - retLen : 			   %llu\n", *retLen);
    // printf("    *    alipay_ble_send_to_mobile - retBuffer :		   ");
    // for(int i=0; i<*retLen; i++){
    //     printf("%d = %02x  ", i, retBuffer[i]);
    // }
    // printf("\n");

    // 对retBuffer进行分包发送 （手机端的模拟发送）
    // send_buffer_by_package(retBuffer, *retLen);
    return RV_OK;
}


/*
func： alipay_iot_ret_unpack
统一的手表端解析手机返回结果的接口

@param      bufferRecv:      手机端调用API后，通过蓝牙传输过来的结果信息
@param      len:             信息的长度
@param      retBuffer:       返回信息的存放处，不需要时直接设置为NULL
@param      retLen:          返回信息存放处的长度

*/
// int32_t
RetValue alipay_iot_ret_unpack(uint8_t * bufferRecv, uint64_t len, uint8_t * retBuffer, uint64_t retLen){
    // clock_t start, end;
    // start = clock();

    Alipay__TCPPackageIotRecv * retData =  alipay__tcppackage_iot_recv__unpack(NULL, len, bufferRecv);
    RetValue res = {0,};
//    printf("### -->res n_read_fds_use:%d\n", res.selectRet.n_read_fds_use);
//    printf("### %s, body_case:%d, bufferRecv:%x\n", __func__, retData->body_case, bufferRecv);
//    print_r(bufferRecv, len);
    switch (retData->body_case)
    {
        case ALIPAY__TCPPACKAGE_IOT_RECV__BODY__NOT_SET:{
            res.ret = RV_UNKNOWN;
            // return res;
            break;
        }

        case ALIPAY__TCPPACKAGE_IOT_RECV__BODY_RET4_BYTE_RECV_MES:{
            // printf("\n    *    in func (alipay_iot_ret_unpack) / API return           %d\n", retData->ret4byterecvmes->ret);
            res.ret = retData->ret4byterecvmes->ret;
            // return res;
            break;
        }

        case ALIPAY__TCPPACKAGE_IOT_RECV__BODY_HOSTENT_RES_MES:{
            if(retData->hostentresmes->n_h_addr_list_0_ip_arr == 0) break;

            for(int i=0; i<4; i++){
                res.ip[i] = retData->hostentresmes->h_addr_list_0_ip_arr[i];
            }

            // return res;
            break;
        }

        case ALIPAY__TCPPACKAGE_IOT_RECV__BODY_BUF_RET_MES:{
            res.ret = retData->bufretmes->ret;
            if(retData->bufretmes->ret != -1){
                alipay_iot_memcpy(retBuffer, retData->bufretmes->buf.data, retData->bufretmes->buf.len);
            }
            // return res;
            break;
        }

        case ALIPAY__TCPPACKAGE_IOT_RECV__BODY_RECV_FROM_RET_MES:{
            // 解析结果到自己定义的sockaddr结构中
            alipay_iot_memcpy(retBuffer, retData->recvfromretmes->buf, retData->recvfromretmes->lenbuf);
            //struct recvFromRes recvfromres;
            //res.recvFromRet = &recvfromres;////(struct recvFromRes *) alipay_iot_ble_malloc (sizeof(struct recvFromRes));
            res.recvFromRet.ret = (int)retData->recvfromretmes->lenbuf;
            if(retData->recvfromretmes->has_addrlen){
                res.recvFromRet.addr.sa_len = retData->recvfromretmes->sa_len;
            }
            if(retData->recvfromretmes->has_sa_family){
                res.recvFromRet.addr.sa_family = retData->recvfromretmes->sa_family;
            }

            // 因为没有用到，所以不强求此处的数据返回
            // 不能直接复制 此处sa_data 没有返回 后续需要补上
            if(retData->recvfromretmes->sa_data != NULL){
                // res.recvFromRet->addr.data.sa_data
                alipay_iot_memcpy(res.recvFromRet.addr.data.sa_data, retData->recvfromretmes->sa_data, sizeof(retData->recvfromretmes->sa_data));
                // alipay_iot_memcpy(res.recvFromRet->addr.sa_data, retData->recvfromretmes->sa_data, sizeof(retData->recvfromretmes->sa_data));
            }
            // return res;
            break;
        }

        case ALIPAY__TCPPACKAGE_IOT_RECV__BODY_SELECT_RES_MES: {
            // 手表端解析参数 并返回到原socket
//             printf("\n watch's select len : %d %d %d \n", (int)retData->selectresmes->n_readfds, (int)retData->selectresmes->n_writefds, (int)retData->selectresmes->n_exceptfds);

            res.selectRet.n_read_fds_use = 0;
            res.selectRet.n_write_fds_use = 0;
            res.selectRet.n_except_fds_use = 0;

            if(retData->selectresmes->n_readfds != 0){
                res.selectRet.n_read_fds_use = retData->selectresmes->n_readfds;
                for (int i = 0; i < retData->selectresmes->n_readfds; i++) {
                    res.selectRet.read_fds[i] = (int)(retData->selectresmes->readfds[i]);
                }
            }

            //  如果是返回值，会出现段错误 在这
            if(retData->selectresmes->n_writefds != 0){
                res.selectRet.n_write_fds_use = retData->selectresmes->n_writefds;
                for (int i = 0; i < retData->selectresmes->n_writefds; i++) {
                    res.selectRet.write_fds[i] = (int)(retData->selectresmes->writefds[i]);
                }
            }
            if(retData->selectresmes->n_exceptfds != 0){
                res.selectRet.n_except_fds_use = retData->selectresmes->n_exceptfds;
                for (int i = 0; i < retData->selectresmes->n_exceptfds; i++) {
                    res.selectRet.except_fds[i] = (int)(retData->selectresmes->exceptfds[i]);
                }
            }

            res.selectRet.ret = retData->selectresmes->ret;
            // return res;
            break;
        }


        default:
            res.ret = RV_UNKNOWN;
            // return res;
    }
    // end = clock();
//     printf("                         ----  watch open  ---  time=%f\n  ", (double)(end - start) / CLOCKS_PER_SEC);
//    printf("AAA\n");
//    print_r(res, sizeof(RetValue));
//    printf("-->res n_read_fds_use:%d\n", res.selectRet.n_read_fds_use);
//    printf("&res:%x\n", res);
    return res;
}

