#pragma once
#include <stdio.h>
#include "parser.h"
#include "primitive.h"


enum ElementType {
    NO_ELEM_TYPE,
    ELEMENT_C_FUNC,
    ELEMENT_EXEC_PRIMITIVE,
    ELEMENT_EXECUTABLE_ARRAY,
    ELEMENT_NUMBER,
    ELEMENT_EXECUTABLE_NAME,
    ELEMENT_LITERAL_NAME
};

enum ExecPrimitiveType {
	OP_EXEC,
	OP_JMP,
	OP_JMP_NOT_IF
};

struct ElementArray;

struct Element {
    enum ElementType etype;
    union {
        int number;
        char *name;
        void (*cfunc)();
        void (*compile_func)(struct Emitter* emitter);
        struct ElementArray *exec_array;
    } u;
};

struct ElementArray {
  int len;
  struct Element elements[0];
};

#define STACK_SIZE 1024
#define MAX_NAME_OP_MUMBERS 256

/*
return 1 if data is successfully pushed into stack.
return 1 if data is successfully popped from stack.
return 1 if data element is successfully copied.
Print all stacks on stdout. 
*/
int stack_push(struct Element* push_elem);
int stack_pop(struct Element* out_elem);
int stack_copy(struct Element* out_elem, int idx);
void stack_print_all();
