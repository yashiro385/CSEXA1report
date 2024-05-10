#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "exp1.h"

double get_current_timecount() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char* argv[]) {
    FILE* fpr;
    FILE* fpw;
    int ret;
    char *buf;

    if (argc != 2) {
        printf("usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    for (int buffer_size = 128; buffer_size <= 1048576; buffer_size *= 2) {
        double total_time = 0;

        for (int i = 0; i < 30; i++) {
            buf = (char *)malloc(buffer_size);
            if (buf == NULL) {
                perror("Error allocating memory for buffer");
                exit(EXIT_FAILURE);
            }
            double t1 = get_current_timecount();
            fpr = fopen(argv[1], "r");
            fpw = fopen("tmp.txt", "w");

            if (fpr == NULL || fpw == NULL) {
                perror("Error opening file");
                exit(EXIT_FAILURE);
            }

           

            while ((ret = fread(buf, sizeof(char), buffer_size, fpr)) > 0) {
                fwrite(buf, sizeof(char), ret, fpw);
            }

            double t2 = get_current_timecount();

            total_time += (t2 - t1);

            fclose(fpr);
            fclose(fpw);

            free(buf);
        }

        double average_time = total_time / 30.0;
        printf("Buffer size: %d, Average Time: %lf\n", buffer_size, average_time);
    }

    return 0;
}
