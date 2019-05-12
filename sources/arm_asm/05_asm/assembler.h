#pragma once

#define ASM_FAILURE -1
#define WORD_BUF_SIZE 256 * 100

struct Emitter {
    int word_buf[WORD_BUF_SIZE];
    int pos;
};