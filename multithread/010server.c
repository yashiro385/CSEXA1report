#include "exp1.h"
#include "exp1lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAXCHILD 1200

// 最大ソケット使用数を保持するグローバル変数
int max_socket_usage = 0;

// ^Cのシグナルハンドラー
void signal_handler(int signum) {
    printf("\nMaximum socket usage: %d\n", max_socket_usage);
    exit(signum);
}

bool echoback(int acc) {
    char buf[512];
    sleep(3);
    ssize_t len = 0;
    if ((len = recv(acc, buf, sizeof(buf), 0)) == -1) {
        perror("recv");
        return false;
    }

    if (len == 0) {
        fprintf(stderr, "recv:EOF\n");
        return false;
    }

    buf[len] = '\0';
    char* retPtr = NULL;
    if ((retPtr = strpbrk(buf, "\r\n")) != NULL) {
        *retPtr = '\0';
    }

    fprintf(stderr, "[client]%s\n", buf);
    strncat(buf, ":OK\r\n", sizeof(buf) - strlen(buf) - 1);
    len = strlen(buf);
    if ((len = send(acc, buf, len, 0)) == -1) {
        perror("send");
        return false;
    }

    return true;
}

void acceptLoop(int sock) {
    int childNum = 0;
    int child[MAXCHILD];
    int i = 0;
    for (i = 0; i < MAXCHILD; i++) {
        child[i] = -1;
    }

    while (true) {
        fd_set mask;
        FD_ZERO(&mask);
        FD_SET(sock, &mask);
        int width = sock + 1;
        int i = 0;
        for (i = 0; i < childNum; i++) {
            if (child[i] != -1) {
                FD_SET(child[i], &mask);
                if ( width <= child[i] ) {
                    width = child[i] + 1;
                }
            }
        }

        fd_set ready = mask;
        struct timeval timeout;
        timeout.tv_sec = 600;
        timeout.tv_usec = 0;

        switch (select(width, (fd_set *) &ready, NULL, NULL, &timeout)) {
            case -1:
                perror("select");
                break;
            case 0:
                break;
            default:
                if (FD_ISSET(sock, &ready)) {
                    struct sockaddr_storage from;
                    socklen_t len = sizeof(from);
                    int acc = 0;
                    if ((acc = accept(sock, (struct sockaddr *) &from, &len)) == -1) {
                        if (errno != EINTR) {
                            perror("accept");
                        }
                    } else {
                        char hbuf[NI_MAXHOST];
                        char sbuf[NI_MAXSERV];
                        getnameinfo((struct sockaddr *) &from, len, hbuf,
                            sizeof(hbuf), sbuf, sizeof(sbuf),
                            NI_NUMERICHOST | NI_NUMERICSERV);
                        fprintf(stderr, "accept:%s:%s\n", hbuf, sbuf);

                        int pos = -1;
                        int i = 0;
                        for (i = 0; i < childNum; i++) {
                            if (child[i] == -1) {
                                pos = i;
                                break;
                            }
                        }

                        if (pos == -1) {
                            if (childNum >= MAXCHILD) {
                                fprintf(stderr, "child is full.\n");
                                close(acc);
                            } else {
                                pos = childNum;
                                childNum = childNum + 1;
                            }
                        }

                        if (pos != -1) {
                            child[pos] = acc;

                            // 最大ソケット使用数を更新
                            if (childNum > max_socket_usage) {
                                max_socket_usage = childNum;
                            }
                        }
                    }
                }

                for (i = 0; i < childNum; i++) {
                    if (child[i] != -1 && FD_ISSET(child[i], &ready)) {
                        if ( echoback(child[i]) == false ) {
                            close(child[i]);
                            child[i] = -1;
                        }
                    }
                }
        }
    }
}

int main(int argc, char **argv) {
    int sock_listen;

    // ^Cのシグナルハンドラーの設定
    signal(SIGINT, signal_handler);

    sock_listen = exp1_tcp_listen(11111);
    if (sock_listen == -1) {
        perror("exp1_tcp_listen");
        exit(EXIT_FAILURE);
    }

    acceptLoop(sock_listen);
    close(sock_listen);

    return 0;
}
