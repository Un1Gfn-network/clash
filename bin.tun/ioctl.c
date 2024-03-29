#include <assert.h>
#include <errno.h>
#include <fcntl.h> // open()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // close()

// #include <bsd/stdlib.h> // humanize_number(3bsd)
#include <arpa/inet.h> // inet_ntop
#include <ifaddrs.h> // getifaddrs()
#include <linux/if_link.h> // struct rtnl_link_stats
#include <linux/if_packet.h> // struct sockaddr_ll
#include <net/if_arp.h> // ARPHRD_* device type
#include <net/route.h> // struct rtentry

#include <linux/sockios.h> // provides SIOCETHTOOL
#include <sys/ioctl.h> // no SIOCETHTOOL in <bits/ioctls.h>

// <net/if.h> <linux/if.h>
//     X            O      IFF_LOWER_UP
//     O            X      struct if_nameindex
#define IFF_LOWER_UP (1<<16)
#include <net/if.h>

#include <linux/if_tun.h>

// struct ethtool_cmd(deprecated) and ethtool_link_settings
// #include <linux/ethtool.h>

#include "./def.h"
#include "./ioctl.h"
#include "./privilege.h"

#define AT(x) ((struct sockaddr_in*)(&(x)))

// #define STR0(x) #x
// #define STR(x) STR0(x)
// #define NAME_FMT "%-"STR(IFNAMSIZ)"s "
#define NAME_FMT "%s "

// static int ioctlfd=-1;

/*void ioctl_init(){
  ESCALATED(ioctlfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP));
  assert(ioctlfd==3);
}*/

/*void ioctl_end(){
  ESCALATED(close(ioctlfd));
  ioctlfd=-1;
}*/

// "net": true for gateway, false for route
/*#define ioctl_add_route(D,IP,VIA) route(true,false,D,IP,VIA)
#define ioctl_del_route(D,IP,VIA) route(false,false,D,IP,VIA)
#define ioctl_add_gateway(D,VIA) route(true,true,D,NULL,VIA)
#define ioctl_del_gateway(D,VIA) route(false,true,D,NULL,VIA)
void ioctl_route(const bool add,const bool net,const char *const dev,const char *const dst,const char *const gw){

  assert(dev&&strlen(dev));
  // assert( ( (bool)dst ^ (bool)net ) == 1 );
  assert(net?(!dst):(dst&&strlen(dst)));
  assert(gw&&strlen(gw));

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

  char s[IFNAMSIZ]={};
  strncpy(s,dev,IFNAMSIZ);
  e.rt_dev=s;

  errno=0;
  int r=0;
  ESCALATED(r=ioctl(ioctlfd,add?SIOCADDRT:SIOCDELRT,&e));
  if(r==-1){
    const int err=errno;
    printf("%d %s\n",err,strerror(err));
    assert(0);    
  }

}*/

/*static inline void steal_flag_u16(short *flags,const short f,const char *const s){
  if(*flags&f){
    printf("%s ",s);
    *flags=*flags&(~f);
  }
}*/

/*static inline void steal_flag_u32(unsigned *flags,const unsigned f,const char *const s){
  if(*flags&f){
    printf("%s ",s);
    *flags=*flags&(~f);
  }
}*/

/*static inline void showaddr(const char *t,struct sockaddr *addr){

  assert(addr);
  (t&&strlen(t))?printf("%s ",t):0;
  // printf("[%d] \n",addr->sa_family);

  if(addr->sa_family==AF_PACKET){

    // packet(7)
    const struct sockaddr_ll *const s=(struct sockaddr_ll*)addr;
    assert(
      s->sll_family==AF_PACKET &&
      s->sll_protocol==0x0 &&
      // sll_ifindex
      // sll_hatype
      s->sll_pkttype==PACKET_HOST &&
      s->sll_halen==6
      // sll_addr[8]
    );
    for(int j=0;j<(s->sll_halen);++j)
      printf("%02X%s",s->sll_addr[j],j<(s->sll_halen-1)?":":" ");

  }else{
    struct sockaddr_in *sin=(struct sockaddr_in*)(addr);
    assert(sin->sin_port==0);

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

}*/

/*static inline void humanize(long n){
  int scale=0;
  char buf[SZ]={};
  for(;;){
    bzero(buf,SZ);
    assert(2<=humanize_number(buf,SZ,n,"",scale,HN_DECIMAL|HN_NOSPACE|HN_B));
    if(0==strncmp(buf,"0.0",3))
      break;
    ++scale;
  }
  scale>0?(scale-=1):0;
  bzero(buf,SZ);
  assert(2<=humanize_number(buf,SZ,n,"",scale,HN_DECIMAL|HN_NOSPACE|HN_B));
  printf("%s ",buf);
  fflush(stdout);
}*/

