#include "include.h"

#if FUNC_REC_EN

#if FUNC_REC_TO_SD

#define F_NAME   "MIC"

static u8 rec_file_name[32] AT(.buf.rec_file);
static CIRCST *rec_circst;
static u8 rec_generate_type AT(.buf.rec_file);

static void rec_index2name(WORD index, char *name)
{
    memcpy(name, F_NAME, 3);
    index = index%10000;

    name[3] = index/10000 + '0';
    name[4] = index%10000/1000 + '0';
    name[5] = index%1000/100 + '0';
    name[6] = index%100/10 + '0';
    name[7] = index%10 + '0';
    if (rec_generate_type == REC_WAV) {
        memcpy(&name[8], ".wav\0", 5);
    } else if (rec_generate_type == REC_MP3) {
        memcpy(&name[8], ".mp3\0", 5);
    } else if (rec_generate_type == REC_OPUS) {
        memcpy(&name[8], ".ops\0", 5);
    }
	memcpy(rec_file_name, name, 13);
    printf("index:%d -> %s\n", index, name);
}

static bool rec_name2index(char *name, WORD *index)
{
    if (rec_generate_type == REC_WAV) {
        if((memcmp(name, F_NAME, 3) != 0) || (memcmp(name+8, ".wav", 4) != 0)){
            return false;
        }
    } else if (rec_generate_type == REC_MP3) {
        if((memcmp(name, F_NAME, 3) != 0) || (memcmp(name+8, ".mp3", 4) != 0)){
            return false;
        }
    } else if (rec_generate_type == REC_OPUS) {
        if((memcmp(name, F_NAME, 3) != 0) || (memcmp(name+8, ".ops", 4) != 0)){
            return false;
        }
    } else {
        return false;
    }

    *index = 0;
    for(u8 i=3; i<=7; i++){
        if((name[i] < '0') || (name[i] > '9')){
            return false;
        }
        *index *= 10;
        *index += name[i] - '0';
    }

    //printf("name:%s -> %d\n", name, *index);
    return true;
}

/**
 * 创建录音文件
 */
bool rec_file_creat(u32 rec_type)
{
    printf("%s\n", __func__);

	FRESULT res;

    rec_circst = (CIRCST *) ab_malloc(fs_circ_container_size());
	if(rec_circst == NULL){
		return false;
	}
    memset(rec_circst, 0, fs_circ_container_size());

#if 0
	if(dev_is_online(DEV_SDCARD)){
		strcpy(photo_circst->path, "B:\\VOICE\0");	//sd
	}else{
		strcpy(photo_circst->path, "A:\\VOICE\0");	//spiflash
	}
#else
	strcpy(rec_circst->path, "B:\\VOICE\0");	//sd
#endif
    if (rec_type == REC_WAV) {
        strcpy(rec_circst->ext, "*.wav\0");
    } else if (rec_type == REC_MP3) {
        strcpy(rec_circst->ext, "*.mp3\0");
    } else if (rec_type == REC_OPUS) {
        strcpy(rec_circst->ext, "*.ops\0");
    }
    rec_generate_type = rec_type;
	rec_circst->index2name = rec_index2name;
	rec_circst->name2index = rec_name2index;
	rec_circst->index_max  = 9999;
	rec_circst->rfile_limit = -1;
	rec_circst->rfile_type = 0;
	rec_circst->swap_hook  = NULL;
	rec_circst->temp_file_is_creat = false;
	rec_circst->del_clust_size = 256;
	res  = f_circ_open(rec_circst, 0);

	if(res != FR_OK){
		ab_free(rec_circst);
		return false;
	}

	return true;
}


/**
 * 释放录音文件
 */
void rec_file_close(u8 rec_type)
{
    printf("%s\n", __func__);
	if(rec_circst != NULL){
#if REC_WAV_SUPPORT
        if (rec_type == REC_WAV) {
            if (!record_wav_header_sync(rec_type)) {
                printf("record_wav_header_sync error\n");
            }
        }
#endif
        f_circ_close(rec_circst);
		ab_free(rec_circst);
	}
}


/**
 * 写录音文件
 */
bool rec_file_write(u8 *buff, u32 len, u8 type)
{
    FRESULT res;

    CIRCST *res_circst = rec_circst;
    res = f_circ_write(res_circst, buff, len, 0);

    return (res == FR_OK);
}


/**
 * 写录音文件
 */
