#include "include.h"
#if HS_TEST_EN
#include "hs_private.h"
#include "iotsec.h"

//extern csi_error_t csi_rng_get_data(uint8_t *data, uint32_t size);
//extern csi_error_t csi_get_huk(uint8_t *uid,uint32_t *len);
//extern csi_error_t csi_open_asset(void **fd, char asset_name[36]);
//extern csi_error_t csi_write_asset(void* fd,uint32_t offset,void *data,uint32_t data_len);
//extern csi_error_t csi_read_asset(void* fd,uint32_t offset,void *buffer,uint32_t* data_len);
//extern csi_error_t csi_close_asset(void* fd);
//extern csi_error_t csi_access_asset(uint32_t *access_flag, char asset_name[36]);
//extern csi_error_t csi_remove_asset(char asset_name[36]);
//extern csi_error_t csi_clear_assets(void);

extern void *Memory_Malloc (unsigned int size);
extern void Memory_Free (void *pv);
//extern void pringt_string_log(char *format, ...);


//void *Memory_Malloc (unsigned int size)
//{
//    return mem_malloc(size);
//}
//
//void Memory_Free (void *pv)
//{
//    mem_free(pv);
//}


#define MyPrintf(...)    a_printf(__VA_ARGS__)

unsigned char pTestBuf[4096]    AT(.hs.pTestBuf);
//static unsigned char pTestBuf[4096];

void IIC_TEST_DEMO(void)
{
	unsigned int i;
	uint32_t dwTmp;
	uint32_t dwReadLen;
	unsigned int addrFile1;
	unsigned int addrFile2;

    memset(pTestBuf, 0, 4096);
    MyPrintf("HS_IIC_Init\n");
	HS_IIC_Init();
    MyPrintf("HS_IIC_Init done\n");

	//pTestBuf = Memory_Malloc(4096);
    memset(pTestBuf, 0, 4096);

    MyPrintf("Memory_Malloc done\n");

	//测试串口打印数据是否正确
	for(i=0; i<256; ++i)
	{
		pTestBuf[i] = 255 - i;
		//printf("pTestBuf[%d]:%d  ", i, pTestBuf[i]);
	}
	MyPrintf("\n");
	MyPrintf("Uart Test,data = 0xFF - 0x00 :\n");
	for (i=0; i< 256; ++i)
	{
		//printf("pTestBuf[%d]:%d  ", i, pTestBuf[i]);
		MyPrintf("%d  ", pTestBuf[i]);
	}
	MyPrintf("\n");


	MyPrintf("IIC_TEST_DEMO test start:\n\n");


	//获取随机数测试
	pTestBuf[0] = csi_rng_get_data(pTestBuf+1, 1024);

	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_rng_get_data ERROR!  ");
		MyPrintf("%02x \n", pTestBuf[0]);
	}
	else
	{
		MyPrintf("csi_rng_get_data OK, random =  \n");
		for (i=0; i< 1024; ++i)
		{
			MyPrintf("%02x", pTestBuf[1+i]);
		}
		MyPrintf("\n");
	}


	//获取SN测试
	pTestBuf[0] = csi_get_huk(pTestBuf+1, &dwTmp);
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_get_huk ERROR!  ");
		MyPrintf("%02x \n", pTestBuf[0]);
	}
	else
	{
		MyPrintf("csi_get_huk OK, ID =  \n");
		for (i=0; i< dwTmp; ++i)
		{
			MyPrintf("%02x", pTestBuf[1+i]);
		}
		MyPrintf("\n");
	}

	//删除所有文件
	pTestBuf[0] = csi_clear_assets();
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_clear_assets ERROR! \n");
	}
	else
	{
		MyPrintf("csi_clear_assets OK \n");
	}


	//创建测试文件1
	lmemset(pTestBuf, 0x00, 36);
	lmemset(pTestBuf+1, 0x11, 32);
	printf("addrFile1: 0x%x\n", addrFile1);
	pTestBuf[0] = csi_open_asset((void**)(&addrFile1), (char *)(pTestBuf+1));
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_open_asset ERROR! \n");
	}
	else
	{
		MyPrintf("csi_open_asset OK \n");
	}

    printf("addrFile1: 0x%x\n", addrFile1);
	//写文件1
	for(i=0; i<256; ++i)
	{
	    pTestBuf[i+1] = i % 256;
	}
	//pTestBuf[0] = csi_write_asset((void*)addrFile1, 128, pTestBuf+1, 256);
    pTestBuf[0] = csi_write_asset((void*)addrFile1, 128, pTestBuf+1, 256);
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_write_asset ERROR! \n");
	}
	else
	{
		MyPrintf("csi_write_asset OK \n");
	}

	//读文件1
	dwReadLen = 1024;
	pTestBuf[0] = csi_read_asset((void*)addrFile1,128, pTestBuf+1, &dwReadLen);
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_read_asset ERROR! \n");
	}
	else
	{
		MyPrintf("csi_read_asset OK \n");
		for(i=0; i<256; ++i)
		{
			if(pTestBuf[i+1] != i)
			{
				break;
			}
		}
		if(i == 256)
		{
			MyPrintf("csi_read_asset -- Data Check OK \n");
		}
		else
		{
			MyPrintf("csi_read_asset -- Data Check ERROR \n");
		}
	}

	//关闭文件1
	pTestBuf[0] = csi_close_asset((void*)addrFile1);
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_close_asset ERROR! \n");
	}
	else
	{
		MyPrintf("csi_close_asset OK \n");
	}


	//创建文件2
	lmemset(pTestBuf, 0x00, 36);
	lmemset(pTestBuf+1, 0x22, 32);
	printf("addrFile2: 0x%x\n", addrFile2);
	pTestBuf[0] = csi_open_asset((void**)(&addrFile2), (char *)(pTestBuf+1));
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_open_asset file2 ERROR! \n");
	}
	else
	{
		MyPrintf("csi_open_asset file2 OK \n");
	}

    printf("addrFile2: 0x%x\n", addrFile2);
	//写文件2
	for(i=0; i<256; ++i)
	{
	    pTestBuf[i+1] = i % 256;
	}

    printf("addrFile2: 0x%x\n", addrFile2);

	pTestBuf[0] = csi_write_asset((void*)addrFile2, 128, pTestBuf+1, 256);
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_write_asset file2 ERROR! \n");
	}
	else
	{
		MyPrintf("csi_write_asset file2 OK \n");
	}


	//读文件2
	dwReadLen = 1024;
	pTestBuf[0] = csi_read_asset((void*)addrFile2,128, pTestBuf+1, &dwReadLen);
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_read_asset file2 ERROR! \n");
	}
	else
	{
		MyPrintf("csi_read_asset file2 OK \n");
		for(i=0; i<256; ++i)
		{
			if(pTestBuf[i+1] != i)
			{
				break;
			}
		}
		if(i == 256)
		{
			MyPrintf("csi_read_asset file2 -- Data Check OK \n");
		}
		else
		{
			MyPrintf("csi_read_asset file2 -- Data Check ERROR \n");
		}
	}

	//关闭文件2
	pTestBuf[0] = csi_close_asset((void*)addrFile2);
	if(pTestBuf[0] != CSI_OK)
	{
		MyPrintf("csi_close_asset file2 ERROR! \n");
	}
	else
	{
		MyPrintf("csi_close_asset file2 OK \n");
	}

	MyPrintf("IIC_TEST_DEMO test END:\n\n");

	//Memory_Free(pTestBuf);
}

#undef MyPrintf

#endif // HS_TEST_EN


