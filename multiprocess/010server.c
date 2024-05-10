#include "exp1.h"
#include "exp1lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAXCHILD 1200

// グローバル変数として最大利用ソケット数を記録する
int max_socket_count = 0;
int current_socket_count = 0;

// シグナルハンドラ関数
void sigint_handler(int sig) {
    printf("Received SIGINT, exiting...\n");
    printf("Maximum number of sockets used: %d\n", max_socket_count);
    exit(EXIT_SUCCESS);
}

bool echoback(int acc) {
    char buf[512];
    sleep(3);
    // ソケットから入力を受け取る
    ssize_t len = 0;
    if ((len = recv(acc, buf, sizeof(buf), 0)) == -1) {
        perror("recv");
        return false;
    }

    if (len == 0) {
        // クライアント側から切断（正常に切断）
        fprintf(stderr, "recv:EOF\n");
        return false;
    }

    // 改行コードを行末に差し替える
    buf[len] = '\0';
    char* retPtr = NULL;
    if ((retPtr = strpbrk(buf, "\r\n")) != NULL) {
        *retPtr = '\0';
    }

    // 入力された内容に ":OK" を付与して送信する
    fprintf(stderr, "[client]%s\n", buf); // コンソールに出力
    strncat(buf, ":OK\r\n", sizeof(buf) - strlen(buf) - 1);
    len = strlen(buf);
    if ((len = send(acc, buf, len, 0)) == -1) { // クライアントに送信
        perror("send");
        return false;
    }

    return true;
}

void acceptLoop(int sock) {
    // クライアント管理配列
    int childNum = 0;
    int child[MAXCHILD];
    int i = 0;
    for (i = 0; i < MAXCHILD; i++) {
        child[i] = -1;
    }

    while (true) {
        // select用マスクの初期化
        fd_set mask;
        FD_ZERO(&mask);
        FD_SET(sock, &mask); // ソケットの設定
        int width = sock + 1;
        int i = 0;
        for (i = 0; i < childNum; i++) {
            if (child[i] != -1) {
                FD_SET(child[i], &mask); // クライアントソケットの設定
                if (width <= child[i]) {
                    width = child[i] + 1;
                }
            }
        }

        // マスクを設定
        fd_set ready = mask;

        // タイムアウト値のセット
        struct timeval timeout;
        timeout.tv_sec = 600;
        timeout.tv_usec = 0;

        switch (select(width, (fd_set*)&ready, NULL, NULL, &timeout)) {
            case -1:
                // エラー処理
                perror("select");
                break;
            case 0:
                // タイムアウト
                break;
            default:
                // I/Oレディあり
                if (FD_ISSET(sock, &ready)) {
                    // サーバソケットレディの場合
                    struct sockaddr_storage from;
                    socklen_t len = sizeof(from);
                    int acc = 0;
                    if ((acc = accept(sock, (struct sockaddr*)&from, &len)) == -1) {
                        // エラー処理
                        if (errno != EINTR) {
                            perror("accept");
                        }
                    } else {
                        // クライアントからの接続が行われた場合
                        char hbuf[NI_MAXHOST];
                        char sbuf[NI_MAXSERV];
                        getnameinfo((struct sockaddr*)&from, len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                                    NI_NUMERICHOST | NI_NUMERICSERV);
                        fprintf(stderr, "accept:%s:%s\n", hbuf, sbuf);

                        // クライアント管理配列に登録
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
                                // 並列数が上限に達している場合は切断する
                                fprintf(stderr, "child is full.\n");
                                close(acc);
                            } else {
                                pos = childNum;
                                childNum = childNum + 1;
                            }
                        }

                        if (pos != -1) {
                            child[pos] = acc;
                            // 新しいソケットが作成されたため、カウントを更新
                            current_socket_count++;
                            if (current_socket_count > max_socket_count) {
                                max_socket_count = current_socket_count;
                            }
                        }
                    }
                }

                // アクセプトしたソケットがレディの場合を確認する
                int i = 0;
                for (i = 0; i < childNum; i++) {
                    if (child[i] != -1 && FD_ISSET(child[i], &ready)) {
                        // クライアントとの通信処理
                        // エコーバックを行う（echoBack関数は自分で作成すること）
                        if (echoback(child[i]) == false) {
                            close(child[i]);
                            child[i] = -1;
                            // ソケットがクローズされたため、カウントを更新
                            current_socket_count--;
                        }
                    }
                }
        }
    }
}

int main(int argc, char **argv) {
    // SIGINT（^C）シグナルのハンドラを設定
    signal(SIGINT, sigint_handler);

    int sock_listen;

    // サーバーソケットの作成
    sock_listen = exp1_tcp_listen(11111);
    if (sock_listen == -1) {
        perror("exp1_tcp_listen");
        exit(EXIT_FAILURE);
    }

    // サーバーループ開始
    acceptLoop(sock_listen);

    // ソケットのクローズ
    close(sock_listen);

    printf("Maximum number of sockets used: %d\n", max_socket_count);

    return 0;
}
