#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <net/if_arp.h>

#include <linux/if.h>

#include "def.h" // TUN

// 11:22:33:44:55:66
#define MAC_L (2*6+5)

// #define SZ 16384
#define SZ 8192

#define eprintf(...) fprintf(stderr,__VA_ARGS__)

// jsrv.c
char *json_load_server();

int fd=-1;

char recvbuf[SZ]={};
int len=0;

char *server=NULL;
char gw[INET_ADDRSTRLEN]={};

typedef struct {
  bool caught;
  unsigned v;
} V32;

void init(){
  fd=socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
  assert(fd==3);
  assert(0==bind(fd,(struct sockaddr*)(&(struct sockaddr_nl){
    .nl_family=AF_NETLINK,
    .nl_pad=0,
    .nl_pid=getpid(),
    .nl_groups=0
  }),sizeof(struct sockaddr_nl)));
}

void end(){
  bzero(gw,INET_ADDRSTRLEN);
  assert(0==close(fd));
  fd=-1;
}

void clearbuf(){
  bzero(recvbuf,SZ);
  len=0;
}

void receive(){
  for(char *p=recvbuf;;){
    const int seglen=recv(fd,p,sizeof(recvbuf)-len,0);
    assert(seglen>=1);
    len+=seglen;
    // printf("0x%X\n",((struct nlmsghdr*)p)->nlmsg_type);
    if(((struct nlmsghdr*)p)->nlmsg_type==NLMSG_DONE||((struct nlmsghdr*)p)->nlmsg_type==NLMSG_ERROR)
      break;
    p+=seglen;
  }
}

/*void poll(){
  const struct nlmsghdr *nh=(struct nlmsghdr*)recvbuf;
  for(;NLMSG_OK(nh,len);nh=NLMSG_NEXT(nh,len)){
    switch(nh->nlmsg_type){
      case NLMSG_NOOP:printf("NLMSG_NOOP ");break;
      case NLMSG_ERROR:printf("NLMSG_ERROR ");break;
      case NLMSG_DONE:printf("NLMSG_DONE ");break;
      case RTM_NEWROUTE:printf("RTM_NEWROUTE ");break;
      case RTM_NEWLINK:printf("RTM_NEWLINK ");break;
      default:assert(false);
    }
    printf("\n");
  }
}*/

void ack(){
  assert(((struct nlmsghdr*)recvbuf)->nlmsg_type==NLMSG_ERROR);
  // assert(((struct nlmsgerr*)NLMSG_DATA((struct nlmsghdr*)recvbuf))->error==0);
  const int e=((struct nlmsgerr*)NLMSG_DATA((struct nlmsghdr*)recvbuf))->error;
  if(e!=0){
    eprintf("%d %s\n",e,strerror(e));
    assert(false);
  }
}

void attr(struct nlmsghdr *const n,const size_t maxlen,const int type,const void *const data){

  #define NEWLEN(x) (NLMSG_ALIGN(n->nlmsg_len)+RTA_ALIGN(RTA_LENGTH(x)))
  #define FILL(x) {assert(NEWLEN(x)<=maxlen);rta->rta_type=type;rta->rta_len=RTA_LENGTH(x);}

  size_t l=0;
  struct rtattr *const rta=(struct rtattr*)(((char*)n)+NLMSG_ALIGN(n->nlmsg_len));

  if(type==IFLA_IFNAME){
    const char *const s=data;
    l=strlen(s)+1;
    FILL(l);
    strcpy(RTA_DATA(rta),s);
  }else if(type==RTA_OIF){
    l=sizeof(int);
    FILL(l);
    *((int*)RTA_DATA(rta))=*((int*)data);
  }else if(type==RTA_DST||type==RTA_GATEWAY){
    l=sizeof(struct in_addr);
    FILL(l);
    bzero(RTA_DATA(rta),l);
    assert(1==inet_pton(AF_INET,data,RTA_DATA(rta)));
  }else{
    assert(false);
  }

  n->nlmsg_len=NEWLEN(l);

  #undef NEWLEN
  #undef FILL

}

