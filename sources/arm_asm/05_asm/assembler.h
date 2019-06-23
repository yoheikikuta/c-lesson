#pragma once

#define ASM_FAILURE -1
#define WORD_BUF_SIZE 4 * 1024
#define STR_SIZE 256

enum WordType {
    NO_WORD_TYPE,
    WORD_SKIP,
    WORD_CODE,
    WORD_STRING,
    WORD_LABEL
};

struct Word {
    enum WordType wtype;
    union {
        int number;
        char* str;
    } u;
};

struct Emitter {
    unsigned char* word_buf;
    int pos;
};