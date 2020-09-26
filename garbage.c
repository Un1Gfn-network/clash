
// sendmsg()
assert(NLMSG_LENGTH(sizeof(struct rtmsg))==sendmsg(fd,&(struct msghdr){
  .msg_name = &(struct sockaddr_nl){.nl_family = AF_NETLINK},
  .msg_namelen = sizeof(struct sockaddr_nl),
  .msg_iov = &(struct iovec){
    .iov_base = &(Req){
      .nh={
        .nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)), // netlink(3)
        .nlmsg_type = RTM_GETROUTE, // rtnetlink(7)
        .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP, /*NLM_F_ACK*/ NLM_F_ECHO
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
    },
    .iov_len = sizeof(Req)
  },
  .msg_iovlen = 1
},0));


eprintf("\n");
eprintf("  %s <rixcloud|ssrcloud>\n",argv[0]);
eprintf("\n");
exit(1);