#define add_route(dst,via) route(true,false,dst,via)
#define del_route(dst,via) route(false,false,dst,via)
#define add_gateway(via) route(true,true,NULL,via)
#define del_gateway(via) route(false,true,NULL,via)
void route(const bool add,const bool gw,const char *const dst,const char *const via){

  // RTM_DELROUTE/RTM_ADDROUTE only
  typedef struct {
    struct nlmsghdr nh;
    struct rtmsg rt;
    char attrbuf[SZ]; // rtnetlink(3)
  } Req;

  assert(0==getuid());
  if(gw)
    assert(!dst);

  Req req={
    .nh={
      .nlmsg_len=NLMSG_LENGTH(sizeof(struct rtmsg)),
      .nlmsg_type= add ? RTM_NEWROUTE : RTM_DELROUTE ,
      .nlmsg_flags=(NLM_F_REQUEST|NLM_F_ACK|(add?(NLM_F_EXCL|NLM_F_CREATE):0)),
      .nlmsg_seq=0,
      .nlmsg_pid=0
    },
    .rt={
      .rtm_family=AF_INET,
      .rtm_dst_len= gw ? 0 : 32 ,
      .rtm_src_len=0,
      .rtm_tos=0,
      .rtm_table=RT_TABLE_MAIN,
      .rtm_protocol= add ? RTPROT_BOOT : RTPROT_UNSPEC,
      .rtm_scope=RT_SCOPE_UNIVERSE,
      .rtm_type=RTN_UNICAST,
      .rtm_flags=0
    },
    .attrbuf={}
  };
  assert(NLMSG_DATA(&req.nh)==&req.rt);

  // struct rtattr *rta=RTM_RTA(NLMSG_DATA(&req.nh));
  // assert(rta==(struct rtattr *)((char*)&req.nh+NLMSG_LENGTH(sizeof(struct rtmsg))));
  // int len=sizeof(Req)-NLMSG_LENGTH(sizeof(struct rtmsg));
  (!gw)?attr(&req.nh,sizeof(Req),RTA_DST,dst):0;
  attr(&req.nh,sizeof(Req),RTA_GATEWAY,via);
  attr(&req.nh,sizeof(Req),RTA_OIF,&((int){3}));

  assert(sizeof(Req)==send(fd,&req,sizeof(Req),0));
  receive();
  ack();
  clearbuf();

}

void ask_route(){

  // RTM_GETROUTE only
  typedef struct {
    struct nlmsghdr nh;
    struct rtmsg rt;
  } Req;

  // rtnetlink(3)
  assert(NLMSG_LENGTH(sizeof(struct rtmsg))==sizeof(Req));
  assert(sizeof(Req)==send(fd,&(Req){
    .nh={
      .nlmsg_len=NLMSG_LENGTH(sizeof(struct rtmsg)),
      .nlmsg_type=RTM_GETROUTE,
      .nlmsg_flags=NLM_F_REQUEST|NLM_F_ROOT,
      .nlmsg_seq=0,
      .nlmsg_pid=0
    },
    .rt={
      .rtm_family=AF_INET,
      .rtm_dst_len=0,
      .rtm_src_len=0,
      .rtm_tos=0,
      .rtm_table=0/*RT_TABLE_MAIN*/,
      .rtm_protocol=RTPROT_UNSPEC,
      .rtm_scope=RT_SCOPE_UNIVERSE,
      .rtm_type=RTN_UNSPEC,
      .rtm_flags=0
    }
  },sizeof(Req),0));
}

void steal_flag_8(unsigned char *const flags,const unsigned char f,const char *const s){
  if(*flags&f){
    printf("%s ",s);
    *flags=*flags&(~f);
  }
}

void steal_flag_32(unsigned *const flags,const unsigned f,const char *const s){
  if(*flags&f){
    printf("%s ",s);
    *flags=*flags&(~f);
  }
}

