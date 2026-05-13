#ifndef _API_MEM_H
#define _API_MEM_H

void func_heap_init(void *heap, u32 size);          //initial func heap
void *func_zalloc(u32 size);                        //malloc & memset 0
void func_free(void *mem_ptr);                      //Free the memory area pointed by mem_ptr
bool func_mem_is_free(void* mem_ptr);               //Check if current pointer is free or not
bool func_heap_is_empty(void);                      //Check if current heap is empty or not
u32 func_heap_get_free_size(void);                  //Get free size of the current heap

typedef void * tlsf_t;
typedef void * pool_t;
typedef void (*tlsf_walker)(void * ptr, size_t size, int used, void * user);

pool_t tlsf_get_pool(tlsf_t tlsf);

/**
 * @brief 遍历tlsf pool，后面两个参数给 NULL 会用默认 walker 打印 block 信息
 *        `tlsf_walk_pool(tlsf_get_pool(tlsf), NULL, NULL)`
 *
 * @param pool
 * @param walker 遍历 block 时的回调
 * @param user 会传递给 walker
 */
void tlsf_walk_pool(pool_t pool, tlsf_walker walker, void * user);

/**
 * 用户堆空间申请
 * size:字节大小
 */
void *ab_malloc(size_t size);

/**
 * 用户堆空间申请并清零
 * size:字节大小
 */
void *ab_zalloc(size_t size);

/**
 * 用户堆空间释放
 * size:字节大小
 */
void ab_free(void *ptr);

/**
 * 用户堆空间申请，带初始化0
 * size*nitems:字节大小
 */
void *ab_calloc(size_t nitems, size_t size);
void *ab_realloc(void *p, size_t new_size);
void assert_malloc_buf_is_valid(void *buf);

// psram heap
void  psram_heap_init(void);
void  psram_heap_uninit(void);
void *psram_heap_malloc(size_t size);
void *psram_heap_calloc(size_t count, size_t size);
void *psram_heap_realloc(void *ptr, size_t size);
void  psram_heap_free(void *ptr);

#endif // _API_MEM_H
