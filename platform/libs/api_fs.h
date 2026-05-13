#ifndef _API_FS_H
#define _API_FS_H


#define FF_LFN_BUF		255
#define FF_SFN_BUF		12

#define DISKCAN_FILENOTE_SIZE       64
#define DISKCAN_FOLDNOTE_SIZE       8

typedef char TCHAR;
typedef uint64_t FSIZE_t;
typedef uint32_t LBA_t;

/* File find match attrible */

typedef enum {
    D_NORMAL = 0,       /* normal                       */
    D_RDONLY = 0x01,    /* read-only file               */
    D_HIDDEN = 0x02,    /* hidden                       */
    D_SYSTEM = 0x04,    /* system                       */
    D_VOLID  = 0x08,    /* volume id                    */
    D_DIR    = 0x10,    /* subdir                       */
    D_ARCHIVE= 0x20,    /* archive bit                  */
    D_FILE   = 0x40,	/* all attribute but D_DIR		*/
    D_FILE_1 = 0x80,    /* contain D_NORMAL,D_RDONLY,D_ARCHIVE */
    D_ALL    = (0x40 | 0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20)
} MATCH_ATTR;

/* File access mode and open method flags (3rd argument of f_open function) */
#define	FA_READ				0x01
#define	FA_WRITE			0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30
#define	FA_OPEN_HIDDEN		0x40

/* File function return code (FRESULT) */
typedef enum {
	FR_OK = 0,				/* (0) Function succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive does not work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to a prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to a prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) Could not find a valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs function aborted due to some problem */
	FR_TIMEOUT,				/* (15) Could not take control of the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated or given buffer is insufficient in size */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > FF_FS_LOCK */
	FR_INVALID_PARAMETER,	/* (19) Given parameter is invalid */
    FR_OK_CIRC_SWAP,        // swap file
} FRESULT;


/* Important information structure (IMPINFO) */
typedef struct {
    void   *fs;
	DWORD	dptr;			/* Current read/write offset find下SFN的位置*/
	DWORD	clust;			/* find Current cluster find下SFN的位置*/
	LBA_t	sect;			/* Current sector (0:Read operation has terminated) find下SFN的位置*/
    DWORD   sclust;         /* file or dir data clust find下切入目录时候获取这个目录指向的位置*/
    DWORD   fln_entry;     //FLN cluster find下LFN的位置（没有的LFN时候就是SFN的位置）
    DWORD   fln_offs;      //FNL offs find下LFN的位置（没有的LFN时候就是SFN的位置）
}IMPINFO;


typedef struct {
	FSIZE_t	fsize;			/* File size */
	WORD	fdate;			/* Modified date */
	WORD	ftime;			/* Modified time */
	WORD	crdate;			/* Created date */
	WORD	crtime;			/* Created time */
	BYTE	fattrib;		/* File attribute */
	TCHAR	altname[FF_SFN_BUF + 1];/* Alternative file name */
	TCHAR	fname[FF_LFN_BUF + 1];	/* Primary file name */
    IMPINFO impinfo;
} FILINFO;


/* Format parameter structure (MKFS_PARM) */

typedef struct {
	BYTE fmt;			/* Format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD) */
	BYTE n_fat;			/* Number of FATs */
	UINT align;			/* Data area alignment (sector) */
	UINT n_root;		/* Number of root directory entries */
	DWORD au_size;		/* Cluster size (byte) */
} MKFS_PARM;

typedef struct {
	TCHAR pattern[12];	    /* Pointer to the matching pattern  */
    bool (*match_func)(BYTE attr, char *name);
    TCHAR* path;		    /* Pointer to the directory to open */
    BYTE *fbuff;
    DWORD fbuff_len;
    BYTE *foldbuff;
    DWORD foldbuff_len;

    BYTE buff[512];
} STDSCAN_LE;


typedef struct {
    BYTE buff[512 + 120];
} FATFS;

/* File object structure (FIL) */

typedef struct {
    BYTE buff[512 + 120];
} FIL;

/*circ process*/
typedef struct {
    char path[32];
    char ext[8];
    void (*index2name)(WORD index, char *name);
    bool (*name2index)(char *name, WORD *index);
    WORD index_max;
    DWORD rfile_limit;
    bool  rfile_type; //1：按时间分文件存储ms单位 0：按大小分文件存储 bytes单位
    bool  temp_file_is_creat;   //是否创建temp文件
    u16 del_clust_size;
    u8 (*swap_hook)(void);      //tempfile hook

	u8 buff[0];
}CIRCST;


