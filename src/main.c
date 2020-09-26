#include <assert.h>
#include <json.h>
#include <stdio.h>
#include <string.h>
// #include <curl/curl.h>
// #include <stdbool.h>
// #include <stdlib.h>

// curl.c
const char *curl_get(const char *const url);
void curl_drop();

// conf.c
void clear_profile();
void yaml2profile(const char *const from_yaml,const char *const server_title);
void profile2json();

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

static inline char *current_server_title(){

  // Parse buf, not a file
  json_tokener *const tok=json_tokener_new();
  assert(tok);

  json_tokener_reset(tok);
  json_object *jobj=json_tokener_parse_ex(tok,curl_get("http://127.0.0.1:6170/proxies/GLOBAL"),-1);
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

static inline char *provider2path(const char *const provider){
  assert(provider);
  const char *l="/home/darren/.clash/";
  const char *r="/config.yaml";
  char *ret=calloc((strlen(l)+strlen(r)+strlen(provider)+1),1);
  strcat(ret,l);
  strcat(ret,provider);
  strcat(ret,r);
  return ret;
}

int main(const int argc,const char **argv){

  assert(
    argc==2 &&
    argv[1] &&
    (0==strcmp(argv[1],"rixcloud")||0==strcmp(argv[1],"ssrcloud"))
  );

  char *server_title=current_server_title();
  printf("%s\n",server_title);
  char *yaml_path=provider2path(argv[1]);

  yaml2profile(yaml_path,server_title);

  profile2json();

  clear_profile();
  free(server_title);
  free(yaml_path);
  server_title=NULL;
  yaml_path=NULL;

  return 0;

}
