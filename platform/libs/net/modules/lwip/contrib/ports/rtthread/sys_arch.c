#include "arch/sys_arch.h"
#include "lwip/mem.h"
#include "lwip/sys.h"

#include "os_api.h"

#define printf(...)     my_printf(__VA_ARGS__)
#if 1
extern void *netif_default;
extern sys_mutex_t lock_tcpip_core;

#define SYS_ARCH_TIMEOUT 0xffffffffUL
#define SYS_MBOX_EMPTY SYS_ARCH_TIMEOUT

void  tcpip_init(void *tcpip_init_done, void *arg);
typedef int8_t err_t;
#endif

u32_t tick_get(void);
uint32_t bb_timer_get(void);
uint32_t bb_timestamp_get(void); // us
void os_task_sleep(uint32_t ms);
uint32_t os_tick_from_ms(uint32_t ms);
u32_t bt_rand(void);

void sys_mark_tcpip_thread(void);
void lwip_thread_gc_init(void);
void lwip_thread_gc_regist(sys_thread_t *thread, sys_sem_t *sem);

static struct os_mutex lwip_mutex;

uint32_t lwip_port_rand(void)
{
    return bt_rand();
}

/*
 * Initialize the ethernetif layer and set network interface device up
 */
static void tcpip_init_done_callback(void *arg)
{
    sys_mark_tcpip_thread();
    os_sem_release((os_sem_t)arg);
}

/**
 * LwIP system initialization
 */
int lwip_system_init(void)
{
    os_err_t rc;
    struct os_semaphore done_sem;
    static uint8_t init_ok = 0;

    if (init_ok)
    {
        my_printf("lwip system already init.\n");
        return 1;
    }
    lwip_thread_gc_init();
    os_mutex_init(&lwip_mutex, "ml", OS_IPC_FLAG_FIFO);

    // extern int eth_system_device_init_private(void);
    // eth_system_device_init_private();

    /* set default netif to NULL */
    netif_default = NULL;

    rc = os_sem_init(&done_sem, "done", 0, OS_IPC_FLAG_FIFO);
    if (rc != OS_EOK)
    {
        LWIP_ASSERT("Failed to create semaphore", 0);

        return -1;
    }

    tcpip_init(tcpip_init_done_callback, (void *)&done_sem);

    /* waiting for initialization done */
    if (os_sem_take(&done_sem, OS_WAITING_FOREVER) != OS_EOK)
    {
        os_sem_detach(&done_sem);

        return -1;
    }
    os_sem_detach(&done_sem);

    // a_printf("lwIP-%d.%d.%d initialized!\n", LWIP_VERSION_MAJOR, LWIP_VERSION_MINOR, LWIP_VERSION_REVISION);

    init_ok = 1;

    return 0;
}

uint32_t sys_now(void)
{
    return tick_get();
}

uint32_t sys_jiffies(void)
{
    return tick_get();
}

sys_prot_t sys_arch_protect(void)
{
    // int err_bak = errno;
    os_mutex_take(&lwip_mutex, OS_WAITING_FOREVER);
    // errno = err_bak;
    return 1;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    os_mutex_release(&lwip_mutex);
}

void sys_arch_msleep(uint32_t delay_ms)
{
    os_task_sleep(delay_ms);
}

#if !LWIP_COMPAT_MUTEX

static uint8_t mutex_cnt = 0;
err_t sys_mutex_new(sys_mutex_t *mutex)
{
    mutex->mut = mem_malloc(sizeof(struct os_mutex));
    if (mutex->mut == NULL) {
        return ERR_MEM;
    }
    char name[4];
    mutex_cnt = (mutex_cnt + 1) % 100;
    snprintf(name, 4, "l%d", mutex_cnt);
    os_err_t ret = os_mutex_init(mutex->mut, name, OS_IPC_FLAG_FIFO);
    if (ret != OS_EOK) {
        mem_free(mutex->mut);
        return ERR_ARG;
    }
    return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    os_mutex_take((os_mutex_t)mutex->mut, OS_WAITING_FOREVER);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    os_mutex_release((os_mutex_t)mutex->mut);
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    os_mutex_detach((os_mutex_t)mutex->mut);
    mem_free(mutex->mut);
    mutex->mut = NULL;
}

#endif /* !LWIP_COMPAT_MUTEX */

static uint8_t sem_cnt = 0;
err_t sys_sem_new(sys_sem_t *sem, uint8_t initial_count)
{
    sem->sem = mem_malloc(sizeof(struct os_semaphore));
    if (sem->sem == NULL) {
        return ERR_MEM;
    }
    char name[4];
    sem_cnt = (sem_cnt + 1) % 100;
    snprintf(name, 4, "l%d", sem_cnt);

    os_err_t ret = os_sem_init(sem->sem, name, initial_count, OS_IPC_FLAG_FIFO);
    if (ret != OS_EOK) {
        mem_free(sem->sem);
        return ERR_ARG;
    }
    return ERR_OK;
}

