#pragma once
#include "assembler.h"

#define DICT_SIZE 256

struct KeyValue {
    int key;
    int value;
};

/*
Put element into the dictionary.
Retunr 1 if sucessfully get the element of the key, 0 else.
*/
void dict_put(int key, int value);
int dict_get(int key, int* out_value);