void print_route(){

  ask_route();
  receive();

  struct nlmsghdr *nh=(struct nlmsghdr*)recvbuf;
  for(;NLMSG_OK(nh,len);nh=NLMSG_NEXT(nh,len)){

    if(nh->nlmsg_type==NLMSG_DONE)
      break;
    assert(
      // nh->nlmsg_len
      nh->nlmsg_type==RTM_NEWROUTE &&
      nh->nlmsg_flags==NLM_F_MULTI &&
      nh->nlmsg_pid==(unsigned)getpid()
    );

    const struct rtmsg *const rtm=(struct rtmsg*)NLMSG_DATA(nh);
    if(rtm->rtm_table!=RT_TABLE_MAIN){
      assert(rtm->rtm_table==RT_TABLE_LOCAL);
      printf("[local] (skipped)\n");
      continue;
    }
    printf("[main] ");
    // printf("[%u] ",rtm->rtm_family);
    // fflush(stdout);
    assert(rtm->rtm_family==AF_INET);
    printf("/%u ",rtm->rtm_dst_len);
    assert(rtm->rtm_src_len==0);
    assert(rtm->rtm_tos==0);
    // printf("%u ",rtm->rtm_protocol);
    if(rtm->rtm_protocol==RTPROT_DHCP) printf("dhcp ");
    else if(rtm->rtm_protocol==RTPROT_BOOT) printf("boot ");
    else assert(false);
    if(rtm->rtm_scope==RT_SCOPE_LINK) printf("link ");
    else if(rtm->rtm_scope==RT_SCOPE_UNIVERSE) printf("universe ");
    else assert(false);
    assert(rtm->rtm_type==RTN_UNICAST);
    assert(rtm->rtm_flags==0);

    printf("||| ");

    struct rtattr *p=(struct rtattr*)RTM_RTA(rtm);
    assert(p==(struct rtattr*)((char*)nh+NLMSG_LENGTH(sizeof(struct rtmsg))));
    int rtl=RTM_PAYLOAD(nh);
    for(;RTA_OK(p,rtl);p=RTA_NEXT(p,rtl)){
      char s[INET_ADDRSTRLEN]={};
      switch(p->rta_type){
        case RTA_DST:
          assert(((struct sockaddr_in*)RTA_DATA(p))->sin_addr.s_addr!=INADDR_ANY);
          assert(s==inet_ntop(AF_INET,RTA_DATA(p),s,INET_ADDRSTRLEN));
          printf("dst %s ",s);
          break;
          // if(((struct sockaddr_in*)RTA_DATA(p))->sin_addr.s_addr==INADDR_ANY){
          //   printf("default ");
          // }else{
          //   assert(s==inet_ntop(AF_INET,RTA_DATA(p),s,INET_ADDRSTRLEN));
          //   printf("dst %s ",s);
          // }
          // break;
        case RTA_OIF:
          printf("dev %d ",*((int*)RTA_DATA(p)));
          break;
        case RTA_GATEWAY:
          assert(s==inet_ntop(AF_INET,RTA_DATA(p),s,INET_ADDRSTRLEN));
          printf("gw %s ",s);
          break;
        case RTA_PRIORITY:
          printf("pri %d ",*((int*)RTA_DATA(p)));
          break;
        case RTA_PREFSRC:
          assert(s==inet_ntop(AF_INET,RTA_DATA(p),s,INET_ADDRSTRLEN));
          printf("prefsrc %s ",s);
          break;
        case RTA_TABLE:
          assert(RT_TABLE_MAIN==*((int*)RTA_DATA(p)));
          break;
        default:
          assert(false);
          // printf("[type %u] ",p->rta_type);
          break;
      }
    }

    printf("\n");

  }

  clearbuf();

}

void external(){
  printf("Terminate? ");
  fflush(stdout);
  while(getchar()!='\n'){}
}

