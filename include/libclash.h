#pragma once

#include <stdbool.h>
#include <shadowsocks.h> // profile_t

// Moved from clash_tun/def.h
#define YAML_PATH "/tmp/clash/config.yaml"

// yaml2profile.c
void yaml2profile(const bool,profile_t *const,const char *const,const char *const);
void profile_inspect();

// resolv.c
char *resolv(const char *domain);

// restful.c
char *now();
