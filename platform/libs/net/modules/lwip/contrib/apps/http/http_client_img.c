#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <stdio.h>

#define printf a_printf

#define SERVER_PORT    "80"               // HTTP 端口
#define BUFFER_SIZE    512
#define OUTPUT_FILE    "downloaded_image.jpg"

// #define IMAGE_PATH "group64/M0B/02/31/wKgMaV1whayCEBZFAJ99ieJkLVc044.mp3" // 64k
#define  IMAGE_PATH "/group64/M0B/02/30/wKgMaV1whaLAcRGQAE_0uZWMCp8270.mp3" //32k
#define SERVER_HOST "openaudio.cos.tx.xmcdn.com"

#define HTTP_GET_REQUEST(path, host) \
    "GET " path " HTTP/1.1\r\n" \
    "Host: " host "\r\n" \
    "Connection: keep-alive\r\n\r\n"

static char buffer[BUFFER_SIZE]  AT(.avio_buf);

void bnep_audio_init(void);
void bnep_audio_exit(void);
void bnep_audio_write(void *buf, unsigned int size);
void bnep_audio_start(void);

static void http_music_thread(void *p)
{
    bnep_audio_start();
}

void http_download_image_socket(void) {
    bnep_audio_init();

    int sock = 0;
    struct addrinfo hints, *res, *p;
    int err;
    // char request[512];
    // char buffer[BUFFER_SIZE];
    // FILE *file = NULL;
    int bytes_received;
    int header_parsed = 0;
    int header_length = 0;

    // 设置 addrinfo 结构
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;       // 使用 IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP 流套接字

    // 解析服务器地址
    err = getaddrinfo(SERVER_HOST, SERVER_PORT, &hints, &res);
    if (err != 0) {
        printf("getaddrinfo failed: %d\n", err);
        goto _fail1;
    }

    // 创建套接字并连接
    for (p = res; p != NULL; p = p->ai_next) {
        // printf("socket %d %d %d\n",p->ai_family, p->ai_socktype, p->ai_protocol);
        // a_print_r(p->ai_addr, p->ai_addrlen);
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0) {
            continue;
        }

        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0) {
            break; // 连接成功
        }

        goto _fail2;
    }

    // 构造 HTTP GET 请求
    const char *request = HTTP_GET_REQUEST(IMAGE_PATH, SERVER_HOST);

    printf("start send\n");
    // 发送请求
    if (send(sock, request, strlen(request), 0) < 0) {
        printf("Failed to send request\n");
        goto _fail2;
    }

    // // 打开文件以保存图片
    // file = fopen(OUTPUT_FILE, "wb");
    // if (file == NULL) {
    //     printf("Failed to open file for writing\n");
    //     close(sock);
    //     return;
    // }

    char flag = 0;
    u32_t all_byte = 0;
    printf("start recv\n");
    // 接收响应
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        // a_printf("===> ");
        // a_print_r(buffer, bytes_received);

        if (!header_parsed) {
            // 查找 HTTP 头部结束
            char *header_end = strstr(buffer, "\r\n\r\n");
            if (header_end) {
                header_length = header_end - buffer + 4;
                bnep_audio_write(buffer + header_length, bytes_received - header_length);
                all_byte += bytes_received - header_length;
                header_parsed = 1;
            }
            // 如果未找到头部结束，继续接收
        } else {
            bnep_audio_write(buffer,bytes_received);
            all_byte += bytes_received;
            if (flag == 0 && all_byte > 512) {
                sys_thread_new("hmt", http_music_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
                flag = 1;
            }
        }
    }

    if (bytes_received < 0) {
        printf("Receive error\n");
    } else {
        printf("Receive End\n");
    }

    // 关闭套接字和文件
    // fclose(file);
_fail2:
    close(sock);
_fail1:
    freeaddrinfo(res);
    bnep_audio_exit();
}
