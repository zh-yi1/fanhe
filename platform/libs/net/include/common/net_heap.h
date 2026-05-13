#ifndef NET_HEAP_H__
#define NET_HEAP_H__

#define WEBCLIENT_HEAP_TAG "webclient"
#define NOPOLL_HEAP_TAG "nopoll"
#define CJSON_HEAP_TAG "cjson"

void *netlib_malloc(size_t size, const char *tag);
void *netlib_calloc(size_t nitems, size_t size, const char *tag);
void *netlib_realloc(void *p, size_t new_size, const char *tag);
void netlib_free(void *p);

#endif
