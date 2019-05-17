#pragma once

#define ASM_FAILURE -1
#define WORD_BUF_SIZE 256 * 100

enum WordType {
    NO_WORD_TYPE,
    WORD_NUMBER,
    WORD_STRING
};

struct Word {
    enum WordType wtype;
    union {
        int number;
        char* str;
    } u;
};

struct Emitter {
    struct Word* words;
    int pos;
};