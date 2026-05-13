#ifndef _FUNC_MUSIC_H
#define _FUNC_MUSIC_H

#define FB_UNICODE      0x00
#define FB_GBK          0x80
#define FB_GOT_FLN      0x10

void func_music_mp3_res_play(u32 addr, u32 len);
bool func_music_filter_switch(u8 rec_type);
void func_music_file_navigation(void);
void func_music_insert_device(u8 dev);
void func_music_remove_device(u8 dev);

#define led_music_busy()
#define led_music_play()
#define led_idle()

#endif // _FUNC_MUSIC_H
