#include "alipay_net_kal.h"
#include "iot_err.h"
#ifdef __ALIPAY_IOT_KV__
#include "alipay_iot_kv.h"
#include "flash_disk.h"
#endif
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
// #include <pthread.h>

void a_printf(const char *format, ...);
#define printf(...)                     a_printf(__VA_ARGS__)

/*
 * @brief: 创建互斥量
 * @param: mutex_name 互斥量的名称
 * @return: 其他 创建成功
 *          NULL 创建失败
 */
alipay_iot_mutex alipay_iot_mutex_create(const char* mutex_name)
{
    // #warning alipay_iot_mutex_create demo
    // alipay_iot_mutex mutex = (alipay_iot_mutex)malloc(sizeof(pthread_mutex_t));
    // if (!mutex) {
    //     //printf("alipay_iot_mutex_create:out of memory\n");
    //     return NULL;
    // }

    // int v = pthread_mutex_init((pthread_mutex_t*)mutex, NULL);
    // if (0 == v) {
    //     return mutex;
    // }
    // //printf("alipay_iot_mutex_create: Init alipay_iot_mutex failed\n");
    // free(mutex);
    return NULL;
}

/*
 * @brief: 对互斥量加锁
 * @param: mutex 要加锁的互斥量
 * @return: 0 加锁成功
 *          -1 加锁失败
 */
int alipay_iot_mutex_lock(alipay_iot_mutex mutex)
{
//    #warning alipay_iot_mutex_lock demo
    // if (!mutex) {
    //     //printf("alipay_iot_mutex_lock: Mutex is NULL\n");
    //     return -1;
    // }

    // return pthread_mutex_lock((pthread_mutex_t*)mutex);
    return 1;
}

/*
 * @brief: 对互斥量解锁
 * @param: mutex 要解锁的互斥量
 * @return: 0 解锁成功
 *          -1 解锁失败
 */
int alipay_iot_mutex_unlock(alipay_iot_mutex mutex)
{
//    #warning alipay_iot_mutex_unlock demo
    // if (!mutex) {
    //     //printf("alipay_iot_mutex_unlock: Mutex is NULL\n");
    //     return -1;
    // }
    // return pthread_mutex_unlock((pthread_mutex_t*)mutex);
    return 1;
}

/*
 * @brief: 删除互斥量
 * @param: mutex 要删除的互斥量
 * @return: 0 删除成功
 *          -1 删除失败
 */
int alipay_iot_mutex_delete(alipay_iot_mutex mutex)
{
//    #warning alipay_iot_mutex_delete demo
    // if (!mutex) {
    //     //printf("alipay_iot_mutex_delete: Mutex is NULL\n");
    //     return -1;
    // }

    // int v = pthread_mutex_destroy((pthread_mutex_t*)mutex);
    // if (v == 0) {
    //     free(mutex);
    // }

    // return v;
    return 1;
}

// #include <stdbool.h>
#ifdef __cplusplus
    // C++ 环境下不需要定义，因为 C++ 有内置的 bool 类型
#else
    // C 环境下的定义
    #define bool  uint8_t
//    #define true  1
//    #define false 0
#endif
/*************************order mutex *************************/
alipay_iot_mutex get_order_mtx()
{
    static alipay_iot_mutex s_mtx_order;
    static bool s_isinitialized_order = false;
    if (!s_isinitialized_order) {
        s_mtx_order = alipay_iot_mutex_create("order_mutex");
        s_isinitialized_order = true;
    }
    return s_mtx_order;
}


/*************************socket adapt*************************/
static bool alipay_socket_init_flag = false;
static int32_t alipay_sockets[ALIPAY_IOT_MAX_IP_SOCKET_NUM] = {-1};
// static aos_mutex_t alipay_socket_lock;
//static alipay_iot_mutex alipay_socket_lock = NULL;

alipay_iot_mutex get_socket_mtx()
{
    static alipay_iot_mutex s_mtx_socket;
    static bool s_isinitialized = false;
    if (!s_isinitialized) {
        s_mtx_socket = alipay_iot_mutex_create("socket_mutex");
        s_isinitialized = true;
    }
    return s_mtx_socket;
}

