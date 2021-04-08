#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*                 S
  U+0000  U+007F   0xxxxxxx
                   L        R
  U+0080  U+07FF   110xxxxx 10xxxxxx
                   L        R        R
  U+0800  U+FFFF   1110xxxx 10xxxxxx 10xxxxxx
                   L        R        R        R
  U+10000 U+10FFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

typedef enum {
  SINGLE,        // 0xxxxxxx
  MULTI_HEADER,  // 11110xxx // 1110xxxx // 110xxxxx
  MULTI_TRAILING // 10xxxxxx
} Class;

static inline Class classify(const unsigned char b, int *const n_R){
  if((b>>(8-1))==0b0/*xxxxxxx*/){*n_R=0      ;return SINGLE;}
  if((b>>(8-2))==0b10/*xxxxxx*/){assert(!n_R);return MULTI_TRAILING;}
  if((b>>(8-3))==0b110/*xxxxx*/){*n_R=1      ;return MULTI_HEADER;}
  if((b>>(8-4))==0b1110/*xxxx*/){*n_R=2      ;return MULTI_HEADER;}
  if((b>>(8-5))==0b11110/*xxx*/){*n_R=3      ;return MULTI_HEADER;}
  assert(false);
}

/*static inline bool isL(const unsigned har b){
  const Class c=classify(b);
  if(c==E_L_21||c==E_L_16||c==E_L_11)
    return true;
  else if(c==SINGLE||c==MULT_TRAILING)
    return false;
  assert(false);
  return false;
}*/

static inline long dumpfile(char **const bufp, const char *const filename){

  FILE *file=fopen(filename,"r");
  assert(file);

  assert(
    0==fseek(file,0,SEEK_END) &&
    0==feof(file) &&
    0==ferror(file)
  );
  const long len=ftell(file);

  static_assert(sizeof(char)==1);
  *bufp=calloc(len+1,sizeof(char));

  rewind(file);
  assert((long long)len==(long long)fread(*bufp,1,len,file));

  fclose(file);
  file=NULL;
  return len;

}

int main(){

  unsigned char *buf=NULL;
  const long len=dumpfile((char**)(&buf),"/home/darren/clash/clash_misc/flag.c");

  char *garbage=NULL;
  assert(len==asprintf(&garbage,"%s",buf));
  assert(garbage);
  free(garbage);

  long counter[5]={};
  int n_R=-1;
  long i=0;
  for(;i<len;++i,n_R=-1){
    switch(classify(buf[i],&n_R)){
    case SINGLE:
      assert(n_R==0);
      ++counter[n_R+1];
      break;
    case MULTI_HEADER:
      assert(1<=n_R&&n_R<=3);
      // for(long j=0;j<n_R;++j)assert(MULTI_TRAILING==classify(buf[++i],&(int){0}));
      for(long j=0;j<n_R;++j)assert(MULTI_TRAILING==classify(buf[++i],NULL));
      ++counter[n_R+1];
      break;
    default:
      assert(false);
    }
  }
  assert(i==len);

  free(buf);
  buf=NULL;

  printf("\n");
  printf("  1-byte  7-bit UTF-8 characters - %ld\n",counter[1]);
  printf("  2-byte 11-bit UTF-8 characters - %ld\n",counter[2]);
  printf("  3-byte 16-bit UTF-8 characters - %ld\n",counter[3]);
  printf("  4-byte 21-bit UTF-8 characters - %ld\n",counter[4]);
  printf("\n");

}
