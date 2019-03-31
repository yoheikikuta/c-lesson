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

struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char onechar;
        char *name;
    } u;
};

#define NAME_SIZE 256

/*
return next character and store one token into Token.
Print all stacked variables on stdout.
*/
int parse_one(int prev_ch, struct Token* out_token);
void parser_print_all();