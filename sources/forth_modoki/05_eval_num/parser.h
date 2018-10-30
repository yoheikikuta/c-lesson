#pragma once
#include <stdio.h>


enum LexicalType {
    NUMBER,
    SPACE,
    EXECUTABLE_NAME,
    LITERAL_NAME,
    OPEN_CURLY,
    CLOSE_CURLY,
    END_OF_FILE,
    UNKNOWN
    };

typedef struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char onechar;
        char *name;
    } u;
} Token;

#define NAME_SIZE 256

/*
return next character and store one token into Token..
*/
int parse_one(int prev_ch, struct Token *out_token);
