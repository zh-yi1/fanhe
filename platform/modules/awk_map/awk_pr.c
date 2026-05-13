#include "include.h"
#include "awk_pr.h"
#include "awk.h"
#include "awk_mem.h"

#include "cjson/cJSON.h"
#include "rt_thread.h"

// #define  AWK_MAP_WIDTH               152
// #define  AWK_MAP_HEIGHT              181
#define  AWK_MAP_WIDTH               248
#define  AWK_MAP_HEIGHT              300

#define AWK_MAP_BMP_WIDTH_ALIGNGED4  (((AWK_MAP_WIDTH + 3) >> 2) << 2)
static bool awk_is_busy AT(.amap_buf);
static u8 zoom_value AT(.amap_buf);

static struct os_mutex awk_mutex;

bool awk_network_init(void);
bool awk_network_uninit(void);

bool awk_map_lock(bool wait)
{
    // printf("locktry:%x %d %x\n", os_thread_self(), awk_mutex.value, awk_mutex.owner);
    // if (!awk_mutex.value) {
    //     os_thread_t thread = awk_mutex.owner;
    //     print_r32(thread->sp, 32);
    // }
    bool ret = (os_mutex_take(&awk_mutex, OS_WAITING_FOREVER) == 0) ? true : false;
    // if (ret) {
    //     printf("lock:%x %x %d %x\n", __builtin_return_address(0), os_thread_self(), awk_mutex.value, awk_mutex.owner);
    // }
    return ret;
}

void awk_map_unlock(void)
{
    os_mutex_release(&awk_mutex);
    // printf("unlock:%x\n", __builtin_return_address(0));
}

/**
 *  屏幕坐标转换成地图渲染图片坐标
 *  Y轴
 */
s32 awk_tp_vx2sx(s32 xo)
{
    s32 xs;

    xs = xo * AWK_MAP_WIDTH / GUI_SCREEN_WIDTH;
    return xs;
}

/**
 *  屏幕坐标转换成地图渲染图片坐标
 *  X轴
 */
s32 awk_tp_vy2sy(s32 yo)
{
    s32 ys;

    ys = yo * AWK_MAP_HEIGHT / GUI_SCREEN_HEIGHT;
    return ys;
}

/**
 *  GUI推屏调用，判断当前是否可以推屏显示
 *  单buff机制，避免推屏的同时地图在渲染导致撕裂
 */
AT(.com_text.awk)
bool tft_te_refresh_is_enable(void)
{
    return !awk_is_busy;
}


static int map_id AT(.amap_buf);

int awk_map_get_id(void)
{
    return map_id;
}

/**
 *  地图回调
 *
 */
void awk_active_callback(bool result, int code, const char* msg)
{
    printf("%s result:%x, code:%x, msg:%s\n", __func__, result, code, msg);
}

extern u32 __amap_buf_start, __amap_buf_end, __amap_buf_size;

