#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <netinet/in.h> // INET_ADDRSTRLEN

#include "./def.h"
#include "./file.h"
#include "./ioctl.h"
#include "./netlink.h"
#include "./privilege.h"
#include "./proc.h"
#include "./profile.h"
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

static inline pid_t start_badvpn(){
  const pid_t f=fork();
  if(f>=1){
    // Parent branch
    // f is child pid
    assert(1000==geteuid());
    printf("starting badvpn-tun2socks %d\n",f);
    return f;
  }else{
    // Child branch
    assert(1000==geteuid());
    assert(f==0);
    // (2/2) Semaphore
    try_unlink(TUN_LOG);
    // int fd=create(TUN_LOG,0644);
    int pfd=open(TUN_LOG,O_CREAT|O_WRONLY|O_TRUNC,0644);
    // int pfd=open(TUN_LOG,O_CREAT|O_WRONLY|O_TRUNC|O_DIRECT|O_SYNC,0644);
    assert(pfd>=3);
    assert(0==close(STDOUT_FILENO));
    assert(STDOUT_FILENO==dup2(pfd,STDOUT_FILENO));
    assert(0==close(pfd));
    // execl(3)
    // execve(2)
    privilege_escalate();
    assert(-1!=execl(
      "/usr/bin/badvpn-tun2socks",
      "badvpn-tun2socks",
      "--tundev"           ,TUN             ,
      "--netif-ipaddr"     ,"10.0.0.2"      ,
      "--netif-netmask"    ,"255.255.255.0" ,
      "--socks-server-addr","127.0.0.1:1080",
      "--socks5-udp",
      (char*)NULL
    ));
    assert(0);
    // Should never reach here
    // privilege_drop();
    // exit(0);
  }
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

void read_r(){
  // (1/2) Semaphore
  // printf("? ");fflush(stdout);
  char s[SZ]={};
  assert(s==fgets(s,SZ,stdin));
}

int main(const int argc,const char **argv){
  privilege_drop();

  assert(
    argc==2 &&
    argv[1] &&
    (0==strcmp(argv[1],"rixcloud")||0==strcmp(argv[1],"ssrcloud"))
  );

  char *yaml_path=provider2path(argv[1]);
  char *server_title=current_server_title();
  printf("\'%s\'\n",server_title);
  yaml2profile(yaml_path,server_title);
  profile2json(server_title);
  free(server_title);
  free(yaml_path);
  server_title=NULL;
  yaml_path=NULL;

  // (1/3) DNS
  // bus_dns_set(DNS);

  // (2/3) Shadowsocks
  kill_sync("clash");
  assert(profile_loaded());
  assert(start_ss());

  // (3/3) TUN & route
  netlink_init();
  set();
  pid_t f=start_badvpn();

  // Impossible to know when badvpn-tun2socks becomes ready for SIGINT without inspecting its code
  sleep(1);
  read_r();

  // (3/3) TUN & route
  printf("killing badvpn-tun2socks %d\n",f);
  ESCALATED(assert(0==kill(f,SIGINT)));
  f=-1;
  reset();
  netlink_end();

  // (2/3) Shadowsocks
  stop_ss();

  // (1/3) DNS
  // bus_dns_reset();

  // assert(profile_loaded());
  // profile_clear();
  // assert(!profile_loaded());

  return 0;

}