void get_gateway(char *const s){

  ask_route();
  receive();

  struct nlmsghdr *nh=(struct nlmsghdr*)recvbuf;
  for(;NLMSG_OK(nh,len);nh=NLMSG_NEXT(nh,len)){

    assert(
      // nh->nlmsg_len
      nh->nlmsg_type==RTM_NEWROUTE &&
      nh->nlmsg_flags==NLM_F_MULTI &&
      nh->nlmsg_seq==0 &&
      nh->nlmsg_pid==(unsigned)getpid()
    );

    const struct rtmsg *const rtm=(struct rtmsg*)NLMSG_DATA(nh);
    if(
      rtm->rtm_dst_len!=0 ||
      rtm->rtm_table!=RT_TABLE_MAIN ||
      rtm->rtm_scope!=RT_SCOPE_UNIVERSE
    ){
      assert(rtm->rtm_dst_len==24);
      assert(rtm->rtm_table==RT_TABLE_LOCAL);
      assert(rtm->rtm_scope==RT_SCOPE_LINK);
      continue;
    }
    assert(
      rtm->rtm_family==AF_INET &&
      // rtm_dst_len
      rtm->rtm_src_len==0 &&
      rtm->rtm_tos==0 &&
      // rtm_table
      (rtm->rtm_protocol==RTPROT_DHCP || rtm->rtm_protocol==RTPROT_BOOT) &&
      // rtm_scope
      rtm->rtm_type==RTN_UNICAST &&
      rtm->rtm_flags==0
    );

    struct rtattr *rta=(struct rtattr*)RTM_RTA(rtm);
    assert(rta==(struct rtattr*)((char*)nh+NLMSG_LENGTH(sizeof(struct rtmsg))));
    int rtl=RTM_PAYLOAD(nh);
    for(;RTA_OK(rta,rtl);rta=RTA_NEXT(rta,rtl)){
      switch(rta->rta_type){
        case RTA_GATEWAY:
          // gw=*(struct in_addr*)RTA_DATA(rta);
          assert(s==inet_ntop(AF_INET,RTA_DATA(rta),s,INET_ADDRSTRLEN));
          assert(0==strcmp(s,"192.168.1.1"));
          break;
        case RTA_OIF:
          assert(3==*((int*)RTA_DATA(rta)));
          break;
        case RTA_PREFSRC:
          break;
        case RTA_PRIORITY:
          assert(303==*((int*)RTA_DATA(rta)));
          break;
        case RTA_TABLE:
          assert(RT_TABLE_MAIN==*((int*)RTA_DATA(rta)));
          break;
        default:
          assert(false);
          break;
      }

    }

    clearbuf();
    return;

  }

}

/*void pos(const void *const p){
  printf("%ld ",(char*)p-recvbuf);
}*/

void catch(V32 *const m,const unsigned v){
  assert(!(m->caught));
  m->caught=true;
  m->v=v;
}

/*void bytes(const void *const p,const int n){
  printf("[ ");
  for(int i=0;i<n;++i)
    printf("0x%02X ",*((unsigned char*)p+i));
  printf("] ");
}*/

void mac_colon(const void *const p,char *const s){
  const unsigned char *const h=p;
  sprintf(s,"%02x:%02x:%02x:%02x:%02x:%02x",h[0],h[1],h[2],h[3],h[4],h[5]);
  // int l=0;
  // for(int i=0;i<6;++i)
  //   l+=sprintf(s+l,"%02X%s",h[i],i<5?":":"");
  // // l+=sprintf(s,"%02x",*((unsigned char*)p));
  // // for(int i=1;i<6;++i)
  // //   l+=sprintf(s+l,":%02x",*((unsigned char*)p+i));
  // assert(l==MAC_L);
}

