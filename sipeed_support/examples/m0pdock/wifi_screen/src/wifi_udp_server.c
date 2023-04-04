
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <lwip/api.h>
#include <lwip/arch.h>
#include <lwip/opt.h>
#include <lwip/inet.h>
#include <lwip/errno.h>
#include <netdb.h>

#include "shell.h"
#include "utils_getopt.h"
#include "bflb_mtimer.h"

// clang-format off
static uint32_t __section(".psram_data") recv_buf[320*2*320];
// clang-format on

shell_sig_func_ptr abort_exec;
ssize_t recv_len = 0;
int sock = -1;

static void test_close(int sig)
{
    if (sock) {
        closesocket(sock);
    }
    abort_exec(sig);
    if (recv_len > 0) {
        printf("Total send data=%lu\r\n", recv_len);
    }
}

#define PING_USAGE               \
    "wifi_udp_server [port]\r\n" \
    "\t port: local listen port, default port %s\r\n"

static void wifi_test_udp_server_init(int argc, char **argv)
{
    abort_exec = shell_signal(SHELL_SIGINT, test_close);
    printf("udp server task start ...\r\n");

    char *port = "5001";
    struct sockaddr_in udp_addr, remote_addr;
    socklen_t addr_len;

    if (argc < 2) {
        printf(PING_USAGE, port);
    }

    /* get port number (argv[1] if present) */
    if (argc > 1) {
        port = argv[1];
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("udp create socket error\r\n");
        goto _exit;
    }
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(atoi(port));
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    memset(udp_addr.sin_zero, 0, sizeof(udp_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr)) != 0) {
        printf("udp bind falied!\r\n");
        goto _exit_clean;
    }

    printf("udp bind success!\r\n");
    printf("Server ip Address : %s:%s\r\n", inet_ntoa(udp_addr.sin_addr.s_addr), port);
    printf("Press CTRL-C to exit.\r\n");

    recv_len = 0;
    memset(recv_buf, 0, sizeof(recv_buf));
    uint8_t recv_buf_idx = 0;
    // struct timeval tv;
    // tv.tv_sec = 1;
    // tv.tv_usec = 0;
    // setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
    while (1) {
#define PIC_SIZE (320 * 320 * 2)
        size_t pic_size_remained = PIC_SIZE;
        while (pic_size_remained) {
            uint8_t *p_recv = ((uint8_t *)recv_buf) + PIC_SIZE * ((recv_buf_idx + 1) % 3) + PIC_SIZE - pic_size_remained;
            recv_len = recvfrom(sock, p_recv, pic_size_remained,
                                0, (struct sockaddr *)&remote_addr, &addr_len);
            if (recv_len <= 0) {
                continue;
            }
            if (recv_len == 1 && p_recv[0] == '\x58') {
                continue;
            }
            if (recv_len == 2) {
                if (p_recv[0] == '\x11' && p_recv[1] == '\x00') {
                    pic_size_remained = PIC_SIZE;
                    continue;
                }
            }

            p_recv[recv_len] = '\0';
            printf("recv[%ld/%ld] from %s\r\n", recv_len, pic_size_remained, inet_ntoa(remote_addr.sin_addr));
            pic_size_remained -= recv_len;
            // printf("recv:%s \r\n", (uint8_t *)p_recv);
            // extern void canvas_update(void *p);
            // canvas_update(recv_buf);
        }
        printf("recv one picture!\r\n");
        extern void canvas_update(void *p);
        recv_buf_idx = (recv_buf_idx + 1) % 3;
        canvas_update(((uint8_t *)recv_buf) + PIC_SIZE * recv_buf_idx);
        // sendto(sock, recv_buf, recv_len, 0, (struct sockaddr *)&remote_addr, addr_len);
    }

_exit_clean:
    closesocket(sock);
_exit:
    return;
}

#ifdef CONFIG_SHELL
#include <shell.h>

int cmd_wifi_udp_server(int argc, char **argv)
{
    wifi_test_udp_server_init(argc, argv);

    return 0;
}

SHELL_CMD_EXPORT_ALIAS(cmd_wifi_udp_server, wifi_udp_server, wifi udp test);
#endif
