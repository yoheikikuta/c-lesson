#include "clesson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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


static int is_space(int c) {
    return c == ' ';
}

static int is_digit(int c) {
    return ('0' <= c) && (c <= '9');
}


int parse_one(int prev_ch, struct Token *out_token) {
    /****
     * 
     * TODO: Implement here!
     * 
    ****/
    int cur_ch = 0;
    //*out_token = //How to initialize?

    // Read the first character and reset prev_ch for the initial case.
    if (prev_ch == EOF) {
        cur_ch = cl_getc();
    } else {
        cur_ch = prev_ch;
    }

    // Parse one unit for each head_char type.
    if (cur_ch == EOF) {
        out_token->ltype = END_OF_FILE;

        return EOF;

    } else if (cur_ch == ' ') {
        out_token->ltype = SPACE;

        // Just move to the next non-space character.
        for (; is_space(cur_ch); cur_ch=cl_getc()) {;}

        return cur_ch;

    } else if (is_digit(cur_ch)) {
        out_token->ltype = NUMBER;

        // Construct integer recursively.
        for (; is_digit(cur_ch); cur_ch=cl_getc()) {
            out_token->u.number = (10 * out_token->u.number) + (cur_ch - '0');
        }

        return cur_ch;

    }

    out_token->ltype = UNKNOWN;
    return EOF;

}


void parser_print_all() {
    int ch = EOF;
    struct Token token = {
        UNKNOWN,
        {0}
    };

    do {
        ch = parse_one(ch, &token);
        if(token.ltype != UNKNOWN) {
            switch(token.ltype) {
                case NUMBER:
                    printf("num: %d\n", token.u.number);
                    break;
                case SPACE:
                    printf("space!\n");
                    break;
                case OPEN_CURLY:
                    printf("Open curly brace '%c'\n", token.u.onechar);
                    break;
                case CLOSE_CURLY:
                    printf("Close curly brace '%c'\n", token.u.onechar);
                    break;
                case EXECUTABLE_NAME:
                    printf("EXECUTABLE_NAME: %s\n", token.u.name);
                    break;
                case LITERAL_NAME:
                    printf("LITERAL_NAME: %s\n", token.u.name);
                    break;

                default:
                    printf("Unknown type %d\n", token.ltype);
                    break;
            }
        }
    }while(ch != EOF);
}





static void test_parse_one_number() {
    char *input = "123";
    int expect = 123;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == NUMBER);
    assert(expect == token.u.number);
}

static void test_parse_one_empty_should_return_END_OF_FILE() {
    char *input = "";
    int expect = END_OF_FILE;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect);
}


static void unit_tests() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();
}

int main() {
    unit_tests();

    //cl_getc_set_src("123 45 add /some { 2 3 add } def");
    //parser_print_all();
    return 1;
}
