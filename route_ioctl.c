// https://www.linuxquestions.org/questions/linux-networking-3/how-to-add-a-gateway-address-using-ioctl-in-c-in-linux-512213/

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <arpa/inet.h>
#include <net/route.h>
#include <sys/ioctl.h>

#include "def.h"

#define AT(x) ((struct sockaddr_in*)(&(x)))
#define DEV "wlp2s0"

// jserv.c
char *json_load_server();

typedef enum {
  DEL,
  ADD
} OP;

int sockfd=-1;
char *server=NULL;
const char *gateway="192.168.1.1"; // Impossible to get routing info w/ ioctl

#define add_route(IP,VIA) route(true,false,IP,VIA)
#define del_route(IP,VIA) route(false,false,IP,VIA)
#define add_gateway(VIA) route(true,true,NULL,VIA)
#define del_gateway(VIA) route(false,true,NULL,VIA)
void route(bool add,bool net,const char *dst,const char *gw){

  assert( ( (bool)dst ^ (bool)net ) == 1 );
  assert(gw);
  struct rtentry e={};

  // Destination
  *AT(e.rt_dst)=(struct sockaddr_in){
    .sin_family=AF_INET,
    .sin_port=0,
    .sin_addr={INADDR_ANY}
  };
  if(!net)
    assert(0!=inet_aton(dst,&(AT(e.rt_dst)->sin_addr)));

  // Gateway
  *AT(e.rt_gateway)=(struct sockaddr_in){
    .sin_family=AF_INET,
    .sin_port=0
  };
  assert(0!=inet_aton(gw,&(AT(e.rt_gateway)->sin_addr)));

  // Genmask
  *AT(e.rt_genmask)=(struct sockaddr_in){
    .sin_family=AF_INET,
    .sin_port=0,
    .sin_addr={net?INADDR_ANY:INADDR_BROADCAST}
  };

  e.rt_flags = RTF_UP|RTF_GATEWAY|RTF_STATIC|(net?0:RTF_HOST) ;
  e.rt_dev=DEV;

  errno=0;
  if(-1==ioctl(sockfd,add?SIOCADDRT:SIOCDELRT,&e)){
    const int err=errno;
    printf("%d %s\n",err,strerror(err));
    assert(false);    
  }

}

void set(){
  assert(0==getuid());
  del_gateway(gateway);
  add_route(server,gateway);
  // add_gateway("10.0.0.2");
}

void reset(){
  assert(0==getuid());
  del_route(server,gateway);
  add_gateway(gateway);
  // del_gateway("10.0.0.2");
}

void init(){
  sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
  assert(sockfd==3);
  server=json_load_server();
  assert(server);
  // printf("%s\n",server);
}

void end(){
  free(server);
  server=NULL;
  close(sockfd);
  sockfd=0;
}

int main(){

  init();

  set();

  getchar();

  reset();

  end();

  return 0;
}
