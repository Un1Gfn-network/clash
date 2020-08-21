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

#include "def.h"

#define AT(x) ((struct sockaddr_in*)(&(x)))
#define DEV "wlp2s0"

typedef enum {
  DEL,
  ADD
} OP;

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

void perform(OP op,struct rtentry *ep){
  int request=-1;
  if(op==DEL)
    request=SIOCDELRT;
  else if(op==ADD)
    request=SIOCADDRT;
  else
    assert(false);
  errno=0;
  if(-1==ioctl(sockfd,request,ep)){
    const int err=errno;
    printf("%d %s\n",err,strerror(err));
    assert(false);    
  }
}

void net(OP op,const char *gw){

  assert(gw);
  struct rtentry e={};

  // Destination
  *AT(e.rt_dst)=(struct sockaddr_in){
    .sin_family=AF_INET,
    .sin_port=0,
    .sin_addr={INADDR_ANY}
  };

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
    .sin_addr={INADDR_ANY}
  };

  e.rt_flags = RTF_UP|RTF_GATEWAY|RTF_STATIC ;
  e.rt_dev=DEV;
  perform(op,&e);

}

void host(OP op,const char *const dst,const char *gw){

  assert(dst);
  assert(gw);
  struct rtentry e={};

  // Destination
  *AT(e.rt_dst)=(struct sockaddr_in){
    .sin_family=AF_INET,
    .sin_port=0
  };
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
    .sin_addr={INADDR_BROADCAST}
  };

  e.rt_flags = RTF_UP|RTF_GATEWAY|RTF_HOST|RTF_STATIC ;
  e.rt_dev=DEV;
  perform(op,&e);

}

#define del_route(IP,VIA) host(DEL,IP,VIA)
#define add_route(IP,VIA) host(ADD,IP,VIA)
#define del_gateway(VIA) net(DEL,VIA)
#define add_gateway(VIA) net(ADD,VIA)

int main(const int argc,const char **argv){

  if( argc!=2 || !argv[1] ){
    printf("\n  %s <on|off>\n\n",argv[0]);
    exit(1);
  }

  assert(0==getuid());

  sockfd=socket(AF_INET,SOCK_DGRAM,0);
  assert(sockfd>=2);
  json_load_server();
  assert(server);

  // printf("%s\n",server);
  // exit(0);

  if(strcmp(argv[1],"on")){
    del_gateway("192.168.1.1");
    add_route(server,"192.168.1.1");
    add_gateway("10.0.0.2");
  }else if(strcmp(argv[1],"off")){
    del_gateway("10.0.0.2");
    del_route(server,"192.168.1.1");
    add_gateway("192.168.1.1");
  }else{
    assert(false);
  }

  free(server);
  server=NULL;
  close(sockfd);
  sockfd=0;

  return 0;
}
