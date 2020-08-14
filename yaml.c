#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <yaml.h>

#define eprintf(...) fprintf(stderr,__VA_ARGS__)
#define SCAN() assert(1==yaml_parser_scan(&parser,&token))
#define DEL() {yaml_token_delete(&token);token=(yaml_token_t){};}
#define LAMBDA(X) ({ X f;})

static yaml_parser_t parser={};
static yaml_token_t token={};

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

static inline void assert_key_print_val(const char *const k){
  assert_token_type(YAML_KEY_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  assert(0==strcmp((const char*)(token.data.scalar.value),k));
  DEL();
  assert_token_type(YAML_VALUE_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  // strcpy(v,(const char*)(token.data.scalar.value));
  eprintf("%s\n",(const char*)(token.data.scalar.value));
  DEL();
}

void extract_server(const char *const filename,const char *const s){

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

  assert_key_assert_val("type","ss");
  assert_key_print_val("server");
  assert_key_ignore_val("port");
  assert_key_ignore_val("cipher");
  assert_key_ignore_val("password");
  assert_key_assert_val("udp","true");

  assert_token_type(YAML_BLOCK_END_TOKEN);

  yaml_parser_delete(&parser);
  parser=(yaml_parser_t){};
  fclose(f);
  f=NULL;

}

