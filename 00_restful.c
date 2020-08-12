#include <assert.h>
#include <curl/curl.h>
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

int main(){

  // printf("%s\n",curl_version());
  // assert(0==curl_global_init(CURL_GLOBAL_NOTHING));

  CURL *curl=curl_easy_init();
  assert(curl);

  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_callback));
  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_HEADER,0L));
  assert(CURLE_OK==curl_easy_setopt(curl,CURLOPT_URL,"http://127.0.0.1:6170/proxies"));
  assert(CURLE_OK==curl_easy_perform(curl));

  printf("%.*s\n",(int)sz,buf);
  free(buf);
  buf=NULL;
  sz=0;

  curl_easy_cleanup(curl);
  curl=NULL;

  curl_global_cleanup();
  return 0;

}
