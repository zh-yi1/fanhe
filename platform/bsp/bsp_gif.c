#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

enum GIF_DEPTH {
    GIF_RGB565      = 2,
    GIF_ARGB8888    = 4,
};

typedef struct
{
    uint8_t *img_buffer, *img_buffer_end;
} stbi__context;

typedef struct
{
   int16_t prefix;
   uint8_t first;
   uint8_t suffix;
} stbi__gif_lzw;

typedef struct
{
    uint32_t w, h;
    uint8_t *out;           // output buffer
    uint8_t *background;    // The current "background" as far as a gif is concerned
    uint8_t *history;

    uint8_t pal[256][4];    // 全局調色板
    uint8_t lpal[256][4];   // 局部調色板
    uint8_t *color_table;   // 當前使用的調色板

#ifdef WIN32
    stbi__gif_lzw codes[4096];  // lzw字典。gif最大碼長12，因此用4096
    uint8_t revstr[4096];      // 反向構建的串，用來消除遞歸
#else
    stbi__gif_lzw *codes;   // lzw字典。gif最大碼長12，因此用4096
    uint8_t *revstr;        // 反向構建的串，用來消除遞歸
#endif

    uint8_t flags;          // read from header
    uint8_t bgindex;
    uint8_t ratio;
    uint8_t eflags;
    int16_t transparent;

    uint32_t step;
    uint8_t pass;

    uint8_t depth;          // 軟件配置的輸出顏色深度

    uint32_t start_x, start_y;
    uint32_t max_x, max_y;
    uint32_t cur_x, cur_y;
    uint32_t line_size;
    uint32_t delay;

    uint8_t *(*lzw) (void *s, void *g); // lzw解码函数，NULL表示默认函数
} stbi__gif;

void bsp_gif_init (void);
void bsp_gif_fini (void);

void bsp_gif_set_stride (uint32_t stride);
void bsp_gif_set_image (uint32_t x, uint32_t y, uint32_t max_x, uint32_t depth);
int bsp_gif_get_offset (void);
void bsp_gif_out_adr (void *adr);
void bsp_gif_in_adr (void *adr, int size);
void bsp_gif_palette_adr (void *adr);
void bsp_gif_interrupt_enable (int en);
void bsp_gif_kick (void);
u32 bsp_gif_wait (void);
uint8_t *bsp_gif_lzw (stbi__context *s, stbi__gif *g);

void stbi__gif_set_img_buf (stbi__context *s, uint8_t *buf, uint32_t size);
void stbi__gif_ram_init (stbi__gif *s, uint8_t *codes, uint8_t *revstr);
const char *stbi__gif_load_header (stbi__context *s, stbi__gif *g);
void stbi__gif_init (stbi__gif *g, uint8_t depth, uint8_t *out, uint8_t *history, uint8_t *background);
uint8_t *stbi__gif_load_next (stbi__context *s, stbi__gif *g, uint8_t *two_back);
void stbi__gif_register_lzw (stbi__gif *g, uint8_t *(*lzw) (void *s, void *g));

//硬件lzw不需要定义，以下定义用于软件lzw
stbi__gif_lzw codes[4096] AT(.gif_buf.code);    // lzw字典。gif最大碼長12，因此用4096
uint8_t revstr[4096] AT(.gif_buf.rev);          // 反向構建的串，用來消除遞歸
stbi__gif g;// AT(.gif_buf.stbi);
stbi__context s ;//AT(.gif_buf.stbi);


void bsp_gif_play(u8 *buf_out, u8 *buf_in, u32 buf_len)
{
    TRACE("%s obuf:%x, ibuf:%x, len:%d\n", __func__, buf_out, buf_in, buf_len);
    bsp_gif_init ();
    stbi__gif_set_img_buf (&s, buf_in, buf_len); // uart_put_bytes (buf, 32);
    u8 *buf_check = (uint8_t*)stbi__gif_load_header (&s, &g);
    if (buf_check) {
        TRACE ("Error: %s\n", buf_out);
        return;
    }

    stbi__gif_ram_init(&g, (uint8_t *)codes, revstr);
    stbi__gif_init (&g, GIF_RGB565, &buf_out[8], NULL, NULL);
#if 1
    stbi__gif_register_lzw (&g, (void*)bsp_gif_lzw);    //硬件解码
#else
    stbi__gif_register_lzw (&g, (void*)NULL);           //软件解码
#endif
    buf_out[0] = 0x50;
    buf_out[1] = 0x41;
    buf_out[2] = 0x02;
    buf_out[3] = 0x00;
    u16 *wptr = (u16*)&buf_out[4];
    wptr[0] = g.w;
    wptr[1] = g.h;

}

bool bsp_gif_play_process(u8 *buf_out, u8 *buf_in, u32 buf_len)
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks, g.delay)) {
        TRACE("%s obuf:%x, ibuf:%x, len:%d, delay:%d\n", __func__, buf_out, buf_in, buf_len, g.delay);
        ticks = tick_get();

        uint8_t *o = stbi__gif_load_next (&s, &g, NULL);

        if (o == NULL || o == (uint8_t *)&s) {
            TRACE("gif end.");
            stbi__gif_set_img_buf (&s, buf_in, buf_len); // uart_put_bytes (buf, 32);
            u8 *buf_check = (uint8_t*)stbi__gif_load_header (&s, &g);
            if (buf_check) {
                TRACE ("Error: %s\n", buf_out);
                return false;
            }
            stbi__gif_ram_init(&g, (uint8_t *)codes, revstr);
            stbi__gif_init (&g, GIF_RGB565, &buf_out[8], NULL, NULL);
        #if 1
            stbi__gif_register_lzw (&g, (void*)bsp_gif_lzw);    //硬件解码
        #else
            stbi__gif_register_lzw (&g, (void*)NULL);           //软件解码
        #endif
        }
        return true;

        TRACE ("frame %2d end : %08X\n", frame, o - buf_in);
    } else {
        return false;
    }


}

void bsp_gif_stop(void)
{
    bsp_gif_fini();
}

