#include <stdbool.h>
#include <time.h>
#include "config.h"
#include "data.pb-c.h"
#include "alipay_net_kal.h"
#include "alipay_blue_tcp.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wreturn-local-addr"

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


#define printf(...)             a_printf(__VA_ARGS__)
#define print_r(...)            a_print_r(__VA_ARGS__)
#define AT(x)                   __attribute__((section(#x)))

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN
void a_printf(const char *format, ...);
void a_print_r(const void *buf, uint cnt);

AT(.alipay_data)
uint8_t alipay_iot_socket_rx_buf[5*1024];
uint16_t alipay_iot_socket_rx_len = 0;
volatile bool alipay_iot_socket_rx_sta = false;

void alipay_iot_socket_rx_sta_set(bool sta)
{
    alipay_iot_socket_rx_sta = sta;
}

bool alipay_iot_socket_rx_sta_get(void)
{
    return alipay_iot_socket_rx_sta;
}

void alipay_iot_socket_rx_callback(uint8_t *ptr, uint16_t len)
{
    static uint16_t w_ptr = 0;
//    alipay_iot_socket_rx_len = len - 1;

    memcpy(alipay_iot_socket_rx_buf + w_ptr, ptr, len - 1);
    w_ptr += len - 1;

//    printf("ptr[%d]:%d\n", len - 1, ptr[len - 1]);
    if (ptr[len - 1] == 0) {
        alipay_iot_socket_rx_sta_set(true);
        alipay_iot_socket_rx_len = w_ptr;
        if (alipay_iot_socket_rx_len > 1024 *5) {
            printf("rx buf overflow\n");
            while(1);
        }
        w_ptr = 0;
//        printf("%s tatol:%d\n", __func__, alipay_iot_socket_rx_len);
    } else if (ptr[len - 1] == 1){
//        w_ptr += len - 1;
    } else {
        printf("### RX ERROR!\n");
//        while(1);
    }
//    os_sem_sit_post();

//    alipay_iot_socket_rx_buf
}

uint8_t *alipay_iot_socket_rxbuf_get(uint16_t* len)
{
//    rx_buf = alipay_iot_socket_rx_buf;
    *len = alipay_iot_socket_rx_len;
//    printf("--->%s:%x, alipay_iot_socket_rx_len:%d, len:%d\n", __func__, alipay_iot_socket_rx_buf, alipay_iot_socket_rx_len, *len);
//    print_r(rx_buf, alipay_iot_socket_rx_len);
    return alipay_iot_socket_rx_buf;
}

/*
func： alipay_iot_socket_create
socket API 参数打包， 会调用 alipay_ble_send_to_mobile（统一的手表端发送接口，维护order）发送给手机端

@param      af:        socket 参数
@param      type:      socket 参数
@param      protocol:  socket 参数
@return     socket_id: 转化完成的socket fd对应的索引值
*/
int32_t alipay_iot_socket_create(int32_t af, alipay_socket_type_enum type, int32_t protocol){
    // 参数检查
    if(type != ALIPAY_IOT_SOC_SOCK_STREAM && type != ALIPAY_IOT_SOC_SOCK_DGRAM){
        return -1;
    }
    TRACE("### %s\n", __func__);
    // printf("    *    socket param | %d %d %d\n", af, type, protocol);
    Alipay__Socket socketSend = ALIPAY__SOCKET__INIT;
    socketSend.domain = af;
    socketSend.type = type;
    socketSend.protocol = protocol;

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    //   记得手动设置case
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_SOCKET_MES;
    packSend.socketmes = &socketSend;
    //模拟蓝牙发送
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    // 接收到 fd-buffer 解包获取ret
    int32_t client_fd = alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0).ret;

    // 转化存储 socket_fd
    int32_t socket_id = alloc_alipay_socket(client_fd);
    if (socket_id == -1) {
        // close(client_fd); // <unistd.h>
        return -1;
    }

    return socket_id;
}
//
//int32_t alipay_iot_socket_unpack(uint8_t *ptr, uint16_t len)
//{
//    // 接收到 fd-buffer 解包获取ret
//    int32_t client_fd = alipay_iot_ret_unpack(ptr, len, NULL, 0).ret;
//
//    // 转化存储 socket_fd
//    int32_t socket_id = alloc_alipay_socket(client_fd);
//    if (socket_id == -1) {
//        // close(client_fd); // <unistd.h>
//        return -1;
//    }
//    return socket_id;
//}



