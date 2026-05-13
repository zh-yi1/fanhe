#include "include.h"
#include "awk.h"
#include "rt_thread.h"
#include "ugzip/uzlib.h"
#include <string.h>
#include <webclient/webclient.h>

#define GZIP_EN 1

#define RESP_BUFF_LEN   512
static uint32_t web_request_id AT(.amap_buf);

const awk_memory_adapter_t *awk_mem_get_adapter(void);
const awk_file_adapter_t *awk_file_get_adapter(void);
void awk_map_get_tile_buf(u8 **buf, u32 *size);
int awk_get_map_id(void);
bool awk_map_lock(bool wait);
void awk_map_unlock(void);
void psram_mem_monitor_run(void);
uint64_t awk_network_adapter_send_adapter(awk_http_request_t *request, awk_http_response_callback_t *callback);

typedef struct {
    awk_http_request_t *request;
    awk_http_response_callback_t *callback;
    u32 request_id;
} awk_http_msg_t;

#define MQ_SIZE 16
#define MQPOOL_SIZE (MQ_SIZE*8)
#define STACK_SIZE 2048

typedef struct {
    struct os_thread thread;
    u8 stack_buf[STACK_SIZE];

    struct os_messagequeue mq;
    u8 mq_buf[MQPOOL_SIZE];

    volatile u8 working;
} awk_network_t;

static awk_network_t awk_network;

#if GZIP_EN
static uzlib_uncomp_t gzip AT(.amap_buf);
#endif

static bool awk_network_header_resp(char *resp_buf, u32 resp_len, const awk_http_request_t *request, awk_http_response_callback_t *callback, u32 request_id)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();

    awk_http_response_t rep_awk;
    char *ptr, *header_ptr;
    bool gzip_flag = false;

    while((header_ptr = strstr((void *)resp_buf, ": ")) == NULL){
        resp_buf += strlen(resp_buf) + 1;
    }

    rep_awk.headers.size = 0;
    for(int i=0; i<resp_len; i++){
        if((resp_buf[i] == ':') && (resp_buf[i+1] == ' ')){
            rep_awk.headers.size++;
        }
    }

    // printf("rep_awk.headers.size:%d\n", rep_awk.headers.size);
    rep_awk.headers.data = (void *)memory_adapter->mem_malloc(sizeof(awk_pair_http_header_t) * rep_awk.headers.size);
    if(rep_awk.headers.data == NULL){
        return gzip_flag;
    }

    ptr = (char *)resp_buf;
    for(int i=0; i<rep_awk.headers.size; i++) {
        rep_awk.headers.data[i].key = ptr;
        header_ptr = strstr(ptr, ": ");
        rep_awk.headers.data[i].value = header_ptr+2;
        ptr += strlen(ptr)+1;
        *header_ptr = 0;
    }

    for(int i=0; i<rep_awk.headers.size; i++){
        if((strcmp(rep_awk.headers.data[i].key, "Content-Encoding") == 0) && (strcmp(rep_awk.headers.data[i].value, "gzip") == 0)){
            gzip_flag = true;
        }

        printf("%s: %s\n", rep_awk.headers.data[i].key, rep_awk.headers.data[i].value);
    }

    rep_awk.url          = request->url;
    rep_awk.request_id   = request_id;
    rep_awk.status_code  = 200;
    rep_awk.body         = NULL;

    // printf("on_receive_header request_id:%d\n", rep_awk.request_id);
    awk_map_lock(true);
    callback->on_receive_header(callback, &rep_awk);
    awk_map_unlock();
    memory_adapter->mem_free((void *)rep_awk.headers.data);

    return gzip_flag;
}

