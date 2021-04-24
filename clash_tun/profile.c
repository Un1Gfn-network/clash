#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // unlink

#include <json.h>
#include <shadowsocks.h>
#include <yaml.h>

#include <libclash.h>

#include "./def.h"
#include "./file.h"
#include "./profile.h"
#include "./shadowsocks.h"

#define eprintf(...) fprintf(stderr,__VA_ARGS__)
#define SCAN() assert(1==yaml_parser_scan(&parser,&token))
#define DEL() {yaml_token_delete(&token);token=(yaml_token_t){};}
#define LAMBDA(X) ({ X f;})

static yaml_parser_t parser={};
static yaml_token_t token={};

// profile=(profile_t){
//   .remote_host="42.157.192.81",
//   .remote_port=16460,
//   .local_addr="127.0.0.1",
//   .local_port=1080,
//   .password="5nJJ95sYf3b20HW3t72",
//   .method="chacha20-ietf-poly1305",
//   .fast_open=1,
//   .mode=1,
//   //
//   .log=SS_LOG
//   // .log="/dev/stdout"
//   // .log="/dev/null"
// };

profile_t profile={

  // Zero
  // .timeout
  // .acl
  // .mtu
  // .mptcp

  // Immutable
  .local_addr=LOCAL_ADDR,
  .local_port=LOCAL_PORT_I,
  .log=SS_LOG,
  .fast_open=1,
  .mode=1,
  .verbose=1

  // Mutable
  // .remote_host // S0
  // .method // S1
  // .password // S2
  // .remote_port // I0

};

#define S0 (profile.remote_host)
#define S1 (profile.method)
#define S2 (profile.password)
#define I0 (profile.remote_port)

bool profile_loaded(){
  assert(
    // Zero
    profile.timeout==0 &&
    (!profile.acl) &&
    profile.mtu==0 &&
    profile.mptcp==0 &&
    // Immutable
    profile.local_addr && 0==strcmp(LOCAL_ADDR,profile.local_addr) &&
    profile.local_port==LOCAL_PORT_I &&
    profile.log && 0==strcmp(profile.log,SS_LOG) &&
    profile.fast_open==1 &&
    profile.mode==1 &&
    profile.verbose==1
  );
  // Mutable
  if(S0){
    assert(
      S1&&S2&&
      strlen(S0)&&strlen(S1)&&strlen(S2)&&
      (I0>=1)
    );
    return true;
  }
  assert(
    (!S0)&&(!S1)&&(!S2)&&
    (I0==0)
  );
  return false;
}

void profile_clear(){
  assert(profile_loaded());
  free(S0);free(S1);free(S2);
  S0=NULL;S1=NULL;S2=NULL;
  I0=0;
  assert(!profile_loaded());
  // https://stackoverflow.com/q/1493936#comment1346424_1493988
  // const unsigned char *const p=(const unsigned char*)(&profile);
  // assert(p[0]==0);
  // assert(0==memcmp(p,p+1,sizeof(profile_t)-1));
}

#undef S0
#undef S1
#undef S2
#undef I0

void profile_inspect(){
  printf("\n");
  printf("%s\n",profile.remote_host);
  printf("%s\n",profile.local_addr);
  printf("%s\n",profile.method);
  printf("%s\n",profile.password);
  printf("%d\n",profile.remote_port);
  printf("%d\n",profile.local_port);
  printf("%d\n",profile.timeout);
  printf("\n");
  printf("%s\n",profile.acl?profile.acl:"null");
  printf("%s\n",profile.log?profile.log:"null");
  printf("%d\n",profile.fast_open);
  printf("%d\n",profile.mode);
  printf("%d\n",profile.mtu);
  printf("%d\n",profile.mptcp);
  printf("%d\n",profile.verbose);
  printf("\n");
}

