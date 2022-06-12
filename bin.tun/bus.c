#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <net/if.h>
#include <systemd/sd-bus.h>

#include "./def.h"
#include "./bus.h"
#include "./privilege.h"

static sd_bus *bus=NULL;
static unsigned ifi=0;

void bus_init(){
  ifi=if_nametoindex(WLO);
  assert(ifi>=1);
  assert(0==unsetenv("DBUS_SESSION_BUS_ADDRESS"));
  assert(0==unsetenv("DBUS_SYSTEM_BUS_ADDRESS"));
  ESCALATED(assert(0<=sd_bus_default_system(&bus)));
  assert(bus);
}

void bus_end(){
  ESCALATED(assert(NULL==sd_bus_flush_close_unref(bus)));
  bus=NULL;
  ifi=0;
}

// https://0pointer.net/blog/the-new-sd-bus-api-of-systemd.html
/*
gdbus introspect --system \
  --dest org.freedesktop.systemd1 \
  --object-path /org/freedesktop/systemd1 \

*/
void resolved_restartservice(sd_bus_error *__restrict const ep,sd_bus_message **__restrict mp){
  assert(0<=sd_bus_call_method(
    bus,
    "org.freedesktop.systemd1", // service destination
    "/org/freedesktop/systemd1", // path to object
    "org.freedesktop.systemd1.Manager", // interface
    "RestartUnit", // method
    ep,
    mp,
    "ss",
    "systemd-resolved.service", // arg1 - name
    "replace" // arg2 - mode // systemctl(1) "--job-mode="
  ));
}

void resolved_flushcache(sd_bus_error *__restrict const ep,sd_bus_message **__restrict mp){
  assert(0<=sd_bus_call_method(
    bus,
    "org.freedesktop.resolve1", // service destination
    "/org/freedesktop/resolve1", // path to object
    "org.freedesktop.resolve1.Manager", // interface
    "FlushCaches", // method
    ep,
    mp,
    ""
  ));
}

void bus_call(void(*f)(sd_bus_error *__restrict const ep,sd_bus_message **__restrict mp)){

  sd_bus_error e=SD_BUS_ERROR_NULL;
  sd_bus_message *m=NULL;
  ESCALATED((*f)(&e,&m));

  assert(!sd_bus_error_is_set(&e));
  sd_bus_error_free(&e);
  e=SD_BUS_ERROR_NULL;

  // https://www.freedesktop.org/software/systemd/man/sd_bus_message_read.html
  // assert(sd_bus_message_is_empty(m));
  sd_bus_message_unref(m);
  m=NULL;

}

/*int main(){
  bus_init();
  bus_call(&f_flush);
  getchar();
  bus_call(&f_setdns);
  getchar();
  bus_call(&f_resetdns);
  bus_end();
  return 0;
}*/
