#pragma once

#define ASM_FAILURE -1
#define WORD_BUF_SIZE 256 * 100

enum ElementType {
    NO_ELEM_TYPE,
    ELEMENT_NUMBER,
    ELEMENT_STRING
};

struct Element {
    enum ElementType etype;
    union {
        int number;
        char* str;
    } u;
};

struct Emitter {
    struct Element* elems;
    int pos;
};