
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

static inline int local_port(){
  buf_http_get("http://127.0.0.1:6170/configs");
  json_tokener_reset(tok);
  json_object *jobj=json_tokener_parse(buf);
  assert(jobj);
  assert(json_type_object==json_object_get_type(jobj));
  const int port=get_field_int(jobj,"socks-port");
  assert(port==1080);
  assert(1==json_object_put(jobj));
  jobj=NULL;
  buf_drop();
  return port;
}

static inline void assert_key_assert_val(const char *const k,const char *const v){
  assert_token_type(YAML_KEY_TOKEN);
  assert(0==scalarcmp(k));
  assert_token_type(YAML_VALUE_TOKEN);
  assert(0==scalarcmp(v));
}

static inline void yaml2json_resolv(const char *const yaml_key,const char *const json_key){
  assert_token_type(YAML_KEY_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  assert(0==strcmp((const char*)(token.data.scalar.value),yaml_key));
  DEL();
  assert_token_type(YAML_VALUE_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  char *ip=resolv((const char*)(token.data.scalar.value));
  assert(ip);
  printf("IP=\"%s\"\n",ip);
  assert(0==json_object_object_add(
    root,
    json_key,
    json_object_new_string(ip)
  ));
  DEL();
  free(ip);
  ip=NULL;
}

static inline void yaml2json(const char *const yaml_key,const char *const json_key){
  assert_token_type(YAML_KEY_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  assert(0==strcmp((const char*)(token.data.scalar.value),yaml_key));
  DEL();
  assert_token_type(YAML_VALUE_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  assert(0==json_object_object_add(
    root,
    json_key,
    json_object_new_string((const char*)(token.data.scalar.value))
  ));
  DEL();
}

static inline void dump(){
  for(;;){
    SCAN();
    type();
    assert(token.type!=YAML_NO_TOKEN);
    DEL();
  }
}

// Get from profile global "profile" instead
char *json_load_server(){

  json_object *j=json_object_from_file(SS_LOCAL_JSON);
  // assert((SS_LOCAL_JSON" not found",j));
  assert(j);
  assert(json_type_object==json_object_get_type(j));

  json_object *j2=json_object_object_get(j,"server");
  assert(j2);
  assert(json_type_string==json_object_get_type(j2));
  char *s=strdup(json_object_get_string(j2));

  assert(1==json_object_put(j));
  return s;

}
