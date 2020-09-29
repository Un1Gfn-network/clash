#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <net/if.h>
#include <systemd/sd-bus.h>

#include "./def.h"
#include "./bus.h"
#include "./privilege.h"

#define SERVICE "org.freedesktop.resolve1"
#define OBJECT "/org/freedesktop/resolve1"
#define INTERFACE "org.freedesktop.resolve1.Manager"
#define METHOD_FLUSHCACHE "FlushCaches"
#define METHOD_SETDNS "SetLinkDNS"

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

void f_flush(sd_bus_error *const ep,sd_bus_message **mp){
  assert(0<=sd_bus_call_method(
    bus,
    SERVICE,OBJECT,INTERFACE,METHOD_FLUSHCACHE,
    ep,
    mp,
    ""
  ));
}

void f_setdns(sd_bus_error *const ep,sd_bus_message **mp){
  assert(0<=sd_bus_call_method(
    bus,
    SERVICE,OBJECT,INTERFACE,METHOD_SETDNS,
    ep,
    mp,
    "ia(iay)",
    (int32_t)ifi,
    2,
      AF_INET,
      4,
        8,8,8,8,
      AF_INET,
      4,
        8,8,4,4
  ));
}

void f_resetdns(sd_bus_error *const ep,sd_bus_message **mp){
  assert(0<=sd_bus_call_method(
    bus,
    "org.freedesktop.resolve1",         // Service destination
    "/org/freedesktop/resolve1",        // Path to object
    "org.freedesktop.resolve1.Manager", // Interface
    "SetLinkDNS",                       // Method
    ep,
    mp,
    "ia(iay)",
    (int32_t)ifi,
    1,
      AF_INET,
      4,
        192,168,1,1
  ));
}

void bus_call(void(*f)(sd_bus_error *const ep,sd_bus_message **mp)){

  sd_bus_error e=SD_BUS_ERROR_NULL;
  sd_bus_message *m=NULL;
  ESCALATED((*f)(&e,&m));

  assert(!sd_bus_error_is_set(&e));
  sd_bus_error_free(&e);
  e=SD_BUS_ERROR_NULL;

  assert(sd_bus_message_is_empty(m));
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