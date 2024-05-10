#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "exp1.h"
#include "exp1lib.h"

double get_current_timecount() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec*1e-9;
}

int main( int argc, char* argv[] ) {
   int sock;
int buf_size = 256;
char buf[buf_size];

   FILE* fp;
  int ret;

  if(argc != 3) {
    printf("usage: %s [ip address] [filename]\n", argv[0]);
    exit(-1);
  }

  sock = exp1_tcp_connect(argv[1], 11111);
  fp = fopen(argv[2], "r");
  ret = fread(buf, sizeof(char), buf_size, fp);
  double t1 = get_current_timecount();
while(ret > 0) {
    send(sock, buf, ret, 0);
    ret = fread(buf, sizeof(char), buf_size, fp);
  }

  double t2 = get_current_timecount();
 fclose(fp);
    close(sock);
  printf("%lf\n", t2-t1);
sleep(1);
  return 0;

}