static void awk_http_get(const awk_http_request_t *request, awk_http_response_callback_t *callback, u32 request_id)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();

    struct webclient_session *session = NULL;
    awk_http_buffer_t post_resp;
    bool is_gzip;
    u8 *buffer = NULL;
    int buffer_size = 512;

    printf("url:%s\n", request->url);

    session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
    if (session == NULL)
    {
        printf("err: awk_http_get webclient_session_create err\n");
        goto __exit;
    }

    for (int i = 0; i < request->headers.size; i++) {
        awk_pair_http_header_t *header = &request->headers.data[i];
        webclient_header_fields_add(session, "%s:%s\r\n", header->key, header->value);
    }

    int get_ret = webclient_get(session, request->url);
    // printf("get code =%d\n", get_ret);
    if (get_ret != 200)
    {
        goto __exit;
    }
#if 0
    char filename[] = "http_a.bin";
    filename[5] += request_id;
    FRESULT res = fs_open(filename, (FA_READ | FA_WRITE | FA_CREATE_NEW) );
#endif

    //header
    is_gzip = awk_network_header_resp(session->header->buffer, session->header->length, request, callback, request_id);

    awk_http_response_t rep_awk = {0};
    int content_length = webclient_content_length_get(session);
    int bytes_read;
    printf("content_length:%d gzip:%d\n", content_length, is_gzip);

    if (is_gzip) {
        buffer_size = 40000;
    }
    if (content_length > 0) {
        buffer_size = content_length;
    }
    buffer = memory_adapter->mem_malloc(buffer_size);
    printf("malloc buffer: %p\n", buffer);
    if (!buffer) {
        printf("%s buffer err\n", __func__);
        goto __exit;
    }

    if(is_gzip == false){
        if (content_length < 0) {
            do {
                bytes_read = webclient_read(session, buffer, buffer_size);
                if (bytes_read <= 0) {
                    break;
                }

                post_resp.buffer = buffer;
                post_resp.length = bytes_read;
                rep_awk.url          = request->url;
                rep_awk.request_id   = request_id;
                rep_awk.status_code  = 200;
                rep_awk.body         = &post_resp;

                awk_map_lock(true);
                callback->on_receive_body(callback, &rep_awk);
                awk_map_unlock();
            } while (1);
        } else {
            bytes_read = webclient_read(session, buffer, buffer_size);

            post_resp.buffer = buffer;
            post_resp.length = buffer_size;
            rep_awk.url          = request->url;
            rep_awk.request_id   = request_id;
            rep_awk.status_code  = 200;
            rep_awk.body         = &post_resp;
            buffer = NULL;

            printf("on_receive_body 1\n");
            // mem_monitor_run();
            awk_map_lock(true);
            callback->on_receive_body(callback, &rep_awk);
            awk_map_unlock();
            printf("on_receive_body 2\n");
            // mem_monitor_run();
        }
    } else {
#if GZIP_EN
        int gzip_ret;
        int wbits;
        u8 *http_gzip = NULL, *tile_data = NULL;
        u32 gzip_len = 0;
        u32 tile_len = 0;

        bytes_read = 0;
        do {
            gzip_ret = webclient_read(session, buffer + bytes_read, buffer_size);
            if (gzip_ret <= 0) {
                break;
            }
            bytes_read += gzip_ret;
            buffer_size -= gzip_ret;
        } while (1);
        gzip_len = bytes_read;
        http_gzip = buffer;

        tile_len = *(u32*)(http_gzip+gzip_len-4);
        printf("gzip_len:%d %d\n", gzip_len, tile_len);

        tile_data = memory_adapter->mem_malloc(tile_len);
        if (!tile_data) {
            printf("gzip len err\n");
            goto __exit;
        }

        uzlib_uncompress_init(&gzip, NULL, 0);
        gzip.source         = http_gzip;
        gzip.source_limit   = http_gzip + gzip_len;

        if(uzlib_parse_zlib_gzip_header(&gzip, &wbits) != UZLIB_HEADER_GZIP){
            printf("ugzpi err\n");
            goto __exit;
        }
        gzip.dest_start = tile_data;
        gzip.dest       = tile_data;
        gzip.dest_limit = tile_data + tile_len;

        gzip_ret = uzlib_uncompress_chksum(&gzip);
        printf("gzip_ret:%d\n", gzip_ret);

        post_resp.buffer = tile_data;
        post_resp.length = tile_len;
        rep_awk.url          = request->url;
        rep_awk.request_id   = request_id;
        rep_awk.status_code  = 200;
        rep_awk.body         = &post_resp;
        awk_map_lock(true);
        callback->on_receive_body(callback, &rep_awk);
        awk_map_unlock();
#endif
    }

    rep_awk.url          = request->url;
    rep_awk.request_id   = request_id;
    rep_awk.status_code  = 200;
    rep_awk.body         = NULL;

    // mem_monitor_run();

    printf("on_success request_id:%d\n", rep_awk.request_id);
    awk_map_lock(true);
    callback->on_success(callback, &rep_awk);
    awk_map_unlock();
    printf("on_success end\n");
    psram_mem_monitor_run();

    // mem_monitor_run();

    //fs_close();
