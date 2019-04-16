#pragma once
#include <stdio.h>
#include "stack.h"


enum ContType {
    CONT_ELEMENT,
    CONT_CONTINUATION
};

struct Continuation {
    enum ContType ctype;
    union {
        struct Element elem;
        struct ElementArray *exec_array;
    } u;
    int pc;
};

#define STACK_SIZE 1024

/*
return 1 if data is successfully pushed into co_stack.
return 1 if data is successfully popped from co_stack.
*/
int co_stack_push(struct Continuation* cont);
int co_stack_pop(struct Continuation* cont);