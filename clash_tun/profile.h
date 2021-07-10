#pragma once

#include <shadowsocks.h> // profile_t

extern profile_t profile;

void profile_inspect();

bool profile_loaded();

void profile_clear();

void profile_to_json();
