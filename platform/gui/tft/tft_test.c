#include "include.h"

#include "lcdde.h"
#include "spide.h"

#define LCD_WIDTH         360

#define gggbbbbb_rrrrrggg_sb(ptr,r,g,b) ({\
*ptr++ = (b>>3) | (g<<3);\
*ptr++ = (g>>5) | r;\
})
#define gggbbbbb_rrrrrggg_sh(ptr,r,g,b) ({\
*ptr++ = (b>>3) | (g<<3) | (r<<8);\
})

#define rrrrrggg_gggbbbbb_sb(ptr,r,g,b) ({\
*ptr++ = (g>>5) | r;\
*ptr++ = (b>>3) | (g<<3);\
})
#define rrrrrggg_gggbbbbb_sh(ptr,r,g,b) ({\
*ptr++ = r | (b<<5) | (g>>5) | (g<<11);\
})


#define RGB565_sb(...) gggbbbbb_rrrrrggg_sb(__VA_ARGS__)
#define RGB565_sh(...) gggbbbbb_rrrrrggg_sh(__VA_ARGS__)

#define bsp_spide_out_rgb565(...) bsp_spide_out_bgr565b(__VA_ARGS__)

void gc9c01_init(lcddev_t *lcd, uint32_t grp, int cd9bit);
void bsp_spide_out_bgr565b(uint8_t *fb, uint32_t w, uint32_t h, uint32_t stride);
void bsp_lcdout_apply_lcdinfo (lcddev_t *lcd);
void bsp_lcdde_init (void);

void bsp_lcdout_set_fb_fmt (uint32_t fmt);
void bsp_spide_dma_kick (uint8_t *buf, uint32_t len);
uint32_t bsp_spide_dma_wait (void);

AT(.disp.line_buf)
uint8_t line_buf[LCD_WIDTH*4*32];   // 按ARGB8888分配32行

lcddev_t lcd;


void bsp_spide_clk_init (void)
{
    CLKDIVCON2 = (CLKDIVCON2 & ~(BIT(4) * 0xF)) | BIT(4) * 2;   // LCDDE div
    CLKGAT2 |= BIT(4);
    RSTCON0 |= BIT(8);
}
void lcddev_init(void)
{
    //lcd.outrgb = TEST_LCDOUT;
    gc9c01_init (&lcd, 1, 0); // group:1 cd9bit:0

    //bsp_lcdout_apply_lcdinfo (&lcd);
}

void gen_pic565_half_part_r (uint8_t *buf, uint32_t width, uint32_t height, uint32_t stride, uint32_t from)
{
    int i, j;
    uint16_t *ptr;

    for (i = 0; i < height; ++i) {
        ptr = (uint16_t*)(buf + i * stride);
        for (j = 0; j < width; ++j) {
            uint8_t r, g, b;
            r = 0xF8; g = b = 0;
            RGB565_sh (ptr,r,g,b);
            //*ptr++ = (r<<8) | (g<<3) | (b>>3);
        }
        ++from;
    }
}

void gen_pic565_half_part_g (uint8_t *buf, uint32_t width, uint32_t height, uint32_t stride, uint32_t from)
{
    int i, j;
    uint16_t *ptr;

    for (i = 0; i < height; ++i) {
        ptr = (uint16_t*)(buf + i * stride);
        for (j = 0; j < width; ++j) {
            uint8_t r, g, b;
            g = 0xFC; r = b = 0;
            RGB565_sh (ptr,r,g,b);
            //*ptr++ = (r<<8) | (g<<3) | (b>>3);
        }
        ++from;
    }
}

void gen_pic888_byte_c (uint8_t *buf, uint32_t width, uint32_t height, uint32_t stride, int c)
{
    int i, j, k;
    uint8_t *ptr;

    for (i = 0; i < height; ++i) {
        ptr = buf + i * stride;
        for (j = 0; j < width; ++j) {
            k = 0;
            *ptr++ = (c == k++) ? 0xFF : 0;
            *ptr++ = (c == k++) ? 0xFF : 0;
            *ptr++ = (c == k++) ? 0xFF : 0;
        }
    }
}

void gen_pic8888_byte_c (uint8_t *buf, uint32_t width, uint32_t height, uint32_t stride, int c)
{
    int i, j, k;
    uint8_t *ptr;

    for (i = 0; i < height; ++i) {
        ptr = buf + i * stride;
        for (j = 0; j < width; ++j) {
            k = 0;
            *ptr++ = (c == k++) ? 0xFF : 0;
            *ptr++ = (c == k++) ? 0xFF : 0;
            *ptr++ = (c == k++) ? 0xFF : 0;
            *ptr++ = (c == k++) ? 0xFF : 0;
        }
    }
}

