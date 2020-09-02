#include <assert.h>
#include <curl/curl.h>
#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define eprintf(...) fprintf(stderr,__VA_ARGS__)

static char *buf=NULL;
static size_t sz=0;
static json_tokener *tok=NULL;

// yaml.c
extern char *write_json(const char *const,const char *const);

static inline void assert_field(const json_object *const j,const char *const k,const char *const v){
  json_object *p=NULL;
  assert(json_object_object_get_ex(j,k,&p));
  assert(p);
  assert(json_type_string==json_object_get_type(p));
  assert(0==strcmp(v,json_object_get_string(p)));
  // assert(1==json_object_put(p));
  // p=NULL;
}

static inline const char *get_field_string(const json_object *const j,const char *const k){
  json_object *p=NULL;
  assert(json_object_object_get_ex(j,k,&p));
  assert(p);
  json_type t=json_object_get_type(p);
  if(t!=json_type_string){
    eprintf("%s\n",json_type_to_name(t));
    assert(false);
  }
  return json_object_get_string(p);
}

static inline int get_field_int(const json_object *const j,const char *const k){
  json_object *p=NULL;
  assert(json_object_object_get_ex(j,k,&p));
  assert(p);
  json_type t=json_object_get_type(p);
  if(t!=json_type_int){
    eprintf("%s\n",json_type_to_name(t));
    assert(false);
  }
  return json_object_get_int(p);
}

static inline size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata){
  assert(size==1);
  assert(nmemb>=1);
  // eprintf("new segment\n");
  assert(NULL!=(buf=realloc(buf,sz+nmemb)));
  memcpy(buf+sz,ptr,nmemb);
  sz+=nmemb;
  return nmemb;
}

static inline void buf_http_get(const char *const url){

  CURL *curl=curl_easy_init();
  assert(curl);

  // char *encode=curl_easy_escape(curl,"http://127.0.0.1:6170/proxies/香港标准 IEPL 中继 19",0);
  // curl_easy_reset(curl);
  // printf("%s\n",encode);
  // assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_URL,encode));
  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_URL,url));
  // assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_URL,"http://127.0.0.1:6170/proxies/香港标准%20IEPL%20中继%2019"));
  // assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_URL,"http://127.0.0.1:6170/configs"));
  // curl_free(encode);
  // encode=NULL;

  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_callback));
  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_HEADER,0L));
  const CURLcode errornum=curl_easy_perform(curl);
  if(errornum!=CURLE_OK){
    eprintf("%s\n",curl_easy_strerror(errornum));
    assert(false);
  }

  curl_easy_cleanup(curl);
  curl=NULL;

  // printf("%.*s\n",(int)sz,buf);
  assert(NULL!=(buf=realloc(buf,++sz)));
  buf[sz-1]='\0';

  // printf("%s",buf);

}

static inline void buf_drop(){
  free(buf);
  buf=NULL;
  sz=0;
}

static inline char *name_now(){

  buf_http_get("http://127.0.0.1:6170/proxies/GLOBAL");

  json_tokener_reset(tok);
  json_object *jobj=json_tokener_parse_ex(tok,buf,-1);
  assert(jobj);
  enum json_tokener_error jerr=json_tokener_get_error(tok);
  if(jerr!=json_tokener_success){
    eprintf("%s\n",json_tokener_error_desc(jerr));
    assert(false);
  }
  assert(json_type_object==json_object_get_type(jobj));

  // printf("%s\n",json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_PLAIN|JSON_C_TO_STRING_SPACED));
  // printf("%s\n",json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_PRETTY));

  assert_field(jobj,"name","GLOBAL");
  assert_field(jobj,"type","Selector");
  const char *const now=get_field_string(jobj,"now");
  assert(now);
  char *ret=strdup(now);
  assert(ret);
  // printf("%s\n",ret);

  assert(1==json_object_put(jobj));
  jobj=NULL;

  buf_drop();

  return ret;

}

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

int main(const int argc,const char **argv){

  if(
    !( argc==2 && argv[1] && (strcmp(argv[1],"rixcloud")||strcmp(argv[1],"ssrcloud")) )
  ){
    eprintf("\n");
    eprintf("  %s <rixcloud|ssrcloud>\n",argv[0]);
    eprintf("\n");
    exit(1);
  }

  assert(argv[1]);

  // printf("%s\n",curl_version());
  assert(0==curl_global_init(CURL_GLOBAL_NOTHING));
  tok=json_tokener_new();
  assert(tok);

  // buf_http_get("http://127.0.0.1:6170/rules");
  // buf_http_get("http://127.0.0.1:6170/proxies");
  // buf_drop();

  // printf("%d\n",local_port());

  char *name=name_now();
  printf("%s\n",name);

  const char *l="/home/darren/.clash/";
  const char *r="/config.yaml";
  const int total=strlen(l)+strlen(r)+strlen(argv[1])+1;
  char filename[total];
  memset(filename,'\0',total);
  strcat(filename,l);
  strcat(filename,argv[1]);
  strcat(filename,r);
  // eprintf("%s\n",filename);
  write_json(filename,name);

  free(name);
  name=NULL;

  json_tokener_free(tok);
  tok=NULL;
  curl_global_cleanup();

  return 0;

}
