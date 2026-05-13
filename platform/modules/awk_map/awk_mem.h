#ifndef AWK_MEM_H_
#define AWK_MEM_H_

typedef enum {
    AWK_TEMP_MEM_IDLE,
    AWK_TEMP_MEM_INIT,
    AWK_TEMP_MEM_READY,
} awk_temp_mem_state_t;

void awk_temp_mm_init(void);
void awk_temp_mm_uninit(void);
bool awk_temp_mm_is_idle(void);
void *awk_temp_malloc(size_t size);
void *awk_temp_realloc(void *ptr, size_t size);
void awk_temp_free(void *ptr);

int awk_objpool_init(void);
void awk_objpool_uninit(void);
void *awk_objpool_alloc(size_t size);
int awk_objpool_free(void *ptr);
int awk_objpool_check(void *ptr);

#endif
