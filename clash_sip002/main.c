#include <assert.h>
#include <libclash.h>
#include <openssl/evp.h> // EVP_EncodeBlock()
#include <string.h>
#include <unistd.h> // fork()
#include <sys/stat.h> // stat()
#include <sys/stat.h> // stat()
#include <qrencode.h>
#include <curl/curl.h> // curl_easy_escape()
#include <readline/readline.h> // readline()

#define SZ 1024

// libqrencode+Unicode

// https://github.com/fukuchi/libqrencode/blob/77de38effc5128cdbf4f765923dafb81f3694276/qrenc.c#L833
// 1 = F = white = solid
// 0 = E = black = whitespace
//     //    //   //     //  //              // Upper Lower
static const char *const F = " ";            //   1     1
static const char *const U = "\342\226\204"; //   1     0
static const char *const D = "\342\226\200"; //   0     1
static const char *const E = "\342\226\210"; //   0     0

static const int margin=4;

static char *SS_URI=NULL;
static QRcode *q=NULL;

// const unsigned char (*a)[q->width]=((unsigned char(*)[q->width])(q->data));
// #define A(Q) ((unsigned char(*)[Q->width])(Q->data))
#define A ((unsigned char(*)[q->width])(q->data))

#define qr_unicode_lone_head(R) qr_unicode_lone(false)
#define qr_unicode_lone_tail(R) qr_unicode_lone(true)
static inline void qr_unicode_lone(const bool head){
  for(int c=0;c<margin;++c)printf(E);
  if(head)for(int c=0;c<(q->width);++c)printf("%s",((A[0][c])&1u)?         U:E);
  else    for(int c=0;c<(q->width);++c)printf("%s",((A[q->width-1][c])&1u)?D:E);
  for(int c=0;c<margin;++c)printf(E);
  puts("");
}

static inline void qr_unicode_duo_empty(){
  for(int c=0;c<margin+(q->width)+margin;++c)
    printf("%s",E);
  puts("");
}

static inline void qr_unicode_duo(const int r0,const int r1){
  assert( 0<=r0 && r0+1==r1 && r1<=(q->width)-1 );
  for(int c=0;c<margin;++c)printf(E);
  for(int c=0;c<(q->width);++c){
    switch( ((A[r0][c]&1u)<<4) | (A[r1][c]&1u) ){
      case 0x11: printf("%s",F); break;
      case 0x10: printf("%s",U); break;
      case 0x01: printf("%s",D); break;
      case 0x00: printf("%s",E); break;
      default:   assert(0); break;
    }
  }
  for(int c=0;c<margin;++c)printf(E);
  puts("");
}

static inline void qr_unicode(){
  printf("m=%d w=%d\n",margin,q->width);
  for(int r=0;r<margin/2;++r)
    qr_unicode_duo_empty();
  if(margin%2){
    qr_unicode_lone_head();
    for(int rr=0;rr<(((q->width)-1)/2);++rr) qr_unicode_duo(2*rr+1,2*rr+2);
    if(((q->width)-1)%2)                     qr_unicode_lone_tail();
  }else{
    for(int rr=0;rr<((q->width)/2);    ++rr) qr_unicode_duo(2*rr,2*rr+1);
    if((q->width)%2)                         qr_unicode_lone_tail();
  }
  for(int r=0;r<(margin+(margin+(q->width)+1)%2)/2;++r)
    qr_unicode_duo_empty();
}

// execl+qrencode
static inline void qr_execl(const char *const uri){
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
// static inline void qr_fbdev(){}

// libqrencode+BMP
// https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
// static inline void qr_bmp(){}

// libqrencode+SDL+OpenGL
// static inline void qr_opengl(){}

// gnutls_base64_encode2(3)
// gnutls_pem_base64_encode(3)
// gnutls_pem_base64_encode2(3)
// EVP_ENCODEINIT(3) EVP_EncodeBlock()
// https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
// https://developer.gnome.org/glib/stable/glib-Base64-Encoding.html
// https://stackoverflow.com/questions/5288076/base64-encoding-and-decoding-with-openssl
static inline void base64encode(char *const dest,const char *const src,const int n){

  EVP_ENCODE_CTX *ctx=EVP_ENCODE_CTX_new();assert(ctx);
  EVP_EncodeInit(ctx);

  const int r=EVP_EncodeBlock((unsigned char*)dest,(const unsigned char*)src,n);
  assert(r>=4&&r%4==0);

  assert(0==EVP_ENCODE_CTX_num(ctx));
  EVP_ENCODE_CTX_free(ctx);ctx=NULL;

}

int main(){

  curl_global_init(CURL_GLOBAL_NOTHING); // yaml2profile() curl_easy_escape()

  char *name=now();
  profile_t p={};
  yaml2profile(false,&p,YAML_PATH,name);
  puts(name);
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

  // Tag (remark)
  CURL *c=curl_easy_init();assert(c);
  // printf("tag, leave empty to use node name: ");fflush(stdout);
  // size_t n=-1;
  // const ssize_t r=getline(&tag,&n,stdout);
  char *tag=readline("tag (leave empty to use node name): ");assert(tag);
  if('\0'==tag[0]){
    // Empty input - use node name as tag
    free(tag);tag=NULL;
    tag=curl_easy_escape(c,name,0);
  }else{
    // Custom tag
    char *tmp=tag;
    tag=curl_easy_escape(c,tag,0);
    free(tmp);tmp=NULL;
  }
  free(name);name=NULL;
  curl_easy_cleanup(c);c=NULL;
  assert(tag&&strlen(tag));

  // "ss://" userinfo "@" hostname ":" port [ "/" ] [ "?" plugin ] [ "#" tag ]
  asprintf(&SS_URI,"ss://%s@%s:%d#%s",
    userinfo,
    p.remote_host,
    p.remote_port,
    tag);
  free(tag);tag=NULL;
  puts(SS_URI);

  // A
  // qr_execl();

  // B
  q=QRcode_encodeString(SS_URI, 0, QR_ECLEVEL_L /*QR_ECLEVEL_H*/, QR_MODE_8, 1);
  printf("v%d\n",q->version);
  assert(5<=q->version);
  // printf("%d\n",q->width);
  //
  qr_unicode();
  // qr_fbdev();
  // qr_bmp();
  // qr_opengl();
  //
  QRcode_free(q);q=NULL;

  free(SS_URI);SS_URI=NULL;
  free(p.remote_host);p.remote_host=NULL;
  free(p.method);p.method=NULL;
  free(p.password);p.password=NULL;

  curl_global_cleanup();

  return 0;

}
 