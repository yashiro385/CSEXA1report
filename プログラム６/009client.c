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
	

  char buf[262144];
  int fd;
  int ret;

  if(argc != 3) {
    printf("usage: %s [ip address] [filename]\n", argv[0]);
    exit(-1);
  }

  sock = exp1_tcp_connect(argv[1], 11111);
  fd = open(argv[2], O_RDONLY);
  ret = read(fd,buf, 262144);
  double t1 = get_current_timecount();
while(ret > 0) {
   write(sock,buf,ret);
   ret = read(fd,buf, 262144);
  }

  double t2 = get_current_timecount();
    close(sock);
close(fd);
  printf("%lf\n", t2-t1);
sleep(1);

  return 0;
}
