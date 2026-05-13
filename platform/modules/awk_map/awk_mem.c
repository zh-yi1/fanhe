#include "include.h"
#include "awk.h"
#include "../bsp/bsp_mem/tlsf.h"

void *awk_objpool_alloc(size_t size);
int awk_objpool_free(void *ptr);
int awk_objpool_check(void *ptr);

u32 awk_map_temp_buff_start_addr(void);
u32 awk_map_temp_buff_end_addr(void);
void mem_monitor_run_ex(void *tlsf);
void mem_monitor_run(void);
void psram_mem_monitor_run(void);

// #define DEBUG_SRAM 1

static void awk_mem_free_adapter(void* ptr)
{
    if (!ptr) {
        return;
    }
    // printf("awk free:0x%p 0x%p\n", ptr, __builtin_return_address(0));
    // printf("tlsf_free src=%x %p\n", ptr, __builtin_return_address(0));
    psram_heap_free(ptr);
    // ab_free(ptr);
}

static void* awk_mem_malloc_adapter(size_t size)
{
    void *ptr;
    ptr = psram_heap_malloc(size);
    // ptr = ab_malloc(size);
    // printf("tlsf_malloc src=%x 1 size=%d %x\n", ptr, size,  __builtin_return_address(0));
    // printf("awk malloc:0x%p 0x%p\n", ptr, __builtin_return_address(0));
    if (!ptr) {
        printf("malloc %d fail\n", size);
        psram_mem_monitor_run();
        WDT_RST();
    }
    return ptr;
}

static void* awk_mem_calloc_adapter(size_t count, size_t size)
{
    void *ptr;
    ptr = psram_heap_calloc(count, size);
    // ptr = ab_calloc(count, size);
    // printf("tlsf_malloc src=%x 2 size=%d %x\n", ptr, size,  __builtin_return_address(0));
    // printf("awk malloc:0x%p 0x%p\n", ptr, __builtin_return_address(0));
    if (!ptr) {
        printf("malloc %d fail\n", count*size);
        WDT_RST();
    }
    return ptr;
}

static void* awk_mem_realloc_adapter(void* ptr, size_t size)
{
    void *new_ptr;
    new_ptr = psram_heap_realloc(ptr, size);
    // new_ptr = ab_realloc(ptr, size);
    // printf("tlsf_malloc src=%x 3 size=%d %x\n", new_ptr, size,  __builtin_return_address(0));
    // if (ptr && (ptr != new_ptr)) {
    //     printf("awk free:0x%p 0x%p\n", ptr, __builtin_return_address(0));
    // }
    // printf("awk malloc:0x%p 0x%p\n", new_ptr, __builtin_return_address(0));
    if (!new_ptr) {
        printf("malloc %d fail\n", size);
        WDT_RST();
    }
    return new_ptr;
}

static const awk_memory_adapter_t awk_memory_adapter = {
    .mem_free = awk_mem_free_adapter,
    .mem_malloc = awk_mem_malloc_adapter,
    .mem_calloc = awk_mem_calloc_adapter,
    .mem_realloc = awk_mem_realloc_adapter,
};

const awk_memory_adapter_t *awk_mem_get_adapter(void)
{
    return &awk_memory_adapter;
}
