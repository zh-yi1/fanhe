#include "include.h"

#define GUI_BACKUP_SIZE     (32*1024)  /* increased to cover ~31k disp restore for elunchbox
                                        * 必须 >= keep_ram_tbl_load_size(实测31308):
                                        * 否则备份走 ab_malloc 动态池, 唤醒引导阶段池被
                                        * 重建 → restore 拷回垃圾 → 醒后即挂死(唤不醒) */

typedef struct {
    u32 start;
    u32 end;
} keep_ram_tbl_t;

AT(.sleep_backup.gui)
u8 gui_backup_buf[GUI_BACKUP_SIZE];

extern u32 __disp_restore_start, __disp_restore_end;

keep_ram_tbl_t keep_ram_tbl[] = {
    [0] = { //DISP GUI显示信息
            .start = (u32)&__disp_restore_start,
            .end = (u32)&__disp_restore_end,
          },
};

u8* keep_ram = NULL;
bool load_flag = false;

static u32 get_keep_ram_size(void)
{
    u32 size = 0;
    for (int i=0; i<sizeof(keep_ram_tbl)/sizeof(keep_ram_tbl[0]); i++) {
        size += keep_ram_tbl[i].end - keep_ram_tbl[i].start;
    }
    return size;
}

static u32 get_keep_ram_tbl_offset(u8 idx)
{
    u32 keep_ram_offset = 0;
    for (int i=0; i<idx; i++) {
        keep_ram_offset += keep_ram_tbl[i].end - keep_ram_tbl[i].start;
    }
    return keep_ram_offset;
}

bool keep_ram_tbl_load(void)
{
//    u32 keep_ram_size = 0;
//    for (int i=0; i<sizeof(keep_ram_tbl)/sizeof(keep_ram_tbl[0]); i++) {
//        printf("%s[%d]-> 0x%x ~ 0x%x\n", __func__, i, keep_ram_tbl[i].start, keep_ram_tbl[i].end);
//        keep_ram_size += keep_ram_tbl[i].end - keep_ram_tbl[i].start;
//    }

    u32 keep_ram_size = get_keep_ram_size();

    printf("%s_size:%d byte\n", __func__, keep_ram_size);

    if (keep_ram_size) {
        if (keep_ram == NULL) {
            if (keep_ram_size <= GUI_BACKUP_SIZE) {
                keep_ram = gui_backup_buf;
            } else {
                keep_ram = (u8*)ab_malloc(keep_ram_size);
            }
        }
        printf("keep_ram[0x%x]\n", keep_ram);
        if (keep_ram == NULL) {
            printf("==>keep_ram malloc memory failed!\n");
            return false;
        }

        //load
        for (int i=0; i<sizeof(keep_ram_tbl)/sizeof(keep_ram_tbl[0]); i++) {
            printf("load [%d]-> 0x%x ~ 0x%x\n", i, keep_ram_tbl[i].start, keep_ram_tbl[i].end);
            memcpy(keep_ram + get_keep_ram_tbl_offset(i), (u8*)(keep_ram_tbl[i].start), keep_ram_tbl[i].end - keep_ram_tbl[i].start);
        }
        printf("mem load succ\n");
    }

    return true;
}

bool keep_ram_tbl_restore(void)
{
    if (keep_ram == NULL) {
        return false;
    }

    u32 keep_ram_size = get_keep_ram_size();

    for (int i=0; i<sizeof(keep_ram_tbl)/sizeof(keep_ram_tbl[0]); i++) {
        printf("\nrestore [%d]-> 0x%x ~ 0x%x\n", i, keep_ram_tbl[i].start, keep_ram_tbl[i].end);
        memcpy((u8*)(keep_ram_tbl[i].start), keep_ram + get_keep_ram_tbl_offset(i), keep_ram_tbl[i].end - keep_ram_tbl[i].start);
    }

    if (keep_ram) {
        if (keep_ram_size > GUI_BACKUP_SIZE) {
            ab_free(keep_ram);
        }
		
        keep_ram = NULL;
    }

    return true;
}