void test_lcd_spide (void)
{
    printf ("%s\n", __func__);
    int i;

    lcd.set_win (0, 0, lcd.width-1, lcd.height-1);
    lcd.pixel_begin ();
    for (i = 0; i < lcd.height; i += 8) {
        gen_pic565_half_part_g (line_buf, lcd.width, 8, lcd.width*2, i);
        bsp_spide_out_rgb565 (line_buf, lcd.width, 8, lcd.width*2);
    }
    lcd.pixel_end ();
}

void test_lcd_spide_dma_565 (void)
{
    printf ("%s\n", __func__);

    int i;

    bsp_lcdout_set_fb_fmt (LCDFB_RGB565);//(LCDFB_BYTES);//
    printf("RRRR: %x\n", DESPICON);
    gen_pic565_half_part_r (line_buf, lcd.width, 8, lcd.width*2, 0);

    lcd.set_win (0, 0, lcd.width-1, lcd.height-1);
    lcd.pixel_begin ();
//bsp_spide_dump_regs ();
    for (i = 0; i < lcd.height; i += 8) {
        //gen_pic565_half_part_3 (line_buf, lcd.width, 8, lcd.width*2, i);
        bsp_spide_dma_kick (line_buf, lcd.width * 8 * 2);
        bsp_spide_dma_wait ();
//bsp_spide_dump_regs ();
    }
    lcd.pixel_end ();
}

void test_lcd_spide_dma_888 (void)
{
    printf ("%s\n", __func__);
    int i;

    bsp_lcdout_set_fb_fmt (LCDFB_RGB888);
    gen_pic888_byte_c (line_buf, lcd.width, 8, lcd.width*3, 1);//0:b 1:g 2:r

    lcd.set_win (0, 0, lcd.width-1, lcd.height-1);
    lcd.pixel_begin ();
//bsp_spide_dump_regs ();
    for (i = 0; i < lcd.height; i += 8) {
        //gen_pic565_half_part_3 (line_buf, lcd.width, 8, lcd.width*2, i);
        bsp_spide_dma_kick (line_buf, lcd.width * 8 * 3);
        bsp_spide_dma_wait ();
    }
    lcd.pixel_end ();
}

void test_lcd_spide_dma_8888 (void)
{
    printf ("%s\n", __func__);
    int i;

    bsp_lcdout_set_fb_fmt (LCDFB_ARGB8888);
    gen_pic8888_byte_c (line_buf, lcd.width, 8, lcd.width*4, 0);//0:b 1:g 2:r 3:a

    lcd.set_win (0, 0, lcd.width-1, lcd.height-1);
    lcd.pixel_begin ();
//bsp_spide_dump_regs ();
    for (i = 0; i < lcd.height; i += 8) {
        //gen_pic565_half_part_3 (line_buf, lcd.width, 8, lcd.width*2, i);
        bsp_spide_dma_kick (line_buf, lcd.width * 8 * 4);
        bsp_spide_dma_wait ();
    }
    lcd.pixel_end ();
}


u8 temp_buf[512];

void tft_init1()
{
    lcddev_init ();
    //bsp_lcdde_init ();
}

void os_gui_draw_test(void);

void tft_test()
{
    printf("tft_test\n");
//#define     OS_FUNC_BASE                    0x84100
//#define     OS_FUNC_BASE8                   (OS_FUNC_BASE + 0x1C0)
//#define     os_spiflash_lock                ((os_void_func)(OS_FUNC_BASE8 + 32))
//#define     os_spiflash_unlock              ((os_void_func)(OS_FUNC_BASE8 + 36))
//typedef void (*os_void_func)(void);
//bool spiflash_read_kick(void *buf, u32 addr, uint len);
//bool spiflash_read_wait(void);
//    os_spiflash_lock();
//    spiflash_read_kick(temp_buf, 0x100C00, 512);
//    spiflash_read_wait();
//    os_spiflash_unlock();
//print_r(temp_buf, 512);

    test_lcd_spide();
    test_lcd_spide_dma_565();
    test_lcd_spide_dma_888();
    test_lcd_spide_dma_8888();

    DESPICON = BIT(27) | BIT(9) | BIT(3) | BIT(2) | BIT(0);                //[28:27]IN RGB565, [9]MultiBit, [3:2]4BIT, [0]EN


    os_gui_draw_test();
}
