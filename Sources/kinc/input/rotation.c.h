#include "rotation.h"

#include <memory.h>

static void (*rotation_callback)(float /*x*/, float /*y*/, float /*z*/) = NULL;

void kinc_set_rotation_callback(void (*value)(float /*x*/, float /*y*/, float /*z*/)) {
  rotation_callback = value;
}

void kinc_internal_on_rotation(float x, float y, float z) {
  if(rotation_callback != NULL){
    rotation_callback(x,y,z);
  }
}
