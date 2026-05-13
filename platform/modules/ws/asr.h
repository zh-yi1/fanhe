#ifndef ASR_ASR_H_
#define ASR_ASR_H_

#if (ASR_SELECT == ASR_WS)

#ifdef __cplusplus
extern "C" {
#endif

// #define ASR_PREFETCH_EN     0
#define ASR_LOG_EN          0

int  Wanson_ASR_Init();

void Wanson_ASR_Reset();

int  Wanson_ASR_Recog(short *buf, int buf_len, const char **text, float *score);

void Wanson_ASR_Release();

#ifdef __cplusplus
}
#endif

#endif //ASR_WS

#endif
