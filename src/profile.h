#pragma once

#include <shadowsocks.h>
#include <stdbool.h>

extern profile_t profile;

void profile_inspect();

bool profile_loaded();

void profile_clear();

void yaml2profile(const char *const from_yaml,const char *const server_title);

void profile2json();