//// 尚未完整
//int32_t alipay_iot_bind(int32_t sockfd, const struct sockaddr *addr){
//    // printf("\n\n bind : %d %d %d %s", sockfd, addr->sa_family, addr->sa_len, addr->sa_data);
//
//    Alipay__Bind bindSend = ALIPAY__BIND__INIT;
//    bindSend.sockfd = sockfd;
//    bindSend.sa_family = addr->sa_family;
//    bindSend.sa_len = addr->sa_len;
//    bindSend.sa_data = alipay_iot_ble_malloc(addr->sa_len);
//    alipay_iot_memcpy(bindSend.sa_data, addr->sa_data, (sizeof(addr->sa_data) / sizeof(char)));
//
//    size_t len = alipay__bind__get_packed_size(&bindSend);
//    uint8_t * buffer = alipay_iot_ble_malloc(len);
//    alipay__bind__pack(&bindSend, buffer);
//
//    // send buffer and receive retValue
//    Alipay__Bind * msg = alipay__bind__unpack(NULL, len, buffer);
//    // printf("\n bind recv : %d %d %d %s", (int)msg->sockfd, (int)msg->sa_family, (int)msg->sa_len, msg->sa_data);
//    alipay__bind__free_unpacked(msg, NULL);
//    alipay_iot_std_free(buffer);
//
//    return 0;
//}


// uint32_t alipay_iot_connect(int sockfd, const struct sockaddr *addr, alipay_iot_socklen_t addrlen){
int32_t alipay_iot_socket_connect(int32_t socket_id, const struct alipay_iot_sockaddr *addr, alipay_iot_socklen_t addrlen){
    int sockfd = -1;
    sockfd = get_origin_fd_by_alipay_socket(socket_id);
    if (sockfd == -1) {
        return -1;
    }
    TRACE("### %s\n", __func__);
    TRACE("\n\n connect param   ｜  %d %d %d %s\n", sockfd, addr->sa_family, addr->sa_len, addr->data.sa_data);
    Alipay__Connect connectSend = ALIPAY__CONNECT__INIT;
    connectSend.sockfd = sockfd;
    connectSend.sa_family = addr->sa_family;
    connectSend.sa_len = addr->sa_len;
    unsigned char ip[4];
    alipay_iot_memcpy(ip, addr->data.sin_data.ip, 4);
    connectSend.ip = (uint32_t *)alipay_iot_ble_malloc(sizeof(uint32_t) * 4);
    connectSend.n_ip = 4;
    for(uint8_t i=0; i<4; i++){
        connectSend.ip[i] = ip[i];
    }

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_CONNECT_MES;
    packSend.connectmes = &connectSend;


    //模拟蓝牙发送
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);
    alipay_iot_std_free(connectSend.ip);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    // 接收解包获取ret
    return alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0).ret;
}


