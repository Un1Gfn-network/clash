#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define eprintf(...) fprintf(stderr,__VA_ARGS__)

char *resolv(const char *domain){

  struct addrinfo *res=NULL;

  struct addrinfo hints={
    .ai_flags=AI_CANONNAME,
    .ai_family=AF_INET
  };

  const int errcode=getaddrinfo(domain,NULL,&hints,&res);
  if(errcode!=0){ 
    eprintf("%s\n",gai_strerror(errcode));
  }

  const struct addrinfo *it=res;
  char *ret=NULL;

  for(int i=0;i<=1;++i,it=it->ai_next){

    assert(it->ai_flags==AI_CANONNAME);
    assert(it->ai_family==AF_INET);
    if(i==0){
      assert(it->ai_socktype==SOCK_STREAM);
      assert(it->ai_protocol==IPPROTO_TCP); // netinet_in.h(0p)
    }else{
      assert(it->ai_socktype==SOCK_DGRAM);
      assert(it->ai_protocol==IPPROTO_UDP); // netinet_in.h(0p)
    }

    assert(it->ai_addrlen==sizeof(struct sockaddr_in));
    assert(it->ai_addr);

    // ip(7)
    const struct sockaddr_in *p=(struct sockaddr_in *)(it->ai_addr);
    assert(p->sin_family==AF_INET);
    assert(p->sin_port==0);
    assert(p->sin_addr.s_addr!=0);
    if(!ret)
      assert((ret=strdup(inet_ntoa(p->sin_addr))));
    else
      assert(0==strcmp(ret,inet_ntoa(p->sin_addr)));
    assert(ret);

    if(i==0){
      assert(it->ai_canonname);
      assert(0==strcmp(domain,it->ai_canonname));
    }else{
      assert(it->ai_canonname==NULL);
    }

  }

  assert(it->ai_next==NULL);

  freeaddrinfo(res);
  res=NULL;

  return ret;

}

/*int main(){

  char *ip=resolv("hk19.edge.bgp.app"); // "jp5.edge.as4809.app"
  assert(ip);
  printf("%s\n",ip);
  free(ip);
  ip=NULL;

  return 0;

}*/
