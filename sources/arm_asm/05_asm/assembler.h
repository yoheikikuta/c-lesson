#pragma once

#define ASM_FAILURE -1
#define WORD_BUF_SIZE 256 * 100
#define STR_SIZE 256

enum WordType {
    NO_WORD_TYPE,
    WORD_SKIP,
    WORD_NUMBER,
    WORD_STRING,
    WORD_LABEL,
    WORD_JUMP
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