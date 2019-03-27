#pragma once
#include <stdio.h>
#include "stack.h"


/*
put {key, value} into the dictionaly, or overwrite the value if the key exists.
return (0) 1 if a given key (not) exists, and out_elem stores Data of the given key.
print all keys in the dictionaly.
*/
void dict_put(char* key, struct Element* elem);
int dict_get(char* key, struct Element* out_elem);
void dict_print_all();
