#include <assert.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

#include <proc/readproc.h>

#include "./proc.h"
#include "./privilege.h"

#define PROC_FLAGS ( \
  PROC_FILLCOM | \
  PROC_FILLSTAT | \
  PROC_EDITCMDLCVT | \
  0 \
)

// https://gitlab.com/procps-ng/procps/-/issues/40
/*void freeproctab(proc_t** tab) {
    proc_t** p;
    for(p = tab; *p; p++)
       freeproc(*p);
    free(tab);
}*/

static inline proc_t *inspect_proc(proc_t *__restrict proc,const char *__restrict const name){
  // printf(" %d %d %d %d | %s | %s\n",
  //   proc->tid,
  //   proc->pgrp,
  //   proc->tgid,
  //   proc->tpgid,
  //   proc->cmd,
  //   *(proc->cmdline)
  // );
  if(0==strcmp(name,proc->cmd)){
    const int pid=proc->tid;
    assert(
      pid==proc->pgrp&&
      pid==proc->tgid&&
      pid==proc->tpgid
    );
    privilege_escalate();
    int pidfd=syscall(__NR_pidfd_open,pid,0);
    assert(pidfd>=0);
    printf("killing %d %s\n",pid,*(proc->cmdline));
    assert(0==syscall(__NR_pidfd_send_signal,pidfd,SIGINT,NULL,0));
    struct pollfd pollfd={
      .fd=pidfd,
      .events=POLLIN,
      .revents=0
    };
    for(;;){
      assert(0<=poll(&pollfd,1,-1));
      // printf("POLLIN(0x%x) = %d\n",POLLIN,(pollfd.revents&POLLIN));
      if(pollfd.revents&POLLIN)break;
      usleep(100000);
    }
    printf("killed\n");
    privilege_drop();
  }
  return proc;
}

void kill_sync(const char *__restrict const name){
  // PROCTAB *PT=openproc(PROC_FLAGS);
  // assert(PT);
  // proc_t *proc=NULL;
  // while(NULL!=(proc=readproc(PT,NULL))){
  //   inspect_proc(proc);
  //   freeproc(proc);
  //   proc=NULL;
  // }
  // closeproc(PT);
  // PT=NULL;
  proc_t **procs=readproctab(PROC_FLAGS);
  for(proc_t **p=procs;*p;++p)
    freeproc(inspect_proc(*p,name));
  free(procs);
  procs=NULL;
}
