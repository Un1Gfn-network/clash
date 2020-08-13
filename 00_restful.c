#include <assert.h>
#include <curl/curl.h>
#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define eprintf(...) fprintf(stderr,__VA_ARGS__)

char *buf=NULL;
size_t sz=0;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata){

  assert(size==1);
  assert(nmemb>=1);

  // eprintf("new segment\n");

  assert(NULL!=(buf=realloc(buf,sz+nmemb)));
  memcpy(buf+sz,ptr,nmemb);
  sz+=nmemb;

  return nmemb;

}

/*void assert_field(const json_object *const j,const char *const k,const char *const v){
  
}*/

int main(){

  // printf("%s\n",curl_version());
  // assert(0==curl_global_init(CURL_GLOBAL_NOTHING));

  CURL *curl=curl_easy_init();
  assert(curl);

  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_callback));
  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_HEADER,0L));
  // assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_URL,"http://127.0.0.1:6170/proxies"));
  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_URL,"http://127.0.0.1:6170/proxies/GLOBAL"));
  assert(CURLE_OK==curl_easy_perform(curl));
  // printf("%.*s\n",(int)sz,buf);
  assert(NULL!=(buf=realloc(buf,++sz)));
  buf[sz-1]='\0';
  // printf("%s\n",buf);

  json_tokener *tok=json_tokener_new();
  assert(tok);
  json_object *jobj=json_tokener_parse_ex(tok,buf,-1);
  enum json_tokener_error jerr=json_tokener_get_error(tok);
  // printf("%s\n",json_tokener_error_desc(jerr));
  assert(jerr==json_tokener_success);
  json_tokener_free(tok);
  tok=NULL;

  json_object *p=NULL;
  assert(json_object_object_get_ex(jobj,"name",&p));
  assert(p);
  assert(json_type_string==json_object_get_type(p));
  assert(0==strcmp("GLOBAL",json_object_get_string(p)));
  // assert(1==json_object_put(p));
  p=NULL;

  // printf("%s\n",json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_PLAIN));
  // printf("%s\n",json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_PLAIN|JSON_C_TO_STRING_SPACED));
  // printf("%s\n",json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_PRETTY));

  assert(1==json_object_put(jobj));
  jobj=NULL;
  free(buf);
  buf=NULL;
  sz=0;
  curl_easy_cleanup(curl);
  curl=NULL;
  curl_global_cleanup();

  return 0;

}