/*void ioctl_netdevice(const char *const name){

  struct ifreq ifr={};
  int r=0;
  #define REFILL() {ifr=(struct ifreq){};strncpy(ifr.ifr_name,name,IFNAMSIZ);r=0;errno=0;}

  // SIOCGIFNAME and SIOCGIFINDEX in onebyone()

  REFILL();
  assert(0==ioctl(ioctlfd,SIOCGIFHWADDR,&ifr));
  switch(ifr.ifr_hwaddr.sa_family){
    case ARPHRD_LOOPBACK:printf("loopback ");break;
    case ARPHRD_ETHER:printf("ethernet ");break;
    case ARPHRD_NONE:printf("noheader ");break;
    default: assert(false);
  }
  const unsigned char *const h=(unsigned char*)ifr.ifr_hwaddr.sa_data;
  printf("hwaddr %02x:%02x:%02x:%02x:%02x:%02x ",h[0],h[1],h[2],h[3],h[4],h[5]);

  REFILL();
  assert(0==ioctl(ioctlfd,SIOCGIFFLAGS,&ifr));
  steal_flag_u16(&(ifr.ifr_flags),IFF_UP,"up");
  steal_flag_u16(&(ifr.ifr_flags),IFF_BROADCAST,"broadcast");
  steal_flag_u16(&(ifr.ifr_flags),IFF_LOOPBACK,"lo");
  steal_flag_u16(&(ifr.ifr_flags),IFF_POINTOPOINT,"p2p");
  steal_flag_u16(&(ifr.ifr_flags),IFF_RUNNING,"running");
  steal_flag_u16(&(ifr.ifr_flags),IFF_NOARP,"noarp");
  steal_flag_u16(&(ifr.ifr_flags),IFF_MULTICAST,"multicast");
  assert(ifr.ifr_flags==0);

  REFILL();
  r=ioctl(ioctlfd,SIOCGIFADDR,&ifr);
  r?assert(r==-1&&errno==EADDRNOTAVAIL):showaddr(NULL,&(ifr.ifr_addr));

  REFILL();
  r=ioctl(ioctlfd,SIOCGIFDSTADDR,&ifr);
  r?assert(r==-1&&errno==EADDRNOTAVAIL):showaddr("dst",&(ifr.ifr_dstaddr));

  REFILL();
  r=ioctl(ioctlfd,SIOCGIFBRDADDR,&ifr);
  r?assert(r==-1&&errno==EADDRNOTAVAIL):showaddr("bcast",&(ifr.ifr_dstaddr));

  REFILL();
  r=ioctl(ioctlfd,SIOCGIFNETMASK,&ifr);
  r?assert(r==-1&&errno==EADDRNOTAVAIL):showaddr("mask",&(ifr.ifr_dstaddr));

  REFILL();
  assert(0==ioctl(ioctlfd,SIOCGIFMTU,&ifr));
  printf("mtu %d ",ifr.ifr_mtu);

  REFILL();
  assert(0==ioctl(ioctlfd,SIOCGIFMAP,&ifr));
  // printf("%zu",sizeof(struct ifmap)); // 24
  #define map ifr.ifr_map
  (map.mem_start||map.mem_end)?printf("mem 0x%lX-0x%lX ",map.mem_start,map.mem_end):0;
  assert(map.base_addr==0);
  map.irq?printf("irq %u ",map.irq):0;
  assert(map.dma==0&&map.port==0);

  REFILL();
  assert(0==ioctl(ioctlfd,SIOCGIFTXQLEN,&ifr));
  printf("txq %d ",ifr.ifr_qlen);

  // REFILL();
  // // struct ethtool_cmd ec={.cmd=ETHTOOL_GSET};
  // struct ethtool_cmd ec={.cmd=};
  // ifr.ifr_data=&ec;
  // r=ioctl(ioctlfd,SIOCETHTOOL,&ifr); // /usr/include/linux/sockios.h
  // if(r!=0){
  //   assert( r==-1 && errno==EOPNOTSUPP);
  // }else{
  //   printf("speed %u %u ",ec.speed,ethtool_cmd_speed(&ec));
  // }

  // REFILL();
  // struct ethtool_link_settings els={.cmd=ETHTOOL_GLINKSETTINGS};
  // ifr.ifr_data=&els;
  // r=ioctl(ioctlfd,SIOCETHTOOL,&ifr); // /usr/include/linux/sockios.h
  // if(r!=0){
  //   assert( r==-1 && errno==EOPNOTSUPP);
  // }else{
  //   printf("[%u 0x%X 0x%X] ",els.speed,els.duplex,els.port);
  // }

  #undef REFILL
  printf("\n");

}*/

