#pragma once

#define ASM_FAILURE -1
#define ASM_RESULT_BUF_SIZE 4 * 1024
#define STR_SIZE 256

enum AsmResultType {
    NO_TYPE,
    SKIP,
    WORD_CODE,
    WORD_STRING,
    LABEL
};

struct AsmResultElem {
    enum AsmResultType type;
    union {
        int number;
        char* str;
    } u;
};

struct Emitter {
    unsigned char* buf;
    int pos;
};