static inline void type(){
  switch(token.type){
    case YAML_NO_TOKEN:eprintf("YAML_NO_TOKEN\n");break;
    case YAML_STREAM_START_TOKEN:eprintf("YAML_STREAM_START_TOKEN\n");break;
    case YAML_STREAM_END_TOKEN:eprintf("YAML_STREAM_END_TOKEN\n");break;
    case YAML_VERSION_DIRECTIVE_TOKEN:eprintf("YAML_VERSION_DIRECTIVE_TOKEN\n");break;
    case YAML_TAG_DIRECTIVE_TOKEN:eprintf("YAML_TAG_DIRECTIVE_TOKEN\n");break;
    case YAML_DOCUMENT_START_TOKEN:eprintf("YAML_DOCUMENT_START_TOKEN\n");break;
    case YAML_DOCUMENT_END_TOKEN:eprintf("YAML_DOCUMENT_END_TOKEN\n");break;
    case YAML_BLOCK_SEQUENCE_START_TOKEN:eprintf("YAML_BLOCK_SEQUENCE_START_TOKEN\n");break;
    case YAML_BLOCK_MAPPING_START_TOKEN:eprintf("YAML_BLOCK_MAPPING_START_TOKEN\n");break;
    case YAML_BLOCK_END_TOKEN:eprintf("YAML_BLOCK_END_TOKEN\n");break;
    case YAML_FLOW_SEQUENCE_START_TOKEN:eprintf("YAML_FLOW_SEQUENCE_START_TOKEN\n");break;
    case YAML_FLOW_SEQUENCE_END_TOKEN:eprintf("YAML_FLOW_SEQUENCE_END_TOKEN\n");break;
    case YAML_FLOW_MAPPING_START_TOKEN:eprintf("YAML_FLOW_MAPPING_START_TOKEN\n");break;
    case YAML_FLOW_MAPPING_END_TOKEN:eprintf("YAML_FLOW_MAPPING_END_TOKEN\n");break;
    case YAML_BLOCK_ENTRY_TOKEN:eprintf("YAML_BLOCK_ENTRY_TOKEN\n");break;
    case YAML_FLOW_ENTRY_TOKEN:eprintf("YAML_FLOW_ENTRY_TOKEN\n");break;
    case YAML_KEY_TOKEN:eprintf("YAML_KEY_TOKEN\n");break;
    case YAML_VALUE_TOKEN:eprintf("YAML_VALUE_TOKEN\n");break;
    case YAML_ALIAS_TOKEN:eprintf("YAML_ALIAS_TOKEN\n");break;
    case YAML_ANCHOR_TOKEN:eprintf("YAML_ANCHOR_TOKEN\n");break;
    case YAML_TAG_TOKEN:eprintf("YAML_TAG_TOKEN\n");break;
    case YAML_SCALAR_TOKEN:eprintf("YAML_SCALAR_TOKEN\n");break;
    default:eprintf("UNKNOWN\n");break;
  }
}

static inline void assert_token_type(yaml_token_type_t tt){
  SCAN();
  if(token.type!=tt){
    type();
    assert(0);
  }
  DEL();
}

static inline int scalarcmp(const char *const s){
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  /*if(token.type!=YAML_SCALAR_TOKEN){
    type();
    assert(0);
    // dump();
  }*/
  const int ret=strcmp((const char*)(token.data.scalar.value),s);
  DEL();
  return ret;
}

static inline int assert_key_compare_val(const char *const k,const char *const v){
  assert_token_type(YAML_KEY_TOKEN);
  assert(0==scalarcmp(k));
  assert_token_type(YAML_VALUE_TOKEN);
  return scalarcmp(v);
}

static inline void assert_key_ignore_val(const char *const k){
  assert_token_type(YAML_KEY_TOKEN);
  assert(0==scalarcmp(k));
  assert_token_type(YAML_VALUE_TOKEN);
  assert_token_type(YAML_SCALAR_TOKEN);
}

