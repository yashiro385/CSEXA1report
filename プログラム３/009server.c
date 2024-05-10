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
   int sock_listen;
  int sock_client;
  struct sockaddr addr;
  int len = 0;
  int ret = 0;
  int buf_size = 256;
char buf[buf_size];
  FILE* fp;

  sock_listen = exp1_tcp_listen(11111);
  sock_client = accept(sock_listen, &addr, (socklen_t*) &len);
double t1 = get_current_timecount();
  fp = fopen("tmp.txt", "w");
  ret = recv(sock_client, buf, buf_size, 0);
  

 while (ret > 0) {
    fwrite(buf, sizeof(char), ret, fp);
    ret = recv(sock_client, buf, buf_size, 0);
  }
  
double t2 = get_current_timecount();
  printf("%lf\n", t2-t1);
  
   close(sock_client);
  

close(sock_listen);
  return 0;

}
