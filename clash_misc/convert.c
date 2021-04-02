// #define _GNU_SOURCE

#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include <gmodule.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <yaml.h>

// Country flag emoji unicode literal
#include <wchar.h>
#include <locale.h>

// 7892 -> 9090
#include "../restful_port.h"

#define SZ        128
#define SZ_CIPHER  64
#define SZ_PORT     8
// #define FILENAME "/home/darren/yaml/01_rixcloud.yaml"
// #define FILENAME "/home/darren/yaml/01_ssrcloud.yaml"

#define EMIT() assert(1==yaml_emitter_emit(&emitter, &event))
#define eprintf(...) fprintf(stderr,__VA_ARGS__)
#define SCAN() assert(1==yaml_parser_scan(&parser,&token))
#define DEL() {yaml_token_delete(&token);token=(yaml_token_t){};}
#define SCALAR(S) {yaml_scalar_event_initialize(&event,NULL,(yaml_char_t*)YAML_STR_TAG,(const yaml_char_t*)S,strlen(S),1,1,YAML_PLAIN_SCALAR_STYLE /*YAML_SINGLE_QUOTED_SCALAR_STYLE*/);EMIT();}
#define SEQ_START() {yaml_sequence_start_event_initialize(&event,NULL,(yaml_char_t *)YAML_SEQ_TAG,1,YAML_BLOCK_SEQUENCE_STYLE /*YAML_ANY_SEQUENCE_STYLE*/);EMIT();}
#define SEQ_END() {yaml_sequence_end_event_initialize(&event);EMIT();}
#define MAP_START() {yaml_mapping_start_event_initialize(&event,NULL,(yaml_char_t *)YAML_MAP_TAG,1,YAML_BLOCK_MAPPING_STYLE /*YAML_ANY_MAPPING_STYLE*/);EMIT();}
#define MAP_END() {yaml_mapping_end_event_initialize(&event);EMIT();}
#define LAMBDA(X) ({ X f;})

#ifdef FILENAME
static FILE *file=NULL;
#endif

static yaml_parser_t parser={};
static yaml_token_t token={};

static yaml_emitter_t emitter={};
static yaml_event_t event={};

static GSList *l_asia=NULL;
static GSList *l_jp=NULL;
static GSList *l_ca_us=NULL;
static GSList *l_eu_uk=NULL;
static GSList *l_stray=NULL;

// Common
static char plain_port[SZ_PORT]={};
static char plain_cipher[SZ_CIPHER]={};
static char plain_password[SZ]={};

// Common
// static char obfs_port[SZ_PORT]={};
// static char obfs_cipher[SZ_CIPHER]={};
// static char obfs_host[SZ]={};

// Unique
typedef struct _Node {
  bool obfs;
  char name[SZ];
  char server[SZ];
} Node;

// https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2
// https://en.wikipedia.org/wiki/Country_code
typedef struct {
  char c[2];
} CC;

typedef struct {
  wchar_t wcs[2+1];
} FlagWCS;

// https://en.wikipedia.org/wiki/UTF-8#Encoding
// U+10000...U+10FFFF 4-byte
typedef struct {
  char mbs[4+4+1];
} FlagMBS;

// https://en.wikipedia.org/wiki/Regional_indicator_symbol
// wchar_t aka int
static wchar_t c2ris(const char c){
  assert('A'<=c&&c<='Z');
  // wchar_t ris_A=L'\U0001F1E6';
  // const wchar_t ris_Z=L'\U0001F1FF';
  // assert((ris_Z-ris_A)==('Z'-'A'));
  #define RIS_A (L'\U0001F1E6')
  #define RIS_Z (L'\U0001F1FF')
  static_assert((RIS_Z-RIS_A)==('Z'-'A'));
  wchar_t r=(wchar_t)RIS_A+(wchar_t)(c-'A');
  assert(RIS_A<=r&&r<=RIS_Z);
  return r;
}

static void cc2wcs(FlagWCS *const dest, const CC *const src){
  *dest=(FlagWCS){.wcs={
    [0]=c2ris((src->c)[0]),
    [1]=c2ris((src->c)[1]),
    [2]=L'\0'
  }};
  // dest->wcs[0]=c2ris((src->c)[0]);
  // dest->wcs[1]=c2ris((src->c)[1]);
  // dest->wcs[2]=L'\0';
}

