#include <assert.h>
#include <json.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h> // INET_ADDRSTRLEN

#include "./conf.h"
#include "./curl.h"
#include "./def.h"
#include "./proc.h"
#include "./ioctl.h"
#include "./netlink.h"
#include "./shadowsocks.h"
#include "./privilege.h"

char gw[INET_ADDRSTRLEN]={};

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

void set(){

  ioctl_tun_create(TUN);
  netlink_tun_addr(TUN,"10.0.0.1",24);
  netlink_up(TUN);

  netlink_get_gateway(gw);
  netlink_del_gateway(WLO,gw);
  netlink_add_gateway(TUN,"10.0.0.2");

  netlink_add_route(WLO,profile.remote_host,gw);

}

void reset(){

  netlink_del_route(WLO,profile.remote_host,gw);

  netlink_del_gateway(TUN,"10.0.0.2");
  netlink_add_gateway(WLO,gw);

  netlink_down(TUN);
  netlink_del_link(TUN);

}

// https://gitlab.com/procps-ng/procps/-/issues/40
/*void freeproctab(proc_t** tab) {
    proc_t** p;
    for(p = tab; *p; p++)
       freeproc(*p);
    free(tab);
}*/

int main(const int argc,const char **argv){

  // privilege_drop();

  // assert(
  //   argc==2 &&
  //   argv[1] &&
  //   (0==strcmp(argv[1],"rixcloud")||0==strcmp(argv[1],"ssrcloud"))
  // );

  // char *yaml_path=provider2path(argv[1]);
  // char *server_title=current_server_title();
  // printf("\'%s\'\n",server_title);
  // yaml2profile(yaml_path,server_title);
  // profile2json(server_title);
  // free(server_title);
  // free(yaml_path);
  // server_title=NULL;
  // yaml_path=NULL;

  // profile=(profile_t){
  //   .remote_host="42.157.192.81",
  //   .remote_port=16460,
  //   .local_addr="127.0.0.1",
  //   .local_port=1080,
  //   .password="5nJJ95sYf3b20HW3t72",
  //   .method="chacha20-ietf-poly1305",
  //   .fast_open=1,
  //   .mode=1,
  //   //
  //   .log=SS_LOG
  //   // .log="/dev/stdout"
  //   // .log="/dev/null"
  // };

  // netlink_init();
  // set();

  kill_clash();
  // if(start_ss()){
  //   printf("? ");fflush(stdout);
  //   fflush(stdout);
  //   char s[SZ]={};
  //   assert(s==fgets(s,SZ,stdin));
  //   stop_ss();
  // }

  // reset();
  // netlink_end();

  // clear_profile();

  return 0;

}
