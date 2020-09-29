#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h> // INET_ADDRSTRLEN

#include "./profile.h"
#include "./def.h"
#include "./ioctl.h"
#include "./netlink.h"
#include "./privilege.h"
#include "./proc.h"
#include "./restful.h"
#include "./shadowsocks.h"

char gw[INET_ADDRSTRLEN]={};

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

int main(const int argc,const char **argv){

  privilege_drop();

  assert(
    argc==2 &&
    argv[1] &&
    (0==strcmp(argv[1],"rixcloud")||0==strcmp(argv[1],"ssrcloud"))
  );

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

  char *yaml_path=provider2path(argv[1]);
  char *server_title=current_server_title();
  printf("\'%s\'\n",server_title);
  yaml2profile(yaml_path,server_title);
  profile2json(server_title);
  free(server_title);
  free(yaml_path);
  server_title=NULL;
  yaml_path=NULL;

  kill_clash();
  assert(profile_loaded());
  assert(start_ss());

  netlink_init();
  set();

  printf("? ");fflush(stdout);
  fflush(stdout);
  char s[SZ]={};
  assert(s==fgets(s,SZ,stdin));
  stop_ss();

  reset();
  netlink_end();

  assert(profile_loaded());
  profile_clear();
  assert(!profile_loaded());

  return 0;

}