uint32_t alipay_iot_socket_recv(int32_t socket_id, void *buf, alipay_iot_size_t len, int32_t flags){
    // printf("\n recv pram : %d %d %d", socket_id, (int)len, flags);
    int sockfd = -1;
    sockfd = get_origin_fd_by_alipay_socket(socket_id);
    if (sockfd == -1) {
        return -1;
    }
    TRACE("### %s\n", __func__);
    Alipay__Recv recvSend = ALIPAY__RECV__INIT;
    recvSend.sockfd = sockfd;
    recvSend.len = len;
    recvSend.flags = flags;

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    // 记得手动设置case
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_RECV_MES;


    packSend.readmes = &recvSend;


    //模拟蓝牙发送, 存放返回的tcp_pack
    uint8_t* retBuffer = alipay_iot_ble_malloc(len + 1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    return alipay_iot_ret_unpack(retBuffer, retLen, buf, len).ret;
}

// 该函数没有实际用到，所以对于后面参数 src_addr 不做传出的处理
// 如果需要获取对方地址信息，初始化一个sockaddr struct空间，并告知存储内存的大小
// 不需要时，则为NULL
uint32_t alipay_iot_socket_recvfrom(int32_t socket_id, void *buf, alipay_iot_size_t len, int32_t flags, struct alipay_iot_sockaddr *src_addr, alipay_iot_socklen_t *addrlen){
    int sockfd = -1;
    sockfd = get_origin_fd_by_alipay_socket(socket_id);
    if (sockfd == -1) {
        return -1;
    }
//    printf("### %s\n", __func__);
//    printf("\n RecvFrom : %d %d %d ", sockfd, (int)len, flags);
    Alipay__RecvFrom recvFromSend = ALIPAY__RECV_FROM__INIT;
    recvFromSend.sockfd = sockfd;
    recvFromSend.lenbuf = len;
    recvFromSend.flags = flags;
    if(src_addr != NULL && addrlen != NULL){
        recvFromSend.addrlen = (int)(*addrlen);
    }

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    // 记得手动设置case
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_RECV_FROM_MES;
    packSend.readmes = &recvFromSend;

    //模拟蓝牙发送, 存放返回的tcp_pack
    uint8_t* retBuffer = alipay_iot_ble_malloc(len + 1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    RetValue ret = alipay_iot_ret_unpack(retBuffer, retLen, buf, len);

    if(src_addr != NULL){
        src_addr->sa_family = ret.recvFromRet.addr.sa_family;
        src_addr->sa_len = ret.recvFromRet.addr.sa_len;
        // 不能直接复制 此处sa_data 没有返回 后续需要补上
        if(1/*src_addr->data.sa_data!= NULL*/){
            alipay_iot_memcpy(src_addr->data.sa_data, ret.recvFromRet.addr.data.sa_data, sizeof(ret.recvFromRet.addr.data.sa_data));
        }else{
            // printf("--------2、 sa data is NULL \n");
        }
        // printf("get server's addr : %d %d %d \n", src_addr->sa_family, src_addr->sa_len, *addrlen);
    }
    return ret.recvFromRet.ret;
}

int32_t alipay_iot_socket_read(int32_t socket_id, void *buf, int32_t count){
    if(count <= 0){
        return -2;
    }
    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(socket_id);
    if (fd == -1) {
        return -1;
    }
    TRACE("### %s\n", __func__);
    // 为了和network层对齐，此处也调用recv来实现
    // 需要mobile端每次recv都带上fcntl参数信息
    // fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    // clock_t start, end;
    // start = clock();
    int n = alipay_iot_socket_recv(socket_id, buf, count, 0);
    // end = clock();
    // printf("retRead : %d ---  time=%f\n  ", n, (double)(end - start) / CLOCKS_PER_SEC);

/*
    // 直接调用read的代码块
    clock_t start, end;
    start = clock();
    int n = read(fd, buf, count);
    end = clock();
    printf("retRead : %d ---  time=%f\n  ", n, (double)(end - start) / CLOCKS_PER_SEC);
*/

/*
    //序列化、反序列化去调用read
    // printf("\n read  : %d %d ", fd, (int)count);
    Alipay__Read readSend = ALIPAY__READ__INIT;
    readSend.count = count;
    readSend.sockfd = fd;

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    // 记得手动设置case
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_READ_MES;
    packSend.readmes = &readSend;

    //模拟蓝牙发送
    uint8_t* retBuffer = alipay_iot_ble_malloc(count + 1024);
    uint64_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);

    int32_t n = alipay_iot_ret_unpack(retBuffer, retLen, buf, count).ret;
*/
    // printf("read api ret : %d\n", n);
    if (n <= 0 || n > count)
      return -2;

    return n;
}

uint32_t alipay_iot_send(int32_t socket_fd, const void *buf, size_t len, int32_t flags){
    int sockfd = -1;
    sockfd = get_origin_fd_by_alipay_socket(socket_fd);
    if (sockfd == -1) {
        return -1;
    }

    TRACE("### %s\n", __func__);
    Alipay__Send sendSend = ALIPAY__SEND__INIT;
    sendSend.sockfd = sockfd;
    sendSend.len = len;
    sendSend.flags = flags;
    sendSend.buf = alipay_iot_ble_malloc(len);
    alipay_iot_memcpy(sendSend.buf, buf, len);

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    //   记得手动设置case
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_SEND_MES;
    packSend.sendmes = &sendSend;

    //模拟蓝牙发送
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);
    alipay_iot_std_free(sendSend.buf);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    // 接收到 fd-buffer 解包获取ret
    // return alipay_iot_ret_4byte_unpack(retBuffer, retLen);
    return alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0).ret;
    // return alipay_iot_tcp_ret_unpack_func(alipay_iot_ret_4byte_unpack, retBuffer, retLen).ret;
}


