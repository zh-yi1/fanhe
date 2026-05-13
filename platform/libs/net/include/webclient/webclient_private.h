#ifndef WEBCLIENT_PRIVATE_H
#define WEBCLIENT_PRIVATE_H


#ifdef __cplusplus
extern "C" {
#endif

#define RT_EOK                          0               /**< There is no error */
#define RT_ERROR                        1               /**< A generic error happens */
#define RT_ETIMEOUT                     2               /**< Timed out */
#define RT_EFULL                        3               /**< The resource is full */
#define RT_EEMPTY                       4               /**< The resource is empty */
#define RT_ENOMEM                       5               /**< No memory */
#define RT_ENOSYS                       6               /**< No system */
#define RT_EBUSY                        7               /**< Busy */
#define RT_EIO                          8               /**< IO error */

#define RT_TRUE true
#define RT_FALSE false

void my_printf(const char *format, ...);
#define LOG_D(...) //my_printf(__VA_ARGS__);my_printf("\n")
#define LOG_I(...) //my_printf(__VA_ARGS__);my_printf("\n")
#define LOG_E(...) //my_printf(__VA_ARGS__);my_printf("\n")

#define rt_kprintf(...) //my_printf(__VA_ARGS__)

#ifndef RT_ASSERT
#define RT_ASSERT(n)                                                         \
    do {                                                                       \
        if (!(n)) {                                                            \
            my_printf("Assert at File(%s), Line(%d)!", __FILE__, __LINE__);       \
            while (1)                                                          \
                ;                                                              \
        }                                                                      \
    } while (0)
#endif

#include "webclient.h"

#ifdef  __cplusplus
    }
#endif

#endif
