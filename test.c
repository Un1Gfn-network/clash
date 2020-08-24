#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>

// struct nlmsghdr nl={.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg))};
struct nlmsghdr nl={};
struct rtmsg rt={};
char buf[8192]={};

// variables used for
// socket communications
int fd=-1;
struct sockaddr_nl la={};
struct sockaddr_nl pa={.nl_family = AF_NETLINK};
struct msghdr msg={};
struct iovec iov;
int rtn;

// buffer to hold the RTNETLINK reply(ies)
char buf[8192];

// RTNETLINK message pointers & lengths
// used when processing messages
struct nlmsghdr *nlp=NULL;
int nll=0;
struct rtmsg *rtp=NULL;
int rtl=0;
struct rtattr *rtap=NULL;

int main(){

  // Open
  fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  assert(fd==3);

  // Bind
  la=(struct sockaddr_nl){
    .nl_family = AF_NETLINK,
    .nl_pid = getpid()
  };
  bind(fd, (struct sockaddr*) &la, sizeof(la));

  // Send
  nl=(struct nlmsghdr){
    .nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)),
    .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
    .nlmsg_type = RTM_GETROUTE
  };
  rt=(struct rtmsg){
    .rtm_family = AF_INET,
    .rtm_table = RT_TABLE_MAIN
  };
  iov=(struct iovec){
    .iov_base = &nl,
    .iov_len = nl.nlmsg_len
  };
  msg=(struct msghdr){
    .msg_name = (void *) &pa,
    .msg_namelen = sizeof(pa),
    .msg_iov = &iov,
    .msg_iovlen = 1
  };
  assert(1<=sendmsg(fd, &msg, 0));

  // Receive
  char *p = buf;
  // read from the socket until the NLMSG_DONE is
  // returned in the type of the RTNETLINK message
  // or if it was a monitoring socket
  while(1) {
    rtn = recv(fd, p, sizeof(buf) - nll, 0);
    nlp = (struct nlmsghdr *) p;
    if(nlp->nlmsg_type == NLMSG_DONE)
      break;
    // increment the buffer pointer to place
    // next message
    p += rtn;
    // increment the total size by the size of
    // the last received message
    nll += rtn;
    if((la.nl_groups & RTMGRP_IPV4_ROUTE)== RTMGRP_IPV4_ROUTE)
      break;
  }

  // Print
  // outer loop: loops thru all the NETLINK
  // headers that also include the route entry
  // header
  nlp = (struct nlmsghdr *) buf;
  for(;NLMSG_OK(nlp, nll);nlp=NLMSG_NEXT(nlp, nll)){
    // get route entry header
    rtp = (struct rtmsg *) NLMSG_DATA(nlp);
    // we are only concerned about the
    // main route table
    if(rtp->rtm_table != RT_TABLE_MAIN)
      continue;
    // string to hold content of the route
    // table (i.e. one entry)
    char dsts[24]={}, gws[24]={}, ifs[16]={}, ms[24]={};
    // inner loop: loop thru all the attributes of
    // one route entry
    rtap = (struct rtattr *) RTM_RTA(rtp);
    rtl = RTM_PAYLOAD(nlp);
    for(;RTA_OK(rtap, rtl);rtap=RTA_NEXT(rtap,rtl)){
      switch(rtap->rta_type){
        // destination IPv4 address
        case RTA_DST:
          inet_ntop(AF_INET, RTA_DATA(rtap), dsts, 24);
          break;
        // next hop IPv4 address
        case RTA_GATEWAY:
          inet_ntop(AF_INET, RTA_DATA(rtap), gws, 24);
          break;
        // unique ID associated with the network
        // interface
        case RTA_OIF:
          sprintf(ifs, "%d", *((int *) RTA_DATA(rtap)));
          break;
        default:
          // assert(false);
          break;
      }
    }
    sprintf(ms, "%d", rtp->rtm_dst_len);
    printf("dst %s/%s gw %s if %s\n", dsts, ms, gws, ifs);
  }
  // close socket
  close(fd);

  fd=-1;

}




