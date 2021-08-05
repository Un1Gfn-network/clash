#include <arpa/inet.h> // inet_ntoa()
#include <assert.h>
#include <netdb.h> // getaddrinfo() // struct addrinfo
#include <stdio.h>
#include <string.h> // strcmp()

#include <libclash.h> // Header of resolv.c restful.c yaml2profile.c

#define common23(IT,P,RET) assert( !(IT->ai_canonname) && strcmp(RET,inet_ntoa(P->sin_addr))==0 )

static inline const struct sockaddr_in *common123(const struct addrinfo *__restrict const it){
  assert(
    it->ai_flags==AI_CANONNAME &&
    it->ai_family==AF_INET &&
    it->ai_addrlen==sizeof(struct sockaddr_in) &&
    it->ai_addr
  );
  const struct sockaddr_in *const p=(const struct sockaddr_in *)(it->ai_addr); // ip(7)
  assert(
    p->sin_family==AF_INET &&
    p->sin_port==0 &&
    p->sin_addr.s_addr!=0
  );
  return p;
}

// Returns dynamically allocated string
char *resolv(const char *__restrict domain){

  // Load
  assert(domain);
  struct addrinfo *res=NULL;
  const int errcode=getaddrinfo(domain,NULL,&(struct addrinfo){
    .ai_flags=AI_CANONNAME,
    .ai_family=AF_INET
  },&res);
  if(errcode!=0){
    printf("%s\n",gai_strerror(errcode));
    assert(0);
  }

  // First
  const struct addrinfo *it=res;
  const struct sockaddr_in *const p1=common123(it);
  assert(
    it->ai_socktype==SOCK_STREAM &&  // netinet_in.h(0p)
    it->ai_protocol==IPPROTO_TCP &&
    it->ai_canonname
  );
  char *const ret=strdup(inet_ntoa(p1->sin_addr));
  assert(ret);
  printf("%s(%s) -> %s\n",domain,it->ai_canonname,ret);

  // Second
  it=it->ai_next;
  const struct sockaddr_in *const p2=common123(it);
  common23(it,p2,ret);
  assert(
    it->ai_socktype==SOCK_DGRAM && // netinet_in.h(0p)
    it->ai_protocol==IPPROTO_UDP
  );

  // Third
  it=it->ai_next;
  const struct sockaddr_in *const p3=common123(it);
  common23(it,p3,ret);
  assert(
    it->ai_socktype==SOCK_RAW && // netinet_in.h(0p)
    it->ai_protocol==IPPROTO_IP
  );

  // printf("%d ",it->ai_socktype); // SOCK_DGRAM=2 SOCK_RAW=3
  // printf("%d ",it->ai_protocol); // IPPROTO_UDP=17 IPPROTO_IP=0
  // printf("%p ",it->ai_canonname);
  // printf("%s ",inet_ntoa(p->sin_addr));
  // printf("\n");

  // Cleanup
  assert(it->ai_next==NULL);
  freeaddrinfo(res);
  res=NULL;
  return ret;

}

/*int main(){
  free(resolv("hk19.edge.bgp.app"));
  free(resolv("sghk03.clashcloud.org"));
  free(resolv("sg23.clashcloud.org"));
  return 0;
}*/
