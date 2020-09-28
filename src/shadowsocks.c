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

#include "./shadowsocks.h"
#include "./def.h"

// https://stackoverflow.com/a/28904385/
pthread_mutex_t mutex=PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

bool started=false;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

const int which=1;
// const int which=0;

// profile_t profile={};
profile_t profile={
  .remote_host="42.157.192.81",
  .remote_port=16460,
  .local_addr="127.0.0.1",
  .local_port=1080,
  .password="5nJJ95sYf3b20HW3t72",
  .method="chacha20-ietf-poly1305",
  .fast_open=1,
  .mode=1,
  // 
  .log=SS_LOG
  // .log="/dev/stdout"
  // .log="/dev/null"
};

static void callback(int socks_fd, int udp_fd,void *data);
static void *start_routine(void *arg){
  assert(!arg);
  printf("starting ss\n");
  assert(-1!=start_ss_local_server_with_callback(profile,callback,NULL));
  return NULL;
}

static void sigint_ignore(){
  struct sigaction oldact={};
  assert(0==sigaction(SIGINT,&(struct sigaction){.sa_handler=SIG_IGN},&oldact));
  assert(oldact.sa_handler==SIG_DFL);
}

static void callback(int socks_fd, int udp_fd,void *data){
  assert(!data);
  assert(
    socks_fd>=3 &&
    udp_fd>=3 &&
    socks_fd!=udp_fd
  );
  which?sleep(1):0;
  printf("ss started, log \'%s\'\n",SS_LOG);

  pthread_mutex_lock(&mutex);
  started=true;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

}

int main(){

  // printf("pid %d\n",getpid());

  // sigint_ignore();

  pthread_t thread=-1;
  assert(0==pthread_create(&thread,NULL,start_routine,NULL));
  assert(thread>=1);
  // printf("thread #%lu\n",thread);

  which?0:sleep(1);

  // while(!started){}
  // assert(0==pthread_kill(thread,SIGINT));
  // printf("ss interrupted\n");

  assert(0==pthread_mutex_lock(&mutex));
  if(!started)
    assert(0==pthread_cond_wait(&cond,&mutex));
  assert(0==pthread_kill(thread,SIGINT));
  printf("ss interrupted\n");
  assert(0==pthread_mutex_unlock(&mutex));

  return 0;

}
