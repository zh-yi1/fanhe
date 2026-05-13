#include "include.h"
#include "awk_adapter.h"
#include "awk_mem.h"

#if AWK_MAP_EN

#define  TRACE_EN   0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

void awk_fs_uninit(void);

const awk_memory_adapter_t *awk_mem_get_adapter(void);
const awk_file_adapter_t *awk_file_get_adapter(void);

static void *fatfs = NULL;
// static char test_buf[1024]= {0};
// const char *test_str = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
// const char *test_str2 = "abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuiozxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjkl";

bool awk_fs_init(void)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    SD0_LDO_EN();    //打开SD供电

    int res = 0;
    fatfs = memory_adapter->mem_malloc(fs_get_sizeof_fatfs());
    // fatfs = ab_malloc(fs_get_sizeof_fatfs());
    if (!fatfs) {
        goto _fail;
    }
    if ((res = fs_mount_v2(fatfs, "B:"))) {
        goto _fail;
    }

    // sys_cb.cur_dev = DEV_SDCARD;
    // fsdisk_callback_init(sys_cb.cur_dev);
    // memset(&msc_cb, 0, sizeof(msc_cb));
    // msc_cb.fname = &fname_buf[0];

    // if(fs_mount() == false){
    //     printf("fs_mount fail\n");
    // }else{
    //     printf("fs_mount ok\n");
    // }

    // ret = fs_open("aaa.txt", FA_READ);
    // printf("open %d\n", ret);
    // if (!ret) {
    //     uint br = 0;
    //     printf("read test\n");
    //     fs_read(read_buf, 512, &br);
    //     printf("read %d\n", br);
    //     printf("%s\n", read_buf);
    //     fs_close();
    // }

    // // void *fs = awk_fs_open("AWK/CACHE/15-2698712406-2698712406.ca", FA_WRITE | FA_CREATE_NEW);
    // // if (fs) {
    // //     printf("open\n");
    // //     awk_fs_close(fs);
    // // }

    // 提前创建好所有文件夹
    fs_mkdir("B:map");
    fs_mkdir("B:awk");
    fs_mkdir("B:awk/off_ca");
    fs_mkdir("B:awk/cache");

    // void *fil = ab_malloc(fs_get_sizeof_fil());
    // u8 *buf = ab_malloc(100);
    // res = fs_open(fil, "B:awk/license", FA_READ);
    // printf("open: %d\n", res);
    // UINT size = 0;
    // fs_read(fil, buf, 100, &size);
    // printf("read:%d\n", size);
    // print_r(buf, size);

    // while (1);

    // int ret;
    // ret = fs_open("bbb.txt", FA_WRITE|FA_CREATE_ALWAYS);
    // printf("write test1:%d\n", ret);
    // if (!ret) {
    //     char *str = "bbbbb";
    //     strcpy(temp_buf, str);
    //     fs_write(temp_buf, 512);
    //     fs_close();
    // }
    // sd0_stop(1);
    // ret = fs_open("bbb.txt", FA_READ);
    // printf("read test1:%d\n", ret);
    // if (!ret) {
    //     uint br = 0;
    //     printf("read test\n");
    //     memset(temp_buf, 0, 6);
    //     fs_read(temp_buf, 512, &br);
    //     printf("read %d\n", br);
    //     printf("%s\n", temp_buf);
    //     fs_close();
    // }

    // ret = fs_open("awk/off_ca/bbb.txt", FA_WRITE|FA_CREATE_ALWAYS);
    // printf("write test2:%d\n", ret);
    // if (!ret) {
    //     char *str = "aaaaa";
    //     strcpy(temp_buf, str);
    //     fs_write(temp_buf, 512);
    //     fs_close();
    // }
    // ret = fs_open("awk/off_ca/bbb.txt", FA_READ);
    // printf("read test2:%d\n", ret);
    // if (!ret) {
    //     uint br = 0;
    //     printf("read test\n");
    //     memset(temp_buf, 0, 6);
    //     fs_read(temp_buf, 512, &br);
    //     printf("read %d\n", br);
    //     printf("%s\n", temp_buf);
    //     fs_close();
    // }
    // while (1);

    // char path1[] = "awk/off_ca/123456789.txt";
    // const awk_file_adapter_t *file_adapter = awk_file_get_adapter();
    // void *handle = file_adapter->file_open(path1, "w");
    // if (handle) {
    //     file_adapter->file_write(test_str, 522, handle);
    //     // file_adapter->file_write(test_str, 522, handle);
    //     file_adapter->file_seek(handle, 8, 0);
    //     file_adapter->file_write(test_str2, 512, handle);
    //     file_adapter->file_close(handle);
    // }

    // char path2[] = "awk/off_ca/123456789.txt";
    // handle = file_adapter->file_open(path2, "r");
    // if (handle) {
    //     int cnt;
    //     memset(test_buf,0,1024);
    //     file_adapter->file_seek(handle, 513, 0);
    //     cnt = file_adapter->file_read(test_buf, 20, handle);
    //     printf("buf(%d)=%s\n", cnt, test_buf);
    //     memset(test_buf,0,1024);
    //     file_adapter->file_seek(handle, 0, 0);
    //     cnt = file_adapter->file_read(test_buf, 1024, handle);
    //     printf("buf(%d)=%s\n", cnt, test_buf);
    //     file_adapter->file_close(handle);
    // }

    // while (1);

    // const awk_file_adapter_t *file_adapter = awk_file_get_adapter();
    // char path[] = "awk/cache/tc.index";
    // void *handle = file_adapter->file_open(path, "r");
    // if (handle) {
    //     file_adapter->file_close(handle);
    // }
    // while (1);

    // const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    // const awk_file_adapter_t *file_adapter = awk_file_get_adapter();
    // char path[] = "awk/cache/15-2675614254-2675614254.ca";
    // int size = file_adapter->file_get_size(path);
    // void *handle = file_adapter->file_open(path, "r");
    // if (size && handle) {
    //     void *buf = memory_adapter->mem_malloc(size);
    //     file_adapter->file_read(buf, size, handle);
    //     print_r(buf, size);
    //     file_adapter->file_close(handle);
    // }

    // const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    // const awk_file_adapter_t *file_adapter = awk_file_get_adapter();
    // char path[] = "awk/cache/15-2675614254-12121212454.ca";
    // void *handle = file_adapter->file_open(path, "w");
    // if (handle) {
    //     int size = 17482;
    //     void *buf = memory_adapter->mem_malloc(size);
    //     // file_adapter->file_read(buf, size, handle);
    //     // print_r(buf, size);
    //     memset(buf, 0x55, size);
    //     file_adapter->file_write(buf, size, handle);
    //     file_adapter->file_close(handle);
    // }

    // char path[] = "B:awk/cache/15-2675514254-2675514254.ca";
    // const awk_file_adapter_t *file_adapter = awk_file_get_adapter();
    // const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    // printf("file: %d %d\n", file_adapter->file_exists(path), file_adapter->file_get_size(path));
    // void *handle = file_adapter->file_open(path, "r");
    // if (handle) {
    //     void *buf = memory_adapter->mem_malloc(1024);
    //     memset(buf, 0, 1024);
    //     file_adapter->file_read(buf, 1024, handle);
    //     print_r(buf, 1024);
    //     file_adapter->file_seek(handle, 32, 0);
    //     memset(buf, 0, 1024);
    //     file_adapter->file_read(buf, 1024, handle);
    //     print_r(buf, 1024);
    //     file_adapter->file_close(handle);
    // }

    // while (1);
