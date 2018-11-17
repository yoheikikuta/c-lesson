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

#define STACK_SIZE 1024

/*
return 1 if data is successfully pushed into stack.
return 1 if data is successfully popped from stack.
*/
int stack_push(Data_t* push_elem);
int stack_pop(Data_t* out_elem);
