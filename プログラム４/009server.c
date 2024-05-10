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
  int bufsize=128;
  char buf[bufsize];
  int fd;



  sock_listen = exp1_tcp_listen(11111);
  sock_client = accept(sock_listen, &addr, (socklen_t*) &len);
double t1 = get_current_timecount();
  
  fd = open("tmp.txt",O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR);
  ret = read(sock_client, buf, bufsize);
  

 while (ret > 0) {
   write(fd, buf,ret);
    ret =  read(sock_client,buf,bufsize);
  }
  double t2 = get_current_timecount();
  printf("%lf\n", t2-t1);
  close(fd);
   close(sock_client);
  close(sock_listen);

  return 0;
}
