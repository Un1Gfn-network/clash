#include <assert.h>
#include <libclash.h>
#include <openssl/evp.h>

#define SZ 1024

void base64encode(){

  EVP_ENCODE_CTX *ctx=EVP_ENCODE_CTX_new();
  assert(ctx);
  EVP_EncodeInit(ctx);

  char t[SZ]={};
  const int r=EVP_EncodeBlock((unsigned char*)t,(const unsigned char*)"howdy",5);
  assert(r>=4&&r%4==0);
  puts(t);

  assert(0==EVP_ENCODE_CTX_num(ctx));
  EVP_ENCODE_CTX_free(ctx);ctx=NULL;

}

int main(){

  // char *now=now();

  // yaml2profile(YAML_PATH,now);

  // free(now);now=NULL;

  // base64encode();
  return 0;

}
