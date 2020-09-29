#pragma once

#include <stdbool.h>
#include <systemd/sd-bus.h>

void bus_init();

void bus_end();

void f_flush(sd_bus_error *const ep,sd_bus_message **mp);

void f_setdns(sd_bus_error *const ep,sd_bus_message **mp);

void f_resetdns(sd_bus_error *const ep,sd_bus_message **mp);

void bus_call(void(*f)(sd_bus_error *const ep,sd_bus_message **mp));