#if 0
    {
        void *fs;
        char buff[100];
        fs = awk_file_open_adapter("awk/123456789abcdefg.bin", "wb");


        awk_file_seek_adapter(fs, 0, 0);
        memset(buff, 0x01, 100);
        awk_file_write_adapter(buff, 100, fs);
        memset(buff, 0x02, 100);
        awk_file_write_adapter(buff, 100, fs);
        memset(buff, 0x03, 100);
        awk_file_write_adapter(buff, 100, fs);
        memset(buff, 0x04, 100);
        awk_file_write_adapter(buff, 100, fs);
        memset(buff, 0x05, 100);
        awk_file_write_adapter(buff, 100, fs);
        memset(buff, 0x06, 100);
        awk_file_write_adapter(buff, 100, fs);

        awk_file_flush_adapter(fs);
        awk_file_close_adapter(fs);
    }

    while(1){
        WDT_CLR();
    }
#endif
    return true;

_fail:
    awk_fs_uninit();
    return false;
}

void awk_fs_uninit(void)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    if (fatfs) {
        memory_adapter->mem_free(fatfs);
        fatfs = NULL;
    }
}

static int mode_to_pf(const char *mode) {
    if (!mode || strlen(mode) == 0) return -1;

    // fs 底层open与read用同一个标志位判断
    int pf_mode = FA_READ;
    char primary = mode[0];

    // 判断主模式：r/w/a
    switch (primary) {
        case 'r':
            pf_mode |= FA_READ;
            break;
        case 'w':
            pf_mode |= FA_WRITE | FA_CREATE_ALWAYS;
            break;
        // case 'a':
        //     pf_mode |= FA_WRITE | FA_CREATE_NEW;
        //     break;
        default:
            return -1; // 无效模式
    }

    if (strchr(mode, '+')) {
        pf_mode |= FA_WRITE;
    }

    return pf_mode;
}