/*void ioctl_onebyone(){

  #ifdef _LINUX_IF_H
  #pragma GCC error "include <net/if.h> instead of <linux/if.h>"
  #endif
  struct if_nameindex *ifn=if_nameindex();
  assert(ifn);
  for(int i=0;;++i){
    if(ifn[i].if_index==0){
      assert(!ifn[i].if_name);
      break;
    }
    // SIOCGIFNAME index to name
    struct ifreq ifrI2N=(struct ifreq){.ifr_ifindex=ifn[i].if_index};
    assert(0==ioctl(ioctlfd,SIOCGIFNAME,&ifrI2N));
    assert(0==strcmp(ifrI2N.ifr_name,ifn[i].if_name));
    // SIOCGIFINDEX name to index
    struct ifreq ifrN2I={};
    strncpy(ifrN2I.ifr_name,ifn[i].if_name,IFNAMSIZ);
    assert(0==ioctl(ioctlfd,SIOCGIFINDEX,&ifrN2I));
    assert((unsigned)ifrN2I.ifr_ifindex==ifn[i].if_index);
    printf("#%d %s | ",ifn[i].if_index,ifn[i].if_name);
    netdevice(ifn[i].if_name);
  }
  if_freenameindex(ifn);
  ifn=NULL;
}*/

/*void ioctl_all_ifconf(){
  struct ifconf ifc={
    .ifc_len=0,
    .ifc_req=NULL
  };
  assert(0==ioctl(ioctlfd,SIOCGIFCONF,&ifc));
  const int sz=ifc.ifc_len;
  assert(sz%sizeof(struct ifreq)==0);
  const int n=sz/sizeof(struct ifreq);
  // printf("[%d*%zu=%d]\n",n,sizeof(struct ifreq),sz);
  char buf[sz];
  bzero(buf,sz);
  ifc.ifc_buf=buf;
  assert(0==ioctl(ioctlfd,SIOCGIFCONF,&ifc));
  assert(
    ifc.ifc_len==sz &&
    (char*)ifc.ifc_req==buf
  );
  for(int i=0;i<n;++i){
    // printf("#%d ",ifc.ifc_req[i].ifr_ifindex);
    printf(NAME_FMT,ifc.ifc_req[i].ifr_name);
    showaddr(NULL,&(ifc.ifc_req[i].ifr_addr));
    printf("\n");
  }
}*/

/*void ioctl_all_getifaddrs(){
  struct ifaddrs *ifa=NULL;
  assert(0==getifaddrs(&ifa));
  assert(ifa);

  for(struct ifaddrs *i=ifa;i;i=i->ifa_next){

    #define PKT(x) ((const struct sockaddr_ll*)x)
    const bool pkt=(i->ifa_addr&&i->ifa_addr->sa_family==AF_PACKET);
    pkt?assert(
      !(i->ifa_netmask) &&
      i->ifa_broadaddr &&
      i->ifa_dstaddr &&
      i->ifa_broadaddr->sa_family==AF_PACKET &&
      i->ifa_dstaddr->sa_family==AF_PACKET
      // &&
      // PKT(i->ifa_addr)->sll_ifindex==PKT(i->ifa_broadaddr)->sll_ifindex &&
      // PKT(i->ifa_addr)->sll_ifindex==PKT(i->ifa_dstaddr)->sll_ifindex
    ):0;

    // pkt?printf("#%d ",PKT(i)->sll_ifindex):0;
    printf(NAME_FMT,i->ifa_name);
    if(pkt){
      switch(PKT(i->ifa_addr)->sll_hatype){
        case ARPHRD_LOOPBACK:printf("loopback ");break;
        case ARPHRD_ETHER:printf("ethernet ");break;
        case ARPHRD_NONE:printf("noheader ");break;
        default:assert(false);
      };
      printf("| ");
    }
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
    if(pkt){
      struct rtnl_link_stats *stats=i->ifa_data;
      assert(stats);
      printf("tx %u ",stats->tx_packets);
      humanize(stats->tx_bytes);
      printf("rx %u ",stats->rx_packets);
      humanize(stats->rx_bytes);
    }
    printf("\n");

    assert(!(flags&IFF_BROADCAST&IFF_POINTOPOINT)); // Mutually exclusive
    i->ifa_addr?showaddr(NULL,i->ifa_addr):0;
    i->ifa_netmask?showaddr("mask",i->ifa_netmask):0;
    i->ifa_broadaddr?showaddr("bcast",i->ifa_broadaddr):0;
    i->ifa_dstaddr?showaddr("dst",i->ifa_dstaddr):0;
    (
      i->ifa_addr ||
      i->ifa_netmask ||
      i->ifa_broadaddr ||
      i->ifa_dstaddr
    )?printf("\n"):0;

    printf("\n");

  }

  free(ifa);
}*/

