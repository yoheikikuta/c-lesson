#include "clesson.h"
#include "cl_util.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


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

static int is_open_curly(int c) {
    return c == '{';
}

static int is_close_curly(int c) {
    return c == '}';
}


int parse_one(int prev_ch, struct Token *out_token) {
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
        // Parse space(s): "  a" -> ' ' + cur_ch='a'
        while (is_space(cur_ch)) {cur_ch=cl_getc();}

        out_token->ltype = SPACE;
        out_token->u.onechar = ' ';
        return cur_ch;

    } else if (is_digit(cur_ch)) {
        // Parse digit(s): "123 " -> 123 + cur_ch=' '
        int num = 0;
        while (is_digit(cur_ch)) {
            num = (10 * num) + (cur_ch - '0');
            cur_ch=cl_getc();
        }

        out_token->ltype = NUMBER;
        out_token->u.number = num;
        return cur_ch;

    } else if (is_alphabet(cur_ch)) {
        // Parse executable name: "a?c " -> "a?c" + cur_ch=' '
        char buf[NAME_SIZE];
        int i;
        for (i=0; is_non_space_char(cur_ch); i++) {
            buf[i] = cur_ch;
            cur_ch=cl_getc();
        }
        buf[i] = '\0';

        char *name;
        name = (char *)malloc(sizeof(char) * (i+1));
        strcpy(name, buf);

        out_token->ltype = EXECUTABLE_NAME;
        out_token->u.name = name;
        return cur_ch;

    } else if (is_slash(cur_ch)) {
        // Parse literal name: "/a?c " -> "a?c" + cur_ch=' '
        cur_ch = cl_getc();

        char buf[NAME_SIZE];
        int i;
        for (i=0; is_non_space_char(cur_ch); i++) {
            buf[i] = cur_ch;
            cur_ch=cl_getc();
        }
        buf[i] = '\0';

        char *name;
        name = (char *)malloc(sizeof(char) * (i+1));
        strcpy(name, buf);

        out_token->ltype = LITERAL_NAME;
        out_token->u.name = name;
        return cur_ch;

    } else if (is_open_curly(cur_ch)) {
        // Parse open curly brace: "{a" -> "{" + cur_ch='a'
        cur_ch = cl_getc();
        out_token->ltype = OPEN_CURLY;
        out_token->u.onechar = '{';
        return cur_ch;

    } else if (is_close_curly(cur_ch)) {
        // Parse close curly brace: "}a" -> "}" + cur_ch='a'
        cur_ch = cl_getc();
        out_token->ltype = CLOSE_CURLY;
        out_token->u.onechar = '}';
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

    assert(EOF == ch);
    assert(NUMBER == token.ltype);
    assert(expect == token.u.number);
}

static void test_parse_one_empty_should_return_END_OF_FILE() {
    char *input = "";
    int expect = END_OF_FILE;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(EOF == ch);
    assert(expect == token.ltype);
}

static void test_parse_one_space() {
    char *input = " 123";
    int expect = ' ';

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert('1' == ch);
    assert(SPACE == token.ltype);
    assert(expect == token.u.onechar);
}

static void test_parse_one_executable_name() {
    char* input = "add";
    char* expect_name = "add";
    int expect_type = EXECUTABLE_NAME;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(EOF == ch);
    assert(expect_type == token.ltype);
    assert(streq(expect_name, token.u.name));
}

static void test_parse_one_literal_name() {
    char* input = "/add";
    char* expect_name = "add";
    int expect_type = LITERAL_NAME;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(EOF == ch);
    assert(expect_type == token.ltype);
    assert(streq(expect_name, token.u.name));
}

static void test_parse_one_open_curly() {
    char* input = "{";
    char expect_onechar = '{';
    int expect_type = OPEN_CURLY;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(EOF == ch);
    assert(expect_type == token.ltype);
    assert(expect_onechar == token.u.onechar);
}

static void test_parse_one_close_curly() {
    char* input = "}";
    char expect_onechar = '}';
    int expect_type = CLOSE_CURLY;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(EOF == ch);
    assert(expect_type == token.ltype);
    assert(expect_onechar == token.u.onechar);
}


static void unit_tests() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();
    test_parse_one_space();
    test_parse_one_executable_name();
    test_parse_one_literal_name();
    test_parse_one_open_curly();
    test_parse_one_close_curly();
}


#if 0
int main() {
    unit_tests();

    cl_getc_set_src("123 45 add /some { 2 3 add } def");
    parser_print_all();
    return 0;
}
#endif
