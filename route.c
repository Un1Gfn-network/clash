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

// #include <net/if.h>
#include <linux/if.h>

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

// For RTM_NEWROUTE and RTM_DELROUTE
typedef struct {
  struct nlmsghdr nh;
  struct rtmsg rt;
  char attrbuf[SZ]; // rtnetlink(3)
} Req;

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
    len += seglen;
    // printf("0x%X\n",((struct nlmsghdr*)p)->nlmsg_type);
    if(((struct nlmsghdr*)p)->nlmsg_type==NLMSG_DONE||((struct nlmsghdr*)p)->nlmsg_type==NLMSG_ERROR)
      break;
    p += seglen;
  }
}

void poll(){
  struct nlmsghdr *nh=(struct nlmsghdr*)recvbuf;
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
}

void ack(){
  assert(((struct nlmsghdr*)recvbuf)->nlmsg_type==NLMSG_ERROR);
  assert(((struct nlmsgerr*)NLMSG_DATA((struct nlmsghdr*)recvbuf))->error==0);
}

void attr(struct nlmsghdr *np,struct rtattr *rp,int type,const void *data){
  rp->rta_type=type;
  int l=0;
  if(type==RTA_DST||type==RTA_GATEWAY){
    l=sizeof(struct in_addr);
    rp->rta_len=RTA_LENGTH(l);
    bzero(RTA_DATA(rp),l);
    assert(1==inet_pton(AF_INET,data,RTA_DATA(rp)));
  }
  else if(type==RTA_OIF){
    l=sizeof(int);
    rp->rta_len=RTA_LENGTH(l);
    *((int*)RTA_DATA(rp))=*((int*)data);
  }else{
    assert(false);
  }
  np->nlmsg_len=NLMSG_ALIGN(np->nlmsg_len)+RTA_LENGTH(l);
}

