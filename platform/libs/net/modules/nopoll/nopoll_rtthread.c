#include "nopoll_rtthread.h"
// #include <rtthread.h>
#include <lwip/sockets.h>
#include "os_api.h"
#include <tinycrypt/tinycrypt_config.h>
#include <tinycrypt/tinycrypt.h>

/*
 * os mutex routine
 */
static noPollPtr   nopoll_rtt_mutex_create (void)
{
    noPollPtr ptr = nopoll_malloc(sizeof(struct os_mutex));
    if (!ptr) {
        return NULL;
    }
    os_mutex_init(ptr, "nop", OS_IPC_FLAG_FIFO);

    return ptr;
}

static void        nopoll_rtt_mutex_lock    (noPollPtr mutex)
{
    os_mutex_t m = (os_mutex_t) mutex;

    os_mutex_take(m, OS_WAITING_FOREVER);
}

static void        nopoll_rtt_mutex_unlock  (noPollPtr mutex)
{
    os_mutex_t m = (os_mutex_t) mutex;

    os_mutex_release(m);
}

static void        nopoll_rtt_mutex_destroy (noPollPtr mutex)
{
    os_mutex_t m = (os_mutex_t) mutex;

    os_mutex_detach(m);
    nopoll_free(m);
}

/*
 * socket send/recv, because these two routines was defined by lwIP macro.
 */
#ifndef SAL_USING_POSIX

#ifdef send
#undef send
#endif

int send(int s, const void *dataptr, size_t size, int flags)
{
    return lwip_send(s, dataptr, size, flags);
}

#ifdef recv
#undef recv
#endif

int recv(int s, void *mem, size_t len, int flags)
{
    return lwip_recv(s, mem, len, flags);
}
#endif

/*
 * random routine, just to use os_tick
 */
unsigned int nopoll_rtt_random(void)
{
    return LWIP_RAND();
}

nopoll_bool nopoll_rtt_base64_encode (const char  * content,
                  int           length,
                  char        * output,
                  int         * output_size)
{
    nopoll_bool result = nopoll_false;

    if (tiny_base64_encode((unsigned char *)output, (int *)output_size, (unsigned char *)content, length) == 0)
        result = nopoll_true;

    return result;
}

nopoll_bool nopoll_rtt_base64_decode (const char * content,
                  int          length,
                  char       * output,
                  int        * output_size)
{
    nopoll_bool result = nopoll_false;

    if (tiny_base64_decode((unsigned char *)output,(int *) output_size, (unsigned char *)content, length) == 0)
        result = nopoll_true;

    return result;
}

#include <net_heap.h>
/*
 * memory related routines
 */
noPollPtr nopoll_malloc(size_t size)
{
    noPollPtr ptr;

    ptr = netlib_malloc(size, NOPOLL_HEAP_TAG);

    return ptr;
}

noPollPtr nopoll_calloc(size_t count, size_t size)
{
    noPollPtr ptr;

    ptr = netlib_calloc(count, size, NOPOLL_HEAP_TAG);

    return ptr;
}

noPollPtr nopoll_realloc(noPollPtr ref, size_t size)
{
    return netlib_realloc(ref, size, NOPOLL_HEAP_TAG);
}

void nopoll_free (noPollPtr ref)
{
    netlib_free(ref);
}

#ifdef __IAR_SYSTEMS_ICC__
#define MICROSECOND_PER_SECOND  1000000UL
#define MICROSECOND_PER_TICK    (MICROSECOND_PER_SECOND / OS_TICK_PER_SECOND)

/* POSIX thread provides clock_gettime function */
#include <time.h>
int gettimeofday(struct timeval *__tp, void *__tzp)
{
    if (__tp)
    {
        os_tick_t tick;

        /* get tick */
        tick = os_tick_get();

        __tp->tv_sec = tick / OS_TICK_PER_SECOND;
        __tp->tv_usec = ((tick % OS_TICK_PER_SECOND) * MICROSECOND_PER_TICK);
    }

    return 0;
}
#endif

/*
 * Initialization routine
 */
int nopoll_rtt_init(void)
{
    nopoll_thread_handlers(nopoll_rtt_mutex_create,
            nopoll_rtt_mutex_destroy,
            nopoll_rtt_mutex_lock,
            nopoll_rtt_mutex_unlock);

    return 0;
}