__exit:
    if (buffer) {
        memory_adapter->mem_free(buffer);
    }
    if (session) {
        webclient_close(session);
    }

    if (request->url) {
        memory_adapter->mem_free(request->url);
    }
    if (request->headers.data) {
        memory_adapter->mem_free(request->headers.data);
    }
    memory_adapter->mem_free((void *)request);

    //mem_monitor_run();
}

static void awk_network_entry(void *param)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    awk_http_msg_t *msg;
    while (1) {
        awk_network.working = 0;
        os_mq_recv(&awk_network.mq, &msg, 4, OS_WAITING_FOREVER);
        awk_network.working = 1;
        // printf("recv %p %p\n", msg, memory_adapter);
        if (msg->request && msg->callback) {
            awk_http_get(msg->request, msg->callback, msg->request_id);
        }
        memory_adapter->mem_free(msg);
    }
}

bool awk_network_init(void)
{
    memset(&awk_network, 0, sizeof(awk_network_t));

    web_request_id = 1;
    os_mq_init(&awk_network.mq, "amq", awk_network.mq_buf, MQ_SIZE, MQPOOL_SIZE, OS_IPC_FLAG_FIFO);
    os_thread_init(&awk_network.thread, "ath", awk_network_entry, NULL, awk_network.stack_buf, STACK_SIZE, 29, -1);
    os_thread_startup(&awk_network.thread);
    return true;
}

void awk_network_uninit(void)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    awk_http_msg_t *msg;
    while (!os_mq_recv(&awk_network.mq, &msg, 4, 0)) {
        memory_adapter->mem_free(msg);
    }

    // TODO: timeout
    while (awk_network.working) {
        bt_thread_check_trigger();
        delay_ms(5);
    }
    os_thread_detach(&awk_network.thread);
    os_mq_detach(&awk_network.mq);
}

uint64_t awk_network_adapter_send_adapter(awk_http_request_t *request, awk_http_response_callback_t *callback)
{
    const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
    // printf("%s %s\n", __func__, request->url);
    // https -> http
    if (strncmp(request->url, "https", 5) == 0) {
        strcpy(request->url + 4, request->url + 5);
    }

    awk_http_msg_t *msg = memory_adapter->mem_malloc(sizeof(awk_http_msg_t));
    if (!msg) {
        return web_request_id;
    }

    printf("web_request_id:%d %p\n", web_request_id, msg);
    msg->request = request;
    msg->callback = callback;
    msg->request_id = web_request_id;
    os_mq_send(&awk_network.mq, &msg, 4);

    return web_request_id++;
}

void awk_network_adapter_cancel_adapter(uint64_t request_id)
{
    printf("%s\n", __func__);
}

// static void *awk_common_request(awk_http_request_t *request)
// {
//     struct webclient_session *session = NULL;

//     session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
//     if (session == NULL) {
//         printf("err: awk_http_get webclient_session_create err\n");
//         return NULL;
//     }

//     for (int i = 0; i < request->headers.size; i++) {
//         awk_pair_http_header_t *header = &request->headers.data[i];
//         webclient_header_fields_add(session, "%s:%s\r\n", header->key, header->value);
//     }