// static const awk_map_coord2d_t points[] = {
//     {113.947221, 22.748493},
//     {113.947060, 22.748467},
//     {113.946982, 22.748467},
//     {113.946982, 22.748467},
//     {113.946336, 22.748473},
//     {113.945040, 22.748497},
//     {113.945040, 22.748497},
//     {113.944711, 22.748498},
//     {113.944581, 22.748507},
//     {113.944585, 22.748498},
//     {113.944585, 22.748498},
//     {113.944564, 22.748507},
//     {113.944633, 22.748880},
//     {113.944633, 22.748880},
//     {113.944633, 22.748880},
//     {113.944669, 22.748878},
//     {113.944669, 22.748878},
//     {113.944638, 22.748861},
//     {113.944632, 22.748879},
//     {113.944632, 22.748879},
//     {113.944835, 22.750064},
//     {113.944931, 22.750605},
//     {113.945011, 22.751034},
//     {113.945044, 22.751131},
//     {113.945247, 22.751581},
//     {113.945296, 22.751720},
//     {113.945366, 22.751919},
//     {113.945419, 22.752208},
//     {113.945568, 22.753007},
//     {113.945756, 22.754443},
//     {113.945761, 22.755248},
//     {113.945831, 22.755645},
//     {113.945853, 22.755774},
//     {113.945901, 22.756019},
//     {113.945976, 22.756502},
//     {113.946066, 22.757065},
//     {113.946120, 22.757644},
//     {113.946152, 22.758314},
//     {113.946157, 22.758486},
//     {113.946179, 22.758915},
//     {113.946190, 22.759044},
//     {113.946222, 22.759237},
//     {113.946292, 22.759429},
//     {113.946324, 22.759665},
//     {113.946340, 22.760497},
//     {113.946350, 22.760787},
//     {113.946350, 22.760840},
//     {113.946382, 22.761371},
//     {113.946451, 22.762084},
//     {113.946505, 22.762438},
//     {113.946693, 22.763928},
//     {113.946704, 22.764271},
//     {113.946607, 22.764990},
//     {113.946602, 22.765269},
//     {113.946666, 22.766625},
//     {113.946677, 22.766931},
//     {113.946693, 22.767940},
//     {113.946693, 22.767940},
//     {113.947029, 22.767859},
//     {113.947026, 22.767832},
//     {113.947026, 22.767832},
//     {113.947029, 22.767860},
//     {113.947447, 22.767760},
//     {113.948299, 22.767565},
//     {113.948573, 22.767512},
//     {113.949365, 22.767338},
//     {113.950109, 22.767171},
//     {113.950736, 22.767024},
//     {113.951368, 22.766803},
//     {113.951470, 22.766755},
//     {113.951759, 22.766614},
//     {113.952193, 22.766404},
//     {113.952926, 22.766049},
//     {113.953151, 22.765941},
//     {113.953494, 22.765805},
//     {113.953923, 22.765713},
//     {113.954163, 22.765697},
//     {113.954480, 22.765702},
//     {113.954919, 22.765738},
//     {113.957702, 22.765967},
//     {113.958286, 22.766074},
//     {113.958533, 22.766142},
//     {113.958671, 22.766180},
//     {113.959159, 22.766366},
//     {113.959389, 22.766451},
//     {113.959742, 22.766595},
//     {113.960166, 22.766766},
//     {113.960369, 22.766867},
//     {113.960845, 22.767059},
//     {113.961321, 22.767213},
//     {113.961669, 22.767330},
//     {113.961900, 22.767405},
//     {113.962097, 22.767468},
//     {113.962473, 22.767581},
//     {113.962917, 22.767713},
//     {113.963757, 22.767968},
//     {113.963757, 22.767968},
//     {113.963772, 22.767949},
//     {113.963777, 22.767957},
//     {113.963777, 22.767957},
//     {113.963751, 22.767969},
//     {113.963751, 22.767987},
//     {113.963751, 22.768004},
//     {113.963751, 22.768004},
//     {113.963751, 22.768052},
//     {113.963768, 22.768065},
//     {113.963768, 22.768074},
//     {113.963781, 22.768087},
//     {113.963829, 22.768113},
//     {113.963846, 22.768117},
// };

/**
 *  地图初始化
 *
 */
void awk_map_init(void)
{
    int res;

    awk_map_view_param_t awk_map_view_param;

    printf("%s %x %x\n", __func__, &__amap_buf_start, &__amap_buf_size);
    u32 size = (u32)&__amap_buf_size;
    if (size) {
        memset((void *)&__amap_buf_start, 0, size);
    }
    os_mutex_init(&awk_mutex, "map", OS_IPC_FLAG_FIFO);
    CLKGAT2 |= BIT(29);
    awk_network_init();
    awk_fs_init();
    awk_map_interface_init();
    awk_activate_device(awk_active_callback);
    awk_map_view_param.port.width = AWK_MAP_WIDTH;//320;
    awk_map_view_param.port.height = AWK_MAP_HEIGHT;//385;

    map_id = awk_map_create_view(awk_map_view_param);
    printf("map id:%d\n", map_id);
    if (map_id < 0) {
        printf("awk_map_create_view err:%d\n", map_id);
    }
    zoom_value = 15;
    res = awk_map_set_level(map_id, zoom_value);

    awk_map_view_port_t view_port;
    view_port.height = AWK_MAP_HEIGHT;//385;
    view_port.width  = AWK_MAP_WIDTH;//320;
    res = awk_map_set_view_port(map_id, view_port);
    if (res < 0) {
        printf("awk_map_set_level err:%d\n", res);
    }

    awk_map_set_coord2d(113.952926, 22.766049);

    // awk_rect_area_t cache_rect;
    // cache_rect.x = -256;
    // cache_rect.y = -256;
    // cache_rect.width = AWK_MAP_HEIGHT + 256 ;
    // cache_rect.height = AWK_MAP_WIDTH + 256 ;

    // awk_map_coord2d_t coord2d;
    // coord2d.lon = 113.256550;
    // coord2d.lat = 23.118113;
    // awk_map_request_tiles(cache_rect, coord2d, 15, AWK_HTTP_PRIORITY_LOW);
    // awk_map_pause_render(map_id);

// awk_map_tile_download_callback_t *awk_download_tile_get_callback(void);
//     awk_map_download_polyline_region("test", (awk_map_coord2d_t *)points, 110, NULL, 0, awk_download_tile_get_callback());
}

