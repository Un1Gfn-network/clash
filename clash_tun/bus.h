#pragma once

#include <stdbool.h>
#include <systemd/sd-bus.h>

void bus_init();

void bus_end();

void f_flush(sd_bus_error *__restrict const ep, sd_bus_message **__restrict mp);

void f_setdns(sd_bus_error *__restrict const ep, sd_bus_message **__restrict mp);

void f_resetdns(sd_bus_error *__restrict const ep, sd_bus_message **__restrict mp);

void bus_call(void(*f)(sd_bus_error *__restrict const ep, sd_bus_message **__restrict mp));
