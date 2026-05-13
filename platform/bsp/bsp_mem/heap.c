#include "include.h"
#include "tlsf.h"
#include "rt_thread.h"

#define MAX_HEAP_SIZE	0x40008

typedef struct {
    u32 free_size;
    u32 used_size;
    uint32_t free_biggest_size;
} mem_monitor_t;

extern u32 __dynamic_pool_start, __dynamic_pool_size;

// clang-format off
static tlsf_t sys_tlsf;
static struct os_mutex sys_tlsf_mutex;
// clang-format on

void customer_heap_init_do(void *buf, u32 size)
{
    sys_tlsf = NULL;
    if (size > 0) {
        printf("malloc heap size:%d\n", size);
        os_mutex_init(&sys_tlsf_mutex, "mm", OS_IPC_FLAG_FIFO);
        sys_tlsf = tlsf_create_with_pool(buf, size);
    }
}

void customer_heap_init(void)
{
    u32 size = min((u32)&__dynamic_pool_size, MAX_HEAP_SIZE);
    printf("dynamic_pool:0x%x, 0x%x, use max size:0x%x\n", &__dynamic_pool_start, &__dynamic_pool_size, size);
    memset((void *)&__dynamic_pool_start, 0, size);
    customer_heap_init_do((void *)&__dynamic_pool_start, size);
//    printf("%s end\n", __func__);
}

void *ab_malloc(size_t size)
{
    os_mutex_take(&sys_tlsf_mutex, OS_WAITING_FOREVER);
    void *ptr = tlsf_malloc(sys_tlsf, size);
    os_mutex_release(&sys_tlsf_mutex);
    if(ptr == NULL){
        printf("ab_malloc no space:0x%x\n", __builtin_return_address(0));
    }
    return ptr;
}

void *ab_zalloc(size_t size)
{
    os_mutex_take(&sys_tlsf_mutex, OS_WAITING_FOREVER);
    void *ptr = tlsf_malloc(sys_tlsf, size);

    if (ptr) {
        memset(ptr, 0, size);
    } else {
        printf("ab warning zalloc ptr NULL:%x\n", size);
    }
    os_mutex_release(&sys_tlsf_mutex);
    return ptr;
}

void ab_free(void *ptr)
{
    os_mutex_take(&sys_tlsf_mutex, OS_WAITING_FOREVER);
    tlsf_free(sys_tlsf, ptr);
    os_mutex_release(&sys_tlsf_mutex);
}

void *ab_calloc(size_t nitems, size_t size)
{
    os_mutex_take(&sys_tlsf_mutex, OS_WAITING_FOREVER);
    size_t total = nitems * size;
    void  *ptr   = tlsf_malloc(sys_tlsf, total);
    if (ptr) {
        memset(ptr, 0, total);
    } else {
        printf("ab warning calloc ptr NULL:%x\n", total);
    }
    os_mutex_release(&sys_tlsf_mutex);
    return ptr;
}

void *ab_realloc(void *p, size_t new_size)
{
    os_mutex_take(&sys_tlsf_mutex, OS_WAITING_FOREVER);
    void *ptr = tlsf_realloc(sys_tlsf, p, new_size);
    if (!p) {
        printf("ab warning realloc ptr NULL:%x\n", new_size);
    }
    os_mutex_release(&sys_tlsf_mutex);
    return ptr;
}

static void mem_walker(void * ptr, size_t size, int used, void * user)
{
    mem_monitor_t * mon_p = user;
    if(used) {
        mon_p->used_size += size;
    }
    else {
//        mon_p->free_cnt++;
        mon_p->free_size += size;
        if(size > mon_p->free_biggest_size)
            mon_p->free_biggest_size = size;
    }
}

void assert_malloc_buf_is_valid(void *buf)
{
    if (buf == NULL) {
        printf("%s: %x at ra: %x\n", __func__, buf, __builtin_return_address(0));
        halt(HALT_MALLOC);
    }
}

void mem_monitor_run(void)
{
    if (sys_tlsf == NULL) {
        printf("%s sys_tlsf=NULL\n", __func__);
        return;
    }

    mem_monitor_t mon = {0};
    tlsf_walk_pool(tlsf_get_pool(sys_tlsf), mem_walker, &mon);
    printf("### ram inof: use:%d%%, frag:%d%%, free size: 0x%x, used size: 0x%x\n", mon.used_size * 100U / (u32)&__dynamic_pool_size, 100 - (mon.free_biggest_size * 100U / mon.free_size), mon.free_size, mon.used_size);
}