/**
 *  地图反初始化
 */
void awk_map_uninit(void)
{
    int res;

    res = awk_map_destroy_view(map_id);
    if (res < 0) {
        printf("awk_map_destroy_view err:%d\n", res);
    }
    printf("awk_map_destroy_view\n");

    res = awk_uninit();
    if (res < 0) {
        printf("awk_uninit err:%d\n", res);
    }
    awk_map_tile_uninit();
    awk_network_uninit();
    awk_fs_uninit();
    os_mutex_detach(&awk_mutex);
    printf("awk_uninit\n");
}


/**
 *  地图设置经纬度坐标
 *
 */
void awk_map_set_coord2d(double lon, double lat)
{
    int res;
    awk_map_coord2d_t coord2d;

    coord2d.lat = lat;
    coord2d.lon = lon;
    //printf("awk_map_set_coord2d\n")F;

    res = awk_map_set_center(map_id, coord2d);
    if (res < 0) {
        printf("awk_map_set_center err:%d\n", res);
    }
}

/**
 *  地图放大缩小
 *  zoom_in_out :true 放大； false 缩小
 */
void awk_map_set_zoom(bool zoom_in_out)
{
    if(zoom_in_out){
        zoom_value++;
        if(zoom_value > 17){
            zoom_value = 17;
        }
    }else{
        if(zoom_value > 4){
            zoom_value--;
        }
    }

    printf("zoom:%d\n", zoom_value);
    awk_map_set_level(map_id, zoom_value);
}


/**
 *  地图渲染显示图片
 *  放在main线程，定时调用
 */
void awk_map_flush(void)
{
    int res = 0;

    if (awk_map_lock(false)) {
        res = awk_map_do_render();
        awk_map_unlock();
    }
    if (res < 0) {
        printf("awk_map_do_render err:%d\n", res);
    }else{
        awk_map_tile_flush();
    }
}


