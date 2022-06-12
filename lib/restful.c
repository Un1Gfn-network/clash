#include <assert.h>
#include <curl/curl.h>
#include <json.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <libclash.h> // Header of resolv.c restful.c yaml2profile.c

// https://gcc.gnu.org/onlinedocs/gcc-4.8.5/cpp/Stringification.html
#define xstr(a) str(a)
#define str(a) #a
#define json_object_put2(J) assert(1==json_object_put(J));J=NULL

#define SZ 1024

static_assert(9090==RESTFUL_PORT);

static char *buf=NULL;
static size_t sz=0;

static size_t write_callback(char *__restrict ptr, size_t size, size_t nmemb, void *__restrict userdata){
  // https://curl.se/libcurl/c/CURLOPT_WRITEDATA.html#DEFAULT
  // By default, this is a FILE * to stdout.
  assert(stdout==userdata);
  assert(size==1);
  assert(nmemb>=1);
  // eprintf("new segment\n");
  assert(NULL!=(buf=realloc(buf,sz+nmemb)));
  memcpy(buf+sz,ptr,nmemb);
  sz+=nmemb;
  return nmemb;
}

static inline const char *buf_get(const char *__restrict const url){

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

static inline void buf_drop(){
  free(buf);
  buf=NULL;
  sz=0;
}

#define field_string_assert(J,K,V) assert(0==strcmp(V,field_string_get(J,K)))
static inline const char *field_string_get(const json_object *__restrict const j,const char *__restrict const k){
  assert(j&&k&&strlen(k));
  json_object *p=json_object_object_get(j,k);assert(p);
  assert(json_type_string==json_object_get_type(p));
  // printf("%s\n",json_type_to_name(json_object_get_type(p)));
  const char *const s=json_object_get_string(p);
  assert(s&&strlen(s));
  return s;
}

// static inline int field_int_get(const json_object *const j,const char *const k){
//   json_object *p=NULL;
//   assert(json_object_object_get_ex(j,k,&p));
//   assert(p);
//   json_type t=json_object_get_type(p);
//   if(t!=json_type_int){
//     printf("%s\n",json_type_to_name(t));
//     assert(0);
//   }
//   return json_object_get_int(p);
// }

static inline json_object *json_get(const char *__restrict const s){
  if(s){
    CURL *c=curl_easy_init();assert(c);
    char *ss=curl_easy_escape(c,s,0);
    curl_easy_cleanup(c);c=NULL;
    assert(ss&&strlen(ss));

    // char t[SZ]={};
    // sprintf(t,"http://127.0.0.1:%u/proxies/%s",RESTFUL_PORT,ss);
    // curl_free(ss);ss=NULL;
    // // sprintf(t,"http://127.0.0.1:%u/proxies/%s",RESTFUL_PORT,s);
    // // assert(0==strcmp("400 Bad Request",buf));
    // buf_get(t);

    // https://stackoverflow.com/a/21884334
    char *t=NULL;
    size_t sizeloc=-1;
    FILE *f=open_memstream(&t,&sizeloc);
    assert(f);
    fprintf(f,"http://127.0.0.1:%u/proxies/%s",RESTFUL_PORT,ss);
    curl_free(ss);ss=NULL;
    fflush(f);
    fclose(f);
    buf_get(t);
    free(t);t=NULL;
    sizeloc=0;

  }else{
    buf_get("http://127.0.0.1:"xstr(RESTFUL_PORT)"/proxies/GLOBAL");
  }
  json_object *j=NULL;
  assert(NULL!=(j=json_tokener_parse(buf))&&json_type_object==json_object_get_type(j));
  buf_drop();
  return j;
}


/* RESTful look up for current node recursively (temporary section)

https://www.url-encode-decode.com/
https://curl.se/libcurl/c/curl_easy_escape.html
https://curl.se/libcurl/c/curl_easy_unescape.html

    STR="$(curl -s http://127.0.0.1:9090/proxies/GLOBAL | jq -r ".now|@uri")"
    OBJ="$(curl -s http://127.0.0.1:9090/proxies/"$STR")"
    [ "$STR" = "$(jq <<<"$OBJ" -r ".name|@uri")" ] || echo "err"
    case "$(jq <<<"$OBJ" -r .type)" in
    Shadowsocks)
      ;;
    Selector)
      STR="$(jq <<<"$OBJ" -r ".now|@uri")"
      ;;
    *)
      echo "err"
      ;;
    esac
    echo "$STR"
    alias urldecode='python -c "import urllib.parse, sys; print(urllib.parse.unquote(sys.argv[1] if len(sys.argv) > 1 else sys.stdin.read()[0:-1]))"'
    urldecode "$STR"
*/

// char* instead of const char*
// strdup() after json_object_put() is required
char *now(){

  json_object *j=NULL;
  char *s=NULL;

  j=json_get(NULL);
  // printf("%s\n",json_object_to_json_string_ext(j,JSON_C_TO_STRING_PLAIN|JSON_C_TO_STRING_SPACED));
  // printf("%s\n",json_object_to_json_string_ext(j,JSON_C_TO_STRING_PRETTY));
  field_string_assert(j,"name","GLOBAL");
  field_string_assert(j,"type","Selector");
  s=strdup(field_string_get(j,"now"));
  json_object_put2(j);

  j=json_get(s);
  field_string_assert(j,"name",s);
  if(0!=strcmp("Shadowsocks",field_string_get(j,"type"))){
    field_string_assert(j,"type","Selector");
    printf("group (%s)\n",s);
    free(s);s=NULL;
    s=strdup(field_string_get(j,"now"));
    json_object_put2(j);
    // Optional
    j=json_get(s);
    field_string_assert(j,"name",s);
    field_string_assert(j,"type","Shadowsocks");
  }
  json_object_put2(j);

  return s;

}