bool rec_file_updata(u8 rec_type)
{
    FRESULT res;

    CIRCST *res_circst = rec_circst;

#if REC_WAV_SUPPORT
    if (rec_type == REC_WAV) {
        if (!record_wav_header_sync(rec_type)) {
            return false;
        }
    }
#endif

    res = f_circ_updata(res_circst);

    return (res == FR_OK);
}

/**
 * 获取录音文件名字
 */
u8 *rec_file_get_name(void)
{
	return rec_file_name;
}

bool rec_file_lseek(FSIZE_t ofs)
{
    FRESULT res;
    CIRCST *res_circst = rec_circst;
    res = fs_circ_wr2lseek(res_circst, ofs);
    return (res == FR_OK);
}


#if REC_WAV_SUPPORT
static u8 wav_header_buf[512] AT(.rec_buf.wav);
const u8 wav_header_tbl[52] = {
    0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20,
	0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x44, 0xAC, 0x00, 0x00, 0x10, 0xB1, 0x02, 0x00,
	0x04, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0x66, 0x61, 0x63, 0x74, 0xC8, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

bool record_wav_init(u32 nch, u32 spr, u32 rec_type)
{
    printf("%s\n", __func__);
    u8 *wbuf = ab_malloc(556);
    assert_malloc_buf_is_valid(wbuf);
    wav_header_t *wavhead = (wav_header_t *)wav_header_buf;

    memset(wav_header_buf, 0, 512);
    memcpy(wav_header_buf, wav_header_tbl, sizeof(wav_header_tbl));
    wavhead->wFormatTag = WAVE_FORMAT_PCM;
    wavhead->nChannels  = nch & 0x03;
    wavhead->nSamplesPerSec  = tbl_sample_rate[spr];
    wavhead->nAvgBytesPerSec = (wavhead->nSamplesPerSec * wavhead->nChannels * PCM_BITS) / 8;
    wavhead->nBlockAlign = (wavhead->nChannels * PCM_BITS) / 8;
    wavhead->wBitsPerSample = PCM_BITS;
    wavhead->data_id = DATA_CKID;
//#if REC_ADPCM_SUPPORT
//    if (rec_type == REC_ADPCM) {
//        wavhead->wFormatTag = WAVE_FORMAT_DVI_ADPCM;
//        wavhead->nAvgBytesPerSec = (wavhead->nSamplesPerSec * wavhead->nChannels * 4) / 8;
//        wavhead->wValidBitsPerSample = (128 - 4 * wavhead->nChannels) * 2 / wavhead->nChannels + 1;		//adpcm nSamplesPerBlock
//        wavhead->wBitsPerSample = 4;
//        wavhead->nBlockAlign = 128;
//    }
//#endif

    memcpy(wbuf, wav_header_buf, 512);
    //预留WAV头空间
    if (rec_file_write(wbuf, 512, REC_WAV) == false) {
        ab_free(wbuf);
        return false;
    }
//    wav_header_fssect = fs_circ_sclust(rec_circst);
//    printf("%s:%d\n", __func__, wav_header_fssect);
    ab_free(wbuf);
    return true;
}

bool record_wav_header_sync(u32 rec_type)
{
    printf("%s:%d\n", __func__, rec_type);
    if (rec_type == REC_WAV || rec_type == REC_ADPCM) {
        u8 *wbuf = ab_malloc(556);
        assert_malloc_buf_is_valid(wbuf);
        wav_header_t *wavhead = (wav_header_t *)wav_header_buf;
        u32 fsize = fs_circ_wr2objs(rec_circst);
        printf("%s fsize:%d\n", __func__, fsize);
        wavhead->riff_size = fsize - 8;
        wavhead->data_size = fsize - 512;
        wavhead->dwSampleLength = (fsize - 512)/wavhead->nBlockAlign;  //Number of samples
        memcpy(wbuf, wav_header_buf, 512);
        rec_file_lseek(0);
        if (rec_file_write(wbuf, 512, REC_WAV) == false) {
            ab_free(wbuf);
            return false;
        }

        ab_free(wbuf);
        rec_file_lseek(fsize);
    }
    return true;
}
#endif

#else

bool record_wav_header_sync(u32 rec_type){return true;}
bool record_wav_init(u32 nch, u32 spr, u32 rec_type){return true;}
bool rec_file_lseek(FSIZE_t ofs){return true;}
u8 *rec_file_get_name(void){return NULL;}
bool rec_file_updata(u8 rec_type){return true;}
bool rec_file_write(u8 *buff, u32 len, u8 type){return true;}
void rec_file_close(u8 rec_type){}
bool rec_file_creat(u32 rec_type){return true;}
#endif // FUNC_REC_TO_SD
#endif // FUNC_REC_EN