// /**
//  *   map tile manage
//  */
// static const u32 awk_map_palette[256] = {
//     0, 64, 128, 192, 8192, 8256, 8320, 8384, 16384, 16448, 16512, 16576, 24576, 24640, 24704, 24768,
//     32768, 32832, 32896, 32960, 40960, 41024, 41088, 41152, 49152, 49216, 49280, 49344, 57344, 57408, 57472, 57536,
//     2097152, 2097216, 2097280, 2097344, 2105344, 2105408, 2105472, 2105536, 2113536, 2113600, 2113664, 2113728, 2121728, 2121792, 2121856, 2121920,
//     2129920, 2129984, 2130048, 2130112, 2138112, 2138176, 2138240, 2138304, 2146304, 2146368, 2146432, 2146496, 2154496, 2154560, 2154624, 2154688,
//     4194304, 4194368, 4194432, 4194496, 4202496, 4202560, 4202624, 4202688, 4210688, 4210752, 4210816, 4210880, 4218880, 4218944, 4219008, 4219072,
//     4227072, 4227136, 4227200, 4227264, 4235264, 4235328, 4235392, 4235456, 4243456, 4243520, 4243584, 4243648, 4251648, 4251712, 4251776, 4251840,
//     6291456, 6291520, 6291584, 6291648, 6299648, 6299712, 6299776, 6299840, 6307840, 6307904, 6307968, 6308032, 6316032, 6316096, 6316160, 6316224,
//     6324224, 6324288, 6324352, 6324416, 6332416, 6332480, 6332544, 6332608, 6340608, 6340672, 6340736, 6340800, 6348800, 6348864, 6348928, 6348992,
//     8388608, 8388672, 8388736, 8388800, 8396800, 8396864, 8396928, 8396992, 8404992, 8405056, 8405120, 8405184, 8413184, 8413248, 8413312, 8413376,
//     8421376, 8421440, 8421504, 8421568, 8429568, 8429632, 8429696, 8429760, 8437760, 8437824, 8437888, 8437952, 8445952, 8446016, 8446080, 8446144,
//     10485760, 10485824, 10485888, 10485952, 10493952, 10494016, 10494080, 10494144, 10502144, 10502208, 10502272, 10502336, 10510336, 10510400, 10510464, 10510528,
//     10518528, 10518592, 10518656, 10518720, 10526720, 10526784, 10526848, 10526912, 10534912, 10534976, 10535040, 10535104, 10543104, 10543168, 10543232, 10543296,
//     12582912, 12582976, 12583040, 12583104, 12591104, 12591168, 12591232, 12591296, 12599296, 12599360, 12599424, 12599488, 12607488, 12607552, 12607616, 12607680,
//     12615680, 12615744, 12615808, 12615872, 12623872, 12623936, 12624000, 12624064, 12632064, 12632128, 12632192, 12632256, 12640256, 12640320, 12640384, 12640448,
//     14680064, 14680128, 14680192, 14680256, 14688256, 14688320, 14688384, 14688448, 14696448, 14696512, 14696576, 14696640, 14704640, 14704704, 14704768, 14704832,
//     14712832, 14712896, 14712960, 14713024, 14721024, 14721088, 14721152, 14721216, 14729216, 14729280, 14729344, 14729408, 14737408, 14737472, 14737536, 14737600,
// };

// typedef struct __attribute__((packed)) tagBITMAPFILEHEADER { // bmfh
//     u16    bfType;          //占2字节
//     u32    bfSize;          //占4字节
//     u16    bfReserved1;     //占2字节
//     u16    bfReserved2;     //占2字节
//     u32   bfOffBits;        //占4字节
// } BITMAPFILEHEADER;

// typedef struct __attribute__((packed)) tagBITMAPINFOHEADER{ // bmih
//     u32  biSize;
//     u32   biWidth; //4字节
//     u32   biHeight;
//     u16   biPlanes;
//     u16   biBitCount;
//     u32  biCompression;
//     u32  biSizeImage;
//     u32   biXPelsPerMeter;
//     u32   biYPelsPerMeter;
//     u32  biClrUsed;
//     u32  biClrImportant;
// } BITMAPINFOHEADER;

// typedef struct __attribute__((packed)) tagBITMAP_FILE{
//       BITMAPFILEHEADER bitmapheader;//文件头
//       BITMAPINFOHEADER bitmapinfoheader;//信息头
//       u32 palette[256];//调色板（可选）
//       u8 buffer[1];   //UCHAR 大小1字节(同C语言的unchar)，指向图像数据信息
// } BITMAP_FILE;

static compo_picturebox_t *awk_map_tile;
// static u8 awk_map_bmp_buf[sizeof(BITMAP_FILE) + AWK_MAP_BMP_WIDTH_ALIGNGED4 * AWK_MAP_HEIGHT];// AT(.amap_bitmap);
static u8 awk_map_bmp_buf[8 + AWK_MAP_BMP_WIDTH_ALIGNGED4 * AWK_MAP_HEIGHT * 2];// AT(.amap_bitmap);

void gpdma1_2d_memcpy(void* dst, void *src, u32 wight, u32 hight, int32_t src_stride, int32_t dst_stride, u32 pixel_bytes);
void awk_map_tile_updata(u32 x_s, u32 y_s, int32_t w, int32_t h, u8 *input_buf, u32 size)
{
    // BITMAP_FILE *bimap_info = (BITMAP_FILE *)awk_map_bmp_buf;
    // u8 (*rbuf)[AWK_MAP_BMP_WIDTH_ALIGNGED4] = (void *)&bimap_info->buffer;

    u16 (*rbuf)[AWK_MAP_BMP_WIDTH_ALIGNGED4] = (u16 (*)[AWK_MAP_BMP_WIDTH_ALIGNGED4])&awk_map_bmp_buf[8];

    // for (u32 i = 0; i < h; i++) {
    //     memcpy(&rbuf[y_s + i][x_s], (u16 *)(input_buf) + w * i, 2 * w);
    // }
    gpdma1_2d_memcpy(&rbuf[y_s][x_s], input_buf, w, h, w, AWK_MAP_BMP_WIDTH_ALIGNGED4, 2);
}

