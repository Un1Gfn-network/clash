#include <assert.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#include "./curl.h"

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

const char *curl_get(const char *const url){

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

void curl_drop(){
  free(buf);
  buf=NULL;
  sz=0;
  curl_global_cleanup();
}
