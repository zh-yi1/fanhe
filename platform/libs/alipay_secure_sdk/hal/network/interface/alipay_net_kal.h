#ifndef ALIPAY_IOT_KAL_H
#define ALIPAY_IOT_KAL_H

#include "alipay_iot_type.h"
#include "iot_err.h"

typedef unsigned int alipay_iot_socklen_t;
typedef long unsigned int alipay_iot_size_t;


#ifdef __LINUX__
#include <pthread.h>
typedef pthread_mutex_t alipay_iot_mutex;

inline alipay_iot_mutex alipay_iot_mutex_create(const char* mutex_name{
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    return mutex;
}

inline int alipay_iot_mutex_delete(alipay_iot_mutex mutex) {
    pthread_mutex_destroy(&mutex);
    return 0;
}

inline int alipay_iot_mutex_lock(alipay_iot_mutex mutex{
    pthread_mutex_lock(&mutex);
    return 0;
}


inline int alipay_iot_mutex_unlock(alipay_iot_mutex mutex){
    pthread_mutex_unlock(&mutex);
    return 0;
}

#else


typedef void * alipay_iot_mutex;
/*
* @brief: 创建互斥量
* @input: mutex_name 互斥量的名称
* @return: 其他 创建成功
*          NULL 创建失败
*/
extern alipay_iot_mutex alipay_iot_mutex_create(const char* mutex_name);

/*
* @brief: 对互斥量加锁
* @input: mutex 要加锁的互斥量
* @return: 0 加锁成功
*          -1 加锁失败
*/
extern int alipay_iot_mutex_lock(alipay_iot_mutex mutex);

/*
* @brief: 对互斥量解锁
* @input: mutex 要解锁的互斥量
* @return: 0 解锁成功
*          -1 解锁失败
*/
extern int alipay_iot_mutex_unlock(alipay_iot_mutex mutex);

/*
* @brief: 删除互斥量
* @input: mutex 要删除的互斥量
* @return: 0 删除成功
*          -1 删除失败
*/
extern int alipay_iot_mutex_delete(alipay_iot_mutex mutex);

#endif


#ifndef __FILE__
#define __FILE__    ""
#endif
#ifndef __LINE__
#define __LINE__    ""
#endif
 /*
 * @brief: ���ֵȼ�����־���?
 * @input: format ��־��ʽ
 * @input: ... ��format��Ӧ�Ĳ���
 * @note:  ÿ����־�Զ����У�����ĸ�ʽ�?: ʱ�� �ļ��� ���� ��־�ȼ� ��־���ݣ���: 2018-08-18 18:06:09|main.c|69|INFO| hello world
 */
#define ALIPAY_IOT_LOG_VERBOSE(tag,format,...)      alipay_iot_log(__FILE__, __LINE__, ALIPAY_IOT_LOG_LVL_VBS, tag, format, ##__VA_ARGS__)
#define ALIPAY_IOT_LOG_DEBUG(tag,format,...)        alipay_iot_log(__FILE__, __LINE__, ALIPAY_IOT_LOG_LVL_DBG, tag, format, ##__VA_ARGS__)
#define ALIPAY_IOT_LOG_INFO(tag,format,...)        alipay_iot_log(__FILE__, __LINE__, ALIPAY_IOT_LOG_LVL_INF, tag, format, ##__VA_ARGS__)
#define ALIPAY_IOT_LOG_WARN(tag,format,...)        alipay_iot_log(__FILE__, __LINE__, ALIPAY_IOT_LOG_LVL_WRN, tag, format, ##__VA_ARGS__)
#define ALIPAY_IOT_LOG_ERROR(tag,format,...)        alipay_iot_log(__FILE__, __LINE__, ALIPAY_IOT_LOG_LVL_ERR, tag, format, ##__VA_ARGS__)
#define ALIPAY_IOT_LOG_FATAL(tag,format,...)        alipay_iot_log(__FILE__, __LINE__, ALIPAY_IOT_LOG_LVL_FTL, tag, format, ##__VA_ARGS__)

// #define ALIPAY_IOT_LOG_VERBOSE(tag,format,...)      LOG_DBG(format, ##__VA_ARGS__)
// #define ALIPAY_IOT_LOG_DEBUG(tag,format,...)       LOG_DBG(format, ##__VA_ARGS__)
// #define ALIPAY_IOT_LOG_INFO(tag,format,...)        LOG_INFO(format, ##__VA_ARGS__)
// #define ALIPAY_IOT_LOG_WARN(tag,format,...)        LOG_WARN(format, ##__VA_ARGS__)
// #define ALIPAY_IOT_LOG_ERROR(tag,format,...)        LOG_ERROR(format, ##__VA_ARGS__)
// #define ALIPAY_IOT_LOG_FATAL(tag,format,...)       LOG_FATAL(format, ##__VA_ARGS__)

/*
 * Protocol Familty.  Currently, we only support Internet
 */
#define ALIPAY_IOT_SOC_PF_INET (0)
#define ALIPAY_IOT_SOC_AF_INET (2)

/* Socket Type */
typedef enum
{
    ALIPAY_IOT_SOC_SOCK_STREAM = 0,      /* stream socket, TCP */
    ALIPAY_IOT_SOC_SOCK_DGRAM = 1,           /* datagram socket, UDP */
}alipay_socket_type_enum;

/* Socket Options */
typedef enum
{
    ALIPAY_IOT_SOC_OOBINLINE     = 0x01 << 0,  /* not support yet */
    ALIPAY_IOT_SOC_LINGER        = 0x01 << 1,  /* linger on close */
    ALIPAY_IOT_SOC_NBIO          = 0x01 << 2,  /* Nonblocking */
    ALIPAY_IOT_SOC_ASYNC         = 0x01 << 3,  /* Asynchronous notification */

    ALIPAY_IOT_SOC_NODELAY       = 0x01 << 4,  /* disable Nagle algorithm or not */
    ALIPAY_IOT_SOC_KEEPALIVE     = 0x01 << 5,  /* enable/disable the keepalive */
    ALIPAY_IOT_SOC_RCVBUF        = 0x01 << 6,  /* set the socket receive buffer size */
    ALIPAY_IOT_SOC_SENDBUF       = 0x01 << 7,  /* set the socket send buffer size */
}alipay_soc_option_enum;

#define ALIPAY_IOT_MAX_IP_SOCKET_NUM   1  //根据实际情况修改，最多16

typedef struct
{
    uint8_t fds_bits[ALIPAY_IOT_MAX_IP_SOCKET_NUM];
} alipay_iot_fd_set;

void alipay_iot_fd_zero(alipay_iot_fd_set* fdset);
void alipay_iot_fd_setbit(int fd, alipay_iot_fd_set* fdset);
/*
* 检查fdset中对应的fd是否有被selected
*/
int  alipay_iot_fd_isset(int fd, alipay_iot_fd_set* fdset);


struct alipay_iot_sockaddr {
  unsigned char sa_len;      //此值忽略。
  unsigned char sa_family;   //目前只支持ALIPAY_IOT_SOC_PF_INET。
  union {
    struct {
        unsigned short port;  //网络字节序
        unsigned char  ip[4];
    }    sin_data;
    char sa_data[14];
  } data;
};

struct alipay_iot_hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#define	h_addr	h_addr_list[0]	/* address, for backward compatibility */
#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
};

alipay_iot_mutex get_order_mtx();
alipay_iot_mutex get_socket_mtx();

int alipay_iot_fd_init();

int alloc_alipay_socket(int origin_fd);

int free_alipay_socket(int s);

int get_origin_fd_by_alipay_socket(int socket);

int get_alipay_socket_by_origin_fd(int fd);

void alipay_iot_fd_setResultbit(int fd, alipay_iot_fd_set* fdset);

int alipay_iot_fd_check(int fd, alipay_iot_fd_set* fdset);

int alipay_iot_fd_isset(int fd, alipay_iot_fd_set* fdset);

void alipay_iot_fd_setbit(int fd, alipay_iot_fd_set* fdset);

void alipay_iot_fd_zero(alipay_iot_fd_set* fdset);

/*********************     ALIPAY_BLUE.C   Socket API     *******************************/

// #include <sys/socket.h>     // struct msghdr (sendMsg API)
#include "alipay_net_kal.h"
#include "data.pb-c.h"


/*
函数名 alipay_iot_ip_socket

请求数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |   4BYTE   |
+-----------+-----------+-----------+
|   domain  |   type    |  protocol |
+-----------+-----------+-----------+
返回数据格式
+-----------+
|   4BYTE   |
+-----------+
|     fd    |
+-----------+
*/
/**
 * 创建套接口。 存储在自定义的KV中 " socket_id : fd ", 注意转化关系
 * domain目前只支持ALIPAY_IOT_SOC_PF_INET
 * type为SOC_SOCK_STREAM表示TCP，为SOC_SOCK_DGRAM表示UDP。
 * protocol填0，除非type填了SOC_SOCK_RAW，则要指定如下之一：
 *     SOC_IPPROTO_IP
 *     SOC_IPPROTO_HOPOPTS
 *     SOC_IPPROTO_ICMP
 *     SOC_IPPROTO_IGMP
 *     SOC_IPPROTO_IPV4
 *     SOC_IPPROTO_IPIP
 *     SOC_IPPROTO_TCP
 *     SOC_IPPROTO_UDP
 *     SOC_IPPROTO_ESP
 *     SOC_IPPROTO_AH
 *     SOC_IPPROTO_IPCOMP
 *     SOC_IPPROTO_RAW
 *     SOC_IPPROTO_DONE
 * 返回-1表示失败，返回其他为套接口的描述符。
 */
#define ALIPAY_IOT_SOCKET_TYPE_SOCK_STREAM
//int32_t alipay_iot_socket(int32_t af, int32_t type, int32_t protocol);
int32_t alipay_iot_socket_create(int32_t af, alipay_socket_type_enum type, int32_t protocol);


/*
函数名 ablue_ip_bind

请求数据格式
+-----------+-----------+-----------+-----------+
|   4BYTE   |   1BYTE   |   1BYTE   |sa_lenBYTE |
+-----------+-----------+-----------+-----------+
|   sockfd  | sa_famlily|   sa_len  |  sa_data  |
+-----------+-----------+-----------+-----------+
返回数据格式
+-----------+
|   4BYTE   |
+-----------+
|     fd    |
+-----------+
*/
//没有维护
// int32_t alipay_iot_bind(int32_t sockfd, const struct sockaddr *addr, socklen_t addrlen);

/*
函数名 ablue_ip_connect

请求数据格式
+-----------+-----------+-----------+-----------+
|   4BYTE   |   1BYTE   |   1BYTE   | sa_lenBYTE|
+-----------+-----------+-----------+-----------+
|   sockfd  | sa_family |   sa_len  |  sa_data  |
+-----------+-----------+-----------+-----------+
返回数据格式
+-----------+
|   4BYTE   |
+-----------+
|     ret   |
+-----------+
*/
/**
 * 若是TCP套接口，则向指定地址发起连接；
 * 若是UDP套接口，则设置缺省的对端地址。
 * name->sa_len无效，
 * name->sa_family必须是SOC_PF_INET，
 * name->data.sin_data.port和name->data.sin_data.ip分别填入端口号和IPv4地址
 * namelen无效。
 * 返回-1表示失败，返回-2表示异步操作正在进行中，返回0表示成功。不会返回别的值。
 */
int32_t alipay_iot_socket_connect(int32_t sockfd, const struct alipay_iot_sockaddr *addr, alipay_iot_socklen_t addrlen);
/*
函数名 ablue_ip_recv

usage: 从已连接的套接字读取数据
请求数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |   4BYTE   |
+-----------+-----------+-----------+
|   sockfd  |    len    |  flags    |
+-----------+-----------+-----------+
返回数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |  lenBYTE  |
+-----------+-----------+-----------+
|     ret   |   len     |   buf     |
+-----------+-----------+-----------+
*/
uint32_t alipay_iot_socket_recv(int32_t socket_id, void *buf, alipay_iot_size_t len, int32_t flags);


/*
函数名 ablue_ip_recvfrom

请求数据格式
+-----------+-----------+-----------+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |   4BYTE   |   2BYTE   |   1BYTE   |   14BYTE  |
+-----------+-----------+-----------+-----------+-----------+-----------+
|   sockfd  |    len    |  flags    | sa_family |   sa_len  |  sa_data  |
+-----------+-----------+-----------+-----------+-----------+-----------+
返回数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |  lenBYTE  |
+-----------+-----------+-----------+
|     ret   |   len     |   buf     |
+-----------+-----------+-----------+
*/
uint32_t alipay_iot_socket_recvfrom(int32_t sockfd, void *buf, alipay_iot_size_t len, int32_t flags, struct alipay_iot_sockaddr *src_addr, alipay_iot_socklen_t *addrlen);


/*
函数名 ablue_ip_read

请求数据格式
+-----------+-----------+
|   4BYTE   |   4BYTE   |
+-----------+-----------+
|   sockfd  |    count  |
+-----------+-----------+
返回数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |  lenBYTE  |
+-----------+-----------+-----------+
|     ret   |   len     |   buf     |
+-----------+-----------+-----------+
*/
int32_t alipay_iot_socket_read(int32_t fd, void *buf, int32_t count);


/*
函数名 ablue_ip_read

请求数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |   4BYTE   |
+-----------+-----------+-----------+
|   sockfd  |    flags  |    len    |
+-----------+-----------+-----------+
返回数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |  lenBYTE  |
+-----------+-----------+-----------+
|     ret   |   len     |   buf     |
+-----------+-----------+-----------+
*/
/**
 * sendto 可以携带对方地址，作用在于建立socket链接，并发送信息
 * send   不携带地址信息，因此需要提前通过socket函数，建立连接，才可以发送信息
*/
uint32_t alipay_iot_send(int32_t socket_fd, const void *buf, size_t len, int32_t flags);

/*
函数名 ablue_ip_sendmsg

请求数据格式
+-----------+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |   4BYTE   |  lenBYTE  |
+-----------+-----------+-----------+-----------+
|   sockfd  |    flags  |    len    |   msg     |
+-----------+-----------+-----------+-----------+
返回数据格式
+-----------+
|   4BYTE   |
+-----------+
|     ret   |
+-----------+
*/
// uint32_t alipay_iot_sendmsg(int32_t sockfd, const struct msghdr *msg, int32_t flags);


/*
函数名 ablue_ip_write

返回数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |  lenBYTE  |
+-----------+-----------+-----------+
|     ret   |   len     |   buf     |
+-----------+-----------+-----------+
*/
int32_t alipay_iot_socket_write(int32_t fd, const void *buf, uint32_t nbyte);


/*
函数名 ablue_ip_fcntl

请求数据格式
+-----------+-----------+-----------+-----------+
|   4BYTE   |           |   4BYTE   |   4BYTE   |
+-----------+-----------+-----------+-----------+
|   sockfd  |           |    cmd    |    val    |
+-----------+-----------+-----------+-----------+
返回数据格式
+-----------+
|   4BYTE   |
+-----------+
|     ret   |
+-----------+
*/
uint32_t alipay_iot_fcntl(int32_t s, int32_t cmd, int32_t val);


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

// struct alipay_iot_hostent * alipay_iot_gethostbyname(const char *name);
uint32_t * alipay_iot_gethostbyname(const char *name);
int32_t alipay_iot_dns(const char *name, unsigned char ip[4]);

// struct alipay_iot_hostent * alipay_iot_hostent_unpack(uint8_t * buffer, size_t len);

/*
函数名 ablue_ip_select

请求数据格式
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   4BYTE   |   1BYTE   |   1BYTE   |   1BYTE   |   4BYTE   |   4BYTE   |rsetlenBYTE|wsetlenBYTE|esetlenBYTE|
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   maxfd   |  rset_len | wset_len  | exset_len | timeo_s   | timeo_ms  |  readfds  | writefds  | exceptfds |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
返回数据格式
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   4BYTE   |   1BYTE   |   1BYTE   |   1BYTE   |rsetlenBYTE|wsetlenBYTE|esetlenBYTE|
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   ret     |  rset_len | wset_len  | exset_len |  readfds  | writefds  | exceptfds |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
*/

// uint32_t alipay_iot_select(int32_t nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct alipay_iot_timeval *timeout);
uint32_t alipay_iot_select(int32_t nfds, alipay_iot_fd_set *readfds, alipay_iot_fd_set *writefds, alipay_iot_fd_set *exceptfds, struct alipay_iot_timeval *timeout);
// typedef struct selectRes{
//     uint32_t ret;
//     fd_set readfds;
//     fd_set writefds;
//     fd_set exceptfds;
//     uint32_t n_readfds;
//     uint32_t n_writefds;
//     uint32_t n_exceptfds;
// }selectRes;

// selectRes * alipay_iot_select_res_unpack(uint8_t * buffer, size_t len);


/*
函数名 ablue_ip_getsockopt

请求数据格式
+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |   4BYTE   |
+-----------+-----------+-----------+
|   sockfd  |   level   |  optname  |
+-----------+-----------+-----------+

返回数据格式
+-----------+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |   nBYTE   |   4BYTE   |
+-----------+-----------+-----------+-----------+
|     fd    |  optname  |   optval  |  optlen   |
+-----------+-----------+-----------+---------------+
            |  SO_ACCEPTCONN    |      1BYTE        |
            +-------------------+-------------------+
            |  SO_BROADCAST     |      1BYTE        |
            +-------------------+-------------------+
            |  SO_DEBUG         |      1BYTE        |
            +-------------------+-------------------+
            |  SO_DONTLINER     |      1BYTE        |
            +-------------------+-------------------+
            |  SO_DONTROUTE     |      1BYTE        |
            +-------------------+-------------------+
            |  SO_ERROR         |      1BYTE        |
            +-------------------+-------------------+
            |  SO_KEEPALIVE     |      1BYTE        |
            +-------------------+-------------------+-------------------+
            |  SO_LINGER        |  l_onoff(4BYTE)   | l_linger(4BYTE)   |
            +-------------------+-------------------+-------------------+
            |  SO_OOBINLINE     |      1BYTE        |
            +-------------------+-------------------+
            |  SO_RCVBUF        |      4BYTE        |
            +-------------------+-------------------+
            |  SO_TYPE          |      4BYTE        |
            +-------------------+-------------------+-------------------+
            |  SO_SNDTIMEO      |  tv_sec(4BYTE)    |  tv_usec(4BYTE)   |
            +-------------------+-------------------+-------------------+
            |  SO_RCVTIMEO      |  tv_sec(4BYTE)    |  tv_usec(4BYTE)   |
            +-------------------+-------------------+-------------------+
            |  SO_SNDBUF        |      4BYTE        |
            +-------------------+-------------------+
            |  SO_RECBUF        |      4BYTE        |
            +-------------------+-------------------+
            |  SO_SNDLOWAT      |      4BYTE        |
            +-------------------+-------------------+
            |  SO_RCVLOWAT      |      4BYTE        |
            +-------------------+-------------------+
            |  SO_BROADCAST     |      4BYTE        |
            +-------------------+-------------------+
*/
uint32_t alipay_iot_getsockopt(int sockfd, int level, int optname, void *optval/*, alipay_iot_socklen_t *optlen*/);

typedef struct getSockoptRes{
    uint32_t ret;
    uint32_t sockfd;
    uint32_t optname;
    char * optval;
    uint32_t optlen;
}getSockoptRes;

getSockoptRes * alipay_iot_getsockopt_res_unpack(uint8_t * buffer, size_t len);


/*
函数名 ablue_ip_setsockopt

请求数据格式,otpval参考ablue_ip_getsockopt中注释
+-----------+-----------+-----------+-----------+
|   4BYTE   |   4BYTE   |   4BYTE   |  nBYTE    |
+-----------+-----------+-----------+-----------+
|   sockfd  |   level   |  optname  |  optval   |
+-----------+-----------+-----------+-----------+
返回数据格式
+-----------+
|   4BYTE   |
+-----------+
|     ret   |
+-----------+
*/
uint32_t alipay_iot_setsockopt(int sockfd, int level, int optname, const void *optval/*, alipay_iot_socklen_t optlen*/);



/*
函数名 ablue_ip_getaddrinfo

请求数据格式
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   4BYTE   |hostlenBYTE|   4BYTE   |serlenBYTE |   4BYTE   |   4BYTE   |   4BYTE   |   4BYTE   |   2BYTE   |  1BYTE    |sadalenBYTE|  1BYTE    |ca_lenBYTE |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   hostlen |    host   | servicelen| service   | ai_flags  | ai_family |ai_socktype|ai_protocol| sa_family |sa_data_len|  sa_data  | ai_ca_len |ai_canonnm |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
返回数据格式
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|   4BYTE   |   1BYTE   |   4BYTE   |  4BYTE    |  4BYTE    |  4BYTE    |   2BYTE   |  1BYTE    |sadalenBYTE|  1BYTE    |ca_lenBYTE |  next     |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|     ret   | res_count | ai_flags  | ai_family |ai_socktype|ai_protocol| sa_family |sa_data_len|  sa_data  | ai_ca_len |ai_canonnm |  next     |
+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
*/
//int alipay_iot_getaddrinfo(const char *node, const char *service,const struct addrinfo *hints,struct addrinfo **res);

/*
函数名 close
*/
int32_t alipay_iot_socket_close(int32_t fd);

/**
 * 设置套接口的参数。
 * level无效。
 * optname取值如下之一：
 *     SOC_OOBINLINE，目前不支持
 *     SOC_LINGER，opval指向soc_linger_struct
 *     SOC_NBIO，opval指向kal_bool
 *     SOC_ASYNC，opval指向kal_uint8
 *     SOC_NODELAY，opval指向kal_bool
 *     SOC_KEEPALIVE，opval指向kal_bool
 *     SOC_RCVBUF，opval指向kal_uint32
 *     SOC_SENDBUF，opval指向kal_uint32
 *     SOC_PKT_SIZE，opval指向kal_uint32
 *     SOC_SILENT_LISTEN，opval指向kal_bool
 *     SOC_QOS，opval指向kal_uint8
 *     SOC_TCP_MAXSEG，opval指向kal_uint8
 *     SOC_IP_TTL，opval指向kal_uint8
 *     SOC_LISTEN_BEARER，opval指向kal_uint8
 *     SOC_UDP_ANY_FPORT，opval指向kal_bool
 *     SOC_WIFI_NOWAKEUP，opval指向kal_bool
 *     SOC_UDP_NEED_ICMP，opval指向kal_bool
 *     SOC_IP_HDRINCL，opval指向kal_bool
 *     SOC_IPSEC_POLICY，opval指向sadb_x_policy
 *     SOC_TCP_ACKED_DATA，opval指向kal_uint32
 *     SOC_TCP_DELAYED_ACK，opval指向kal_bool
 *     SOC_TCP_SACK，opval指向kal_bool
 *     SOC_TCP_TIME_STAMP，opval指向kal_bool
 *     SOC_TCP_ACK_MSEG，opval指向kal_bool
 * 返回-1表示失败，返回0表示成功。不会返回别的值。
 */
int8_t alipay_iot_socket_setsockopt(int          s,
                          int          level,
                          int          optname,
                          const void  *opval,
                          unsigned int optlen);

/**
 * 获取套接口的参数。
 * level无效。
 * optname取值如下之一：
 *     SOC_OOBINLINE，目前不支持
 *     SOC_LINGER，opval指向soc_linger_struct
 *     SOC_NBIO，opval指向kal_bool
 *     SOC_ASYNC，opval指向kal_uint8
 *     SOC_NODELAY，opval指向kal_bool
 *     SOC_KEEPALIVE，opval指向kal_bool
 *     SOC_RCVBUF，opval指向kal_uint32
 *     SOC_SENDBUF，opval指向kal_uint32
 *     SOC_NREAD，opval指向kal_uint32
 *     SOC_PKT_SIZE，opval指向kal_uint32
 *     SOC_SILENT_LISTEN，opval指向kal_bool
 *     SOC_QOS，opval指向kal_uint8
 *     SOC_TCP_MAXSEG，opval指向kal_uint8
 *     SOC_IP_TTL，opval指向kal_uint8
 *     SOC_LISTEN_BEARER，opval指向kal_uint8
 *     SOC_UDP_ANY_FPORT，opval指向kal_bool
 *     SOC_WIFI_NOWAKEUP，opval指向kal_bool
 *     SOC_UDP_NEED_ICMP，opval指向kal_bool
 *     SOC_IP_HDRINCL，opval指向kal_bool
 *     SOC_IPSEC_POLICY，opval指向sadb_x_policy
 *     SOC_TCP_ACKED_DATA，opval指向kal_uint32
 *     SOC_TCP_DELAYED_ACK，opval指向kal_bool
 *     SOC_TCP_SACK，opval指向kal_bool
 *     SOC_TCP_TIME_STAMP，opval指向kal_bool
 *     SOC_TCP_ACK_MSEG，opval指向kal_bool
 * 返回-1表示失败，返回0表示成功。不会返回别的值。
 */
int8_t alipay_iot_socket_getsockopt(int         s,
                          int            level,
                          int          optname,
                          void          *opval,
                          unsigned int *optlen);



int alipay_iot_socket_sendto(int                        s,
                             const void                *dataptr,
                             size_t                     size,
                             int                        flags,
                             const struct alipay_iot_sockaddr *to,
                             unsigned int               tolen);

int alipay_iot_socket_bind(int s, const struct alipay_iot_sockaddr *name, unsigned int namelen);

/****************** ALIPAY_BLE_OS.C   API **************************/

#include "alipay_iot_type.h"

#ifdef __LINUX__
#include <stdlib.h>

#define alipay_iot_ble_malloc malloc
#define alipay_iot_ble_realloc realloc
#define alipay_iot_std_free free

#else

void *alipay_iot_ble_malloc(size_t size);

void *alipay_iot_ble_realloc(void *ptr, size_t new_size);

void alipay_iot_std_free(void *ptr);
#endif

#define alipay_iot_free(ptr) do{\
	if(ptr != NULL)\
	{\
		alipay_iot_std_free(ptr);\
		ptr = NULL;\
	}\
}while(0)

#ifdef __LINUX__
#include <string.h>
#define alipay_iot_memset memset
#define alipay_iot_memcpy memcpy

//#include <unistd.h>
inline void alipay_iot_sleep(unsigned long milliseconds) {
//    usleep(milliseconds *1000);
}

inline void alipay_task_sleep(uint32_t ms) {
    alipay_iot_sleep(ms);
}

#else

extern void *alipay_iot_memset(void *s, int c, size_t n);
extern void *alipay_iot_memcpy(void *dest, const void *src, size_t n);


/*
* @brief: 使任务休眠milliseconds毫秒
* @input: milliseconds 休眠的时间，单位毫秒
*/
extern void alipay_iot_sleep(unsigned long milliseconds);

extern void alipay_task_sleep(uint32_t ms);

#endif

/*
* @brief: 获取UTC时间
* note:  1.精确级别,微妙级别
*        2.受系统时间修改影响
*        3.返回的秒数是从1970年1月1日0时0分0秒开始
*/
void alipay_iot_gettimeofday(alipay_iot_timeval *now);

#endif