//     int ret = webclient_get(session, request->url);
//     if (ret != 200) {
//         // printf("get ret=%d\n", ret);
//         webclient_close(session);
//         return NULL;
//     }
//     return session;
// }

// static void awk_download_tile_file(awk_http_request_t *request, char* tile_file_key)
// {
//     const awk_file_adapter_t *file_adapter = awk_file_get_adapter();

//     struct webclient_session *session = NULL;
//     char *buffer;
//     u32 buffer_size;

//     awk_map_get_tile_buf((u8 **)&buffer, &buffer_size);
//     if (!buffer) {
//         goto __exit;
//     }
//     // printf("buf info:%x %d\n", buffer, buffer_size);

//     session = awk_common_request(request);
//     if (!session) {
//         goto __exit;
//     }

//     int content_length = webclient_content_length_get(session);
//     int bytes_read;
//     // printf("content_length:%d\n", content_length);

//     // json
//     if (webclient_read(session, buffer, buffer_size) <= 0) {
//         goto __exit;
//     }
//     const char *start = strstr(buffer, "http://");
//     const char *end   = strpbrk(start, "\"");

//     if (!start || !end) {
//         goto __exit;
//     }
//     int len = end - start;
//     strncpy(buffer, start, len);
//     // printf("url:%s\n", buffer);

//     webclient_close(session);
//     awk_http_request_t new_request = { 0 };

//     new_request.url = buffer;
//     session         = awk_common_request(&new_request);
//     if (session) {
//         content_length = webclient_content_length_get(session);
//         size_t size = file_adapter->file_get_size(tile_file_key);
//         // printf("length:%s %d\n", tile_file_key, content_length);

//         if (size == content_length) {
//             goto __exit;
//         }

//         void *handle   = file_adapter->file_open(tile_file_key, "w");
//         if (!handle) {
//             goto __exit;
//         }

//         do {
//             bytes_read = webclient_read(session, buffer, buffer_size);
//             if (bytes_read <= 0) {
//                 break;
//             }
//             int res = file_adapter->file_write(buffer, bytes_read, handle);
//         } while (1);
//         file_adapter->file_close(handle);
//     }

// __exit:
//     if (session) {
//         webclient_close(session);
//     }
// }

// static void awk_download_tile_on_success(const char *key, awk_map_tile_download_info_t *infos, uint32_t tile_download_size) {
//     const awk_memory_adapter_t *memory_adapter = awk_mem_get_adapter();
//     // printf("%s %s %x %d\n", __func__, key, infos, tile_download_size);
//     for (int i = 0;i < tile_download_size; i++) {
//         // printf("download info %s : %d\n", infos[i].tile_file_key, infos[i].tile_type);

//         awk_http_request_t *request = awk_map_download_get_request(infos[i].tile_file_key);
//         // printf("url: %s\n", request->url);
//         // https -> http
//         if (strncmp(request->url, "https", 5) == 0) {
//             strcpy(request->url + 4, request->url + 5);
//         }

//         if (infos[i].tile_type == AWK_MAP_TILE_TYPE_TILE) {
//             char buf[32];
//             snprintf(buf, 32, "map/%s", infos[i].tile_file_key);
//             awk_download_tile_file(request, buf);
//         }
//         if (request->url) {
//             memory_adapter->mem_free(request->url);
//         }
//         if (request->headers.data) {
//             memory_adapter->mem_free(request->headers.data);
//             request->headers.data = NULL;
//             request->headers.size = 0;
//         }
//         memory_adapter->mem_free(request);
//     }
//     memory_adapter->mem_free(infos);
// }

// static void awk_download_tile_on_fail(const char *key, int32_t error_code, const char* msg) {
//     printf("%s %s %x %s\n", __func__, key, error_code, msg);
// }

// static awk_map_tile_download_callback_t tile_download_callback = {
//     .on_success = awk_download_tile_on_success,
//     .on_fail = awk_download_tile_on_fail,
// };

// awk_map_tile_download_callback_t *awk_download_tile_get_callback(void)
// {
//     return &tile_download_callback;
// }
