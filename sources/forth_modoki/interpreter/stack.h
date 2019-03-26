#pragma once
#include <stdio.h>
#include "parser.h"

/*
enum DataType {
    NUMBER,
    LITERAL_NAME,
    UNKNOWN,
};
*/

typedef struct Data {
    //enum DataType dtype;
    enum LexicalType dtype;
    union {
        int number;
        char *name;
    } u;
} Data_t;

enum ElementType {
    NO_ELEM_TYPE,
    ELEMENT_C_FUNC,
    ELEMENT_EXECUTABLE_ARRAY,
    ELEMENT_NUMBER,
    ELEMENT_EXECUTABLE_NAME,
    ELEMENT_LITERAL_NAME
};

struct ElementArray;

typedef struct Element {
    enum ElementType etype;
    union {
        int number;
        char *name;
        void (*cfunc)();
        struct ElementArray *exec_array;
    } u;
};

typedef struct ElementArray {
  int len;
  struct Element elements[0];
};

#define STACK_SIZE 1024

/*
return 1 if data is successfully pushed into stack.
return 1 if data is successfully popped from stack.
*/
int stack_push(Data_t* push_elem);
int stack_pop(Data_t* out_elem);
