#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>
#include <stdint.h>
#include <stdarg.h>

// #define S0(x) #x
// #define S(x) S0(x)

sd_bus *bus=NULL;

void init(){

  assert(0==unsetenv("DBUS_SESSION_BUS_ADDRESS"));
  assert(0==unsetenv("DBUS_SYSTEM_BUS_ADDRESS"));

  // A
  // assert(0<=sd_bus_open_user(&bus));

  // B
  // https://stackoverflow.com/questions/4384765/whats-the-difference-between-pretty-function-function-func
  // assert(0<=sd_bus_open_user_with_description(&bus, __FILE__":"S(__LINE__)":"__func__"():""send notification"));
  // assert(0<=sd_bus_open_user_with_description(&bus, __FILE__":"S(__LINE__)": send notification"));

  // C
  assert(0<=sd_bus_default_user(&bus));

}

void end(){

  // C1
  // assert(0==sd_bus_flush());
  // sd_bus_close();
  // assert(NULL==sd_bus_unref(bus));

  // C2
  assert(NULL==sd_bus_flush_close_unref(bus));

  // C3
  // sd_bus_default_flush_close();

  bus=NULL;

}

int main() {

  init();

  sd_bus_message *m=NULL;
  sd_bus_error e=SD_BUS_ERROR_NULL;
  assert(0<=sd_bus_call_method(
    bus,
    "org.freedesktop.Notifications",  // Service destination
    "/org/freedesktop/Notifications", // Path to object
    "org.freedesktop.Notifications",  // Interface
    "Notify",                         // Method
    &e,
    &m,
    "susssasa{sv}i",
    "app_name",
    0,
    "app_icon",
    "summary",
    "body",
    NULL,
    NULL,
    1333
  ));

  assert(!sd_bus_error_is_set(&e));
  sd_bus_error_free(&e);

  assert(!sd_bus_message_is_empty(m));
  char type='\0';
  assert(1==sd_bus_message_peek_type(m,&type,NULL));
  assert(type=='u');
  assert(sd_bus_message_has_signature(m,"u"));
  assert(0==strcmp("u",sd_bus_message_get_signature(m,true)));
  uint32_t u=0;
  assert(1==sd_bus_message_read(m,"u",&u));  
  assert(!sd_bus_message_peek_type(m,NULL,NULL));
  printf("out: id=%d\n",u);
  assert(u>=2);

  sd_bus_message_unref(m);

  end();

  return EXIT_SUCCESS;

}
