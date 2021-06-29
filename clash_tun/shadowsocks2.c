// make clean && make shadowsocks.o && gcc shadowsocks.o -pthread -lshadowsocks-libev && ./a.out

#include <assert.h>
#include <pthread.h>
#include <shadowsocks.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "./def.h"
#include "./file.h"
#include "./profile.h"
#include "./shadowsocks2.h"
#include "./status.h"

static pthread_t thread=0;

static void callback(int socks_fd, int udp_fd,void *data){
  assert(!data);
  assert(
    socks_fd>=3 &&
    udp_fd>=3 &&
    socks_fd!=udp_fd
  );
  assert(profile.log);
  printf("ss running, log \'%s\'\n",profile.log);
  status_change_to(UP);
}

static void *start_routine(void *arg){
  assert(!arg);
  try_unlink(SS_LOG);
  printf("starting ss\n");

  // Show profile w/ lots of printf()
  // profile_inspect();

  if(-1==start_ss_local_server_with_callback(profile,callback,NULL)){
    // Failed
    status_change_from_to(DOWN,FAIL);
    printf("ss failed\n");
  }else{
    // Succeeded, served and manually stopped
    assert(UP==status_wait_change_from(DOWN));
  }
  return NULL;
}

bool start_ss(){
  assert(thread==0);
  status_init();
  assert(0==pthread_create(&thread,NULL,start_routine,NULL));
  assert(thread>=1);
  switch(status_wait_change_from(DOWN)){
    case UP:return true;break;
    case FAIL:return false;break;
    default:assert(0);break;
  }
}

void stop_ss(){
  status_change_from_to(UP,DOWN);
  assert(0==pthread_kill(thread,SIGINT));
  printf("ss interrupted\n");
}

/*static void sigint_ignore(){
  struct sigaction oldact={};
  assert(0==sigaction(SIGINT,&(struct sigaction){.sa_handler=SIG_IGN},&oldact));
  assert(oldact.sa_handler==SIG_DFL);
}*/

/*int main(){

  // printf("pid %d\n",getpid());
  // sigint_ignore();

  start_ss();

  stop_ss();

  return 0;

}*/
