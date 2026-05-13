#ifndef _BSP_PHOTO_VIEW_H_
#define _BSP_PHOTO_VIEW_H_


/**
 * photo view init
 */
void bsp_photo_view_init(u32 res_addr, u32 res_size);


/**
 * photo view uninit
 */
void bsp_photo_view_uninit(void);



/**
 * bsp_photo_view_start
 */
void *bsp_photo_view_start(FIL* fp);


void *bsp_photo_get_disp_buff(void);




#endif