void print_link(){

  // REM_GETLINK only
  typedef struct {
    struct nlmsghdr nh;
    struct ifinfomsg ifi;
  } Req;

  assert(NLMSG_LENGTH(sizeof(struct ifinfomsg))==sizeof(Req));
  assert(sizeof(Req)==send(fd,&(Req){
    .nh={
      .nlmsg_len=NLMSG_LENGTH(sizeof(struct ifinfomsg)),
      .nlmsg_type=RTM_GETLINK,
      .nlmsg_flags=NLM_F_REQUEST|NLM_F_ROOT,
      .nlmsg_seq=0,
      .nlmsg_pid=0
    },
    .ifi={
      .ifi_family=AF_UNSPEC,
      // .ifi_family=AF_INET,
      .ifi_type=0,
      .ifi_index=0,
      .ifi_flags=0,
      .ifi_change=0xFFFFFFFF,
    }
  },sizeof(Req),0));

  receive();

  printf("\n");
  struct nlmsghdr *nh=(struct nlmsghdr*)recvbuf;
  for(;NLMSG_OK(nh,len);nh=NLMSG_NEXT(nh,len)){

    if(nh->nlmsg_type==NLMSG_DONE)
      break;

    // Part 1 nlmsghdr
    assert(
      // nh->nlmsg_len
      nh->nlmsg_type==RTM_NEWLINK &&
      nh->nlmsg_flags==NLM_F_MULTI &&
      nh->nlmsg_seq==0 &&
      nh->nlmsg_pid==(unsigned)getpid()
    );

    // Part 2 ifinfomsg
    struct ifinfomsg *const ifm=(struct ifinfomsg*)NLMSG_DATA(nh);
    assert(
      ifm->ifi_family==AF_UNSPEC &&
      // ifi_type
      // ifi_index
      // ifi_flags
      ifm->ifi_change==0
    );
    printf("#%d ",ifm->ifi_index);
    switch(ifm->ifi_type){
      // /usr/include/net/if_arp.h
      case ARPHRD_LOOPBACK:printf("loopback ");break;
      case ARPHRD_ETHER:printf("ethernet ");break;
      case ARPHRD_NONE:printf("noheader ");break;
      default:assert(false);break;
    }
    #ifdef _NET_IF_H
    #pragma GCC error "include <linux/if.h> instead of <net/if.h>"
    #endif
    // /usr/include/linux/if.h
    steal_flag_32(&(ifm->ifi_flags),IFF_UP,"up");
    steal_flag_32(&(ifm->ifi_flags),IFF_BROADCAST,"broadcast");
    steal_flag_32(&(ifm->ifi_flags),IFF_LOOPBACK,"lo");
    steal_flag_32(&(ifm->ifi_flags),IFF_POINTOPOINT,"p2p");
    steal_flag_32(&(ifm->ifi_flags),IFF_RUNNING,"running");
    steal_flag_32(&(ifm->ifi_flags),IFF_NOARP,"noarp");
    steal_flag_32(&(ifm->ifi_flags),IFF_MULTICAST,"multicast");
    steal_flag_32(&(ifm->ifi_flags),IFF_LOWER_UP,"l1up");
    assert(ifm->ifi_flags==0);
    printf("\n");

    // Part 3 rtattr
    const struct rtattr *rta=IFLA_RTA(ifm); // /usr/include/linux/if_link.h
    assert(rta==(struct rtattr*)((char*)nh+NLMSG_LENGTH(sizeof(struct ifinfomsg))));
    int rtl=RTM_PAYLOAD(nh);

    V32 cur_mtu={};
    V32 min_mtu={};
    V32 max_mtu={};
    V32 carrier_ch={};
    V32 carrier_up={};
    V32 carrier_dn={};

    char hwaddr[MAC_L+1]={};
    char bcast[MAC_L+1]={};
    char perm[MAC_L+1]={};

    for(;RTA_OK(rta,rtl);rta=RTA_NEXT(rta,rtl)){
      switch(rta->rta_type){

        // /usr/include/linux/if_link.h
        // https://elixir.bootlin.com/linux/v5.8.5/C/ident/IFLA_TXQLEN
        // ip -d l

        case IFLA_IFNAME:printf("%s ",(const char*)RTA_DATA(rta));break;

        // https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
        // /sys/class/net/<iface>/operstate (RFC2863)
        // https://www.kernel.org/doc/Documentation/networking/operstates.txt
        case IFLA_OPERSTATE:switch(*(uint8_t*)RTA_DATA(rta)){
          case IF_OPER_UNKNOWN:printf("unk ");break;
          case IF_OPER_DOWN:printf("down ");break;
          case IF_OPER_UP:printf("up ");break;
          default:assert(false);
        }break;
        case IFLA_LINKMODE:switch(*(unsigned char*)RTA_DATA(rta)){
          case IF_LINK_MODE_DEFAULT:printf("default ");break;
          case IF_LINK_MODE_DORMANT:printf("dormant ");break;
          default:assert(false);break;
        }break;
        // https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
        case IFLA_CARRIER:switch(*(unsigned char*)RTA_DATA(rta)){
          case 0:printf("physical down ");break;
          case 1:printf("physical up ");break;
          default:assert(false);break;
        }
        break;

        // /sys/class/net/<iface>/carrier_changes
        case IFLA_CARRIER_CHANGES:   catch(&carrier_ch,*(unsigned*)RTA_DATA(rta));break;
        case IFLA_CARRIER_UP_COUNT:  catch(&carrier_up,*(unsigned*)RTA_DATA(rta));break;
        case IFLA_CARRIER_DOWN_COUNT:catch(&carrier_dn,*(unsigned*)RTA_DATA(rta));break;
        case IFLA_MTU:    catch(&cur_mtu,*(unsigned*)RTA_DATA(rta));break;
        case IFLA_MIN_MTU:catch(&min_mtu,*(unsigned*)RTA_DATA(rta));break;
        case IFLA_MAX_MTU:catch(&max_mtu,*(unsigned*)RTA_DATA(rta));break;

        // 1-byte
        case IFLA_PROTO_DOWN:assert(*(unsigned char*)RTA_DATA(rta)==0);break;
        // 4-byte
        case IFLA_GROUP:
        case IFLA_PROMISCUITY:assert(0==*(unsigned*)RTA_DATA(rta));break;

        case IFLA_NUM_TX_QUEUES:
        case IFLA_NUM_RX_QUEUES:assert(1==*(unsigned*)RTA_DATA(rta));break;
        // IFF_MULTI_QUEUE
        // case IFLA_NUM_TX_QUEUES:
        // case IFLA_NUM_RX_QUEUES:printf("[%u] ",*(unsigned*)RTA_DATA(rta));break;

        case IFLA_GSO_MAX_SEGS:assert(65535==*(unsigned*)RTA_DATA(rta));break;
        case IFLA_GSO_MAX_SIZE:assert(65536==*(unsigned*)RTA_DATA(rta));break;

        case IFLA_TXQLEN:printf("txq %u ",*(unsigned*)RTA_DATA(rta));break;
        case IFLA_QDISC:printf("%s ",(char*)RTA_DATA(rta));break;

        case IFLA_ADDRESS:     mac_colon(RTA_DATA(rta),hwaddr);break;
        case IFLA_BROADCAST:   mac_colon(RTA_DATA(rta),bcast);break;
        case IFLA_PERM_ADDRESS:mac_colon(RTA_DATA(rta),perm);break;

        // /usr/include/linux/if_link.h
        // struct rtnl_link_stats
        // struct rtnl_link_stats64 
        case IFLA_STATS64:printf("(stats64_%zu) ",RTA_PAYLOAD(rta));break;
        case IFLA_STATS:printf("(stats_%zu) ",RTA_PAYLOAD(rta));break;
        case IFLA_XDP:printf("(xdp_%zu) ",RTA_PAYLOAD(rta));break; // https://en.wikipedia.org/wiki/Express_Data_Path
        case IFLA_LINKINFO:printf("(linkinfo_%zu) ",RTA_PAYLOAD(rta));break;
        case IFLA_AF_SPEC:printf("(afspec_%zu) ",RTA_PAYLOAD(rta));break;
        case IFLA_MAP:printf("(map_%zu) ",RTA_PAYLOAD(rta));break;

        // case IFLA_XXX:bytes(RTA_DATA(rta),RTA_PAYLOAD(rta));break;

        // default:printf("#%u# ",rta->rta_type);break;
        default:assert(false);break;

      }
    }
    printf("\n");

    bool addr=false;
    if(strlen(perm)){
      assert(strlen(perm)==MAC_L);
      printf("perm %s ",perm);
      addr=true;
    }
    if(strlen(hwaddr)){
      assert(strlen(hwaddr)==MAC_L);
      assert(strlen(bcast)==MAC_L);
      printf("hwaddr %s ",hwaddr);
      printf("bcast %s ",bcast);
      addr=true;
    }
    addr?printf("\n"):0;

    assert(
      cur_mtu.caught &&
      min_mtu.caught &&
      max_mtu.caught
    );
    printf("mtu %u < %u < %u \n",
      min_mtu.v,
      cur_mtu.v,
      max_mtu.v
    );

    assert(
      carrier_up.caught &&
      carrier_dn.caught &&
      carrier_ch.caught
    );
    printf("carrier change up %u down %u total %u \n",
      carrier_up.v,
      carrier_dn.v,
      carrier_ch.v
    );

    // Finish
    printf("\n");

  }

  clearbuf();

}