// uint32_t alipay_iot_sendmsg(int32_t sockfd, const struct msghdr *msg, int32_t flags){
//     // printf("\n sendMsg : %d %d, %d, %d", sockfd, flags,  msg->msg_namelen,  msg->msg_controllen);
//     Alipay__Msg msgSend = ALIPAY__MSG__INIT;
//     if(msg->msg_name != NULL){
//         msgSend.msg_name = (char *)alipay_iot_ble_malloc(msg->msg_namelen);
//         strcpy(msgSend.msg_name, msg->msg_name);
//         msgSend.msg_namelen = msg->msg_namelen;
//     }else{
//         msgSend.msg_name = NULL;
//         msgSend.msg_namelen = 0;
//     }

//     msgSend.msg_iovlen = msg->msg_iovlen;
//     msgSend.msg_iov = alipay_iot_ble_malloc(sizeof(Alipay__Iovec *) * msg->msg_iovlen);
//     for(int i=0; i < msg->msg_iovlen; i++){
//         Alipay__Iovec iovecSend = ALIPAY__IOVEC__INIT;
//         iovecSend.iov_len = msg->msg_iov[i].iov_len;
//         iovecSend.iov_base = msg->msg_iov[i].iov_base; // need copy ?
//         msgSend.msg_iov[i] = &iovecSend;
//     }


//     if(msg->msg_control != NULL){
//         msgSend.msg_control = (char *)alipay_iot_ble_malloc(msg->msg_controllen);
//         strcpy(msgSend.msg_control, msg->msg_control);
//         msgSend.msg_controllen = msg->msg_controllen;
//     }else{
//         msgSend.msg_control = NULL;
//         msgSend.msg_controllen = 0;
//     }
//     msgSend.msg_flags = msg->msg_flags;

//     Alipay__SendMsg sendMsgSend = ALIPAY__SEND_MSG__INIT;
//     sendMsgSend.sockfd = sockfd;
//     sendMsgSend.flags = flags;
//     sendMsgSend.msg = &msgSend;


//     size_t lenBuffer = alipay__send_msg__get_packed_size(&sendMsgSend);
//     uint8_t* buffer = alipay_iot_ble_malloc(lenBuffer);
//     alipay__send_msg__pack(&sendMsgSend, buffer);

