#pragma once

#include <stdbool.h>
#include <shadowsocks.h> // profile_t

void yaml2profile(const bool,profile_t *const,const char *const,const char *const);
