// https://www.linuxquestions.org/questions/linux-networking-3/how-to-add-a-gateway-address-using-ioctl-in-c-in-linux-512213/

#include <assert.h>
#include <errno.h>
#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <arpa/inet.h>
#include <net/route.h>
#include <sys/ioctl.h>
// #include <netinet/in.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <sys/types.h>

#include "def.h"

#define DEV "wlp2s0"

char *device=NULL;

char *server=NULL;

int sockfd=-1;

void json_load_server(){

  json_object *j=json_object_from_file(SS_LOCAL_JSON);
  assert(j);
  assert(json_type_object==json_object_get_type(j));

  json_object *j2=json_object_object_get(j,"server");
  assert(j2);
  assert(json_type_string==json_object_get_type(j2));
  server=strdup(json_object_get_string(j2));

  assert(1==json_object_put(j));

}

typedef enum {
  DEL,
  ADD
} OP;

void route(OP op,const char *dst,const char *via){

  // Only default gateway is implemented so far
  assert(!dst);

  // if(dst)
  //   printf("%s\n",dst);

  printf("ip route %s %s via %s dev %s\n", op==DEL?"del":"add", dst?dst:"default", via, DEV);

  struct rtentry e={};

  #define AT(x) ((struct sockaddr_in*)(&(x)))

  // Destination(0.0.0.0 default)
  *AT(e.rt_dst)=(struct sockaddr_in){.sin_family=AF_INET,.sin_port=0};
  if(dst)
    assert(0!=inet_aton(dst,&(AT(e.rt_dst)->sin_addr))); // Add normal route
  else
    AT(e.rt_dst)->sin_addr=(struct in_addr){INADDR_ANY}; // Add default gateway

  // Via(mandatary)
  assert(via);
  *AT(e.rt_gateway)=(struct sockaddr_in){.sin_family=AF_INET,.sin_port=0};
  assert(0!=inet_aton(via,&(AT(e.rt_gateway)->sin_addr)));

  // Mask
  // if(dst){
  //   *AT(e.rt_genmask)=(struct sockaddr_in){.sin_family=AF_INET,.sin_port=0};
  //   assert(0!=inet_aton("0.0.0.0",&(AT(e.rt_genmask)->sin_addr)));
  // }
  // else
  *AT(e.rt_genmask)=(struct sockaddr_in){.sin_family=AF_INET,.sin_port=0,.sin_addr={INADDR_ANY}};

  // e.rt_flags = dst ? (RTF_UP|RTF_STATIC) : (RTF_UP|RTF_STATIC|RTF_GATEWAY) ;
  e.rt_flags = RTF_UP|RTF_STATIC|RTF_GATEWAY;

  // e.rt_metric=303+1;

  e.rt_dev=DEV;

  int request=-1;
  if(op==DEL)
    request=SIOCDELRT;
  else if(op==ADD)
    request=SIOCADDRT;
  else
    assert(false);

  errno=0;
  if(-1==ioctl(sockfd,request,&e)){
    const int err=errno;
    printf("%d %s\n",err,strerror(err));
    assert(false);    
  }

}

void del_gateway(const char *gw){
  route(DEL,NULL,gw);
}

void add_gateway(const char *gw){
  route(ADD,NULL,gw);
}

void del_route_via(const char *dst,const char *via){
  route(DEL,dst,via);
}

void add_route_via(const char *dst,const char *via){
  route(ADD,dst,via);
}

void set(){

  del_gateway("192.168.1.1");

  // add_route_via(server,"192.168.1.1");
  // add_gateway("10.0.0.2");

}

void reset(){

  // del_gateway("10.0.0.2");
  // del_route_via(server,"192.168.1.1");

  add_gateway("192.168.1.1");

}

int main(){

  sockfd=socket(AF_INET,SOCK_DGRAM,0);
  assert(sockfd>=2);
  json_load_server();
  assert(server);

  // printf("%s\n",server);
  // exit(0);

  set();
  reset();
  set();
  reset();

  /*reset();
  set();
  reset();
  set();*/

  free(server);
  server=NULL;
  close(sockfd);
  sockfd=0;

  return 0;
}
