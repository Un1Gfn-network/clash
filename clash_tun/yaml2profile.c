#include <assert.h>
#include <yaml.h>
#include <libclash.h> // resolv()

#include "./yaml2profile.h"

#define SCAN() assert(1==yaml_parser_scan(&parser,&token))
#define DEL() yaml_token_delete(&token);token=(yaml_token_t){}
#define TYPE(T) assert(token.type==T)
#define VAL ((const char*)(token.data.scalar.value))
#define CMP(S) strcmp(VAL,S)
// #define LAMBDA(X) ({ X f;})
#define eprintf(...) fprintf(stderr,__VA_ARGS__)

static yaml_parser_t parser={};
static yaml_token_t token={};

static inline char *k_v_dup(const char *const k){
  SCAN();TYPE(YAML_KEY_TOKEN);DEL();
  SCAN();assert(0==strcmp(k,VAL));DEL();
  SCAN();TYPE(YAML_VALUE_TOKEN);DEL();
  SCAN();TYPE(YAML_SCALAR_TOKEN);char *const ret=strdup(VAL);DEL();
  return ret;
}

static inline int k_v_cmp(const char *const k,const char *const v){
  SCAN();TYPE(YAML_KEY_TOKEN);DEL();
  SCAN();assert(0==strcmp(k,VAL));DEL();
  SCAN();TYPE(YAML_VALUE_TOKEN);DEL();
  SCAN();TYPE(YAML_SCALAR_TOKEN);const int ret=strcmp(v,VAL);DEL();
  return ret;
}

void yaml2profile(const bool resolve_domain,profile_t *const profile,const char *const from_yaml,const char *const server_title){

  // Init
  yaml_parser_initialize(&parser);
  yaml_parser_set_encoding(&parser,YAML_UTF8_ENCODING);
  assert(from_yaml);
  FILE *f=fopen(from_yaml,"r");assert(f);
  yaml_parser_set_input_file(&parser,f);
  SCAN();TYPE(YAML_STREAM_START_TOKEN);DEL();
  SCAN();TYPE(YAML_BLOCK_MAPPING_START_TOKEN);DEL();

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
  SCAN();TYPE(YAML_VALUE_TOKEN);DEL();

  // https://github.com/yaml/libyaml/issues/91

  SCAN();TYPE(YAML_BLOCK_ENTRY_TOKEN);DEL();
  /*SCAN();
  if(token.type==YAML_BLOCK_SEQUENCE_START_TOKEN){
    eprintf("indented\n");
    SCAN();TYPE(YAML_BLOCK_ENTRY_TOKEN);DEL();
  }
  else if(token.type==YAML_BLOCK_ENTRY_TOKEN){
    eprintf("not indented\n");
  }else{
    assert(0);
  }
  DEL();*/

  SCAN();TYPE(YAML_BLOCK_MAPPING_START_TOKEN);DEL();

  assert(server_title);
  while(0!=k_v_cmp("name",server_title)){
    // static int count=0;
    // eprintf("non-match %3d\n",++count);
    SCAN();while(token.type!=YAML_BLOCK_END_TOKEN){
      DEL();
      SCAN();
    }DEL();
    SCAN();if(token.type!=YAML_BLOCK_ENTRY_TOKEN){
      eprintf("\'%s\' not found in %s\n",server_title,from_yaml);
      eprintf("wrong provider?\n");
      eprintf("proxy-groups? (iteration not yet implemented)\n");
      assert(0);
    }DEL();
    SCAN();TYPE(YAML_BLOCK_MAPPING_START_TOKEN);DEL();
  }

  // Node found
  assert(0==k_v_cmp("type","ss"));
  char *domain=k_v_dup("server");assert(domain&&strlen(domain));
  char *remote_port=k_v_dup("port");assert(remote_port&&strlen(remote_port));
  profile->method=k_v_dup("cipher");
  profile->password=k_v_dup("password");
  assert(0==k_v_cmp("udp","true"));
  assert(NULL!=(profile->remote_host=(resolve_domain?resolv(domain):strdup(domain))));
  assert(1<=(profile->remote_port=atoi(remote_port)));
  free(domain);
  free(remote_port);
  domain=NULL;

  SCAN();TYPE(YAML_BLOCK_END_TOKEN);DEL();

  yaml_parser_delete(&parser);parser=(yaml_parser_t){};
  fclose(f);f=NULL;

}