//POSIX标准-文件相关适配层代码示例：
static void* awk_file_open_adapter(const char *filename, const char *mode)
{
    TRACE("%s file:%s, mode:%s\n", __func__, filename, mode);
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();

    void *fil = NULL;

    fil = memory_adapter->mem_malloc(fs_get_sizeof_fil());
    if (!fil) {
        goto _fail;
    }

    int ret = fs_open_v2(fil, filename, mode_to_pf(mode));
    if (ret != FR_OK) {
        my_printf("%s err:%d\n", __func__, ret);
        goto _fail;
    }
    return fil;

_fail:
    if (fil) {
        memory_adapter->mem_free(fil);
    }
    return NULL;
}

static int awk_file_close_adapter(void* handler)
{
    TRACE("%s->0x%x\n", __func__, handler);

    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();

    if (!handler) {
        return 0;
    }
    fs_close_v2(handler);
    memory_adapter->mem_free(handler);
    return 0;
}

static int awk_file_seek_adapter(void *handler, long offset, int where)
{
    TRACE("%s->0x%x, offset:%x, where:%d\n", __func__, handler, offset, where);

    if (where != 0) {
        return -1;
    }
    int ret = fs_lseek_v2(handler, offset);
    if (ret != FR_OK) {
        my_printf("%s err:%d\n", __func__, ret);
        return -1;
    }
    return 0;
}

static int awk_file_flush_adapter(void *handler)
{
    // TRACE("%s->0x%x\n", __func__, handler);
    return 0;
}

static size_t awk_file_read_adapter(void *ptr, size_t size, void* handler)
{
    TRACE("%s->0x%x, size:%d\n", __func__, handler, size);
    unsigned int btr = 0;
    fs_read_v2(handler, ptr, size, &btr);

    return btr;
}

static size_t awk_file_write_adapter(void *ptr, size_t size, void* handler)
{
    TRACE("%s->0x%x 0x%x, size:%d\n", __func__, ptr, handler, size);

    unsigned int btr = 0;
    fs_write_v2(handler, ptr, size, &btr);
    return btr;
}

static bool awk_file_exists_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    void *fno = memory_adapter->mem_malloc(fs_get_sizeof_filinfo());
    if (!fno) {
        return false;
    }
    int ret = (fs_stat(path, fno) == FR_OK);
    memory_adapter->mem_free(fno);
    return ret;
}

static bool awk_file_dir_exists_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    // 默认都存在，一开始全部创建好
    return true;
}

static int awk_file_remove_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    return 1;
}

static int awk_file_mkdir_adapter(const char *path, uint16_t model)
{
    TRACE("%s:%s\n", __func__, path);
    int ret = fs_mkdir(path);
    if (ret != FR_OK) {
        TRACE("err:%d\n", ret);
    }
    return 1;
}