void sys_sem_signal(sys_sem_t *sem)
{
    os_sem_release(sem->sem);
}

uint32_t sys_arch_sem_wait(sys_sem_t *sem, uint32_t timeout_ms)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

    os_tick_t tick = OS_WAITING_FOREVER;
    if (timeout_ms) {
        tick = os_tick_from_ms(timeout_ms);
    }
    os_err_t ret = os_sem_take(sem->sem, tick);
    if (ret != OS_EOK) {
        // printf("%s timeout_ms=%d ret=%d\n", __func__, timeout_ms, (int)ret);
        return SYS_ARCH_TIMEOUT;
    }

    /* Old versions of lwIP required us to return the time waited.
     This is not the case any more. Just returning != SYS_ARCH_TIMEOUT
     here is enough. */
    return 1;
}

void sys_sem_free(sys_sem_t *sem)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

    os_sem_detach(sem->sem);
    mem_free(sem->sem);
    sem->sem = NULL;
}

#define MBOX_MQ_LEN 4
static uint8_t mq_cnt = 0;
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("size > 0", size > 0);

    int pool_size = (MBOX_MQ_LEN + 4) * size;

    char name[4];
    mq_cnt = (mq_cnt + 1) % 100;
    snprintf(name, MBOX_MQ_LEN, "l%d", mq_cnt++);

    mbox->mbx = mem_malloc(sizeof(struct os_messagequeue));
    if (mbox->mbx == NULL) {
        return ERR_MEM;
    }
    void *msgpool = mem_malloc(pool_size);
    if (msgpool == NULL) {
        return ERR_MEM;
    }

    os_mq_init(mbox->mbx, name, msgpool, 4, pool_size, OS_IPC_FLAG_FIFO);
    return ERR_OK;
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    uint32_t addr = (uint32_t)msg;
    os_err_t ret = os_mq_send(mbox->mbx, &addr, 4);
    while (ret == -OS_EFULL) {
        os_task_sleep(1);
        ret = os_mq_send(mbox->mbx, &addr, 4);
    }
    if (ret != OS_EOK) {
        printf("%s failed\n", __func__);
    }
}

// AT(.com_text.lwip)
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    uint32_t addr = (uint32_t)msg;
    os_err_t ret = os_mq_send(mbox->mbx, &addr, 4);
    if (ret != OS_EOK) {
        printf("%s failed %p %p\n", __func__, mbox->mbx, __builtin_return_address(0));
        return ERR_MEM;
    }

    return ERR_OK;
}

// AT(.com_text.lwip)
err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    printf("no defined\n");
    return ERR_IF;
    // return sys_mbox_trypost(mbox, msg);
}

uint32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, uint32_t timeout_ms)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    uint32_t addr = 0;
    os_tick_t tick = OS_WAITING_FOREVER;
    if (timeout_ms) {
        tick = os_tick_from_ms(timeout_ms);
    }
    os_err_t ret = os_mq_recv(mbox->mbx, &addr, 4, tick);
    *msg = (void *)addr;
    if (ret != OS_EOK) {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }

    // LWIP_DEBUGF(SYS_DEBUG, ("mbox recv 0x%x from queue\r\n", *msg));

    /* Old versions of lwIP required us to return the time waited.
     This is not the case any more. Just returning != SYS_ARCH_TIMEOUT
     here is enough. */
    return 1;
}

uint32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    uint32_t addr = 0;
    os_err_t ret = os_mq_recv(mbox->mbx, &addr, 4, 0);
    *msg = (void *)addr;
    if (ret != OS_EOK) {
        *msg = NULL;
        return SYS_MBOX_EMPTY;
    }

    return 0;
}

// AT(.com_text.lwip)
// int sys_mbox_is_empty(sys_mbox_t *mbox)
// {
//     int ret = 0;
//     os_mq_t mq = (os_mq_t)mbox->mbx;
//     GLOBAL_INT_DISABLE();
//     if (mq->entry == 0) {
//         ret = 1;
//     }
//     GLOBAL_INT_RESTORE();
//     return ret;
// }

void sys_mbox_free(sys_mbox_t *mbox)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    struct os_messagequeue *mbx = mbox->mbx;
    mem_free(mbx->msg_pool);
    os_mq_detach(mbox->mbx);
    mem_free(mbox->mbx);
    mbox->mbx = NULL;
}

