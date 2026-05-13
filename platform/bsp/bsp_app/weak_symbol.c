#include "include.h"

WEAK void ab_app_search_phone(bool en)
{

}

WEAK void app_phone_type_set(u8 type)
{

}

WEAK bool ab_app_is_connect(void)
{
    return false;
}

WEAK u8 app_phone_type_get(void)
{
    return 0;
}

WEAK uint8_t app_distance_unit_get(void)
{
    return 0;
}

WEAK void *sbrk(int inc) {printf("### %s:%x\n", __func__);return NULL;}


//将以下函数重定义出来
WEAK void abort(void)
{
    printf("ABORT PROCESS WDT_RST NOW!\n");
    WDT_RST();
}

#if SECURITY_TRANSITCODE_EN
WEAK size_t write(int fd, const void *buf, size_t count) {printf("### %s\n", __func__);return count;}


WEAK size_t read(int fd, const void *buf, size_t count) {printf("### %s\n", __func__);return count;}

WEAK int close(int fd) {printf("### %s\n", __func__);return 0;}
WEAK int fstat(int fd, void *buf) {printf("### %s\n", __func__);return 0;}
WEAK int isatty(int fd){printf("### %s\n", __func__);return 0;}
WEAK _off_t lseek(int fd, _off_t offset, int whence){printf("### %s\n", __func__);return 0;}


#endif
