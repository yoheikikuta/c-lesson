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

static int is_alphabet(int c) {
    return (('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z'));
}

static int is_non_space_char(int c) {
    return ('!' <= c) && (c <= '~');
}

static int is_slash(int c) {
    return c == '/';
}


int parse_one(int prev_ch, struct Token *out_token) {
    /****
     * 
     * TODO: Implement here!
     * 
    ****/
    int cur_ch = 0;

    // Set the head character as cur_ch
    if (prev_ch == EOF) {
        cur_ch = cl_getc();
    } else {
        cur_ch = prev_ch;
    }

    // Parse one unit for each cur_ch type.
    if (cur_ch == EOF) {
        out_token->ltype = END_OF_FILE;
        return cur_ch;

    } else if (is_space(cur_ch)) {
        // Just move to the next non-space character.
        for (cur_ch; is_space(cur_ch); cur_ch=cl_getc()) {;}

        out_token->ltype = SPACE;
        out_token->u.onechar = ' ';
        return cur_ch;

    } else if (is_digit(cur_ch)) {
        // Construct integer recursively up to the next non-digit character.
        int num = 0;
        for (cur_ch; is_digit(cur_ch); cur_ch=cl_getc()) {
            num = (10 * num) + (cur_ch - '0');
        }

        out_token->ltype = NUMBER;
        out_token->u.number = num;
        return cur_ch;

    } else if (is_alphabet(cur_ch)) {
        // Copy strings up to the next space character.
        char *name, *dummy;
        dummy = name = (char *)malloc(sizeof(unsigned char) * NAME_SIZE);
        for (cur_ch; is_non_space_char(cur_ch); cur_ch=cl_getc()) {
            *dummy++ = cur_ch;
        }
        *dummy = '\0';

        out_token->ltype = EXECUTABLE_NAME;
        out_token->u.name = name;
        return cur_ch;

    } else if (is_slash(cur_ch)) {
        // Read the next character to skip slash.
        cur_ch = cl_getc();
        // Copy strings up to the next space character.
        char *name, *dummy;
        dummy = name = (char *)malloc(sizeof(unsigned char) * NAME_SIZE);
        for (cur_ch; is_non_space_char(cur_ch); cur_ch=cl_getc()) {
            *dummy++ = cur_ch;
        }
        *dummy = '\0';

        out_token->ltype = LITERAL_NAME;
        out_token->u.name = name;
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

static void test_parse_one_space() {
    char *input = " 123";
    int expect = ' ';

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == '1');
    assert(token.ltype == SPACE);
    assert(expect == token.u.onechar);
}

static int is_two_strs_equal(char *s1, char *s2) {
    // Return 1 if two strings are identical.
    return strcmp(s1, s2) == 0;
}

static void test_parse_one_executable_name() {
    char* input = "add";
    char* expect_name = "add";
    int expect_type = EXECUTABLE_NAME;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect_type);
    assert(is_two_strs_equal(expect_name, token.u.name));
}

static void test_parse_one_literal_name() {
    char* input = "/add";
    char* expect_name = "add";
    int expect_type = LITERAL_NAME;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect_type);
    assert(is_two_strs_equal(expect_name, token.u.name));
}


static void unit_tests() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();
    test_parse_one_space();
    test_parse_one_executable_name();
    test_parse_one_literal_name();
}


int main() {
    unit_tests();

    //cl_getc_set_src("123 45 add /some { 2 3 add } def");
    //parser_print_all();
    return 1;
}