static void wcs2mbs(FlagMBS *const dest, const FlagWCS *const src){
  // Non-restartable
  assert(8==wcstombs(NULL,src->wcs,0));
  assert(8==wcstombs(dest->mbs,src->wcs,9));
  // Restartable
  // const wchar_t *p=src->wcs;
  // mbstate_t t={};
  // assert(1==mbsinit(&t));
  // assert(8==wcsrtombs(NULL,&p,0,&t));
  // assert(1==mbsinit(&t));
  // assert(p==src->wcs);
  // assert(8==wcsrtombs(dest->mbs,&p,9,&t));
  // assert(1==mbsinit(&t));
  // assert(p==NULL);
  assert(dest->mbs[8]=='\0');
}

static void cc2wcs2mbs(FlagMBS *const dest, const CC *const src){
  FlagWCS w={};
  cc2wcs(&w,src);
  wcs2mbs(dest,&w);
}

static bool strstrVA(const char *const haystack, ...){
  va_list ap;
  va_start(ap,haystack);
  const char *needle=NULL;
  while((needle=(const char*)va_arg(ap,const char*)))
    if(strstr(haystack,needle))
      return true;
  return false;
}

static void group(const char *const s){
  bool stray=true;
  //                                        hk   jp    kr                ru              sg      tw 
  if(strstrVA(s,"ASYNCHRONOUS TRANSFERMODE","港","日本","韓國","韓国","韩国","俄羅斯","俄罗斯","新加坡","臺灣","台灣","台湾",NULL))
                                                                     { l_asia =g_slist_prepend(l_asia ,strdup(s)); stray=false; }
  if(strstr(s,"日本")||strcasestr(s,"ntt"))/*補補補*/                  { l_jp   =g_slist_prepend(l_jp   ,strdup(s)); stray=false; }
  if(strstrVA(s,"美國","美国","加拿大",NULL))/*補補補*/                  { l_ca_us=g_slist_prepend(l_ca_us,strdup(s)); stray=false; }
  if(strstrVA(s,"荷蘭","荷兰","德國","德国","英國","英国",NULL))/*補補補*/ { l_eu_uk=g_slist_prepend(l_eu_uk,strdup(s)); stray=false; }
  if(stray)/**/                                                      { l_stray=g_slist_prepend(l_stray,strdup(s));              }
}

static void emit_scalar(const char *s,...){
  va_list vl;
  va_start(vl,s);
  while(s!=NULL){
    SCALAR(s);
    s=va_arg(vl,const char *);
  }
}

static void emitter_begin(){
  // Init
  yaml_emitter_initialize(&emitter);
  yaml_emitter_set_output_file(&emitter,stdout);
  yaml_emitter_set_encoding(&emitter,YAML_UTF8_ENCODING);
  yaml_emitter_set_canonical(&emitter,0);
  yaml_emitter_set_indent(&emitter,2);
  yaml_emitter_set_width(&emitter,-1);
  yaml_emitter_set_unicode(&emitter,1);
  yaml_emitter_set_break(&emitter,YAML_LN_BREAK);
  // Header
  yaml_stream_start_event_initialize(&event,YAML_UTF8_ENCODING);
  EMIT();
  yaml_document_start_event_initialize(&event,/*&((yaml_version_directive_t){1,2})*/NULL,NULL,NULL,1);
  EMIT();
  MAP_START();
  emit_scalar(
    "port","8080",
    "socks-port","1080",
    "allow-lan","true",
    "mode","Global",
    "log-level","info",
    "external-controller","127.0.0.1:"RESTFUL_PORT, // yacd updated defaults
    // "external-controller","\'127.0.0.1:6170\'",
    "secret","",
    NULL
  );
}

static void emitter_end(){
  MAP_END();
  yaml_document_end_event_initialize(&event,1);
  EMIT();
  yaml_stream_end_event_initialize(&event);
  EMIT();
  yaml_emitter_delete(&emitter);
  emitter=(yaml_emitter_t){};
}

/*static void type(const yaml_token_type_t t){
  switch(t){
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
}*/

static void assert_token_type(yaml_token_type_t tt){
  SCAN();
  assert(token.type==tt);
  DEL();
}

