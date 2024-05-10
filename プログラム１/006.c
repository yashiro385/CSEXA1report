#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h> 
#include "exp1.h"

#define MIN_BUFFER_SIZE 128
#define MAX_BUFFER_SIZE 1048576

double get_current_timecount() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char* argv[]) {
    int fpr;
    int fpw;
    int ret;
    char *buf;

    if (argc != 2) {
        printf("usage: %s [filename]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int buffer_size = MIN_BUFFER_SIZE; buffer_size <= MAX_BUFFER_SIZE; buffer_size *= 2) {
        double total_time = 0;

        for (int j = 0; j < 30; j++) {
            buf = (char *)malloc(buffer_size);
            if (buf == NULL) {
                perror("Error allocating memory for buffer");
                exit(EXIT_FAILURE);
            }

            double t1 = get_current_timecount();

            fpr = open(argv[1], O_RDONLY);
            fpw = open("tmp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);

            if (fpr == -1 || fpw == -1) {
                perror("Error opening file");
                exit(EXIT_FAILURE);
            }

            ret = read(fpr, buf, buffer_size);

            while (ret > 0) {
                write(fpw, buf, ret);
                ret = read(fpr, buf, buffer_size);
            }

            double t2 = get_current_timecount();

            total_time += (t2 - t1);

            close(fpr);
            close(fpw);

            free(buf); // バッファを解放する
        }

        double average_time = total_time / 30.0;
        printf("Buffer size: %d, Average Time: %lf\n", buffer_size, average_time);
    }

    return 0;
}
