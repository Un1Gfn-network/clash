#pragma once

#include <stdbool.h>
#include <shadowsocks.h> // profile_t

// Moved from clash_tun/def.h
#define YAML_PATH "/tmp/clash/config.yaml"

// yaml2profile.c
void yaml2profile(const bool,profile_t *__restrict const,const char *__restrict const,const char *__restrict const);
void profile_inspect();

// resolv.c
char *resolv(const char *__restrict domain);

// restful.c
char *now();
