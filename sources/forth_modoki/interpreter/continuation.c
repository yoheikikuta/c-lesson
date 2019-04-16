#include <stdio.h>
#include "cl_util.h"
#include "continuation.h"
#include "stack.h"


static struct Continuation co_stack[STACK_SIZE];
static int co_stack_pos = 0;

int co_stack_push(struct Continuation* cont) {
    if (co_stack_pos == STACK_SIZE) {
        return 0;
    } else {
        co_stack[co_stack_pos++] = *cont;

        return 1;
    }
}

int co_stack_pop(struct Continuation* out_cont) {
    if (co_stack_pos == 0) {
        return 0;
    } else {
        *out_cont = co_stack[--co_stack_pos];

        return 1;
    }
}

void co_load_variable(int pos, struct Element* out_elem) {
    // pos = 2, co_stack [co1, co2, co3, co4] -> out_elem = (Element of co2)
    *out_elem = co_stack[co_stack_pos - pos - 1].u.elem;
}