// void awk_map_get_tile_buf(u8 **buf, u32 *size)
// {
//     BITMAP_FILE *bimap_info = (BITMAP_FILE *)awk_map_bmp_buf;
//     if (buf && size) {
//         *buf = (u8 *)&bimap_info->buffer;
//         *size = AWK_MAP_BMP_WIDTH_ALIGNGED4 * AWK_MAP_HEIGHT;
//     }
// }

void awk_map_tile_flush(void)
{
    // BITMAP_FILE *bimap_info = (BITMAP_FILE *)awk_map_bmp_buf;

    // bimap_info->bitmapheader.bfType = 0x4d42;
    // bimap_info->bitmapheader.bfSize = 14 + 40 + 1024 + AWK_MAP_BMP_WIDTH_ALIGNGED4 * AWK_MAP_HEIGHT;
    // bimap_info->bitmapheader.bfReserved1 = 0;
    // bimap_info->bitmapheader.bfReserved2 = 0;
    // bimap_info->bitmapheader.bfOffBits   = 14 + 40 + 1024;

    // bimap_info->bitmapinfoheader.biSize   = 40;
    // bimap_info->bitmapinfoheader.biWidth  = AWK_MAP_WIDTH;
    // bimap_info->bitmapinfoheader.biHeight = AWK_MAP_HEIGHT;
    // bimap_info->bitmapinfoheader.biPlanes = 1;
    // bimap_info->bitmapinfoheader.biBitCount = 8;
    // bimap_info->bitmapinfoheader.biCompression = 0;
    // bimap_info->bitmapinfoheader.biSizeImage = AWK_MAP_BMP_WIDTH_ALIGNGED4 * AWK_MAP_HEIGHT;
    // bimap_info->bitmapinfoheader.biXPelsPerMeter = 0;
    // bimap_info->bitmapinfoheader.biYPelsPerMeter = 0;
    // bimap_info->bitmapinfoheader.biClrUsed = 256;
    // bimap_info->bitmapinfoheader.biClrImportant = 0;

    compo_picturebox_set_ram(awk_map_tile, (void *)awk_map_bmp_buf);
    compo_picturebox_set_pos(awk_map_tile,  GUI_SCREEN_WIDTH/2, GUI_SCREEN_HEIGHT/2);
    compo_picturebox_set_size(awk_map_tile, GUI_SCREEN_WIDTH,  GUI_SCREEN_HEIGHT);
    compo_picturebox_set_visible(awk_map_tile, true);
    gui_widget_refresh();
}

/**
 *  地图组件初始化
 */
void awk_map_tile_init(compo_form_t *frm)
{
    // awk_map_bmp_buf = ab_malloc(sizeof(BITMAP_FILE) + AWK_MAP_BMP_WIDTH_ALIGNGED4 * AWK_MAP_HEIGHT);

    // BITMAP_FILE *bimap_info = (BITMAP_FILE *)awk_map_bmp_buf;
    // memcpy(bimap_info->palette, awk_map_palette, sizeof(awk_map_palette));

    memset(awk_map_bmp_buf, 0x55, sizeof(awk_map_bmp_buf));
    PUT_LE32(awk_map_bmp_buf, 0x24150);
    PUT_LE16(&awk_map_bmp_buf[4], AWK_MAP_WIDTH);
    PUT_LE16(&awk_map_bmp_buf[6], AWK_MAP_HEIGHT);
    awk_map_tile = compo_picturebox_create(frm, 0);
    // compo_picturebox_set_visible(awk_map_tile, false);
    // widget_set_align_center(awk_map_tile->img, true);
}

/**
 *  地图组件反初始化
 */
void awk_map_tile_uninit(void)
{
    // if(awk_map_bmp_buf != NULL){
    //     ab_free(awk_map_bmp_buf);
    // }
}

/**
 *  获取地图ID
 */
int awk_get_map_id(void)
{
    return map_id;
}

