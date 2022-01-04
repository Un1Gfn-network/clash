#pragma once

#include <stdbool.h>
#include <systemd/sd-bus.h>

void bus_init();

void bus_end();

void resolved_restartservice(sd_bus_error *__restrict const, sd_bus_message **__restrict);

void resolved_flushcache(sd_bus_error *__restrict const, sd_bus_message **__restrict);

void bus_call(void(*f)(sd_bus_error *__restrict const, sd_bus_message **__restrict));
