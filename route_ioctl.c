#include <assert.h>
#include <errno.h>
#include <ifaddrs.h> // getifaddrs()
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <arpa/inet.h> // inet_ntop
#include <linux/if_link.h> // struct rtnl_link_stats
#include <net/if_arp.h> // ARPHRD_* device type
#include <net/route.h> // struct rtentry

// #include <net/if.h> // netdevice(7) but no IFF_LOWER_UP
#include <linux/if.h> // provides IFF_LOWER_UP

#include "def.h"

#define AT(x) ((struct sockaddr_in*)(&(x)))
#define DEV "wlp2s0"

// #define STR0(x) #x
// #define STR(x) STR0(x)
// #define NAME_FMT "%-"STR(IFNAMSIZ)"s "
#define NAME_FMT "%s "

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

void steal_flag_u16(short *flags,const short f,const char *const s){
  if(*flags&f){
    printf("%s ",s);
    *flags=*flags&(~f);
  }
}

void steal_flag_u32(unsigned *flags,const unsigned f,const char *const s){
  if(*flags&f){
    printf("%s ",s);
    *flags=*flags&(~f);
  }
}

void addr(const char *t,struct sockaddr *addr){

  assert(addr);
  if(addr->sa_family==AF_PACKET){
    printf("%s (pkt) ",t);
    return;
  }

  // printf("[%d] \n",sin->sin_family);
  struct sockaddr_in *sin=(struct sockaddr_in*)(addr);
  assert(sin->sin_port==0);

  (t&&strlen(t))?printf("%s ",t):0;

  if(sin->sin_family==AF_INET6){
    char s[INET6_ADDRSTRLEN]={};
    assert(s==inet_ntop(AF_INET6,&(sin->sin_addr),s,INET6_ADDRSTRLEN));
    printf("[%s] ",s);
  }else if(sin->sin_family==AF_INET){
    char s[INET_ADDRSTRLEN]={};
    assert(s==inet_ntop(AF_INET,&(sin->sin_addr),s,INET_ADDRSTRLEN));
    printf("%s ",s);
  }else{
    assert(false);
  }

}

void netdevice(const int i){

  struct ifreq ifr0={.ifr_ifindex=i};
  assert(0==ioctl(sockfd,SIOCGIFNAME,&ifr0));
  printf("%d ",ifr0.ifr_ifindex);
  printf(NAME_FMT,ifr0.ifr_name);

  struct ifreq ifr={};
  int r=0;
  #define REFILL() {ifr=(struct ifreq){};strcpy(ifr.ifr_name,ifr0.ifr_name);r=0;errno=0;}

  REFILL();
  assert(0==ioctl(sockfd,SIOCGIFFLAGS,&ifr));
  steal_flag_u16(&(ifr.ifr_flags),IFF_UP,"up");
  steal_flag_u16(&(ifr.ifr_flags),IFF_BROADCAST,"broadcast");
  steal_flag_u16(&(ifr.ifr_flags),IFF_LOOPBACK,"lo");
  steal_flag_u16(&(ifr.ifr_flags),IFF_POINTOPOINT,"p2p");
  steal_flag_u16(&(ifr.ifr_flags),IFF_RUNNING,"running");
  steal_flag_u16(&(ifr.ifr_flags),IFF_NOARP,"noarp");
  steal_flag_u16(&(ifr.ifr_flags),IFF_MULTICAST,"multicast");
  assert(ifr.ifr_flags==0);

  REFILL();
  r=ioctl(sockfd,SIOCGIFADDR,&ifr);
  r?assert(r==-1&&errno==EADDRNOTAVAIL):addr(NULL,&(ifr.ifr_addr));

  REFILL();
  r=ioctl(sockfd,SIOCGIFDSTADDR,&ifr);
  r?assert(r==-1&&errno==EADDRNOTAVAIL):addr("dst",&(ifr.ifr_dstaddr));

  REFILL();
  r=ioctl(sockfd,SIOCGIFBRDADDR,&ifr);
  r?assert(r==-1&&errno==EADDRNOTAVAIL):addr("bcast",&(ifr.ifr_dstaddr));

  REFILL();
  r=ioctl(sockfd,SIOCGIFNETMASK,&ifr);
  r?assert(r==-1&&errno==EADDRNOTAVAIL):addr("mask",&(ifr.ifr_dstaddr));

  REFILL();
  assert(0==ioctl(sockfd,SIOCGIFMTU,&ifr));
  printf("mtu %d ",ifr.ifr_mtu);

  REFILL();
  assert(0==ioctl(sockfd,SIOCGIFHWADDR,&ifr));
  switch(ifr.ifr_hwaddr.sa_family){
    case ARPHRD_LOOPBACK:printf("loopback ");break;
    case ARPHRD_ETHER:printf("ethernet ");break;
    case ARPHRD_NONE:printf("noheader ");break;
  }
  const unsigned char *const h=(unsigned char*)ifr.ifr_hwaddr.sa_data;
  printf("hwaddr %02x:%02x:%02x:%02x:%02x:%02x ",h[0],h[1],h[2],h[3],h[4],h[5]);


  REFILL();
  assert(0==ioctl(sockfd,SIOCGIFMAP,&ifr));
  // printf("%zu",sizeof(struct ifmap)); // 24
  #define map ifr.ifr_map
  (map.mem_start||map.mem_end)?printf("mem 0x%lX-0x%lX ",map.mem_start,map.mem_end):0;
  assert(map.base_addr==0);
  map.irq?printf("irq %u ",map.irq):0;
  assert(map.dma==0&&map.port==0);

  REFILL();
  assert(0==ioctl(sockfd,SIOCGIFTXQLEN,&ifr));
  printf("txq %d ",ifr.ifr_qlen);

  #undef REFILL
  printf("\n");

}

