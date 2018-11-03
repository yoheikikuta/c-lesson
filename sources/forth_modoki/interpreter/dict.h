#pragma once
#include "stack.h"
#include <stdio.h>


typedef struct KeyValue {
    char *key;
    struct Data value;
} KeyValue;

#define DICT_SIZE 1024

/*
put {key, value} into the dictionaly, or overwrite the value if the key exists.
return (0) 1 if a given key (not) exists, and out_elem stores Data of the given key.
print all keys in the dictionaly.
*/
void dict_put(char* key, struct Data *elem);
int dict_get(char* key, struct Data *out_elem);
void dict_print_all();