//     // 接收到后
//     Alipay__SendMsg * msgGet = alipay__send_msg__unpack(NULL, lenBuffer, buffer);
//     // printf("\n sendMsg : %d %d %d %d", (int)msgGet->sockfd, (int)msgGet->flags, (int)msgGet->msg->msg_namelen, (int)msgGet->msg->msg_controllen);
//     alipay__send_msg__free_unpacked(msgGet, NULL);
//     alipay_iot_std_free(buffer);


//     // 接收到 ret-buffer
//     // return alipay_iot_ret_unpack(buffer, len);

//     return 0;
// }

int32_t alipay_iot_socket_write(int32_t socket_fd, const void *buf, uint32_t nbyte){
    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(socket_fd);
    if (fd == -1) {
        return -1;
    }
    TRACE("### %s\n", __func__);
    Alipay__Write writeSend = ALIPAY__WRITE__INIT;
    writeSend.sockfd = fd;
    writeSend.nbyte = nbyte;

    writeSend.buffer.data = (uint8_t *)buf;
    writeSend.buffer.len = nbyte;

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    //   记得手动设置case
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_WRITE_MES;
    packSend.writemes = &writeSend;

    //模拟蓝牙发送
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    // 接收到 fd-buffer 解包获取ret
    return alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0).ret;
}


uint32_t alipay_iot_fcntl(int32_t s, int32_t cmd, int32_t val){
    printf("\n fcntl : %d %d %d ", s, cmd, val);
    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1) {
        return -1;
    }
    TRACE("### %s\n", __func__);
    Alipay__Fcntl fcntlSend = ALIPAY__FCNTL__INIT;
    fcntlSend.sockfd = fd;
    fcntlSend.cmd = cmd;
    fcntlSend.val = val;

     Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    //   记得手动设置case
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_FCNTL_MES;
    packSend.writemes = &fcntlSend;

    //模拟蓝牙发送
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    // 接收到 fd-buffer 解包获取ret
    return alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0).ret;

    // size_t len = alipay__fcntl__get_packed_size(&fcntlSend);
    // uint8_t* buffer = alipay_iot_ble_malloc(len);
    // alipay__fcntl__pack(&fcntlSend, buffer);

    // Alipay__Fcntl * msg = alipay__fcntl__unpack(NULL, len, buffer);
    // alipay__fcntl__free_unpacked(msg, NULL);
    // alipay_iot_std_free(buffer);

}


/*
函数名 ablue_ip_gethostbyname

请求数据格式
+-----------+-----------+
|   2BYTE   |   lenBYTE |
+-----------+-----------+
|  name_len |    name   |
+-----------+-----------+
返回数据格式
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   2BYTE   | lenBYTE   |  2BYTE    |  lenBYTE  |  4BYTE    |  2BYTE    |  lenBYTE  |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
| h_name_len|   h_name  |aliases_len| h_aliases | addrtype  | list_len  |h_addr_list|
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
*/

uint32_t * alipay_iot_gethostbyname(const char *name){
    TRACE("### %s\n", __func__);
    Alipay__Name nameSend = ALIPAY__NAME__INIT;
    nameSend.name = alipay_iot_ble_malloc(strlen(name) + 1);
    memset(nameSend.name, '\0', strlen(name));
    alipay_iot_memcpy(nameSend.name, name, strlen(name));
    nameSend.lenname = strlen(name);

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_NAME_MES;
    packSend.namemes = &nameSend;

    //模拟蓝牙发送 buffer长度可能不止1024
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);
    alipay_iot_std_free(nameSend.name);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    // 接收到 fd-buffer 解包获取ret
    // return alipay_iot_ret_4byte_unpack(retBuffer, retLen);

    return alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0).ip;


}