#if LWIP_NETCONN_SEM_PER_THREAD
static uint32_t get_thread_tls_addr(os_thread_t thread);
#endif
void sys_arch_netconn_sem_alloc_do(uint32_t tls_addr);
void sys_arch_netconn_sem_free_do(uint32_t tls_addr);
sys_thread_t sys_thread_new(const char    *name,
                            lwip_thread_fn thread,
                            void          *arg,
                            int            stacksize,
                            int            prio)
{
    LWIP_ASSERT("invalid stacksize", stacksize > 0);

    sys_thread_t lwip_thread;
    lwip_thread.thread_handle = NULL;
    void *stack = mem_malloc(stacksize);
    if (stack == NULL) {
        return lwip_thread;
    }
    os_thread_t tid = mem_malloc(sizeof(struct os_thread));
    if (tid == NULL) {
        mem_free(stack);
        return lwip_thread;
    }

    uint8_t real_prio = prio;
    if (real_prio <= 18) {
        real_prio = 19;
        printf("%s real_prio=19\n", __func__);
    }

    os_thread_init(tid, name, thread, arg, stack, stacksize, real_prio, -1);
    lwip_thread.thread_handle = tid;
#if LWIP_NETCONN_SEM_PER_THREAD
    uint32_t tls_addr = get_thread_tls_addr(tid);
    sys_arch_netconn_sem_alloc_do(tls_addr);
    lwip_thread_gc_regist(&lwip_thread, *(sys_sem_t **)tls_addr);
#else
    lwip_thread_gc_regist(&lwip_thread, NULL);
#endif
    os_thread_startup(tid);
    return lwip_thread;
}

/** Flag the core lock held. A counter for recursive locks. */
static uint8_t        lwip_core_lock_count;
static os_thread_t lwip_core_lock_holder_thread;

void sys_lock_tcpip_core(void)
{
    // int err_bak = errno;
    sys_mutex_lock(&lock_tcpip_core);
    // errno = err_bak;
    if (lwip_core_lock_count == 0) {
        lwip_core_lock_holder_thread = os_thread_self();
    }
    lwip_core_lock_count++;
}

void sys_unlock_tcpip_core(void)
{
    lwip_core_lock_count--;
    if (lwip_core_lock_count == 0) {
        lwip_core_lock_holder_thread = 0;
    }
    sys_mutex_unlock(&lock_tcpip_core);
}

static os_thread_t lwip_tcpip_thread;

void sys_mark_tcpip_thread(void)
{
    lwip_tcpip_thread = os_thread_self();
}

void sys_check_core_locking(const char *file, unsigned int line)
{
    if (lwip_tcpip_thread != 0) {
        os_thread_t current_thread = os_thread_self();

        if (!((current_thread == lwip_core_lock_holder_thread) &&
              (lwip_core_lock_count > 0))) {
            printf("Function called without core lock(%s): failed at %d in "
                   "%s\n",
                   __func__,
                   line,
                   file);
            lwip_wdt_rst();
        }
    }
}

#if LWIP_NETCONN_SEM_PER_THREAD
static uint32_t get_thread_tls_addr(os_thread_t thread)
{
    uint32_t stack_addr = (uint32_t)thread->stack_addr;
    return (stack_addr);
}

sys_sem_t* sys_arch_netconn_sem_get(void)
{
    // TP寄存器已被编译器优化占用，改为使用线程栈顶存储TLS
    sys_sem_t* sem = *(sys_sem_t**)get_thread_tls_addr(os_thread_self());
    return sem;
}

void sys_arch_netconn_sem_alloc_do(uint32_t tls_addr)
{
    sys_sem_t *sem;
    err_t err;

    // 分配存放 sys_sem_t 指针的内存 (如果 sys_sem_t 是指针，这里需要分配一个指针指向的空间)
    // 注意：lwIP 的 sys_sem_new 通常需要一个 sys_sem_t* 作为输出参数
    // 这里我们 malloc 一个 sys_sem_t 对象来持久保存
    sem = (sys_sem_t*)mem_malloc(sizeof(sys_sem_t));
    if (sem == NULL) {
        // 处理内存分配失败，通常需要 Assert 或 Log
        *(sys_sem_t**)tls_addr = NULL;
        return;
    }

    // 创建一个新的信号量 (初始计数通常为0)
    err = sys_sem_new(sem, 0); 
    if (err != ERR_OK) {
        mem_free(sem);
        return;
    }

    // my_printf("sem alloc: %x %x\n", tls_addr, sem);
    *(sys_sem_t**)tls_addr = sem;
}

void sys_arch_netconn_sem_alloc(void)
{
    // // TP寄存器已被编译器优化占用，现在将sem指针存储在线程栈顶
    // sys_arch_netconn_sem_alloc_do(get_thread_tls_addr());
}

void sys_arch_netconn_sem_free_do(uint32_t tls_addr)
{
    sys_sem_t *sem;

    // 1. 获取信号量
    sem = *(sys_sem_t**)tls_addr;

    if ((sem != NULL) && (sem != (sys_sem_t*)0x23232323)) {
        // 2. 销毁信号量资源
        sys_sem_free(sem);

        // 3. 释放存放 sys_sem_t 的内存
        mem_free(sem);

        // 4. 清空 TLS 槽位
        *(sys_sem_t**)tls_addr = NULL;
    }
}

void sys_arch_netconn_sem_free(void)
{
    // sys_arch_netconn_sem_free_do(get_thread_tls_addr());
}
#endif
