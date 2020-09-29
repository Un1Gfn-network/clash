#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proc/readproc.h>

#include "./proc.h"
#include "./privilege.h"

#define PROC_FLAGS ( \
  PROC_FILLCOM | \
  PROC_FILLSTAT | \
  PROC_EDITCMDLCVT | \
  0 \
)

static inline proc_t *inspect_proc(proc_t *proc){
  // printf(" %d %d %d %d | %s | %s\n",
  //   proc->tid,
  //   proc->pgrp,
  //   proc->tgid,
  //   proc->tpgid,
  //   proc->cmd,
  //   *(proc->cmdline)
  // );
  if(0==strcmp("clash",proc->cmd)){
    assert(
      proc->tid==proc->pgrp&&
      proc->pgrp==proc->tgid&&
      proc->tgid==proc->tpgid
    );
    ESCALATED(assert(0==kill(proc->tid,SIGINT)));
    printf("%d %s killed\n",proc->tid,*(proc->cmdline));
  }
  return proc;
}

void kill_clash(){

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
    freeproc(inspect_proc(*p));
  free(procs);
  procs=NULL;

}
