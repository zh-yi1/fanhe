#ifndef _API_IIS_H
#define _API_IIS_H

/**
 * @param io_map (mclk,bclk,do,di,lrclk) 1: B4,H5,H5,H6,H7 2:E2,F1,F2,E1,F3
 * @param is_slave false: master mode; true: slave mode
 * @param is_32bit false: 16bit; true: 32bit
 * @param is_normal false: left justified mode; true: iis normal mode
 * @param has_di 
 * @param has_do 
 * @param has_mclk 
 */
// clang-format off
void iis_base_init(u8 io_map, bool is_slave, bool is_32bit, bool is_normal, bool has_di, bool has_do, bool has_mclk);
// clang-format on
bool iis_spr_set(u8 spr);
void iis_dma_init(u32 *dmai_buf, u32 *dmao_buf, u8 samples);
void iis_start(void);
void iis_stop(void);

#endif