FRESULT fs_open (FIL* fp, const TCHAR* path, BYTE mode);				/* Open or create a file */
FRESULT fs_close (FIL* fp);											/* Close an open file object */
FRESULT fs_read (FIL* fp, void* buff, UINT btr, UINT* br);			/* Read data from the file */
FRESULT fs_write (FIL* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to the file */
FRESULT fs_lseek (FIL* fp, FSIZE_t ofs);								/* Move file pointer of the file object */
FRESULT fs_truncate (FIL* fp);										/* Truncate the file */
FRESULT fs_sync (FIL* fp);											/* Flush cached data of the writing file */
FRESULT fs_mkdir (const TCHAR* path);								/* Create a sub directory */
FRESULT fs_unlink (const TCHAR* path);								/* Delete an existing file or directory */
FRESULT fs_rename (const TCHAR* path_old, const TCHAR* path_new);	/* Rename/Move a file or directory */
FRESULT fs_stat (const TCHAR* path, FILINFO* fno);					/* Get file status */
FRESULT fs_chmod (const TCHAR* path, BYTE attr, BYTE mask);			/* Change attribute of a file/dir */
FRESULT fs_utime (const TCHAR* path, const FILINFO* fno);			/* Change timestamp of a file/dir */
FRESULT fs_mtime (FIL* fp, DWORD mtime);                             /* change creat time*/
FRESULT fs_chdir (const TCHAR* path);								/* Change current directory */
FRESULT fs_chdrive (const TCHAR* path);								/* Change current drive */
FRESULT fs_getcwd (TCHAR* buff, UINT len);							/* Get current directory */
u64 fs_getfree (const TCHAR* path);	                            /* Get number of free bytes on the drive */
FRESULT fs_getlabel (const TCHAR* path, TCHAR* label, DWORD* vsn);	/* Get volume label */
FRESULT fs_setlabel (const TCHAR* label);							/* Set volume label */
FRESULT fs_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
FRESULT fs_expand (FIL* fp, FSIZE_t fsz, BYTE opt);					/* Allocate a contiguous block to the file */
FRESULT fs_mount (FATFS* fs, const TCHAR* path, BYTE opt);			/* Mount/Unmount a logical drive */
FRESULT fs_mkfs (const TCHAR* path, const MKFS_PARM* opt, void* work, UINT len);	/* Create a FAT volume */
FRESULT fs_fdisk (BYTE pdrv, const LBA_t ptbl[], void* work);		/* Divide a physical drive into some partitions */
FRESULT fs_setcp (WORD cp);											/* Set current code page */
bool fs_eof(FIL* fp);                                            /* is point to file end */
int fs_error(void);                                                 /* get file errno */
u64 fs_tell(FIL* fp);                                            /* get file current ptr */
u64 fs_size(FIL* fp);                                            /* get file current size */
FRESULT fs_rmdir(const TCHAR* path);                                /* remove folder */
FRESULT fs_unmount (const TCHAR* path);                             /* unmount disk*/



/**
 * @brief 在指定文件夹里搜索（不会搜索子目录的）
 * @param[in] fno:  文件信息
 * @param[in] path: 待搜索文件夹路径
 * @param[in] pattern:查找条件，match_func非NLL有效
 * @param[in] find_attr:查找属性，比如是文件夹或文件，match_func非NLL有效
 * @param[in] match_func:匹配函数，赋NLL时候匹配pattern和find_attr；
 * @return FRESULT
 **/
FRESULT fs_findfirst (FILINFO* fno, const TCHAR* path, const TCHAR* pattern, MATCH_ATTR find_attr, bool (*match_func)(BYTE attr, char *name));

/**
 * @brief 查找文件或文件夹(不会搜索子目录的，只能配合fs_findfirst使用)
 * @param[in] fno: 文件信息，存储查找结果
 * @return FRESULT
 **/
FRESULT fs_findnext (FILINFO* fno);

/**
 * @brief 通过fno打开文件
 * @param[in] fno: 文件信息
 * @param[in] mode:打开文件权限FA_READ， FA_WRITE等
 * @return：-1打开失败，其他打开成功，返回fd，用于read，write等；
 **/
FRESULT fs_openfno (FIL* fp, FILINFO *fno, BYTE mode);

/**
 * @brief 通过fno删除文件
 * @param[in] fno: 文件信息
 **/
FRESULT fs_unlinkfno (FILINFO *fno);

/**
 * @brief 通过fno切目录
 * @param[in] fno: 文件信息
 **/
void f_chdirfno (FILINFO *fno);

/**
 * @brief 查找文件或文件夹(这个函数会搜索子目录的)
 * @param[in] std: 全局搜索信息
 * @param[in] fno: 文件信息，存储查找结果
 * @param[in] path:查找路径
 * @param[in] pattern:查找条件，match_func非NLL有效
 * @param[in] find_attr:查找属性，比如是文件夹或文件，match_func非NLL有效
 * @param[in] match_func:匹配函数，赋NLL时候匹配pattern和find_attr；
 * @return FRESULT
 **/
FRESULT fs_findall_frist (FILINFO* fno, const TCHAR* path, const TCHAR* pattern, MATCH_ATTR find_attr, bool (*match_func)(BYTE attr, char *name));

/**
 * @brief 查找文件或文件夹(这个函数会搜索子目录的，只能配合fs_findall_frist使用))
 * @param[in] std: 全局搜索信息，f_findall_frist调用过的那个
 * @param[in] fno: 文件信息，存储查找结果
 * @return FRESULT
 **/
FRESULT fs_findall_next (FILINFO* fno);

/**
 * @brief 扫描全盘，得到文件个数和文件夹个数
 * @param[in] sca:  扫描条件结构体
                 -> pattern[12] : 存储匹配条件
                 -> fbuff       : 指向一块buff，不能为NULL
                 -> fbuff_len   : buff大小
                 -> foldbuff       : 指向一块buff，可以为NULL，为NULL的时候表示不记录文件夹信息
                 -> foldbuff_len   : buff大小
 * @param[in] file_num:返回文件数量
 * @param[in] fold_num:返回文件夹数量
 * @return FRESULT
 **/
FRESULT fs_scan_disk (STDSCAN_LE *find_sca, DWORD *file_num, DWORD *fold_num);

/**
 * @brief 通过file id获取文件信息
          这个文件信息可以用于f_openfno open文件；
 * @param[in] index:  file id
 * @param[in] fno: 文件信息，存储结果
 * @return FRESULT
 **/
FRESULT fs_node2file_info (STDSCAN_LE *find_sca, DWORD index, FILINFO* fno);

/**
 * @brief 通过fold id获取文件夹信息
 * @param[in] index:  fold id
 * @param[in] fno: 文件夹信息，存储结果
 * @return FRESULT
 **/
FRESULT fs_node2folder_info (STDSCAN_LE *find_sca, DWORD index, FILINFO* fno);

/**
 * @brief 通过file id转换所在文件夹id
 * @param[in] file_index:  file id
 * @param[in] fold_index: 存储结果
 * @return FRESULT
 **/
FRESULT fs_file2folder_index (STDSCAN_LE *find_sca, DWORD file_index, DWORD* fold_index);

/**
 * @brief 通过文件夹id获取文件夹里第一个文件的id
 * @param[in] fold_index:  fold id
 * @param[in] file_index: 存储结果
 * @return FRESULT
 **/
FRESULT fs_folder2file_index (STDSCAN_LE *find_sca, DWORD fold_index, DWORD* file_index);

/**
 * @brief 通过文件夹id获取文件夹里文件数量
 * @param[in] fold_index:  fold id
 * @param[in] file_num: 存储结果
 * @return FRESULT
 **/
FRESULT fs_numoffolder (STDSCAN_LE *find_sca, DWORD fold_index, DWORD* file_num);


/**
 * @brief 建立快速seek
 * @param[in] fd:  文件句柄
 * @param[in] buff: buff指针
 * @param[in] len:  buff长度
 * @return FRESULT
 **/
FRESULT fs_creat_fastmap (FIL* fp, void* buff, u32 len);

/**
 * @brief 直接拼接，fp2拼接在fp1后面
 * @param[in] fd1:  文件句柄1.
 * @param[in] fd2:  文件句柄2.
 * @return FRESULT
 **/
FRESULT fs_cat(FIL* fp1, FIL* fp2);

/**
 * @brief 初始化fs cache，可以用来加速读写速度
 * @return FRESULT
 **/
void fs_fcache_init (void);

/**
 * @brief 反初始化fs cache
 * @return FRESULT
 **/
void fs_fcache_uninit (void);

/**
 * @brief 获取默认定义的circ
 * @return 指针
 **/
void* fs_circ_container(void);

/**
 * @brief 获取默认定义的circ大小bytes
 * @return 大小
 **/
u32 fs_circ_container_size(void);

/**
 * @brief 循环写创建函数
 * @fcirc 写指针
 * @free_per 剩余多少容量触发
 * @return FRESULT
 **/
FRESULT f_circ_open(CIRCST* fcirc, BYTE free_per);

/**
 * @brief 循环写函数
 * @fcirc 写指针
 * @buff  写内容
 * @btw   写大小
 * @is_en 是否需要自动创建下一个文件；
 * @return FRESULT
 **/
FRESULT f_circ_write(CIRCST* fcirc, const void* buff, UINT btw, bool is_en);


/**
 * @brief 关闭写循环函数
 * @fcirc 指针
 * @return FRESULT
 **/
FRESULT f_circ_close(CIRCST* fcirc);


/**
 * @brief 同步更新
 * @fcirc 指针
 * @return FRESULT
 **/
FRESULT f_circ_updata(CIRCST* fcirc);

//FRESULT fs_circ_lseek2begin(CIRCST* fcirc);

FRESULT fs_circ_wr2lseek(CIRCST* fcirc, FSIZE_t ofs);
u64 fs_circ_wr2objs(CIRCST* fcirc);
FIL* fs_circ_fp(CIRCST* fcirc);
u32 fs_circ_sclust(CIRCST* fcirc);
#endif // _API_FS_H
