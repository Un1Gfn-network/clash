// make clean && make shadowsocks.o && gcc shadowsocks.o -pthread -lshadowsocks-libev && ./a.out

// https://stackoverflow.com/a/7297011/
// #define _GNU_SOURCE
// #define __USE_GNU

#include <assert.h>
#include <pthread.h>
#include <shadowsocks.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "./profile.h"
#include "./def.h"
#include "./shadowsocks.h"

static pthread_t thread=0;

// https://stackoverflow.com/a/28904385/
static pthread_mutex_t mutex=PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

typedef enum {
  DOWN,
  FAIL,
  UP
} Status;
static Status status=DOWN;

// https://stackoverflow.com/q/16522858/
static void pthread_change_status(const Status s){
  (status==DOWN)?assert(s==UP||s==FAIL):assert(s==DOWN);
  pthread_mutex_lock(&mutex);
  status=s;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
}

static void pthread_wait_stat_change_from(const Status s0){
  assert(0==pthread_mutex_lock(&mutex));
  while(status==s0){
    assert(0==pthread_cond_wait(&cond,&mutex));
  }
  assert(status!=s0); // start_routine() sets status to either UP or FAIL
  assert(0==pthread_mutex_unlock(&mutex));
}

static void callback(int socks_fd, int udp_fd,void *data){
  assert(!data);
  assert(
    socks_fd>=3 &&
    udp_fd>=3 &&
    socks_fd!=udp_fd
  );
  assert(profile.log);
  printf("ss running, log \'%s\'\n",profile.log);
  pthread_change_status(UP);
}

static void *start_routine(void *arg){
  assert(!arg);
  assert(status==DOWN);
  printf("starting ss\n");
  // profile_inspect();
  if(-1==start_ss_local_server_with_callback(profile,callback,NULL)){
    // Failed
    pthread_change_status(FAIL);
    printf("ss failed\n");
  }else{
    // Succeeded, served and manually stopped
    pthread_wait_stat_change_from(DOWN);
    assert(status==UP);
  }
  return NULL;
}

bool start_ss(){
  (status==FAIL)?status=DOWN:0;
  assert(thread==0);
  assert(0==pthread_create(&thread,NULL,start_routine,NULL));
  assert(thread>=1);
  pthread_wait_stat_change_from(DOWN);
  switch(status){
    case UP:return true;break;
    case FAIL:return false;break;
    default:assert(0);break;
  }
}

void stop_ss(){
  assert(status==UP);
  assert(0==pthread_kill(thread,SIGINT));
  printf("ss interrupted\n");
  pthread_change_status(DOWN);
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
