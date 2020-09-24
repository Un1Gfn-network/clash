#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
// #include <stdint.h>
#include <systemd/sd-bus.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
  int32_t padding;
  int32_t d;
  int32_t padding2;
} Data;

int h1(sd_bus_message *m,void *userdata,sd_bus_error *e){
  assert(!sd_bus_error_is_set(e));
  assert(!sd_bus_message_is_empty(m));
  printf("\n");
  if(!sd_bus_message_has_signature(m,"xx")){
    printf("[");
    bool first=true;
    char t='\0';
    while(1==sd_bus_message_peek_type(m,&t,NULL)){
      assert(t=='s');
      const char *s=NULL;
      assert(1==sd_bus_message_read(m,"s",&s));
      printf(first?"%s":",%s",s);
      first=false;
    }
    printf("]");
  }
  printf("...first, global callbacks installed with sd_bus_add_filter() are called...\n");
  return 0;
}

int h2(sd_bus_message *m,void *userdata,sd_bus_error *e){
  assert(!sd_bus_error_is_set(e));
  assert(!sd_bus_message_is_empty(m));
  if(sd_bus_message_has_signature(m,"s")){}
  printf("...second, callbacks attached directly to the request object path are called...\n");
  return 0;
}

int h3_method_multiply(sd_bus_message *m,void *userdata,sd_bus_error *e){
  assert(!sd_bus_error_is_set(e));
  assert(!sd_bus_message_is_empty(m));
  assert(sd_bus_message_has_signature(m,"xx"));
  int64_t x=0,y=0;
  assert(1==sd_bus_message_read(m,"xx",&x,&y));
  printf("...followed by any D−Bus method callbacks attached to the request object path, interface and member...\n");
  return sd_bus_reply_method_return(m,"x",x*y);
}

int h3_method_divide(sd_bus_message *m,void *userdata,sd_bus_error *e){
  assert(!sd_bus_error_is_set(e));
  assert(!sd_bus_message_is_empty(m));
  assert(sd_bus_message_has_signature(m,"xx"));
  int64_t x=0,y=0;
  assert(1==sd_bus_message_read(m,"xx",&x,&y));
  assert(y!=0L);
  printf("...followed by any D−Bus method callbacks attached to the request object path, interface and member...\n");
  return sd_bus_reply_method_return(m,"x",x/y);
}

int h4(sd_bus *bus,const char *path,const char *interface,const char *property,sd_bus_message *reply,void *userdata,sd_bus_error *e){
  assert(!sd_bus_error_is_set(e));
  assert(!sd_bus_message_is_empty(reply));
  // printf("%s\n",sd_bus_message_get_signature(reply,true));
  const char *g=NULL;
  uint8_t *y=NULL;
  sd_bus_message_read(reply,"v","gy",&g,&y);
  assert((!g)&&(!y));
  printf("...finally, the property callbacks attached to the request object path, interface and member are called...\n");
  assert(userdata);
  // https://github.com/systemd/systemd/blob/26c65933babfb80130e0e9e5551cef1ee0c5ecf3/src/home/homed-home-bus.c#L40
  return sd_bus_message_append(reply,"i",*((int32_t*)userdata));
}

const sd_bus_vtable vtable[]={
  SD_BUS_VTABLE_START(0),
  SD_BUS_METHOD("Multiply","xx","x",&h3_method_multiply ,SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("Divide"  ,"xx","x",&h3_method_divide   ,SD_BUS_VTABLE_UNPRIVILEGED),
  // https://github.com/systemd/systemd/blob/90e74a66e663f1776457d599cb7d5ce44785a56c/src/core/dbus-job.c#L137
  SD_BUS_PROPERTY("Data"  ,"i"     ,&h4,offsetof(Data,d),SD_BUS_VTABLE_PROPERTY_CONST),
  // SD_BUS_PROPERTY("Data"  ,"i"     ,NULL,offsetof(Data,d),SD_BUS_VTABLE_PROPERTY_CONST),
  SD_BUS_VTABLE_END
};

int main(){

  Data data={.d=114514};

  // https://stackoverflow.com/q/43414858/what-is-a-slot-in-sd-bus-c-language
  // sd_bus_slot *slot=NULL;
  sd_bus *bus=NULL;

  assert(0==unsetenv("DBUS_SESSION_BUS_ADDRESS"));
  assert(0==unsetenv("DBUS_SYSTEM_BUS_ADDRESS"));
  assert(1==sd_bus_default_user(&bus));

  assert(0==sd_bus_add_filter(bus,NULL,h1,NULL));
  assert(0==sd_bus_add_object(bus,NULL,"/net/poettering/Calculator",h2,NULL));
  const int r=sd_bus_add_object_vtable(
    bus,
    NULL,
    "/net/poettering/Calculator", // Path-to-object
    "net.poettering.Calculator",  // Interface
    vtable,
    &data
  );
  if(r<0){
    printf("%s\n",strerror(-r));
    assert(false);
  }

  /* Take a well-known service name so that clients can find us */
  assert(1==sd_bus_request_name(bus,"net.poettering.Calculator",0));

  for(;;){

    /* Process requests */
    int r=sd_bus_process(bus,NULL);
    assert(r>=0);
    if(r==0) /* Wait for the next request to process */
      assert(0<=sd_bus_wait(bus,(uint64_t) -1));
    /* we processed a request,try to process another one,right-away */
  }

  // sd_bus_slot_unref(slot);
  assert(NULL==sd_bus_flush_close_unref(bus));
  bus=NULL;
  return EXIT_SUCCESS;

}
