#include "include.h"
#include "awk_adapter.h"
#include "awk_mem.h"

#if AWK_MAP_EN

#define  TRACE_EN   1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

FRESULT pf_seek_bytes(void *fp, DWORD b_ofs, u8 whence);
FRESULT pf_read (void *fp, void* buff, UINT btr, UINT* br);  /* Read data from the open file */
FRESULT pf_write (void *fp, void* buff, UINT btw);           /* Write data to the open file */
FRESULT pf_close(void *fp);                                  /* Close File */
FRESULT pf_opendir_path (void *dj, void *fp, const char* path);           /* Open a directory from path */

const awk_memory_adapter_t *awk_mem_get_adapter(void);
const awk_file_adapter_t *awk_file_get_adapter(void);

#define LONG_NAME_MAX              100
static char lfn_buf[LONG_NAME_MAX];

// static char test_buf[1024]= {0};
// const char *test_str = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
// const char *test_str2 = "abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuioqwertyuiozxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjklghjkl";

void awk_fs_init(void)
{
    SD0_LDO_EN();    //打开SD供电

    sys_cb.cur_dev = DEV_SDCARD;
    fsdisk_callback_init(sys_cb.cur_dev);
    // memset(&msc_cb, 0, sizeof(msc_cb));
    // msc_cb.fname = &fname_buf[0];
    fs_var_init();

    if(fs_mount() == false){
        printf("fs_mount fail\n");
    }else{
        printf("fs_mount ok\n");
    }

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
    fs_mkdir("map");
    fs_mkdir("awk");
    fs_mkdir("awk/off_ca");
    fs_mkdir("awk/cache");

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
}

//POSIX标准-文件相关适配层代码示例：
static void* awk_file_open_adapter(const char *filename, const char *mode)
{
    TRACE("%s file:%s, mode:%s\n", __func__, filename, mode);
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();

    void *fil = NULL;
    char *copy = NULL;

    fil = memory_adapter->mem_malloc(fs_get_sizeof_fil());
    if (!fil) {
        goto _fail;
    }

    copy = memory_adapter->mem_malloc(strlen(filename) + 1);
    if (!copy) {
        goto _fail;
    }
    strcpy(copy, filename);

    int ret = fs_open_ex(fil, copy, &lfn_buf, mode);
    if (ret != FR_OK) {
        my_printf("%s err:%d\n", __func__, ret);
        goto _fail;
    }
    if (copy) {
        memory_adapter->mem_free(copy);
    }
    return fil;

_fail:
    if (fil) {
        memory_adapter->mem_free(fil);
    }
    if (copy) {
        memory_adapter->mem_free(copy);
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
    fs_lock();
    pf_close(handler);
    sd0_stop(1);
    fs_unlock();
    memory_adapter->mem_free(handler);
    return 0;
}

static int awk_file_seek_adapter(void *handler, long offset, int where)
{
    TRACE("%s->0x%x, offset:%x, where:%d\n", __func__, handler, offset, where);

    fs_lock();
    int ret = pf_seek_bytes(handler, offset, where);
    fs_unlock();
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
    fs_lock();

    int ret = pf_read(handler, ptr, size, &btr);
    if (ret) {
        printf("pf_read %d\n", ret);
    }
    fs_unlock();

    return btr;
}

static size_t awk_file_write_adapter(void *ptr, size_t size, void* handler)
{
    TRACE("%s->0x%x 0x%x, size:%d\n", __func__, ptr, handler, size);

    fs_lock();
    int ret = pf_write(handler, ptr, size);
    if (ret) {
        printf("write %d\n", ret);
    }

    fs_unlock();
    return size;
}

static bool awk_file_exists_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    void *ptr = awk_file_open_adapter(path, "r");
    if (ptr) {
        awk_file_close_adapter(ptr);
        return true;
    }
    return false;
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
    fs_lock();
    int ret = fs_mkdir(path);
    fs_unlock();
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
    u8 fil_t[fs_get_sizeof_fil()];
    memset(&fil_t, 0, fs_get_sizeof_fil());

    if (!dir) {
        return NULL;
    }
    // todo:dir->fn 是内部临时buf
    fs_lock();
    int ret = pf_opendir_path(dir, &fil_t, path);
    fs_unlock();
    // printf("dj:%s\n", dir->fn);
    // print_r(dir->dir, 32);
    if (ret != FR_OK) {
        memory_adapter->mem_free(dir);
        return NULL;
    }

    return dir;
}