int32_t alipay_iot_dns(const char *name, unsigned char ip[4]){
    TRACE("### %s\n", __func__);
    Alipay__Name nameSend = ALIPAY__NAME__INIT;
    nameSend.name = alipay_iot_ble_malloc(strlen(name) + 1);
    memset(nameSend.name, '\0', strlen(name));
    alipay_iot_memcpy(nameSend.name, name, strlen(name));
    nameSend.lenname = strlen(name);

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_NAME_MES;
    packSend.namemes = &nameSend;

    print_r(nameSend.name, strlen(name) + 2);
    //模拟蓝牙发送 buffer长度可能不止1024
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);
    alipay_iot_std_free(nameSend.name);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);

    printf("### [%x, %x, %d]\n", retBuffer, &retBuffer, retLen);
    // 接收到 fd-buffer 解包获取ret
    RetValue ip_get = alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0);
    // 此处的ip返回copy到参数中
    for(int32_t i=0; i < 4; i++){
        ip[i] = ip_get.ip[i];
    }
/*
    ip[0] = 183;
    ip[1] = 2;
    ip[2] = 172;
    ip[3] = 42;
*/

    return 0;
}


uint32_t alipay_iot_select(int32_t nfds, alipay_iot_fd_set *readfds, alipay_iot_fd_set *writefds, alipay_iot_fd_set *exceptfds, struct alipay_iot_timeval *timeout){

//    alipay_iot_fd_set test_readfds;
//    printf("last entry:%x\n", __builtin_return_address(0));
    int origin_fd = -1;
    int origin_max = -1;

    if(nfds <= 0){
        printf("select - nfds error %d\n", nfds);
        return -1;
    }
    TRACE("### %s\n", __func__);
    Alipay__Select selectSend = ALIPAY__SELECT__INIT;
     TRACE("select nfds : %d\n", nfds); // 1

    // 将fd_set中的文件描述符添加到字段中
    // 发送的是真实的socket_fd
    for (int i = 0; i < nfds + 1; i++) {

        origin_fd = get_origin_fd_by_alipay_socket(i);
        if (origin_fd == -1) {
            TRACE("fail to get alipay sock %d origin sock\r\n", i);
            continue;
        }
         TRACE("nfds %d origin_fd %d\n", i, origin_fd);
        if (origin_fd > origin_max - 1) {
            origin_max = origin_fd + 1;
        }

        if(readfds == NULL && writefds == NULL && exceptfds == NULL) break;
        if (readfds != NULL && alipay_iot_fd_check(i, readfds)) {
             TRACE("\n 1、 read %d\n", get_origin_fd_by_alipay_socket(i));
            selectSend.readfds = alipay_iot_ble_realloc(selectSend.readfds, (selectSend.n_readfds + 1) * sizeof(uint32_t));
            // selectSend.readfds[selectSend.n_readfds++] = i;
            selectSend.readfds[selectSend.n_readfds++] = get_origin_fd_by_alipay_socket(i);
        }
        if (writefds != NULL && alipay_iot_fd_check(i, writefds)) {
             TRACE("\n 1、 write %d, selectSend.writefds:%x\n", get_origin_fd_by_alipay_socket(i), selectSend.writefds);
            selectSend.writefds = alipay_iot_ble_realloc(selectSend.writefds, (selectSend.n_writefds + 1) * sizeof(int32_t));
            selectSend.writefds[selectSend.n_writefds++] = get_origin_fd_by_alipay_socket(i);
        }
        if (exceptfds != NULL && alipay_iot_fd_check(i, exceptfds)) {
             TRACE("\n 1、 except %d\n", get_origin_fd_by_alipay_socket(i));
            selectSend.exceptfds = alipay_iot_ble_realloc(selectSend.exceptfds, (selectSend.n_exceptfds + 1) * sizeof(int32_t));
            selectSend.exceptfds[selectSend.n_exceptfds++] = get_origin_fd_by_alipay_socket(i);
        }
    }

    if(origin_max <= 0){
        return -1;
    }
    selectSend.nfds = origin_max;

    if(timeout != NULL){
        selectSend.tv_sec = timeout->tv_sec;
        selectSend.tv_usec = timeout->tv_usec;
    }else{
        selectSend.tv_sec = 0;
        selectSend.tv_usec = 0;
    }

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    //   记得手动设置case
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_SELECT_MES;
    packSend.selectmes = &selectSend;

    //模拟蓝牙发送
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);
    if (selectSend.readfds) {
        alipay_iot_std_free(selectSend.readfds);
    }

    if (selectSend.writefds) {
        alipay_iot_std_free(selectSend.writefds);
    }

    if (selectSend.exceptfds) {
        alipay_iot_std_free(selectSend.exceptfds);
    }

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    // 接收到 fd-buffer 解包获取ret
    RetValue ret = alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0);


    if(ret.selectRet.ret == 0){
        return ret.selectRet.ret;
    }

