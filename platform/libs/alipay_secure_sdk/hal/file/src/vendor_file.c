#include "include.h"
#include "alipay_common.h"

#if SECURITY_PAY_EN && SECURITY_TRANSITCODE_EN

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

uint os_spiflash_read(void *buf, u32 addr, uint len);
void os_spiflash_program(void *buf, u32 addr, uint len);
void os_spiflash_erase(u32 addr);
int filesystem_delete_all(void);

// 定义文件系统结构体
struct FileSystem {
    char filename[64];
    u32 filesize;
    u32 fileoffset;
};

// 文件系统初始化


struct file_system_t {
    struct FileSystem filesystem[FILE_NUM];
    u8 file_count;
    u32 ofs;
} file_system_cb AT(.alipay_file);


int initializeFileSystem(void)
{
    // 初始化文件系统

    if (cm_read8(PARAM_ALI_FILE_VALID) == 1) {
        os_spiflash_read(&file_system_cb, FILE_INFO_ADDR, sizeof(file_system_cb));
//        TRACE("***ALIPAY FILE read form flash***\n");
//        listFiles();
    } else {
        filesystem_delete_all();
    }
    return true;
}

// Flash 操作函数
int readFromFlash(int offset, void *buffer, int size) {
    // 模拟从 Flash 中读取数据
    TRACE("Reading data from Flash at offset %d, size:%d\n", offset, size);
    os_spiflash_read(buffer, FILE_BASE_ADDR + offset, size);
    TRACE("rbuf:%s\n", buffer);
    return 0;
}

int writeToFlash(int offset, void *buffer, int size) {
    // 模拟将数据写入 Flash

    char buf[size];

    memset(buf, 0, size);

    memcpy(buf, buffer, size);

    TRACE("buf:%s\n", buf);
    TRACE("Writing data to Flash at offset %d, size:%d, buf:%x\n", offset, size, buf);
    os_spiflash_program(buf, FILE_BASE_ADDR + offset, size);
    return 0;
}

int eraseFlashBlock(int addr) {
    // 模拟擦除 Flash 中的数据块
    TRACE("Erasing Flash block at addr:%x\n", addr);
    os_spiflash_erase(addr);
    return 0;
}

// 文件和目录操作函数
struct FileSystem *createFile(const char *filename) {
    // 创建一个新文件

    struct FileSystem *file = &file_system_cb.filesystem[file_system_cb.file_count];
//    struct FileSystem *file = &filesystem[0];//(struct FileSystem *)malloc(sizeof(struct FileSystem));
    strcpy(file->filename, filename);
    TRACE("CreateFile name:%s\n", file->filename);
    file->filesize = 0;
    file->fileoffset = 0;
    file_system_cb.file_count++;
    return file;
}

int deleteFile(struct FileSystem *file) {
    // 删除文件

    if (file_system_cb.file_count <= 0) {
        TRACE("%s fail count:%d\n", file_system_cb.file_count);
        return false;
    }
    char file_name[8];
    sprintf(file_name, "File%d", file_system_cb.file_count);
    strcpy(file->filename, file_name);
    file_system_cb.file_count--;
    TRACE("delete and create new:%s\n", file_name);
    return 0;
}

struct FileSystem *openFile(const char *filename) {
    // 打开已存在的文件
    struct FileSystem *file;
    for (int i = 0; i < FILE_NUM; i++) {
        file = &file_system_cb.filesystem[i];
        if (strcmp(file->filename, filename) == 0) {
            TRACE("### Open found filename[%d]:%s\n", i, filename);
            return file;
        }
    }

    TRACE("openFile %s not found\n", filename);
    return NULL;
}

int closeFile(struct FileSystem *file) {
    // 关闭文件

    os_spiflash_erase(FILE_INFO_ADDR);
    os_spiflash_program(&file_system_cb, FILE_INFO_ADDR, sizeof(file_system_cb));

    TRACE("close file and write info size %d\n", sizeof(file_system_cb));

    cm_write8(PARAM_ALI_FILE_VALID, 1);
    cm_sync();
    return 0;
}

// 文件和目录管理函数
int findFile(const char *filename) {
    // 查找文件
    struct FileSystem *file;
    for (int i = 0; i < FILE_NUM; i++) {
        file = &file_system_cb.filesystem[i];
        if (strcmp(file->filename, filename) == 0) {
            printf("found filename:%s\n", filename);
            return true;
        }
    }
    TRACE("findFile %s not found\n", filename);
    return false;
}


int readFromFile(struct FileSystem *file, void *buffer, uint32_t* read_len) {
    // 从文件中读取数据

    *read_len = *read_len < file->filesize ? *read_len : file->filesize;

    if (*read_len == 0) {
        TRACE("empty file!\n");
        return 0;
    }

    TRACE("### [%d]read_%s:%d, file->filesize:%d\n", file_system_cb.file_count, file->filename, *read_len, file->filesize);
    return readFromFlash(file->fileoffset, buffer, *read_len);
}

