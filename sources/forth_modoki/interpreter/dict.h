#pragma once
#include <stdio.h>
#include "stack.h"

enum ElementType {
    NO_ELEM_TYPE,
    ELEMENT_C_FUNC
};

struct Element {
    enum ElementType etype;
    union {
        void (*cfunc)();
    } u;
};

/*
put {key, value} into the dictionaly, or overwrite the value if the key exists.
return (0) 1 if a given key (not) exists, and out_elem stores Data of the given key.
print all keys in the dictionaly.
*/
void dict_put(char* key, Data_t* elem);
int dict_get(char* key, Data_t* out_elem);
void dict_print_all();
