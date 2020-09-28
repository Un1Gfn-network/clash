#pragma once

#include <shadowsocks.h>
#include <stdbool.h>

extern profile_t profile;

bool start_ss();

void stop_ss();