void ioctl_tun_create(const char *__restrict const dev){
  assert(dev&&strlen(dev));
  struct ifreq ifc={.ifr_flags=IFF_TUN|IFF_NO_PI};
  strncpy(ifc.ifr_name,dev,IFNAMSIZ);
  int tunfd=open("/dev/net/tun",O_RDWR);
  assert(tunfd>=3);
  privilege_escalate();
  assert(0==ioctl(tunfd,TUNSETIFF,&ifc));
  assert(0==ioctl(tunfd,TUNSETPERSIST,1));
  privilege_drop();
  close(tunfd);
  tunfd=-1;
}

/*static inline void cidr_mask(unsigned n,struct in_addr *sin_addr_p){
  assert(n<=32);
  unsigned host=0;
  for(;n>0;--n)
    host|=((1<<(32U-n))); // ((1<<31)>>(n-1));
  sin_addr_p->s_addr=htonl(host);
}*/

/*void ioctl_tun_addr(const char *const dev,const char *const ipv4,unsigned n){

  struct sockaddr_in *sin=NULL;

  struct ifreq addr={};
  strncpy(addr.ifr_name,dev,IFNAMSIZ);
  sin=(struct sockaddr_in*)(&(addr.ifr_addr));
  sin->sin_family=AF_INET;
  sin->sin_port=0;
  assert(1==inet_pton(AF_INET,ipv4,&(sin->sin_addr)));
  assert(0==ioctl(ioctlfd,SIOCSIFADDR,&addr));

  struct ifreq mask={};
  strncpy(mask.ifr_name,dev,IFNAMSIZ);
  sin=(struct sockaddr_in*)(&(mask.ifr_netmask));
  sin->sin_family=AF_INET;
  sin->sin_port=0;
  cidr_mask(n,&(sin->sin_addr));
  // char s[INET_ADDRSTRLEN]={};
  // assert(s==inet_ntop(AF_INET,&(sin->sin_addr),s,INET_ADDRSTRLEN));
  // printf("%s\n",s);
  assert(0==ioctl(ioctlfd,SIOCSIFNETMASK,&mask));

}*/

/*#define ioctl_up(D) flags(true,D)
#define ioctl_down(D) flags(false,D) // Does not change qdisc from fq_codel to noop
void ioctl_flags(const bool up,const char *const dev){
  privilege_escalate();
  struct ifreq ifr={};
  strncpy(ifr.ifr_name,dev,IFNAMSIZ);
  assert(0==ioctl(ioctlfd,SIOCGIFFLAGS,&ifr));
  if(up) ifr.ifr_flags|=IFF_UP;
  else ifr.ifr_flags&=(~((short)IFF_UP));
  assert(0==ioctl(ioctlfd,SIOCSIFFLAGS,&ifr));
  privilege_drop();
}*/

/*void set(){

  tun_create(TUN);
  // exit(0);
  tun_addr(TUN,"10.0.0.1",24);
  up(TUN);

  del_gateway(WLO,gateway);
  add_gateway(TUN,"10.0.0.2");

  server=json_load_server();
  assert(server);
  printf("%s\n",server);
  add_route(WLO,server,gateway);

}*/

/*void reset(){

  del_route(WLO,server,gateway);
  free(server);
  server=NULL;

  del_gateway(TUN,"10.0.0.2");
  add_gateway(WLO,gateway);

  down(TUN); // Does not change qdisc from fq_codel to noop
  // tun_del(TUN); // Fail

}*/

/*int main(){

  init();

  set();
  getchar();
  reset();

  // printf("\n");
  // onebyone();
  // printf("\n");
  // all_ifconf();
  // printf("\n");
  // all_getifaddrs();
  // printf("\n");

  end();

  return 0;
}*/
