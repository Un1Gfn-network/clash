#pragma once

#include <stdbool.h>
#include <shadowsocks.h> // profile_t

// yaml2profile.c
void yaml2profile(const bool,profile_t *const,const char *const,const char *const);

// resolv.c
char *resolv(const char *domain);

// restful.c
char *now();