static int awk_file_rmdir_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);

    return 1;
}

static void* awk_file_opendir_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    void *dir = memory_adapter->mem_malloc(fs_get_sizeof_dir());
    if (!dir) {
        return NULL;
    }
    int ret = fs_opendir_v2(dir, path);
    if (ret != FR_OK) {
        memory_adapter->mem_free(dir);
        return NULL;
    }

    return dir;
}

static int awk_file_closedir_adapter(void *dir)
{
    TRACE("%s\n", __func__);
    if (!dir) {
        return 1;
    }
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    fs_closedir_v2(dir);
    memory_adapter->mem_free(dir);

    return 1;
}

static bool awk_file_readdir_adapter(void *dir, awk_readdir_result *result)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    TRACE("%s, node size:%d, return:%x\n", __func__, sizeof(awk_file_node), __builtin_return_address(0));

    awk_file_node *node;
    FRESULT res;
    result->nodes = NULL;
    result->size = 0;

    void *fno = memory_adapter->mem_malloc(fs_get_sizeof_filinfo());
    if (!fno) {
        return 0;
    }
    while (1) {
        res = fs_readdir_v2(dir, fno);
        if (res != FR_OK || fs_filinfo_fname(fno)[0] == 0) {
            break;
        }
        if (fs_filinfo_fname(fno)[0] == '.') {
            break;
        }
        if (!fs_filinfo_is_dir(fno)) {
            result->size++;
            result->nodes = memory_adapter->mem_realloc(result->nodes, sizeof(awk_file_node) * result->size);
            if (!result->nodes) {
                result->size = 0;
                break;
            }
            int num = result->size - 1;
            node = &result->nodes[num];
            strcpy(node->file_name, fs_filinfo_fname(fno));
            node->file_size = fs_filinfo_fsize(fno);
        }
    }

    memory_adapter->mem_free(fno);
    TRACE("%s node:%x, num:%d\n", __func__, result->nodes, result->size);

    return 1;
}

static size_t awk_file_get_size_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    size_t size = 0;
    void *fno = memory_adapter->mem_malloc(fs_get_sizeof_filinfo());
    if (!fno) {
        return size;
    }
    if (fs_stat(path, fno) == FR_OK) {
        size = fs_filinfo_fsize(fno);
    }
    memory_adapter->mem_free(fno);
    TRACE("%s->%d\n", __func__, size);
    return size;
}

static long awk_file_get_last_access_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    return 1;
}

static int awk_file_rename_adapter(const char *old_name, const char *new_name)
{
    TRACE("%s\n", __func__);
    int ret = fs_rename_v2(old_name, new_name);
    if (ret != FR_OK) {
        printf("err:%d\n", ret);
    }
    return 1;
}

static bool awk_file_unzip_adapter(const char *zip_file, const char *out_dir)
{
    TRACE("%s\n", __func__);
    return 1;
}

static const awk_file_adapter_t awk_file_adapter = {
    .file_open = awk_file_open_adapter,
    .file_close = awk_file_close_adapter,
    .file_read = awk_file_read_adapter,
    .file_write = awk_file_write_adapter,
    .file_mkdir = awk_file_mkdir_adapter,
    .file_exists = awk_file_exists_adapter,
    .file_remove = awk_file_remove_adapter,
    .file_opendir = awk_file_opendir_adapter,
    .file_closedir = awk_file_closedir_adapter,
    .file_readdir = awk_file_readdir_adapter,
    .file_seek = awk_file_seek_adapter,
    .file_flush = awk_file_flush_adapter,
    .file_rmdir = awk_file_rmdir_adapter,
    .file_dir_exists = awk_file_dir_exists_adapter,
    .file_get_size = awk_file_get_size_adapter,
    .file_get_last_access = awk_file_get_last_access_adapter,
    .file_rename = awk_file_rename_adapter,
    .file_unzip = awk_file_unzip_adapter,
};

const awk_file_adapter_t *awk_file_get_adapter(void)
{
    return &awk_file_adapter;
}
#endif

