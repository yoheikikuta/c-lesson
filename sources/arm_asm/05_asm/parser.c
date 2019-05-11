#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "cl_getline.h"

#define PARSE_FAIL -1

struct Substring {
   char *str;
   int len;
};

struct Emitter;

void emit_word(struct Emitter* emitter, int oneword);

int parse_register(char *str, int out_register);

int skip_comma(char *str);

void assert_two_num_eq(int num1, int num2) {
    return assert(num1 == num2);
}

void assert_str_substr_eq(char* str, struct Substring* substr) {
    char extracted_substr[BUF_SIZE] = {'\0'};
    strncpy(&extracted_substr, substr->str, substr->len);

    return assert(strcmp(str, extracted_substr) == 0);
}

int is_space(int c) {
    return c == ' ';
}

int is_underscore(int c) {
    return c == '_';
}

int is_digit(int c) {
    return ('0' <= c) && (c <= '9');
}

int is_alphabet(int c) {
    return (('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z'));
}

int is_inst_character(int c) {
    return is_underscore(c) || is_digit(c) || is_alphabet(c);
}

// "  mov r1, r2" ->
//   return 5 (len including spaces)
//   out_subs->str = ['m','o','v','\0', ...]
//   out_subs->len = 3 (len not including spaces) 
int parse_one(char *str, struct Substring* out_subs) {    
    int len_read_ch = 0;
    int len_inst_ch = 0;
    int head_ch = str[len_read_ch];

    // Skip spaces at the beginning of the string.
    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    out_subs->str = &str[len_read_ch];

    do {
        if (head_ch == '\0') {
            return EOF;
        } else if (is_inst_character(head_ch)) {
            len_inst_ch++;
        } else {
            return PARSE_FAIL;
        }
        head_ch = str[++len_read_ch];
    } while (!is_space(head_ch));

    out_subs->len = len_inst_ch;

    return len_read_ch;
}

// 
// TEST
// 

static void test_parse_one_movr1r2_mov() {
	char *input = "mov r1, r2";
	char *expect = "mov";
	int expect_len_read = 3;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_movr1r2_mov_with_sp() {
	char *input = "  mov r1, r2";
	char *expect = "mov";
	int expect_len_read = 5;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_only_sp() {
	char *input = "   ";
	int expect_len_read = EOF;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void unittests() {
    test_parse_one_movr1r2_mov();
    test_parse_one_movr1r2_mov_with_sp();
    test_parse_one_only_sp();

    printf("All unittests successfully passed.\n");
}


int main(int argc, char* argv[]) {
    unittests();
    return 0;
}