#include "include.h"
#include "awk.h"
#include "awk_mem.h"

#if AWK_MAP_EN
#if !BT_PANU_EN
#error "AWK_MAP must open BT_PANU_EN!"
#endif
#if !NOC_PSRAM_EN
#error "AWK_MAP must open NOC_PSRAM_EN!"
#endif
#endif

#define     TRACE_EN            0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

void mem_monitor_run_ex(void *tlsf);
void mem_monitor_run(void);
const awk_memory_adapter_t *awk_mem_get_adapter(void);
const awk_file_adapter_t *awk_file_get_adapter(void);
const awk_render_adapter_t *awk_render_get_adapter(void);

uint64_t awk_network_adapter_send_adapter(awk_http_request_t *request, awk_http_response_callback_t *callback);
void awk_network_adapter_cancel_adapter(uint64_t request_id);

AT(.text.sys_clk)
u8 get_sd_rate(void)
{
    return 24;  //unit: M
}

void __assert_func(const char *file, const char *func, int line, const char *failedexpr)
{
    TRACE("assert at file:%s/%s, line:%d\n", file, func, line);
    while(1);
}


uint64_t awk_get_system_time_adapter(void)
{
    TRACE("%s\n", __func__);
    return RTCCNT;
}

int awk_printf_adapter(const char* __fmt, ...)
{
    /*char log_buf[512] = {0};
	va_list arg_list;
	va_start(arg_list, __fmt);
	vsprintf(log_buf, __fmt, arg_list);

	printf("*** %s\n", log_buf);
	va_end(arg_list);*/
// #if TRACE_EN
    // printf(__fmt);
    // printf("\n");
// #endif
    return 1;
}

uint64_t awk_get_thread_id_adapter(void)
{
    return 30;
}

////1、离线模式 & 离线路径
//
//context.tile_load_mode = AWK_MAP_TILE_LOAD_OFFLINE;    //离线mos
//context.offline_map_dir = "offline_path”; // 离线地图数据所在路径
////2、中心点（latlon: 40.002713, 116.489486）
//
///**
// * @brief 设置地图中心，方法的调用需要在主流程线程中
// * @param {uint32_t} map_id 地图实例id
// * @param {coord2D} coord2d 地图中心点经纬度坐标
// * @return {成功返回0, 失败返回error: -1表示没有初始化 -3初始化的线程和当前调用不一致}
// */
//int32_t awk_map_set_center(uint32_t map_id, awk_map_coord2d_t coord2d);
////3、zoom（15/16）
//
///**
// * @brief 设置地图级别，方法的调用需要在主流程线程中
// * @param {uint32_t} map_id 地图实例id
// * @param {float} level 地图级别
// * @return {成功返回0, 失败返回error: -1表示没有初始化 -3初始化的线程和当前调用不一致 -4表示level级别不合法}
// */
//int32_t awk_map_set_level(uint32_t map_id, float level);

bool awk_on_tile_file(const char* tile_file_key, char *file_path, size_t *file_offset, size_t *file_size)
{
    // printf("%s %s\n", __func__, tile_file_key);
    snprintf(file_path, 256, "map/%s", tile_file_key);
    const awk_file_adapter_t *file_adapter = awk_file_get_adapter();
    size_t size = file_adapter->file_get_size(file_path);
    if (size != 0) {
        return true;
    }
    return false;
}

void awk_assert(int expression)
{
    if (!expression) {
        printf("!!! awk assert:%d\n", expression);
        printf("assert: %p\n", __builtin_return_address(0));
        // void *fp = __builtin_frame_address(0);
        // printf("fp:%x\n", fp);
        // print_r32(fp - 16, 64);
        while(1);
    }
}

static awk_context_t context AT(.amap_buf);
//构建创建及运行地图所需的环境变量，主要需要实现内存、文件、系统、网络、渲染等适配器所需能力接口和提供一些基础参数（设备id、key等）。
void awk_map_interface_init(void)
{
    memset(&context, 0, sizeof(awk_context_t));  //务必先memset
    context.device_id = "xuqjtest123";
    context.key = "da4332849a96e764f0c66b2a11aff836";
    context.root_dir = "B:awk";                 //SDK内部文件夹根路径
    context.tile_style = AWK_MAP_TILE_STYLE_STANDARD_GRID;  //AWK_MAP_TILE_STYLE_GRID_AND_POI;
    context.tile_pixel_mode = AWK_PIXEL_MODE_RGB_565;
    context.tile_clip_load = true;
    context.tile_mem_cache_max_size = 0;     //栅格图内存缓存限值，单位：KB
    context.tile_disk_cache_max_size = 1;  //栅格图磁盘缓存限值，单位：MB
    context.poi_tile_mem_cache_max_size = 0; // poi瓦片内存缓存最大空间，单位：KB
    context.poi_tile_disk_cache_max_size = 0; //poi磁盘缓存最大空间，单位: MB
    context.max_file_count_in_dir = 32;
    context.tile_zip = true;

    //内存相关适配
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    memcpy(&context.memory_adapter, memory_adapter, sizeof(awk_memory_adapter_t));

    //文件相关适配
    const awk_file_adapter_t *file_adapter = awk_file_get_adapter();
    memcpy(&context.file_adapter, file_adapter, sizeof(awk_file_adapter_t));

    //网络相关适配
    context.network_adapter.send = awk_network_adapter_send_adapter;
    context.network_adapter.cancel = awk_network_adapter_cancel_adapter;

    //渲染绘制相关适配
    const awk_render_adapter_t *render_adapter = awk_render_get_adapter();
    memcpy(&context.render_adapter, render_adapter, sizeof(awk_render_adapter_t));

    //其他系统相关适配
    context.system_adapter.get_system_time = awk_get_system_time_adapter;
    context.system_adapter.log_printf = awk_printf_adapter;
    context.thread_adapter.get_thread_id = awk_get_thread_id_adapter;

    // 瓦片相关
    context.tile_file_adapter.on_tile_file = awk_on_tile_file;
    context.custom_adapter.custom_sscanf = my_sscanf;
    context.custom_adapter.custom_assert = awk_assert;

    //离线地图
    context.tile_load_mode = AWK_MAP_TILE_LOAD_ONLINE;    //离线mos
    context.offline_map_dir = "B:map"; // 离线地图数据所在路径

    // mem_monitor_run();

    printf("awk_init\n");
    int32_t res = awk_init(&context);
    printf("awk_init res:%x\n", res);

    // mem_monitor_run();
    if (res) {
        printf("awk_init err:%d\n", res);
        return;
    }
}

