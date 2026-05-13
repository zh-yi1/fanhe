#ifndef __BSP_OPUS_H
#define __BSP_OPUS_H

/**
 * @brief 启动mic，启动pcm数据压缩算法
 * @param[in] 无
 * @return 无
 **/
void bsp_opus_encode_start(bool flag, u32 spr, u32 bitrate);

/**
 * @brief 关闭mic，停止pcm数据压缩算法
 * @param[in] 无
 * @return 无
 **/
void bsp_opus_encode_stop(bool flag);

/**
 * @brief 获取当前mic缓存的压缩数据长度
 * @param[in] 无
 * @return 无
 **/
u16 opus_enc_data_len_get(void);

/**
 * @brief 获取当前mic缓存的压缩数据
 * @param[in] 无
 * @return 无
 **/
bool bsp_opus_get_enc_frame(u8 *buff, u16 len);

/**
 * @brief 获取当前是否在编码
 * @param[in] 无
 * @return 无
 **/
bool bsp_opus_is_encode(void);



#endif
