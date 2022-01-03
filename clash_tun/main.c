#include <assert.h>
#include <curl/curl.h> // curl_global_init() curl_global_cleanup()
#include <errno.h> // errno
#include <fcntl.h> // open()
#include <netinet/in.h> // INET_ADDRSTRLEN
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> // free()
#include <unistd.h> // fork() getuid() close() STDOUT_FILENO dup2() execl() sleep

#include <libclash.h>

#include "./bus.h"
#include "./def.h"
#include "./ioctl.h"
#include "./netlink.h"
#include "./privilege.h"
#include "./proc.h"
#include "./profile.h"
#include "./shadowsocks2.h"

static char gw[INET_ADDRSTRLEN]={};

static inline pid_t start_tun2socks(){
  const pid_t pid=fork();
  if(pid>=1){
    /**********************************
     * this branch runs in the parent *
     * this pid belongs to the child  *
     **********************************/
    assert(1000==geteuid());
    printf("starting tun2socks with pid %d\n",pid);
    return pid;
  }else{
    /*********************************
     * this branch runs in the child *
     * this pid is dummy             *
     *********************************/
    // https://stackoverflow.com/a/1777294/
    assert(1000==geteuid());
    assert(pid==0);
    printf("badvpn-tun2socks running, log \'%s\'\n",TUN_LOG);
    // FILE *freopen(const char *restrict pathname, const char *restrict mode,FILE *restrict stream);
    // int fd=create(TUN_LOG,0644);
    int pfd=open(TUN_LOG,O_CREAT|O_WRONLY|O_TRUNC/*|O_DIRECT|O_SYNC*/,0644);
    assert(pfd>=3);
    assert(0==close(STDOUT_FILENO));
    assert(STDOUT_FILENO==dup2(pfd,STDOUT_FILENO));
    assert(0==close(pfd));
    // execl(3)
    // execve(2)
    privilege_escalate();
    assert(-1!=execl(
      "/usr/bin/tun2socks", // executable
      "tun2socks", // "$0"
      "-device"  ,"tun://"TUN,
      "-loglevel","info",
      "-proxy"   ,"socks5://127.0.0.1:1080",
      (char*)NULL
    ));
    // Should never reach here
    assert(0);
    // privilege_drop();
    // exit(0);
  }
}

static inline void set(){

  ioctl_tun_create(TUN);
  netlink_tun_addr(TUN,"10.0.0.1",24);
  netlink_up(TUN);

  netlink_get_gateway(gw);
  printf("replacing gateway %s with %s\n",gw,"10.0.0.2");
  netlink_del_gateway(WLO,gw);
  netlink_add_gateway(TUN,"10.0.0.2");

  netlink_add_route(WLO,profile.remote_host,gw);

}

static inline void reset(){

  netlink_del_route(WLO,profile.remote_host,gw);

  netlink_del_gateway(TUN,"10.0.0.2");
  printf("recovering gateway %s\n",gw);
  netlink_add_gateway(WLO,gw);

  netlink_down(TUN);
  netlink_del_link(TUN);

}

static inline void read_r(){
  printf("<Press Enter to Terminate> ");fflush(stdout);
  char s[SZ]={};
  // sleep(1);
  assert(s==fgets(s,SZ,stdin));
}

static inline void unlink_nofail(const char *__restrict const f){
   const int i=unlink(f);
   if(i==-1){
     assert(errno=ENOENT);
   }else{
     assert(i==0);
     printf("removed \'%s\'\n",f);
   }
 }

/*int main(){
  netlink_init();
  netlink_get_gateway(gw);
  printf("%s\n",gw);
  netlink_end();
}*/

int main(const int argc,const char **__restrict argv){

  printf("\e]0;%s\a","clash_tun");fflush(stdout);

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
  // profile_inspect(&profile);
  unlink_nofail(SS_LOCAL_JSON);
  profile_to_json(name);
  free(name);name=NULL;

  // (1/3) DNS
  bus_init();
  bus_call(&f_setdns);
  bus_call(&f_flush);

  // (2/3) Shadowsocks
  kill_sync("clash");
  assert(profile_loaded());
  unlink_nofail(SS_LOG);
  assert(start_ss());

  // (3/3) TUN & route
  netlink_init();
  set();
  unlink_nofail(TUN_LOG);
  printf("\e]0;%s\a","clash_tun - tun2socks");fflush(stdout);
  // puts("please invoke tun2socks manually");
  pid_t pid=start_tun2socks();

  // Impossible to know when badvpn-tun2socks becomes ready for SIGINT without inspecting its code
  // status_*() doesn't work between processes
  sleep(1);
  read_r();

  // (3/3) TUN & route
  // printf("please make sure tun2socks is killed, then press <ENTER> ");fflush(stdout);
  // getchar();
  printf("killing tun2socks %d\n",pid);
  ESCALATED(assert(0==kill(pid,SIGINT)));
  pid=-1;
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
