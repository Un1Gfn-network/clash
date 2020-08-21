// https://www.linuxquestions.org/questions/linux-networking-3/how-to-add-a-gateway-address-using-ioctl-in-c-in-linux-512213/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/route.h>
#include <sys/ioctl.h>
// #include <netinet/in.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <sys/types.h>

#include <errno.h>

int main() {

  struct rtentry e={};

  #define AT(x) ((struct sockaddr_in*)(&(x)))

  // default
  *AT(e.rt_dst)=(struct sockaddr_in){.sin_family=AF_INET,.sin_port=0,.sin_addr=INADDR_ANY};

  // via 192.168.1.1
  *AT(e.rt_gateway)=(struct sockaddr_in){.sin_family=AF_INET,.sin_port=0};
  assert(0!=inet_aton("192.168.1.1",&(AT(e.rt_gateway)->sin_addr)));

  // ?
  *AT(e.rt_genmask)=(struct sockaddr_in){.sin_family=AF_INET,.sin_port=0,.sin_addr=INADDR_ANY};

  e.rt_flags=RTF_UP|RTF_GATEWAY|RTF_STATIC;

  // metric 303
  e.rt_metric=303+1;

  // dev wlp2s0
  char *dev="wlp2s0";
  e.rt_dev=dev;

  int sockfd=socket(AF_INET,SOCK_DGRAM,0);
  assert(sockfd>=2);
  errno=0;
  assert(-1!=ioctl(sockfd,SIOCADDRT,&e));
  // assert(-1!=ioctl(sockfd,SIOCDELRT,&e));
  assert(errno==0);
  sockfd=0;

  return 0;
}
