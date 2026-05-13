#ifndef __BSP_AUDIO_H__
#define __BSP_AUDIO_H__

#define AUDIO_PATH_SPEAKER         0
#define AUDIO_PATH_BTMIC           1
#define AUDIO_PATH_USBMIC          2
#define AUDIO_PATH_RECORDER        3
#define AUDIO_PATH_MODEMMIC        4
#define AUDIO_PATH_OPUS            5
#define AUDIO_PATH_VIDEO_REC       6
#define AUDIO_PATH_ASR             7
void audio_path_init(u8 path_idx);
void audio_path_exit(u8 path_idx);
void audio_path_start(u8 path_idx);


#endif //__BSP_AUDIO_H__