// RTM_NEWLINK/RTM_DELLINK only
typedef struct {
  struct nlmsghdr nh;
  struct ifinfomsg ifi;
  char attrbuf[SZ];
} Req_chlink;

// Fail
/*void tun_create(const char *const dev){

  assert(0==getuid());
  assert(dev);

  Req_chlink req={
    .nh={
      .nlmsg_len=NLMSG_LENGTH(sizeof(struct ifinfomsg)),
      .nlmsg_type=RTM_NEWLINK,
      .nlmsg_flags=NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL|NLM_F_CREATE,
      .nlmsg_seq=0,
      .nlmsg_pid=getpid()
    },
    .ifi={
      .ifi_family=AF_UNSPEC,
      .ifi_type=ARPHRD_NONE,
      .ifi_index=0, // ?
      .ifi_flags=IFF_POINTOPOINT|IFF_NOARP|IFF_MULTICAST,
      .ifi_change=0xFFFFFFFF,
    },
    .attrbuf={}
  };

  attr(&req.nh,sizeof(Req_chlink),IFLA_IFNAME,dev);
  // attr(&req.nh,sizeof(Req_chlink),IFLA_LINKMODE,IF_LINK_MODE_DEFAULT);

  assert(sizeof(Req_chlink)==send(fd,&req,sizeof(Req_chlink),0));
  receive();
  ack();
  clearbuf();

}*/