void conf(){
  struct ifconf ifc={
    .ifc_len=0,
    .ifc_req=NULL
  };
  assert(0==ioctl(sockfd,SIOCGIFCONF,&ifc));
  const int sz=ifc.ifc_len;
  assert(sz%sizeof(struct ifreq)==0);
  const int n=sz/sizeof(struct ifreq);
  // printf("[%d*%zu=%d]\n",n,sizeof(struct ifreq),sz);
  char buf[sz];
  bzero(buf,sz);
  ifc.ifc_buf=buf;
  assert(0==ioctl(sockfd,SIOCGIFCONF,&ifc));
  assert(
    ifc.ifc_len==sz &&
    (char*)ifc.ifc_req==buf
  );
  for(int i=0;i<n;++i){
    printf("#%d ",ifc.ifc_req[i].ifr_ifindex);
    printf(NAME_FMT,ifc.ifc_req[i].ifr_name);
    addr(NULL,&(ifc.ifc_req[i].ifr_addr));
    printf("\n");
  }
}

void conf2(){
  struct ifaddrs *ifa=NULL;
  assert(0==getifaddrs(&ifa));
  assert(ifa);

  for(struct ifaddrs *i=ifa;i;i=i->ifa_next){
    printf(NAME_FMT,i->ifa_name);
    const unsigned flags=i->ifa_flags;
    steal_flag_u32(&(i->ifa_flags),IFF_UP,"up");
    steal_flag_u32(&(i->ifa_flags),IFF_BROADCAST,"broadcast");
    steal_flag_u32(&(i->ifa_flags),IFF_LOOPBACK,"lo");
    steal_flag_u32(&(i->ifa_flags),IFF_POINTOPOINT,"p2p");
    steal_flag_u32(&(i->ifa_flags),IFF_RUNNING,"running");
    steal_flag_u32(&(i->ifa_flags),IFF_NOARP,"noarp");
    steal_flag_u32(&(i->ifa_flags),IFF_MULTICAST,"multicast");
    steal_flag_u32(&(i->ifa_flags),IFF_LOWER_UP,"l1up");
    assert(i->ifa_flags==0);

    if(i->ifa_addr&&i->ifa_addr->sa_family==AF_PACKET){
      assert(
        !(i->ifa_netmask) &&
        i->ifa_broadaddr->sa_family==AF_PACKET &&
        i->ifa_dstaddr->sa_family==AF_PACKET
      );
      struct rtnl_link_stats *stats=i->ifa_data;
      assert(stats);
      printf("tx %u %uB rx %u %uB ",
        stats->tx_packets,
        stats->tx_bytes,
        stats->rx_packets,
        stats->rx_bytes
      );
    }else{
      i->ifa_addr?addr(NULL,i->ifa_addr):0;
      i->ifa_netmask?addr("mask",i->ifa_netmask):0;
      assert(!(flags&IFF_BROADCAST&IFF_POINTOPOINT)); // Mutually exclusive
      i->ifa_broadaddr?addr("bcast",i->ifa_broadaddr):0;
      i->ifa_dstaddr?addr("dst",i->ifa_dstaddr):0;
    }

    printf("\n");

  }

  free(ifa);
}

int main(){

  init();

  // set();
  // getchar();
  // reset();

  // printf("\n");
  // netdevice(1);
  // netdevice(2);
  // netdevice(3);
  // netdevice(5);
  // printf("\n");
  // conf();
  // printf("\n");
  conf2();
  // printf("\n");

  end();

  return 0;
}
