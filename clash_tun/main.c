#include <assert.h>
#include <curl/curl.h> // curl_global_init() curl_global_cleanup()
#include <fcntl.h> // open()
#include <netinet/in.h> // INET_ADDRSTRLEN
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> // free()
#include <unistd.h> // fork() getuid() close() STDOUT_FILENO dup2() execl() sleep

#include <libclash.h>

#include "./bus.h"
#include "./def.h"
#include "./file.h"
#include "./ioctl.h"
#include "./netlink.h"
#include "./privilege.h"
#include "./proc.h"
#include "./profile.h"
#include "./shadowsocks2.h"

char gw[INET_ADDRSTRLEN]={};

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
    // https://stackoverflow.com/a/1777294/
    assert(1000==geteuid());
    assert(f==0);
    try_unlink(TUN_LOG);
    printf("badvpn-tun2socks running, log \'%s\'\n",TUN_LOG);
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
  printf("replacing gateway %s\n",gw);
  netlink_del_gateway(WLO,gw);
  netlink_add_gateway(TUN,"10.0.0.2");

  netlink_add_route(WLO,profile.remote_host,gw);

}

void reset(){

  netlink_del_route(WLO,profile.remote_host,gw);

  netlink_del_gateway(TUN,"10.0.0.2");
  printf("recovering gateway %s\n",gw);
  netlink_add_gateway(WLO,gw);

  netlink_down(TUN);
  netlink_del_link(TUN);

}

void read_r(){
  printf("<Press Enter to Terminate> ");fflush(stdout);
  char s[SZ]={};
  // sleep(1);
  assert(s==fgets(s,SZ,stdin));
}

/*int main(){
  netlink_init();
  netlink_get_gateway(gw);
  printf("%s\n",gw);
  netlink_end();
}*/

int main(const int argc,const char **argv){

  // Required by now() and resolv()
  assert(0==curl_global_init(CURL_GLOBAL_NOTHING));

  // char *s=now();
  // puts(s);
  // free(s);s=NULL;
  // // free(now());
  // exit(0);

  privilege_drop();

  assert(argc==1&&argv[1]==NULL);

  // Get current active node from clash RESTful API
  char *name=now();
  printf("\'%s\'\n",name);

  assert(!profile_loaded());
  yaml2profile(true,&profile,YAML_PATH,name);
  assert(profile_loaded());
  // profile_inspect();
  profile_to_json(name);
  free(name);name=NULL;

  // (1/3) DNS
  bus_init();
  bus_call(&f_setdns);
  bus_call(&f_flush);

  // (2/3) Shadowsocks
  kill_sync("clash");
  assert(profile_loaded());
  assert(start_ss());

  // (3/3) TUN & route
  netlink_init();
  set();
  pid_t f=start_badvpn();

  // Impossible to know when badvpn-tun2socks becomes ready for SIGINT without inspecting its code
  // status_*() doesn't work between processes
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
  bus_call(&f_resetdns);
  bus_call(&f_flush);
  bus_end();

  // assert(profile_loaded());
  // profile_clear();
  // assert(!profile_loaded());

  curl_global_cleanup();

  return 0;

}
