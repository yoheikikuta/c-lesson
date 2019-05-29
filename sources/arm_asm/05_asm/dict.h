#pragma once
#include "assembler.h"

#define DICT_SIZE 256

struct KeyValue {
    int key;
    struct Word value;
};

/*
Put element into the dictionary.
Retunr 1 if sucessfully get the element of the key, 0 else.
*/
void dict_put(int key, struct Word* elem);
int dict_get(int key, struct Word* out_elem);