#pragma once
#include <stdio.h>
#include "stack.h"


struct Continuation {
    struct ElementArray *exec_array;
    int pc;
};

#define STACK_SIZE 1024

/*
return 1 if data is successfully pushed into co_stack.
return 1 if data is successfully popped from co_stack.
*/
int co_stack_push(struct Continuation* cont);
int co_stack_pop(struct Continuation* cont);