int alipay_iot_fd_init()
{
    int ret = 0;

    if (alipay_socket_init_flag == true) {
        //printf("alipay fd have already inited, should be called twice\n");
        return -1;
    }

    memset(alipay_sockets, -1, sizeof(alipay_sockets));
    alipay_socket_init_flag = true;
    return ret;
}

int alloc_alipay_socket(int origin_fd)
{
    int i;

    if (alipay_socket_init_flag != true) {
        alipay_iot_fd_init();
        // //printf("alipay fd haven't init yet\n");
        // return -1;
    }

    alipay_iot_mutex_lock(get_socket_mtx());

    for (i = 0; i < ALIPAY_IOT_MAX_IP_SOCKET_NUM; i++) {
        if (alipay_sockets[i] == -1) {
            alipay_sockets[i] = origin_fd;
            alipay_iot_mutex_unlock(get_socket_mtx());
            return i;
        }
    }

    alipay_iot_mutex_unlock(get_socket_mtx());
    return -1;
}

int free_alipay_socket(int s)
{
    if (alipay_socket_init_flag != true) {
        // //printf("alipay fd haven't init yet\n");
        // return -1;
        alipay_iot_fd_init();
    }

    if (s >= ALIPAY_IOT_MAX_IP_SOCKET_NUM || s < 0) {
        return -1;
    }

    alipay_iot_mutex_lock(get_socket_mtx());
    alipay_sockets[s] = -1;
    alipay_iot_mutex_unlock(get_socket_mtx());

    return 0;
}

int get_origin_fd_by_alipay_socket(int socket)
{
    int fd = -1;
    if (alipay_socket_init_flag != true) {
         //printf("alipay fd haven't init yet\n");
        // return -1;
        alipay_iot_fd_init();
    }

    if (socket >= ALIPAY_IOT_MAX_IP_SOCKET_NUM || socket < 0){
//        printf("invalid input socket %d\n", socket);
        return -1;
    }

    alipay_iot_mutex_lock(get_socket_mtx());
    fd = alipay_sockets[socket];
    alipay_iot_mutex_unlock(get_socket_mtx());
    return fd;
}

int get_alipay_socket_by_origin_fd(int fd)
{
    int socket = -1;
    if (alipay_socket_init_flag != true) {
        // //printf("alipay fd haven't init yet\n");
        // return -1;
        alipay_iot_fd_init();
    }

    if (fd < 0){
        //printf("invalid input fd %d\n", fd);
        return -1;
    }

    alipay_iot_mutex_lock(get_socket_mtx());
    for(uint32_t i=0; i<ALIPAY_IOT_MAX_IP_SOCKET_NUM; i++){
        if(fd == alipay_sockets[i]){
            socket = i;
        }
    }
    alipay_iot_mutex_unlock(get_socket_mtx());
    return socket;
}

void alipay_iot_fd_setResultbit(int fd, alipay_iot_fd_set* fdset)
{
    if(fdset != NULL && fd >=0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM )
    {
//        printf("%s:%x, %x, fd:%d, &fdset:%x\n", __func__, &fdset->fds_bits[fd], fdset, fd, &fdset);
        fdset->fds_bits[fd] |= 0x02;
    }
}

int alipay_iot_fd_check(int fd, alipay_iot_fd_set* fdset)
{
    if(fdset != NULL && fd >=0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM )
    {
        return fdset->fds_bits[fd] & 0x01;
    }
    return 0;
}

int alipay_iot_fd_isset(int fd, alipay_iot_fd_set* fdset)
{
    // printf("isset fd: %d\n", fd);

    if(fdset != NULL && fd >=0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM )
    {
        return fdset->fds_bits[fd] & 0x02;
    }
    return 0;
}

void alipay_iot_fd_setbit(int fd, alipay_iot_fd_set* fdset)
{
    // printf("setbit fd: %d\n", fd);

    if(fdset != NULL && fd >=0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM )
    {
        fdset->fds_bits[fd] |= 0x01;
    }
}

void alipay_iot_fd_zero(alipay_iot_fd_set* fdset)
{
    if(fdset != NULL)
    {
        memset(fdset, 0, sizeof(alipay_iot_fd_set));
    }
}
