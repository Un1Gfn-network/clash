#include <assert.h>
#include <curl/curl.h>
#include <json.h>
#include <stdlib.h>
#include <string.h>

#include <libclash.h>

// https://gcc.gnu.org/onlinedocs/gcc-4.8.5/cpp/Stringification.html
#define xstr(a) str(a)
#define str(a) #a

static char *buf=NULL;
static size_t sz=0;

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata){
  assert(size==1);
  assert(nmemb>=1);
  // eprintf("new segment\n");
  assert(NULL!=(buf=realloc(buf,sz+nmemb)));
  memcpy(buf+sz,ptr,nmemb);
  sz+=nmemb;
  return nmemb;
}

static inline const char *curl_get(const char *const url){

  assert(0==curl_global_init(CURL_GLOBAL_NOTHING));
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
    printf("%s\n",curl_easy_strerror(errornum));
    assert(0);
  }

  curl_easy_cleanup(curl);
  curl=NULL;

  // printf("%.*s\n",(int)sz,buf);
  assert(NULL!=(buf=realloc(buf,++sz)));
  buf[sz-1]='\0';

  // printf("%s",buf);
  return buf;

}

static inline void curl_drop(){
  free(buf);
  buf=NULL;
  sz=0;
  curl_global_cleanup();
}

static inline void assert_field(const json_object *const j,const char *const k,const char *const v){
  json_object *p=NULL;
  assert(json_object_object_get_ex(j,k,&p));
  assert(p);
  assert(json_type_string==json_object_get_type(p));
  assert(0==strcmp(v,json_object_get_string(p)));
}

static inline const char *get_field_string(const json_object *const j,const char *const k){
  json_object *p=NULL;
  assert(json_object_object_get_ex(j,k,&p));
  assert(p);
  json_type t=json_object_get_type(p);
  if(t!=json_type_string){
    printf("%s\n",json_type_to_name(t));
    assert(0);
  }
  return json_object_get_string(p);
}

static inline int get_field_int(const json_object *const j,const char *const k){
  json_object *p=NULL;
  assert(json_object_object_get_ex(j,k,&p));
  assert(p);
  json_type t=json_object_get_type(p);
  if(t!=json_type_int){
    printf("%s\n",json_type_to_name(t));
    assert(0);
  }
  return json_object_get_int(p);
}

char *current_server_title(){

  // Parse buf, not a file
  json_tokener *const tok=json_tokener_new();
  assert(tok);

  json_tokener_reset(tok);
  json_object *jobj=json_tokener_parse_ex(tok,curl_get("http://127.0.0.1:"xstr(RESTFUL_PORT)"/proxies/GLOBAL"),-1);
  assert(jobj);
  enum json_tokener_error jerr=json_tokener_get_error(tok);
  if(jerr!=json_tokener_success){
    printf("%s\n",json_tokener_error_desc(jerr));
    assert(0);
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

  curl_drop();
  json_tokener_free(tok);

  return ret;

}
