#pragma once

#include <shadowsocks.h> // profile_t

extern profile_t profile;

// Moved to lib/yaml2profile.c
// void profile_inspect();

bool profile_loaded();

void profile_clear();

void profile_to_json();
