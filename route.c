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

// rtnetlink(3)
typedef struct {
  struct nlmsghdr nh;
  struct rtmsg    rt;
} Req;

int main(){

  // Open
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  assert(fd==3);

  // Bind
  assert(0==bind(fd,(struct sockaddr*)(&(struct sockaddr_nl){
    .nl_family=AF_NETLINK,
    .nl_pad=0,
    .nl_pid=getpid(),
    .nl_groups=0
  }),sizeof(struct sockaddr_nl)));

  // Send
  Req req={
    .nh={
      .nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)), // netlink(3)
      .nlmsg_type = RTM_GETROUTE, // rtnetlink(7)
      .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP /*NLM_F_ACK*/ /*NLM_F_ECHO*/
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
  };

  assert(sizeof(Req)==req.nh.nlmsg_len);
  assert((void*)&req==(void*)&(req.nh));
  const int sent=sendmsg(fd,&(struct msghdr){
    .msg_name = &(struct sockaddr_nl){.nl_family = AF_NETLINK},
    .msg_namelen = sizeof(struct sockaddr_nl),
    .msg_iov = &(struct iovec){
      .iov_base = &req,
      .iov_len = sizeof(Req)
    },
    .msg_iovlen = 1
  },0);
  assert(sent==NLMSG_LENGTH(sizeof(struct rtmsg)));
  // printf("%d %zu %zu\n",sent,NLMSG_LENGTH(sizeof(struct rtmsg)),sizeof(struct nlmsghdr)+sizeof(struct rtmsg));

  // Receive
  char buf[SZ]={};
  int progress=0;
  for(char *p=buf;;){
    const int seg=recv(fd,p,sizeof(buf)-progress,0);
    // printf("received %d bytes\n",seg);
    if(((struct nlmsghdr*)p)->nlmsg_type == NLMSG_DONE)
      break;
    p += seg;
    progress += seg;
  }

  // Print
  // netlink(7)
  for(
    struct nlmsghdr *nh=(struct nlmsghdr*)buf;
    NLMSG_OK(nh, progress);
    nh=NLMSG_NEXT(nh, progress)
  ){

    assert(nh->nlmsg_type==RTM_NEWROUTE);
    assert(nh->nlmsg_pid==(unsigned)getpid());
    assert(nh->nlmsg_flags==NLM_F_MULTI);

    const struct rtmsg *entry=(struct rtmsg *)NLMSG_DATA(nh);
    if(entry->rtm_table != RT_TABLE_MAIN){
      assert(entry->rtm_table==RT_TABLE_LOCAL);
      continue;
    }

    struct rtattr *p = (struct rtattr *)RTM_RTA(entry);
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
        case RTA_GATEWAY:
          assert(s==inet_ntop(AF_INET,RTA_DATA(p),s,INET_ADDRSTRLEN));
          printf("via %s ",s);
          break;
        case RTA_OIF:
          printf("dev %d ",*((int*)RTA_DATA(p)));
          break;
        case RTA_PRIORITY:
          printf("pri %d ",*((int*)RTA_DATA(p)));
          break;
        default:
          printf("[0x%X] ",p->rta_type);
          break;
      }

    }

    printf("\n");

  }

    close(fd);
    fd=-1;

}


