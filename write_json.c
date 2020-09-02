#include <assert.h>
#include <json.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <yaml.h>
#include <errno.h>

#include "def.h"

#define eprintf(...) fprintf(stderr,__VA_ARGS__)
#define SCAN() assert(1==yaml_parser_scan(&parser,&token))
#define DEL() {yaml_token_delete(&token);token=(yaml_token_t){};}
#define LAMBDA(X) ({ X f;})

static yaml_parser_t parser={};
static yaml_token_t token={};

static json_object *root=NULL;

// resolv.c
extern char *resolv(const char *);

void type(){
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

static inline void dump(){
  for(;;){
    SCAN();
    type();
    assert(token.type!=YAML_NO_TOKEN);
    DEL();
  }
}

static inline void assert_token_type(yaml_token_type_t tt){
  SCAN();
  if(token.type!=tt){
    type();
    assert(false);
  }
  DEL();
}

static inline int scalarcmp(const char *const s){
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  /*if(token.type!=YAML_SCALAR_TOKEN){
    type();
    assert(false);
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

static inline void appendjson(const char *const json_key,const char *const v){
  assert(0==json_object_object_add(
    root,
    json_key,
    json_object_new_string(v)
  ));
}

void write_json(const char *const filename,const char *const s){

  // Init
  yaml_parser_initialize(&parser);
  yaml_parser_set_encoding(&parser,YAML_UTF8_ENCODING);
  assert(filename);
  FILE *f=fopen(filename,"r");
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
    assert(false);
  }
  DEL();*/

  assert_token_type(YAML_BLOCK_MAPPING_START_TOKEN);

  assert(s);
  while(0!=assert_key_compare_val("name",s)){
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
      eprintf("%s not found in %s\n",s,filename);
      exit(1);
    }
    DEL();
    assert_token_type(YAML_BLOCK_MAPPING_START_TOKEN);
  }

  assert(NULL!=(root=json_object_new_object()));

  assert_key_assert_val("type","ss");
  yaml2json_resolv("server","server");
  yaml2json("port","server_port");
  yaml2json("cipher","method");
  yaml2json("password","password");
  appendjson("local_address","127.0.0.1");
  appendjson("local_port","1080");
  appendjson("mode","tcp_only");
  assert_key_assert_val("udp","true");

  assert_token_type(YAML_BLOCK_END_TOKEN);

  // assert(0==json_object_to_fd(STDOUT_FILENO,root,JSON_C_TO_STRING_PRETTY|JSON_C_TO_STRING_SPACED));
  const int i=unlink(SS_LOCAL_JSON);
  if(i==-1){
    assert(errno=ENOENT);
  }else{
    assert(i==0);
    printf("removed \'%s\'\n",SS_LOCAL_JSON);
  }
  assert(0==json_object_to_file_ext(SS_LOCAL_JSON,root,JSON_C_TO_STRING_PRETTY|JSON_C_TO_STRING_SPACED));
  printf("created \'%s\'\n",SS_LOCAL_JSON);
  assert(1==json_object_put(root));
  root=NULL;

  yaml_parser_delete(&parser);
  parser=(yaml_parser_t){};
  fclose(f);
  f=NULL;

}