static inline char *assert_key_get_val(const char *const k){
  assert_token_type(YAML_KEY_TOKEN);
  assert(0==scalarcmp(k));
  assert_token_type(YAML_VALUE_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  char *ret=strdup((const char*)(token.data.scalar.value));
  DEL();
  return ret;
}

void yaml2profile(const char *const from_yaml,const char *const server_title){

  assert(!profile_loaded());

  // Init
  yaml_parser_initialize(&parser);
  yaml_parser_set_encoding(&parser,YAML_UTF8_ENCODING);
  assert(from_yaml);
  FILE *f=fopen(from_yaml,"r");
  assert(f);
  yaml_parser_set_input_file(&parser,f);
  assert_token_type(YAML_STREAM_START_TOKEN);
  assert_token_type(YAML_BLOCK_MAPPING_START_TOKEN);

  // Skip general config
  for(;;){
    SCAN();
    if(token.type==YAML_KEY_TOKEN){
      DEL();
      SCAN();
      if(token.type==YAML_SCALAR_TOKEN)
        if(0==strcmp("proxies",(const char*)(token.data.scalar.value))){
          DEL();
          break;
        }
    }
    DEL();
  }

  // Get into the first entry
  assert_token_type(YAML_VALUE_TOKEN);

  // https://github.com/yaml/libyaml/issues/91

  assert_token_type(YAML_BLOCK_ENTRY_TOKEN);
  /*SCAN();
  if(token.type==YAML_BLOCK_SEQUENCE_START_TOKEN){
    eprintf("indented\n");
    assert_token_type(YAML_BLOCK_ENTRY_TOKEN);
  }
  else if(token.type==YAML_BLOCK_ENTRY_TOKEN){
    eprintf("not indented\n");
  }else{
    assert(0);
  }
  DEL();*/

  assert_token_type(YAML_BLOCK_MAPPING_START_TOKEN);

  assert(server_title);
  while(0!=assert_key_compare_val("name",server_title)){
    // static int count=0;
    // eprintf("non-match %3d\n",++count);
    SCAN();
    while(token.type!=YAML_BLOCK_END_TOKEN){
      DEL();
      SCAN();
    }
    DEL();
    SCAN();
    if(token.type!=YAML_BLOCK_ENTRY_TOKEN){
      eprintf("\'%s\' not found in %s\n",server_title,from_yaml);
      eprintf("wrong provider?\n");
      eprintf("proxy-groups? (iteration not yet implemented)\n");
      assert(0);
    }
    DEL();
    assert_token_type(YAML_BLOCK_MAPPING_START_TOKEN);
  }

  assert(!profile_loaded());
  //
  assert(0==assert_key_compare_val("type","ss"));
  char *domain=assert_key_get_val("server");
  char *remote_port=assert_key_get_val("port");
  profile.method=assert_key_get_val("cipher");
  profile.password=assert_key_get_val("password");
  assert(0==assert_key_compare_val("udp","true"));
  //
  assert(domain);
  assert(remote_port);
  assert(NULL!=(profile.remote_host=resolv(domain)));
  assert(1<=(profile.remote_port=atoi(remote_port)));
  free(domain);
  free(remote_port);
  domain=NULL;
  //
  // profile_inspect();
  assert(profile_loaded());

  assert_token_type(YAML_BLOCK_END_TOKEN);

  yaml_parser_delete(&parser);
  parser=(yaml_parser_t){};
  fclose(f);
  f=NULL;

}

static inline void match(const char *const string,const char *const regex){
  // printf("%s\n",regex);
  regex_t preg={};
  assert(0==regcomp(&preg,regex,REG_EXTENDED));
  regmatch_t pmatch={};
  assert(0==regexec(&preg,string,1,&pmatch,0));
  // printf("#%d (%d,%d)\n",i,pmatch[i].rm_so,pmatch[i].rm_eo);
  assert(
    pmatch.rm_so==0&&
    pmatch.rm_eo==(int)strlen(string)
  );
  regfree(&preg);
}

static inline void appendjson(json_object *const root,const char *const k,const char *const v){
  match(k,"[0-9A-Za-z_]+");
  match(v,"[0-9A-Za-z_.-]+");
  // const int ql=strlen(v)+2+1;
  // char vq[ql];
  // memset(vq,'\"',ql);
  // strcpy(vq+1,v);
  assert(0==json_object_object_add(
    root,
    k,
    json_object_new_string(v/*vq*/)
  ));
}

// {
//   "server": "127.127.127.127",
//   "server_port": "114514",
//   "local_address": "127.0.0.1",
//   "local_port": "1080",
//   "password": "ItuYNH19tMHEBnAuGSwn",
//   "method": "chacha20-ietf-poly1305",
//   "fast_open": "true"
//   "mode": "tcp_and_udp"
// }
void profile2json(const char *const server_title){
  assert(profile_loaded());
  json_object *root=json_object_new_object();
  assert(root);
  char server_port[8]={};
  sprintf(server_port,"%d",profile.remote_port);
  appendjson(root,"server"       ,profile.remote_host);
  appendjson(root,"server_port"  ,server_port);
  appendjson(root,"local_address",profile.local_addr);
  appendjson(root,"local_port"   ,LOCAL_PORT_S);
  appendjson(root,"password"     ,profile.password);
  appendjson(root,"method"       ,profile.method);
  appendjson(root,"fast_open"    ,"true");assert(profile.fast_open);
  appendjson(root,"mode"         ,"tcp_and_udp");assert(profile.mode);
  // assert(0==json_object_to_fd(STDOUT_FILENO,root,JSON_C_TO_STRING_PRETTY|JSON_C_TO_STRING_SPACED));
  try_unlink(SS_LOCAL_JSON);
  assert(0==json_object_to_file_ext(
    SS_LOCAL_JSON,
    root,
    JSON_C_TO_STRING_PRETTY|JSON_C_TO_STRING_SPACED
  ));
  printf("created \'%s\'\n",SS_LOCAL_JSON);
  assert(1==json_object_put(root));
  root=NULL;
  // Newline
  FILE *f=fopen(SS_LOCAL_JSON,"a");
  assert(f);
  fprintf(f,"\n");
  if(server_title){
    assert(strlen(server_title));
    fprintf(f,"// %s\n",server_title);
  }
  fclose(f);f=NULL;
}