static int awk_file_closedir_adapter(void *dir)
{
    TRACE("%s\n", __func__);
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    memory_adapter->mem_free(dir);

    return 1;
}

static bool readdir_cb(void *info, u32 size, void *user_data)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();

    awk_file_node *node;
    awk_readdir_result *result = user_data;

    if (!size) {
        return false;
    }
    result->size++;
    result->nodes = memory_adapter->mem_realloc(result->nodes, sizeof(awk_file_node) * result->size);
    if (!result->nodes) {
        // 提前结束
        result->size = 0;
        return true;
    }
    int num = result->size - 1;
    node = &result->nodes[num];
    u16 name_len = 98;
    pf_get_fname(info, lfn_buf, &name_len);
    // printf("name_len=%d\n", name_len);
    // print_r(lfn_buf, name_len);
    if (name_len > 64) {
        name_len = 64;
    }
    memcpy(node->file_name, lfn_buf, name_len);
    node->file_size = size;
    TRACE("scan file:%s %s %d %d\n", node->file_name, lfn_buf, name_len, size);
    return false;
}

static bool awk_file_readdir_adapter(void *dir, awk_readdir_result *result)
{
    TRACE("%s, node size:%d, return:%x\n", __func__, sizeof(awk_file_node), __builtin_return_address(0));

    result->nodes = NULL;
    result->size = 0;
    fs_readdir_ex(dir, readdir_cb, result);

    TRACE("%s node:%x, num:%d\n", __func__, result->nodes, result->size);

    return 1;
}

static bool get_size_cb(void *info, u32 size, void *user_data)
{
    awk_file_node *node = user_data;
    u16 name_len = 98;
    pf_get_fname(info, lfn_buf, &name_len);
    TRACE("scan file:%s %s %d %d\n", node->file_name, lfn_buf, name_len, size);
    if (strcmp(lfn_buf, node->file_name) == 0) {
        node->file_size = size;
        return true;
    }
    return false;
}

static size_t awk_file_get_size_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    awk_file_node node = {0};
    u8 dir[fs_get_sizeof_dir()];

    char *last_sep = fs_get_path_last_sep(path);
    node.file_name[0] = 0;
    if (last_sep != NULL) {
        u8 fil[fs_get_sizeof_fil()];
        memset(&fil, 0, fs_get_sizeof_fil());
        // 计算目录部分的长度
        int dir_len = last_sep - path;

        if (dir_len >= 64) {
            return 0;
        }

        // 拷贝目录部分
        strncpy(node.file_name, path, dir_len);
        node.file_name[dir_len] = '\0';  // 手动添加字符串结束符

        fs_lock();
        int ret = pf_opendir_path((void *)dir, &fil, node.file_name);
        fs_unlock();
        if (ret != FR_OK) {
            TRACE("open dir fail\n");
            return 0;
        }
        strcpy(node.file_name, last_sep + 1);
    }
    fs_readdir_ex((void *)dir, get_size_cb, &node);

    TRACE("%s->%d\n", __func__, node.file_size);
    return node.file_size;
}

static long awk_file_get_last_access_adapter(const char *path)
{
    TRACE("%s:%s\n", __func__, path);
    return 1;
}

static int awk_file_rename_adapter(const char *old_name, const char *new_name)
{
    TRACE("%s\n", __func__);
    fs_lock();
    int ret = fs_rename(old_name, new_name, new_name);
    fs_unlock();
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