//    print_r(ret, sizeof(RetValue));
//    printf("fds_use:%d\n", ret.selectRet.n_read_fds_use);
    for(int i = 0; i < ret.selectRet.n_read_fds_use; i++){
        int socket_id = get_alipay_socket_by_origin_fd((int)(ret.selectRet.read_fds[i]));
        TRACE("select get res - readfds: %d socket id: %d, readfds:%x, n_read_fds_use:%d\n", ret.selectRet.read_fds[i], socket_id, readfds, ret.selectRet.n_read_fds_use);

//        print_r((uint8_t *)readfds, 16);
        alipay_iot_fd_setResultbit(socket_id, readfds);
    }

    for(int i = 0; i < ret.selectRet.n_write_fds_use; i++){
        int socket_id = get_alipay_socket_by_origin_fd((int)(ret.selectRet.write_fds[i]));
//         TRACE("select get res - writefds: %d socket id: %d\n", ret.selectRet.write_fds[i], socket_id);
        alipay_iot_fd_setResultbit(socket_id, writefds);
    }

    for(int i = 0; i < ret.selectRet.n_except_fds_use; i++){
        int socket_id = get_alipay_socket_by_origin_fd((int)(ret.selectRet.except_fds[i]));
         TRACE("select get res - except_fds: %d socket id: %d\n", ret.selectRet.except_fds[i], socket_id);
        alipay_iot_fd_setResultbit(socket_id, exceptfds);
    }
    return ret.selectRet.ret;
}


int32_t alipay_iot_socket_close(int32_t socket_fd){
    int fd = -1;
    int ret  = -1;
    fd = get_origin_fd_by_alipay_socket(socket_fd);
        if (fd == -1) {
            return -1;
    }
    TRACE("### %s\n", __func__);
    TRACE("\n\n close param   ｜  %d \n", fd);
    Alipay__Close closeSend = ALIPAY__CLOSE__INIT;
    closeSend.fd = fd;

    Alipay__TCPPackageIotSend packSend = ALIPAY__TCPPACKAGE_IOT_SEND__INIT;
    packSend.body_case = ALIPAY__TCPPACKAGE_IOT_SEND__BODY_CLOSE_MES;
    packSend.closemes = &closeSend;

    //模拟蓝牙发送
    uint8_t* retBuffer = alipay_iot_ble_malloc(1024);
    uint16_t retLen = 0;
    alipay_ble_send_to_mobile(&packSend, retBuffer, &retLen);
    alipay_iot_std_free(retBuffer);

    retBuffer = alipay_iot_socket_rxbuf_get(&retLen);
    // 接收解包获取ret
    ret = alipay_iot_ret_unpack(retBuffer, retLen, NULL, 0).ret;

    ret |= free_alipay_socket(socket_fd);
    if (ret != 0) {
//        printf("close socket %d origin is %d ret %d", s, fd, ret);
        return -1;
    }
    return ret;
}


