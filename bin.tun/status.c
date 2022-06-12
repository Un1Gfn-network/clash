#include <assert.h>
#include <pthread.h>

#include "./status.h"

// https://stackoverflow.com/a/7297011/
// #define _GNU_SOURCE
// #define __USE_GNU

static Status status=DOWN;

// https://stackoverflow.com/a/28904385/
static pthread_mutex_t mutex=PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

void status_init(){
  assert(0==pthread_mutex_trylock(&mutex));
  status=DOWN;
  assert(0==pthread_cond_destroy(&cond));
  assert(0==pthread_cond_init(&cond,NULL));
  assert(0==pthread_mutex_unlock(&mutex));
}

// https://stackoverflow.com/q/16522858/
void status_change_to(const Status s){
  assert(0==pthread_mutex_lock(&mutex));
  assert(status!=s);
  status=s;
  pthread_cond_signal(&cond);
  assert(0==pthread_mutex_unlock(&mutex));
}

// https://stackoverflow.com/q/16522858/
void status_change_from_to(const Status s0,const Status s1){
  assert(0==pthread_mutex_lock(&mutex));
  assert(status==s0);
  assert(s0!=s1);
  status=s1;
  pthread_cond_signal(&cond);
  assert(0==pthread_mutex_unlock(&mutex));
}

Status status_wait_change_from(const Status s0){
  assert(0==pthread_mutex_lock(&mutex));
  while(status==s0){
    assert(0==pthread_cond_wait(&cond,&mutex));
  }
  assert(status!=s0); // start_routine() sets status to either UP or FAIL
  Status ret=status;
  assert(0==pthread_mutex_unlock(&mutex));
  return ret;
}
