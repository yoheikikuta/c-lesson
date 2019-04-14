#pragma once
#include "stack.h"

struct Emitter {
  struct Element *elems;
  int pos;
};

/*
register all primitives to the dictionaly.
*/
void register_all_primitive();