uint32_t alipay_iot_getsockopt(int sockfd, int level, int optname, void *optval/*, socklen_t *optlen*/){
    TRACE("\n get Sock opt: %d %d %d", sockfd, level, optname);
    // Alipay__GetSockopt getSockoptSend = ALIPAY__GET_SOCKOPT__INIT;
    // getSockoptSend.sockfd = sockfd;
    // getSockoptSend.level = level;
    // getSockoptSend.optname = optname;

    // size_t len = alipay__get_sockopt__get_packed_size(&getSockoptSend);
    // uint8_t* buffer = malloc(len);
    // alipay__get_sockopt__pack(&getSockoptSend, buffer);

    // Alipay__GetSockopt * msg = alipay__get_sockopt__unpack(NULL, len, buffer);
    // printf("\n get Sock opt: %d %d %d", msg->sockfd, msg->level, msg->optname);
    // alipay__get_sockopt__free_unpacked(msg, NULL);
    // free(buffer);

    // 接收到 ret
//    return alipay_getsockopt_res_unpack(buffer, len)->ret;

    // 空处理
    TRACE("### %s\n", __func__);
    return 0;
}


getSockoptRes * alipay_iot_getsockopt_res_unpack(uint8_t * buffer, size_t len){
    Alipay__GetSockoptRes * msg = alipay__get_sockopt_res__unpack(NULL, len, buffer);
    if(msg->ret == -1) return NULL;
    TRACE("### %s\n", __func__);
    static getSockoptRes res;
    res.ret = msg->ret;
    res.sockfd = msg->sockfd;
    res.optname = msg->optname;
    res.optlen = msg->optlen;
    char optval[res.optlen * sizeof(char)];
//    res.optval = (char *)alipay_iot_ble_malloc(res.optlen * sizeof(char));
    res.optval = optval;
    strcpy(res.optval, msg->optval);

    return &res;
}

uint32_t alipay_iot_setsockopt(int sockfd, int level, int optname, const void *optval/*, socklen_t optlen*/){
    TRACE("\n Set Sock opt: %d %d %d %d", 0, level, optname, 0);
    // Alipay__SetSockopt setSockoptSend = ALIPAY__SET_SOCKOPT__INIT;
    // setSockoptSend.sockfd = sockfd;
    // setSockoptSend.level = level;
    // setSockoptSend.optname = optname;
    // setSockoptSend.optlen = optlen;
    // setSockoptSend.optval = malloc(optlen);
    // strcpy(setSockoptSend.optval, optval);

    // size_t len = alipay__set_sockopt__get_packed_size(&setSockoptSend);
    // uint8_t* buffer = malloc(len);
    // alipay__set_sockopt__pack(&setSockoptSend, buffer);

    // Alipay__SetSockopt * msg = alipay__set_sockopt__unpack(NULL, len, buffer);
    // printf("\n Set Sock opt: %d %d %d %d", msg->sockfd, msg->level, msg->optname, msg->optlen);
    // alipay__set_sockopt__free_unpacked(msg, NULL);
    // free(buffer);

    // 接收到 ret
    // return alipay_iot_ret_unpack(buffer, len);
    TRACE("### %s\n", __func__);
    // 空处理
    return 0;
}


int8_t alipay_iot_socket_setsockopt(int          s,
                          int          level,
                          int          optname,
                          const void  *opval,
                          unsigned int optlen) {
//    #warning alipay_iot_socket_setsockopt demo
    return 0;
}

int8_t alipay_iot_socket_getsockopt(int         s,
                          int            level,
                          int          optname,
                          void          *opval,
                          unsigned int *optlen) {
//    #warning alipay_iot_socket_getsockopt demo
    return 0;
}


int alipay_iot_socket_bind(int s, const struct alipay_iot_sockaddr *name, unsigned int namelen)
{
    return 0;
}

int alipay_iot_socket_sendto(int                        s,
                             const void                *dataptr,
                             size_t                     size,
                             int                        flags,
                             const struct alipay_iot_sockaddr *to,
                             unsigned int               tolen)
{
    return 0;
}

#endif
#pragma GCC diagnostic pop

