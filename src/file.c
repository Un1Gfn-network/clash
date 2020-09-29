#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "./file.h"

void try_unlink(const char *const f){
  const int i=unlink(f);
  if(i==-1){
    assert(errno=ENOENT);
  }else{
    assert(i==0);
    printf("removed \'%s\'\n",f);
  }
}
