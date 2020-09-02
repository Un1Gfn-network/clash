#include <assert.h>
#include <json.h>
#include <string.h>

#include "def.h"

char *json_load_server(){

  json_object *j=json_object_from_file(SS_LOCAL_JSON);
  assert(j);
  assert(json_type_object==json_object_get_type(j));

  json_object *j2=json_object_object_get(j,"server");
  assert(j2);
  assert(json_type_string==json_object_get_type(j2));
  char *s=strdup(json_object_get_string(j2));

  assert(1==json_object_put(j));
  return s;

}
