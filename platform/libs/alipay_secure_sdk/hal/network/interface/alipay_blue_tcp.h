
#ifndef alipay_blue_tcp_h
#define alipay_blue_tcp_h

#include "data.pb-c.h"
//#include <socket.h>   //fd_set
#include "alipay_net_kal.h"

struct recvFromRes{
    int32_t ret;
    struct alipay_iot_sockaddr addr;
};

// 暂时设置为最多监听3个FD
struct selectRes{
    int ret;
    uint32_t n_read_fds_use;
    uint32_t read_fds[3];
    uint32_t n_write_fds_use;
    uint32_t write_fds[3];
    uint32_t n_except_fds_use;
    uint32_t except_fds[3];
};

typedef union {
    int32_t ret;
    uint32_t ip[4];     // hostent返回的IP地址的四个数形式
    struct recvFromRes recvFromRet;
    struct selectRes selectRet;
}   RetValue;



int32_t alipay_ble_send_to_mobile(Alipay__TCPPackageIotSend * packSend, uint8_t *retBuffer, uint16_t *retLen);

RetValue alipay_iot_ret_unpack(uint8_t * bufferRecv, uint64_t len, uint8_t * retBuffer, uint64_t retLen);

#endif /* alipay_blue_tcp_h */
