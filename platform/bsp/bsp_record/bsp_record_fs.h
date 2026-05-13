#ifndef _RECORD_FILE_H_
#define _RECORD_FILE_H_

#define WAVE_FORMAT_PCM              0x0001
#define WAVE_FORMAT_DVI_ADPCM        0x0011
#define PCM_BITS                     16
#define DATA_CKID                    0x61746164

bool rec_file_creat(u32 rec_type);
void rec_file_close(u8 type);
bool rec_file_write(u8 *buff, u32 len, u8 type);
bool rec_file_updata(u8 type);
u8 *rec_file_get_name(void);
bool record_wav_init(u32 nch, u32 spr, u32 rec_type);
bool record_wav_header_sync(u32 rec_type);


typedef struct {
    ///RIFF WAVE Chunk
    u32 riff_id;                    //Chunk ID: “RIFF”
    u32 riff_size;                  //文件实际长度-8
    u32 wave_id;                    //WAVE ID: “WAVE”

    ///fmt Chunk
    u32 fmt_id;                     //Chunk ID: “fmt ”
    u32 fmt_size;                   //Chunk size: 16
    u16 wFormatTag;                 //Format code:  WAVE_FORMAT_PCM(0x0001),  WAVE_FORMAT_DVI_ADPCM(0x0011)
    u16 nChannels;                  //Number of interleaved channels
    u32 nSamplesPerSec;             //Sampling rate
    u32 nAvgBytesPerSec;            //Data rate:   (nSamplesPerSec * wBitsPerSample * nChannels)/8
    u16 nBlockAlign;                //Data block size (bytes) :  (wBitsPerSample * nChannels)/8
    u16 wBitsPerSample;             //Bits per sample (16bits)
    u16 cbSize;                     //Size of the extension (2bytes)
    u16 wValidBitsPerSample;        //Number of valid bits

    ///fact Chunk
    u32 fact_id;                    //Chunk ID: “fact”
    u32 fact_size;                  //0x01C8, skip to data Chunk begin
    u32 dwSampleLength;             //Number of samples (per channel)

    u8 reserved[452];

    ///data Chunk
    u32 data_id;                    //Chunk ID: “data”
    u32 data_size;                  //实际PCM数据长度： 文件长度-512
} wav_header_t;

#endif // _RECORD_FILE_H_