int writeToFile(struct FileSystem *file, void *buffer, int size) {
    // 将数据写入文件
//    file->filesize += size;

    file->filesize = size;     //覆盖写

    file->fileoffset = file_system_cb.ofs;
    TRACE("--->[%d]write %s:%d, fileoffset:%d, size:%d\n", file_system_cb.file_count, file->filename, file->filesize, file->fileoffset, size);
    writeToFlash(file->fileoffset, buffer, size);
    file_system_cb.ofs += size;
    return 0; //
}


void listFiles(void) {
    // 列出文件系统中的文件
    TRACE("*****%s*****\n", __func__);
    struct FileSystem *file;// = &file_system_cb.filesystem;
    for (int i = 0; i < FILE_NUM; i++) {
        file = &file_system_cb.filesystem[i];
        if (file->filesize) {
            TRACE("[%d] %s: size=%d, offset=%d\n", i, file->filename, file->filesize, file->fileoffset);
        }
    }
}

int my_file_test(void) {
    initializeFileSystem();

    struct FileSystem *file1 = createFile("newFile1.txt");
    writeToFile(file1, "This is a new file1.", strlen("This is a new file1."));
    closeFile(file1);

    struct FileSystem *file2 = createFile("newFile2.txt");
    writeToFile(file2, "This is a new file2.", strlen("This is a new file2."));
    closeFile(file2);

    struct FileSystem *file3 = createFile("newFile3.txt");
    writeToFile(file3, "This is a new file3.", strlen("This is a new file3."));
//    deleteFile(file3);
    closeFile(file3);

    struct FileSystem *file4 = createFile("newFile4.txt");
    writeToFile(file4, "This is a new file4.", strlen("This is a new file4."));
    closeFile(file4);

    struct FileSystem *file = openFile("newFile4.txt");
    if (file != NULL) {
        char buffer[32];
        uint32_t len = sizeof(buffer);
        memset(buffer, 0, sizeof(buffer));
        writeToFile(file4, "My modify file4.", strlen("This is a new file4."));
        readFromFile(file, buffer, &len);
        TRACE("Read from newFile4.txt: %s\n", buffer);
        closeFile(file);
    }

    listFiles();

    return 0;
}



/*
 * 打开文件名为filename的文件，要求权限为可读写；
 * 参数：filename [in] 文件名(字符串)
 * 返回值: 文件句柄: 成功
 *        -1 或者 NULL: 失败
*/
void* alipay_open_rsvd_part(PARAM_IN char filename[128])
{
    struct FileSystem *file_ptr = openFile(filename);

    if (file_ptr) {
        return file_ptr;
    } else {
        return createFile(filename);
    }

}

int filesystem_delete_all(void)
{
    TRACE("%s start\n", __func__);
    cm_write8(PARAM_ALI_FILE_VALID, 0);
    cm_sync();
    memset(&file_system_cb, 0, sizeof(file_system_cb));
    for(u8 i=0;i<FILE_TOTAL_SIZE/4096;i++) {
        eraseFlashBlock(FILE_BASE_ADDR + 4096 * i);
        WDT_CLR();
    }
    TRACE("%s finish\n", __func__);
    return 0;
}

/*
 * 往文件中写数据
 * 参数：fd [in] 文件句柄
 *      data [in] 要写入的数据
 *      data_len [in] 要写入的数据的长度
 * 返回值: 0: 写入成功
 *        -1: 写入失败
*/
int alipay_write_rsvd_part(PARAM_IN void* fd, PARAM_IN void *data, PARAM_IN uint32_t data_len)
{
    return writeToFile(fd, data, data_len);
}

/*
 * 从提供给alipay kv存储的文件中读取数据
 * 参数：fd [in] 文件句柄
 *      buffer [out] 存放读取的数据的缓存
 *      read_len [in] 允许读取的最大长度
 *      read_len [out] 实际读取的数据长度
 * 返回值: 0: 读取成功
 *        -1: 读取失败,或者长度为0
*/
int alipay_read_rsvd_part(PARAM_IN void* fd,PARAM_OUT void *buffer, PARAM_INOUT uint32_t* read_len)
{
    return readFromFile(fd, buffer, read_len);
}

/*
 * 关闭打开的文件
 * 参数：fd [in] 文件句柄
 * 返回值: 0: 关闭成功
 *        -1: 关闭失败
*/
int alipay_close_rsvd_part(PARAM_IN void* fd)
{
    return closeFile(fd);
}

/*
 * 查看文件是否存在
 * 参数: filename [in] 文件名(字符串)
 * 返回值: 1: 文件存在
          0: 文件不存在
 */
int alipay_access_rsvd_part(PARAM_IN char filename[128])
{
    return findFile(filename);
}

/*
 * 删除文件
 * 参数: filename [in] 文件名(字符串)
 * 返回值: 0: 删除成功
 *       -1: 删除失败
 */
int alipay_remove_rsvd_part(PARAM_IN char filename[128])
{
    struct FileSystem *file_ptr = openFile(filename);
    if (file_ptr) {
        return deleteFile(file_ptr);
    } else {
        return -1;
    }
}

/*
 * 清除所有通过alipay_write_rsvd_part存储的alipay文件
 * 返回值: 0: 清除成功
 *       -1: 清除失败
 */
int alipay_clear_rsvd_part(void)
{
    return filesystem_delete_all();
}
#endif

