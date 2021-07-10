#include <assert.h>
#include <regex.h> // regex_t regcomp() regmatch_t regexec() regfree()
#include <stdbool.h>
#include <stdio.h> // FILE fopen() fprintf() fclose()
#include <string.h> // strcmp() strlen()
#include <unistd.h> // unlink

#include <json.h>
#include <shadowsocks.h> // profile_t

#include "./def.h"
#include "./file.h" // try_unlink()
#include "./shadowsocks2.h"

#include "./profile.h"

profile_t profile={

  // Zero
  // .acl
  // .mtu
  // .mptcp

  // Immutable
  .local_addr=LOCAL_ADDR,
  .local_port=LOCAL_PORT_I,
  .timeout=TIMEOUT,
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
    profile.timeout==TIMEOUT &&
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
  printf("remote_host = %s\n",profile.remote_host);
  printf("local_addr  = %s\n",profile.local_addr);
  printf("method      = %s\n",profile.method);
  printf("password    = %s\n",profile.password);
  printf("remote_port = %d\n",profile.remote_port);
  printf("local_port  = %d\n",profile.local_port);
  printf("timeout     = %d\n",profile.timeout);
  printf("\n");
  printf("acl       = %s\n",profile.acl?profile.acl:"NULL");
  printf("log       = %s\n",profile.log?profile.log:"NULL");
  printf("fast_open = %d\n",profile.fast_open);
  printf("mode      = %d\n",profile.mode);
  printf("mtu       = %d\n",profile.mtu);
  printf("mptcp     = %d\n",profile.mptcp);
  printf("verbose   = %d\n",profile.verbose);
  printf("\n");
}

// Diagnose
// Better (not applicable here) https://stackoverflow.com/a/3168470
// puts(type());
/*static const char *type(){
  switch(token.type){
    case YAML_NO_TOKEN:                   return "YAML_NO_TOKEN";                   break;
    case YAML_STREAM_START_TOKEN:         return "YAML_STREAM_START_TOKEN";         break;
    case YAML_STREAM_END_TOKEN:           return "YAML_STREAM_END_TOKEN";           break;
    case YAML_VERSION_DIRECTIVE_TOKEN:    return "YAML_VERSION_DIRECTIVE_TOKEN";    break;
    case YAML_TAG_DIRECTIVE_TOKEN:        return "YAML_TAG_DIRECTIVE_TOKEN";        break;
    case YAML_DOCUMENT_START_TOKEN:       return "YAML_DOCUMENT_START_TOKEN";       break;
    case YAML_DOCUMENT_END_TOKEN:         return "YAML_DOCUMENT_END_TOKEN";         break;
    case YAML_BLOCK_SEQUENCE_START_TOKEN: return "YAML_BLOCK_SEQUENCE_START_TOKEN"; break;
    case YAML_BLOCK_MAPPING_START_TOKEN:  return "YAML_BLOCK_MAPPING_START_TOKEN";  break;
    case YAML_BLOCK_END_TOKEN:            return "YAML_BLOCK_END_TOKEN";            break;
    case YAML_FLOW_SEQUENCE_START_TOKEN:  return "YAML_FLOW_SEQUENCE_START_TOKEN";  break;
    case YAML_FLOW_SEQUENCE_END_TOKEN:    return "YAML_FLOW_SEQUENCE_END_TOKEN";    break;
    case YAML_FLOW_MAPPING_START_TOKEN:   return "YAML_FLOW_MAPPING_START_TOKEN";   break;
    case YAML_FLOW_MAPPING_END_TOKEN:     return "YAML_FLOW_MAPPING_END_TOKEN";     break;
    case YAML_BLOCK_ENTRY_TOKEN:          return "YAML_BLOCK_ENTRY_TOKEN";          break;
    case YAML_FLOW_ENTRY_TOKEN:           return "YAML_FLOW_ENTRY_TOKEN";           break;
    case YAML_KEY_TOKEN:                  return "YAML_KEY_TOKEN";                  break;
    case YAML_VALUE_TOKEN:                return "YAML_VALUE_TOKEN";                break;
    case YAML_ALIAS_TOKEN:                return "YAML_ALIAS_TOKEN";                break;
    case YAML_ANCHOR_TOKEN:               return "YAML_ANCHOR_TOKEN";               break;
    case YAML_TAG_TOKEN:                  return "YAML_TAG_TOKEN";                  break;
    case YAML_SCALAR_TOKEN:               return "YAML_SCALAR_TOKEN";               break;
    default:                              return "UNKNOWN";                         break;
  }
}*/

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
void profile_to_json(const char *const server_title){
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