void tun_del(const char *const dev){

  assert(0==getuid());
  assert(dev);

  Req_chlink req={
    .nh={
      .nlmsg_len=NLMSG_LENGTH(sizeof(struct ifinfomsg)),
      .nlmsg_type=RTM_DELLINK,
      .nlmsg_flags=NLM_F_REQUEST|NLM_F_ACK,
      .nlmsg_seq=0,
      .nlmsg_pid=getpid()
    },
    .ifi={
      .ifi_family=AF_UNSPEC,
      .ifi_type=0,
      .ifi_index=0,
      .ifi_flags=0,
      .ifi_change=0xFFFFFFFF,
    },
    .attrbuf={}
  };

  attr(&req.nh,sizeof(Req_chlink),IFLA_IFNAME,dev);

  assert(sizeof(Req_chlink)==send(fd,&req,sizeof(Req_chlink),0));
  receive();
  ack();
  clearbuf();

}

void cidr_mask(unsigned prefixlen,struct in_addr *sin_addr_p){
  assert(prefixlen<=32);
  unsigned host=0;
  for(;prefixlen>0;--prefixlen)
    host|=((1<<(32U-prefixlen))); // ((1<<31)>>(prefixlen-1));
  sin_addr_p->s_addr=htonl(host);
}

void print_addr(){

  typedef struct {
    struct nlmsghdr nh;
    struct ifaddrmsg ifa;
  } Req;

  assert(NLMSG_LENGTH(sizeof(struct ifaddrmsg))==sizeof(Req));
  assert(sizeof(Req)==send(fd,&(Req){
    .nh={
      .nlmsg_len=NLMSG_LENGTH(sizeof(struct ifaddrmsg)),
      .nlmsg_type=RTM_GETADDR,
      .nlmsg_flags=NLM_F_REQUEST|NLM_F_ROOT,
      .nlmsg_seq=0,
      .nlmsg_pid=getpid()
    },
    .ifa={
      .ifa_family=AF_UNSPEC, // AF_INET/AF_INET6
      .ifa_prefixlen=0,
      .ifa_flags=0,
      .ifa_scope=0,
      .ifa_index=0
    }
  },sizeof(Req),0));

  receive();

  printf("\n");
  struct nlmsghdr *nh=(struct nlmsghdr*)recvbuf;
  for(;NLMSG_OK(nh,len);nh=NLMSG_NEXT(nh,len)){

    if(nh->nlmsg_type==NLMSG_DONE)
      break;

    assert(
      nh->nlmsg_type==RTM_NEWADDR &&
      nh->nlmsg_flags==NLM_F_MULTI &&
      nh->nlmsg_seq==0 &&
      nh->nlmsg_pid==(unsigned)getpid()
    );

    // ID
    struct ifaddrmsg *const ifa=(struct ifaddrmsg*)NLMSG_DATA(nh);
    printf("#%u ",ifa->ifa_index);

    // Load table
    void* bt[IFA_MAX+1]={};
    struct rtattr *rta=(struct rtattr*)IFA_RTA(ifa);
    int rtl=IFA_PAYLOAD(nh);
    for(;RTA_OK(rta,rtl);rta=RTA_NEXT(rta,rtl)){
      assert(
        rta->rta_type==IFA_LABEL ||
        rta->rta_type==IFA_ADDRESS ||
        rta->rta_type==IFA_LOCAL ||
        rta->rta_type==IFA_BROADCAST ||
        rta->rta_type==IFA_FLAGS ||
        rta->rta_type==IFA_CACHEINFO
      );
      (rta->rta_type!=IFA_CACHEINFO)?bt[rta->rta_type]=RTA_DATA(rta):0;
    }

    // Name
    (bt[IFA_LABEL])?printf("%s ",(char*)bt[IFA_LABEL]):0;

    // Scope
    switch(ifa->ifa_scope){
      case RT_SCOPE_UNIVERSE:printf("universe ");break;
      case RT_SCOPE_LINK:printf("link ");break;
      case RT_SCOPE_HOST:printf("host ");break;
      default:assert(false);break;
    }

    // Flags in ifaddrmsg
    steal_flag_8(&(ifa->ifa_flags),IFA_F_PERMANENT,"perm");
    assert(ifa->ifa_flags==0);

    printf("| ");

    // Flags in rtattr
    ((*(unsigned*)(bt[IFA_FLAGS]))&IFA_F_NOPREFIXROUTE)?assert(ifa->ifa_scope==RT_SCOPE_UNIVERSE):0;
    steal_flag_32((unsigned*)(bt[IFA_FLAGS]),IFA_F_PERMANENT,"perm");
    steal_flag_32((unsigned*)(bt[IFA_FLAGS]),IFA_F_NOPREFIXROUTE,"noprefixroute");
    // printf("%zu ",RTA_PAYLOAD(rta));
    // printf("0x%X ",*(unsigned*)(bt[IFA_FLAGS]));fflush(stdout);
    assert(0==*(unsigned*)(bt[IFA_FLAGS]));

    // const struct ifa_cacheinfo *const ifci=bt[IFA_CACHEINFO];
    // printf("(%u %u %u %u) ",
    //   ifci->ifa_prefered,
    //   ifci->ifa_valid,
    //   ifci->cstamp,
    //   ifci->tstamp
    // );
    printf("\n");

    // Address
    assert(ifa->ifa_family==AF_INET||ifa->ifa_family==AF_INET6);
    if(!bt[IFA_ADDRESS]){
      assert((!bt[IFA_LOCAL])&&(!bt[IFA_BROADCAST]));
    }else{
      char s[INET6_ADDRSTRLEN]={};
      assert(s==inet_ntop(ifa->ifa_family,bt[IFA_ADDRESS],s,INET6_ADDRSTRLEN));
      printf("%s/%u ",s,ifa->ifa_prefixlen);
      bzero(s,INET6_ADDRSTRLEN);
      if(bt[IFA_BROADCAST]){
        assert(s==inet_ntop(ifa->ifa_family,bt[IFA_BROADCAST],s,INET6_ADDRSTRLEN));
        printf("bcast %s ",s);
        bzero(s,INET6_ADDRSTRLEN);
      }
      (bt[IFA_LOCAL])?(printf("local "),assert(0==memcmp(bt[IFA_ADDRESS],bt[IFA_LOCAL],sizeof(struct in_addr)))):0;
      printf("\n");
    }

    printf("\n");


  }

  clearbuf();

}

void tun_addr(const char *const dev,const char *const ipv4,unsigned n){
  
}

void set(){

  // tun_up();
  // tun_flush();
  tun_addr(TUN,"10.0.0.1",24);

  // get_gateway(gw);
  // del_gateway(gw);
  // add_gateway("10.0.0.2");

  // server=json_load_server();
  // assert(server);
  // printf("%s\n",server);
  // add_route(server,gw);

}

void reset(){

  // del_route(server,gw);
  // free(server);
  // server=NULL;

  // del_gateway("10.0.0.2");
  // add_gateway(gw);

  // tun_flush();
  // tun_down();
  tun_del(TUN);

}

int main(){

  init();
  print_addr();
  // print_link();
  // print_route();

  // set();
  // print_link();
  // print_route();

  // external();
  // reset();
  // print_link();
  // print_route();

  end();

}


