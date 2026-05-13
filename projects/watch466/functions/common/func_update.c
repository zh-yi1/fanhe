#include "include.h"
#include "func.h"
#include "func_update.h"

#if USB_SD_UPDATE_EN

typedef struct _func_upd_t {
    uint sta;
} func_upd_t;

func_upd_t f_upd;

const char upd_filename[13] = UPD_FILENAME;

AT(.text.func.update)
void func_update(void)
{
    int res;
    res = updatefile_init(upd_filename);
    if (res == -1) {
        return;
    }
    if (res == 0) {
        return;
    }

    updateproc();                               //升级
    while (1);
}
#endif  //USB_SD_UPDATE_EN
