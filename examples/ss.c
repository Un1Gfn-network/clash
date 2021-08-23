// gcc -Wall -Wextra ss.c -lshadowsocks-libev

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <shadowsocks.h>

#define LOG "/tmp/ss-local.log"

#undef SECRETSECRETSECRET

/*

ss-local \
-s SECRETSECRETSECRET \
-b "0.0.0.0" \
-m "chacha20-ietf-poly1305" \
-k SECRETSECRETSECRET \
-p SECRETSECRETSECRET \
-l 1080 \
-t 60 \
-v

*/

static profile_t profile={

  .remote_host = SECRETSECRETSECRET,
  .local_addr  = "0.0.0.0",
  .method      = "chacha20-ietf-poly1305",
  .password    = SECRETSECRETSECRET,
  .remote_port = SECRETSECRETSECRET,
  .local_port  = 1080,
  .timeout     = 60,

  // .acl       = NULL,
  // .log       = LOG,
  // .fast_open = 1,
  // .mode      = 1,
  // .mtu       = 0,
  // .mptcp     = 0,
  // .verbose   = 1

};

int main(){

  puts("starting...");
  puts("tail -f "LOG);

  assert(0==start_ss_local_server(profile));

  // assert(false);
  return 0;

}
