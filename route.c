#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>

#define SZ 8192
// #define SZ 16384

int fd=-1;

char buf[SZ]={};
struct nlmsghdr *nh=(struct nlmsghdr*)buf;
int len=0;

void init(){
  // Open
  fd=socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
  assert(fd==3);
  // Bind
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
  bzero(buf,SZ);
  nh=(struct nlmsghdr*)buf;
  len=0;
}

void receive(){
  for(char *p=buf;;){
    const int seg=recv(fd,p,sizeof(buf)-len,0);
    if(((struct nlmsghdr*)p)->nlmsg_type == NLMSG_DONE)
      break;
    p += seg;
    len += seg;
    // printf("len=%d\n",len);
  }
}

// [0xF] dst 8.8.8.8 via 192.168.1.1 dev 3
// void add(){

//   typedef struct {
//     struct nlmsghdr nh;
//     struct rtmsg    rt;
//   } Req;
//   assert(NLMSG_LENGTH(sizeof(struct rtmsg))==sizeof(Req));

//   assert(sizeof(Req)==send(fd,&(Req){
//     .nh={
//       .nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)), // netlink(3)
//       .nlmsg_type = RTM_NEWROUTE, // rtnetlink(7)
//       .nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE,
//       .nlmsg_seq=0,
//       .nlmsg_pid=0
//     },
//     .rt={
//       .rtm_family=AF_INET,
//       .rtm_dst_len=0,
//       .rtm_src_len=0,
//       .rtm_tos=0,
//       .rtm_table=RT_TABLE_MAIN,
//       .rtm_protocol=RTPROT_UNSPEC,
//       .rtm_scope=RT_SCOPE_UNIVERSE,
//       .rtm_type=RTN_UNSPEC,
//       .rtm_flags=0
//     }
//   },sizeof(Req),0));
// }

void show(){

  // rtnetlink(3)
  typedef struct {
    struct nlmsghdr nh;
    struct rtmsg    rt;
  } Req;
  assert(NLMSG_LENGTH(sizeof(struct rtmsg))==sizeof(Req));

  assert(sizeof(Req)==send(fd,&(Req){
    .nh={
      .nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)), // netlink(3)
      .nlmsg_type = RTM_GETROUTE, // rtnetlink(7)
      .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP, /*NLM_F_ACK*/ /*NLM_F_ECHO*/
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
  },sizeof(Req),0));

  // printf("%d %zu %zu\n",sent,NLMSG_LENGTH(sizeof(struct rtmsg)),sizeof(struct nlmsghdr)+sizeof(struct rtmsg));

  receive();
  // printf("\n");

  // Print
  // netlink(7)
  for(;NLMSG_OK(nh, len);nh=NLMSG_NEXT(nh, len)){

    // printf("len=%d\n",len);

    assert(nh->nlmsg_type==RTM_NEWROUTE);
    assert(nh->nlmsg_pid==(unsigned)getpid());
    assert(nh->nlmsg_flags==NLM_F_MULTI);

    const struct rtmsg *payload=(struct rtmsg*)NLMSG_DATA(nh);
    if(payload->rtm_table!=RT_TABLE_MAIN){
      assert(payload->rtm_table==RT_TABLE_LOCAL);
      continue;
    }
    assert(payload->rtm_family==AF_INET);
    printf("/%u ",payload->rtm_dst_len);
    assert(payload->rtm_src_len==0);
    assert(payload->rtm_tos==0);
    // printf("%u ",payload->rtm_protocol);
    if(payload->rtm_protocol==RTPROT_DHCP)
      printf("dhcp ");
    else if(payload->rtm_protocol==RTPROT_BOOT)
      printf("boot ");
    if(payload->rtm_scope==RT_SCOPE_LINK)
      printf("link ");
    else if(payload->rtm_scope==RT_SCOPE_UNIVERSE)
      printf("universe ");
    else
      assert(false);
    assert(payload->rtm_type==RTN_UNICAST);
    assert(payload->rtm_flags==0);

    printf("||| ");

    struct rtattr *p = (struct rtattr *)RTM_RTA(payload);
    int rtl = RTM_PAYLOAD(nh);

    for(;RTA_OK(p, rtl);p=RTA_NEXT(p,rtl)){
      char s[INET_ADDRSTRLEN]={};
      switch(p->rta_type){
        case RTA_DST:
          if(((struct sockaddr_in*)RTA_DATA(p))->sin_addr.s_addr==INADDR_ANY){
            printf("default ");
          }else{
            assert(s==inet_ntop(AF_INET,RTA_DATA(p),s,INET_ADDRSTRLEN));
            printf("dst %s ",s);
          }
          break;
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
          break;
        default:
          printf("[type %u] ",p->rta_type);
          break;
      }
    }

    printf("\n");

  }

}

int main(){

  init();

  clearbuf();
  show();

  // clearbuf();
  // show();

  // clearbuf();
  // show();

  end();
}