static void parser_begin(){

  yaml_parser_initialize(&parser);
  yaml_parser_set_encoding(&parser,YAML_UTF8_ENCODING);

  #ifdef FILENAME
  file=fopen(FILENAME,"r");
  yaml_parser_set_input_file(&parser,file);
  #else
  yaml_parser_set_input_file(&parser,stdin);
  #endif

  assert_token_type(YAML_STREAM_START_TOKEN);
  assert_token_type(YAML_BLOCK_MAPPING_START_TOKEN);

  // Skip raw header
  for(;;){
    SCAN();
    if(token.type==YAML_KEY_TOKEN){
      DEL();
      SCAN();
      if(token.type==YAML_SCALAR_TOKEN)
        if(0==strcmp((const char*)(token.data.scalar.value),"proxies")){
          DEL();
          break;
        }
    }
    DEL();
  }

  assert_token_type(YAML_VALUE_TOKEN);
  assert_token_type(YAML_BLOCK_SEQUENCE_START_TOKEN);

}

static void parser_end(){

  yaml_parser_delete(&parser);
  parser=(yaml_parser_t){};

  #ifdef FILENAME
  fclose(file);
  file=NULL;
  #endif

}

static void extract_value_with_key(const char *const k,char *v){
  assert_token_type(YAML_KEY_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  assert(0==strcmp((const char*)(token.data.scalar.value),k));
  DEL();
  assert_token_type(YAML_VALUE_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  strcpy(v,(const char*)(token.data.scalar.value));
  DEL();
}

static void assert_value_with_key(const char *const k,const char *const v0){
  assert_token_type(YAML_KEY_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  assert(0==strcmp((const char*)(token.data.scalar.value),k));
  DEL();
  assert_token_type(YAML_VALUE_TOKEN);
  SCAN();
  assert(token.type==YAML_SCALAR_TOKEN);
  assert(0==strcmp((const char*)(token.data.scalar.value),v0));
  DEL();
}

/*static void skip_node(){
  bool test=false;
  for(;;){
    SCAN();
    type(token.type);
    if(token.type==YAML_BLOCK_MAPPING_START_TOKEN){
      for(int i=0;i<20;++i){
        DEL();
        SCAN();
        type(token.type);
      }
      assert(false);
    }
    if(token.type==YAML_BLOCK_END_TOKEN){
      DEL();
      break;
    }
    DEL();
  }
}*/


// lean: Expected n(YAML_BLOCK_END_TOKEN) - n(YAML_BLOCK_MAPPING_START_TOKEN)
static void skip_node(int lean){
  for(;;){
    SCAN();
    if(token.type==YAML_BLOCK_MAPPING_START_TOKEN)
      ++lean;
    if(token.type==YAML_BLOCK_END_TOKEN){
      if(lean==0){
        DEL();
        break;
      }
      assert(lean>=1);
      --lean;
    }
    DEL();
  }
}

static void emit_node(Node *n){
  assert(!(n->obfs));
  MAP_START();
  emit_scalar(
    "name",n->name,
    "type","ss",
    "server",n->server,
    "port",plain_port,
    "cipher",plain_cipher,
    "password",plain_password,
    "udp","true",
    NULL
  );
  MAP_END();
}

static void emit_and_destroy_group(const char *const title,GSList *l){

  if(!l){
    eprintf("skip empty group %s\n",title);
    return;
  }

  MAP_START();

  emit_scalar(
    "name",title,
    "type","select",
    NULL
  );

  SCALAR("proxies");

  SEQ_START();

  g_slist_foreach(
    l,
    LAMBDA(void f(gpointer data, __attribute__((__unused__)) gpointer user_data){
      SCALAR(data);
    }),
    NULL
  );

  g_slist_free_full(l,g_free);

  SEQ_END();

  MAP_END();

}

/*static void test(){

  // FlagMBS m={};
  // cc2wcs2mbs(&m,&(CC){"US"});

  // assert(0==fwide(stdout,0));
  // printf("%s\n",m.mbs);

  // const wchar_t *name = L"r\u00e9sum\u00e9";
  // const wchar_t *name = L"\U0001F1ED\U0001F1F0";
  // wprintf(L"name is %ls\n",name);

}*/

int main(){

  setlocale(LC_CTYPE,"en_US.UTF-8");

  // test();
  // eprintf("\n");
  // exit(0);

  emitter_begin();
  parser_begin();

  SCALAR("proxies");

  SEQ_START();
  // eprintf("A\n");

  for(;;){

    // eprintf("B\n");

    // If sequence has ended
    SCAN();
    if(token.type==YAML_BLOCK_END_TOKEN)
      break;
    if(token.type!=YAML_BLOCK_ENTRY_TOKEN)
      assert(false);

    assert_token_type(YAML_BLOCK_MAPPING_START_TOKEN);

    Node node={};

    extract_value_with_key("name",node.name);
    if(
      strstr(node.name,"回國")||strstr(node.name,"中國標準 BGP 邊緣")||
      strstr(node.name,"回国")||strstr(node.name,"中国标准 BGP 边缘")
    ){
      eprintf("drop cn node %s\n",node.name);
      skip_node(0);
      continue;
    }
    char type[8]={};
    extract_value_with_key("type",type);
    if(0!=strcmp("ss",type)){
      eprintf("drop %s node %s\n",type,node.name);
      skip_node(0);
      continue;
    }
    extract_value_with_key("server",node.server);
    if(1==inet_pton(AF_INET6,node.server,&((struct in6_addr){}))){
      eprintf("drop IPv6 node %s %s\n",node.name,node.server);
      skip_node(0);
      continue;
    }
    if(0==strcmp("127.0.0.1",node.server)){
      eprintf("drop loopback node %s\n",node.name);
      skip_node(0);
      continue;
    }

    char port[SZ_PORT]={};
    char cipher[SZ_CIPHER]={};
    char password[SZ]={};
    extract_value_with_key("port",port);
    extract_value_with_key("cipher",cipher);
    extract_value_with_key("password",password);

    assert_value_with_key("udp","true");

    SCAN();

    // Plain
    if(token.type==YAML_BLOCK_END_TOKEN){

      node.obfs=false;
      if(0==strlen(plain_port)&&0==strlen(plain_cipher)&&0==strlen(plain_password)){
        strcpy(plain_port,port);
        strcpy(plain_cipher,cipher);
        strcpy(plain_password,password);
        eprintf("\n");
        eprintf("plain port     - %s\n",plain_port);
        eprintf("plain cipher   - %s\n",plain_cipher);
        eprintf("plain password - %s\n",plain_password);
        eprintf("\n");
      }else if(3<=strlen(plain_port)&&3<=strlen(plain_cipher)&&3<=strlen(plain_password)){
        assert(0==strcmp(port,plain_port));
        assert(0==strcmp(cipher,plain_cipher));
        assert(0==strcmp(password,plain_password));
      }else{
        assert(false);
      }

    // Obfs
    }else if(token.type==YAML_KEY_TOKEN){

      eprintf("drop obfs node %s %s\n",node.name,node.server);
      skip_node(0);
      continue;

      // node.obfs=true;

      // assert(0==strcmp(password,"CNIX"));

      // eprintf("\n");
      // eprintf("obfs cipher - %s\n",obfs_cipher);
      // eprintf("obfs port   - %s\n",obfs_port);
      // eprintf("obfs host   - %s\n",obfs_host);
      // eprintf("\n");

    // Error
    }else{

      assert(false);

    }

    DEL();

    group(node.name);
    emit_node(&node);

  }

  SEQ_END();

  SCALAR("proxy-groups");

  SEQ_START();

  // FlagMBS m={};
  // cc2wcs2mbs(&m,&(CC){"MO"});emit_and_destroy_group(m.mbs,mo);mo=NULL;

  FlagMBS m={};
  cc2wcs2mbs(&m,&(CC){"HK"});emit_and_destroy_group(m.mbs,l_asia);l_asia=NULL;
  cc2wcs2mbs(&m,&(CC){"JP"});emit_and_destroy_group(m.mbs,l_jp);l_jp=NULL;
                              emit_and_destroy_group("NA",l_ca_us);l_ca_us=NULL;
  cc2wcs2mbs(&m,&(CC){"EU"});emit_and_destroy_group(m.mbs,l_eu_uk);l_eu_uk=NULL;
                              emit_and_destroy_group("XX",l_stray);l_stray=NULL;

  SEQ_END();

  parser_end();

  emitter_end();

}