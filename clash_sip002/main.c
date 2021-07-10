#include <assert.h>
#include <libclash.h>
#include <openssl/evp.h>
#include <string.h>
#include <unistd.h> // fork()
#include <sys/stat.h> // stat()

#define SZ 1024

static inline void base64encode(char *const dest,const char *const src,const int n){

  EVP_ENCODE_CTX *ctx=EVP_ENCODE_CTX_new();assert(ctx);
  EVP_EncodeInit(ctx);

  const int r=EVP_EncodeBlock((unsigned char*)dest,(const unsigned char*)src,n);
  assert(r>=4&&r%4==0);

  assert(0==EVP_ENCODE_CTX_num(ctx));
  EVP_ENCODE_CTX_free(ctx);ctx=NULL;

}

// execl+qrencode
static inline void qr(const char *const uri){
  // system();
  pid_t i=fork();
  if(0==i){
    static const char *const bin="/usr/bin/qrencode";
    struct stat statbuf={};
    // stat(2)
    assert(0==stat(bin,&statbuf));
    assert(!statbuf.st_uid);
    assert(!statbuf.st_gid);
    // inode(7)
    assert( (statbuf.st_mode&S_IROTH));
    assert(!(statbuf.st_mode&S_IWOTH));
    assert( (statbuf.st_mode&S_IXOTH));
    // warning: missing sentinel in function call
    execl(bin,bin,"-tUTF8",uri,(char*)NULL);
    // Should not reach here
    assert(0);
  }else{
    assert(2<=i);
  }
}

// libqrencode+fbdev
// https://gist.github.com/Un1Gfn/59998ce82e1fc0e53519c5a676f63716
// static inline void qr(const char *const uri){}

// libqrencode+BMP
// https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
// static inline void qr(const char *const uri){}

// libqrencode+Unicode
// https://github.com/fukuchi/libqrencode/blob/77de38effc5128cdbf4f765923dafb81f3694276/qrenc.c#L833
// static inline void qr(const char *const uri){
//   static const char *const E = " ";
//   static const char *const L = "\342\226\204";
//   static const char *const U = "\342\226\200";
//   static const char *const F = "\342\226\210";

// }

// libqrencode+SDL+OpenGL
// static inline void qr(const char *const uri){}

int main(){

  char *name=now();
  profile_t p={};
  yaml2profile(false,&p,YAML_PATH,name);
  // profile_inspect(&p);

  // method ':' password '\0'
  const size_t sz=strlen(p.method)+1+strlen(p.password)+1;
  char userinfo_raw[sz];
  bzero(userinfo_raw,sz);
  sprintf(userinfo_raw,"%s:%s",p.method,p.password);
  assert(userinfo_raw[sz-1]=='\0');
  assert(userinfo_raw[sz-2]!='\0');
  char userinfo[((sz-1)+2)/3*4+1];
  base64encode(userinfo,userinfo_raw,sz-1);
  puts(userinfo_raw);
  puts(userinfo);

  // "ss://" userinfo "@" hostname ":" port [ "/" ] [ "?" plugin ] [ "#" tag ]
  char *SS_URI=NULL;
  asprintf(&SS_URI,"ss://%s@%s:%d#%s",
    userinfo,
    p.remote_host,
    p.remote_port,
    // name);
    "test");
  free(name);name=NULL;

  puts(SS_URI);
  qr(SS_URI);
  free(SS_URI);SS_URI=NULL;

  free(p.remote_host);p.remote_host=NULL;
  free(p.method);p.method=NULL;
  free(p.password);p.password=NULL;

  return 0;

}