#define add_route(dst,via) route(true,false,dst,via)
#define del_route(dst,via) route(false,false,dst,via)
#define add_gateway(via) route(true,true,NULL,via)
#define del_gateway(via) route(false,true,NULL,via)
void route(bool add,bool gw,const char *dst,const char *via){

  assert(0==getuid());
  if(gw)
    assert(!dst);

  Req req={
    .nh={
      .nlmsg_len=NLMSG_LENGTH(sizeof(struct rtmsg)),
      .nlmsg_type= add ? RTM_NEWROUTE : RTM_DELROUTE ,
      .nlmsg_flags= add ? (NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL|NLM_F_CREATE) : (NLM_F_REQUEST|NLM_F_ACK) ,
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

  struct rtattr *rta=RTM_RTA(NLMSG_DATA(&req.nh));
  assert(rta==(struct rtattr *)((char*)&req.nh+NLMSG_LENGTH(sizeof(struct rtmsg))));
  int len=sizeof(Req)-NLMSG_LENGTH(sizeof(struct rtmsg));
  if(!gw){
    attr(&req.nh,rta,RTA_DST,dst);
    rta=RTA_NEXT(rta,len);
  }
  attr(&req.nh,rta,RTA_GATEWAY,via);
  rta=RTA_NEXT(rta,len);
  attr(&req.nh,rta,RTA_OIF,&((int){3}));

  assert(sizeof(Req)==send(fd,&req,sizeof(Req),0));
  receive();
  ack();
  clearbuf();

}

void ask_route(){

  // For RTM_GETROUTE only
  typedef struct {
    struct nlmsghdr nh;
    struct rtmsg rt;
  } Req_getroute;

  // rtnetlink(3)
  assert(NLMSG_LENGTH(sizeof(struct rtmsg))==sizeof(Req_getroute));
  assert(sizeof(Req_getroute)==send(fd,&(Req_getroute){
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
      .rtm_table=RT_TABLE_MAIN,
      .rtm_protocol=RTPROT_UNSPEC,
      .rtm_scope=RT_SCOPE_UNIVERSE,
      .rtm_type=RTN_UNSPEC,
      .rtm_flags=0
    }
  },sizeof(Req_getroute),0));
}

void steal_flag(unsigned *flags,const unsigned f,const char *const s){
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

    const struct rtmsg *rtm=(struct rtmsg*)NLMSG_DATA(nh);
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
    if(rtm->rtm_protocol==RTPROT_DHCP)
      printf("dhcp ");
    else if(rtm->rtm_protocol==RTPROT_BOOT)
      printf("boot ");
    else
      assert(false);
    if(rtm->rtm_scope==RT_SCOPE_LINK)
      printf("link ");
    else if(rtm->rtm_scope==RT_SCOPE_UNIVERSE)
      printf("universe ");
    else
      assert(false);
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

    const struct rtmsg *rtm=(struct rtmsg*)NLMSG_DATA(nh);
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

void set(){

  // tun_create();
  // tun_up();
  // tun_flush();
  // tun_addr("10.0.0.1");

  get_gateway(gw);
  assert(0==strcmp(gw,"192.168.1.1"));
  del_gateway(gw);
  // add_gateway("10.0.0.2");

  server=json_load_server();
  assert(server);
  // printf("%s\n",server);
  add_route(server,gw);

}

void reset(){

  del_route(server,gw);
  free(server);
  server=NULL;

  // del_gateway("10.0.0.2");
  add_gateway(gw);
  bzero(gw,INET_ADDRSTRLEN);

  // tun_flush();
  // tun_down();
  // tun_del();

}

void pos(const void *const p){
  printf("%ld ",(char*)p-recvbuf);
}

typedef struct {
  bool caught;
  unsigned v;
} V32;

void catch(V32 *m,unsigned v){
  assert(!(m->caught));
  m->caught=true;
  m->v=v;
}

void bytes(const void *const p,const int n){
  printf("[ ");
  for(int i=0;i<n;++i)
    printf("0x%02X ",*((unsigned char*)p+i));
  printf("] ");
}

void print_link(){

  typedef struct {
    struct nlmsghdr nh;
    struct ifinfomsg ifi;
  } Req_getlink;

  assert(NLMSG_LENGTH(sizeof(struct ifinfomsg))==sizeof(Req_getlink));
  assert(sizeof(Req_getlink)==send(fd,&(Req_getlink){
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
      .ifi_change=0,
    }
  },sizeof(Req_getlink),0));

  receive();

  // poll();

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
      nh->nlmsg_pid==(unsigned)getpid()
    );

    // Part 2 ifinfomsg
    struct ifinfomsg *ifm=(struct ifinfomsg*)NLMSG_DATA(nh);
    assert(
      ifm->ifi_family==AF_UNSPEC &&
      ifm->ifi_change==0
    );
    printf("#%d ",ifm->ifi_index);
    switch(ifm->ifi_type){
      // /usr/include/net/if_arp.h
      case ARPHRD_LOOPBACK:printf("loopback ");break;
      case ARPHRD_ETHER:printf("Ethernet 10/100Mbps ");break;
      case ARPHRD_NONE:printf("noheader ");break;
      default:assert(false);break;
    }
    #ifdef _NET_IF_H
    #pragma GCC error "include <linux/if.h> instead of <net/if.h>"
    #endif
    // /usr/include/linux/if.h
    steal_flag(&(ifm->ifi_flags),IFF_UP,"up");
    steal_flag(&(ifm->ifi_flags),IFF_BROADCAST,"broadcast");
    steal_flag(&(ifm->ifi_flags),IFF_LOOPBACK,"lo");
    steal_flag(&(ifm->ifi_flags),IFF_POINTOPOINT,"p2p");
    steal_flag(&(ifm->ifi_flags),IFF_RUNNING,"running");
    steal_flag(&(ifm->ifi_flags),IFF_NOARP,"noarp");
    steal_flag(&(ifm->ifi_flags),IFF_MULTICAST,"multicast");
    steal_flag(&(ifm->ifi_flags),IFF_LOWER_UP,"l1up");
    assert(ifm->ifi_flags==0);
    printf("\n");
    // printf("||| ");

    // Part 3 rtattr
    struct rtattr *rta=IFLA_RTA(ifm); // /usr/include/linux/if_link.h
    assert(rta==(struct rtattr*)((char*)nh+NLMSG_LENGTH(sizeof(struct ifinfomsg))));
    int rtl=RTM_PAYLOAD(nh);

    V32 cur_mtu={};
    V32 min_mtu={};
    V32 max_mtu={};
    V32 carrier_up={};
    V32 carrier_dn={};

    for(;RTA_OK(rta,rtl);rta=RTA_NEXT(rta,rtl)){
      switch(rta->rta_type){

        // /usr/include/linux/if_link.h
        // https://elixir.bootlin.com/linux/v5.8.5/C/ident/IFLA_TXQLEN
        // ip -d l

        case IFLA_IFNAME:printf("%s ",(const char*)RTA_DATA(rta));break;
        case IFLA_TXQLEN:printf("txq %u ",*(unsigned*)RTA_DATA(rta));break;
        case IFLA_OPERSTATE:switch(*(uint8_t*)RTA_DATA(rta)){
          case IF_OPER_UNKNOWN:printf("unk ");break;
          case IF_OPER_DOWN:printf("down ");break;
          case IF_OPER_UP:printf("up ");break;
          default:assert(false);
        }break;

        /* https://www.kernel.org/doc/Documentation/networking/operstates.txt
        rta_len=5 little-endian
        ---------------------
        rta         rta+4
        |           |
        00 00 00 00 08
        01
        ---------------------*/
        case IFLA_LINKMODE:
          assert(1==RTA_PAYLOAD(rta));
          switch(*(unsigned char*)RTA_DATA(rta)){
            case 0x01:printf("M ");break;
            case 0x00:printf("m ");break;
            default:assert(false);break;
          }
          break;

        case IFLA_MTU:    catch(&cur_mtu,*(unsigned*)RTA_DATA(rta));break;
        case IFLA_MIN_MTU:catch(&min_mtu,*(unsigned*)RTA_DATA(rta));break;
        case IFLA_MAX_MTU:catch(&max_mtu,*(unsigned*)RTA_DATA(rta));break;

        case IFLA_GROUP:
        case IFLA_PROMISCUITY:
          assert(0==*(unsigned*)RTA_DATA(rta));break;
        case IFLA_NUM_TX_QUEUES:assert(1==*(unsigned*)RTA_DATA(rta));break;
        case IFLA_GSO_MAX_SEGS:assert(65535==*(unsigned*)RTA_DATA(rta));break;
        case IFLA_GSO_MAX_SIZE:assert(65536==*(unsigned*)RTA_DATA(rta));break;
        case IFLA_NUM_RX_QUEUES:assert(1==*(unsigned*)RTA_DATA(rta));break;

        case IFLA_CARRIER:
          assert(1==RTA_PAYLOAD(rta));
          switch(*(unsigned char*)RTA_DATA(rta)){
            case 0x01:printf("C ");break;
            case 0x00:printf("c ");break;
            default:assert(false);break;
          }
          break;

        case IFLA_QDISC:printf("qdisc %s ",(char*)RTA_DATA(rta));break;
        case IFLA_CARRIER_CHANGES:printf("[%lu]. ",RTA_PAYLOAD(rta));break;
        case IFLA_PROTO_DOWN:assert(*((char*)RTA_DATA(rta)+4)==0x08);assert(*(unsigned*)RTA_DATA(rta)==0);break;

        case IFLA_CARRIER_UP_COUNT:  catch(&carrier_up,*(unsigned*)RTA_DATA(rta));break;
        case IFLA_CARRIER_DOWN_COUNT:catch(&carrier_dn,*(unsigned*)RTA_DATA(rta));break;

        case IFLA_MAP:printf("[%lu]. ",RTA_PAYLOAD(rta));break;

        case IFLA_ADDRESS:
          printf("hwaddr ");
          for(int i=0;i<5;++i)
            printf("%02X:",*((unsigned char*)RTA_DATA(rta)+i));
          printf("%02X ",*((unsigned char*)RTA_DATA(rta)+5));
          break;

        case IFLA_BROADCAST:
          printf("bcast ");
          for(int i=0;i<5;++i)
            printf("%02X:",*((unsigned char*)RTA_DATA(rta)+i));
          printf("%02X ",*((unsigned char*)RTA_DATA(rta)+5));
          break;

        // /usr/include/linux/if_link.h
        // struct rtnl_link_stats
        // struct rtnl_link_stats64 
        case IFLA_STATS64:printf("stats64. ");break;
        case IFLA_STATS:printf("stats. ");break;

        case IFLA_XDP:printf("xdp. ");break; // https://en.wikipedia.org/wiki/Express_Data_Path

        case IFLA_PERM_ADDRESS:
          printf("perm ");
          for(int i=0;i<5;++i)
            printf("%02X:",*((unsigned char*)RTA_DATA(rta)+i));
          printf("%02X ",*((unsigned char*)RTA_DATA(rta)+5));
          break;

        case IFLA_AF_SPEC:printf("afspec. ");break;

        /*
        case IFLA_AF_SPEC:printf(". ");break;
        *
        case IFLA_AF_SPEC:printf("[%lu]. ",RTA_PAYLOAD(rta));break;
        case IFLA_AF_SPEC:bytes(RTA_DATA(rta),RTA_PAYLOAD(rta));break;
        *
        case IFLA_AF_SPEC:printf("[%u 0x%08X] ",*(unsigned*)RTA_DATA(rta),*(unsigned*)RTA_DATA(rta));break;
        case IFLA_AF_SPEC:printf("??? %u ",*(unsigned*)RTA_DATA(rta));break;
        case IFLA_AF_SPEC:printf("??? %s ",(char*)RTA_DATA(rta));break;
        */

        default:printf("%u ",rta->rta_type);break;

      }
    }

    assert(
      cur_mtu.caught &&
      min_mtu.caught &&
      max_mtu.caught
    );
    printf("mtu %u_(%u)_%u ",
      min_mtu.v,
      cur_mtu.v,
      max_mtu.v
    );

    assert(
      carrier_up.caught &&
      carrier_dn.caught
    );
    printf("carrier cnt up_%u down_%u ",
      carrier_up.v,
      carrier_dn.v
    );

    // Finish
    printf("\n\n");

  }

  clearbuf();

}

int main(){

  init();

  print_link();

  // print_route();
  // set();
  // print_route();
  // external();
  // reset();
  // print_route();

  end();

}


