#include "parser.h"
#include "cl_util.h"
#include <stdio.h>


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

int is_register(int c) {
    return (c == 'r') || (c == 'R');
}

// "  mov r1, r2" ->
//   return 5 (len including spaces)
//   out_subs->str = ['m','o','v','\0', ...]
//   out_subs->len = 3 (len not including spaces) 
int parse_one(char* str, struct Substring* out_subs) {    
    int len_read_ch = 0;
    int len_inst_ch = 0;
    int head_ch = str[len_read_ch];

    // Skip spaces at the beginning of the string.
    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    out_subs->str = &str[len_read_ch];

    do {
        if (is_inst_character(head_ch)) {
            len_inst_ch++;
        } else if (head_ch == '\0') {
            return EOF;
        } else {
            return PARSE_FAILURE;
        }
        head_ch = str[++len_read_ch];
    } while (is_inst_character(head_ch));

    out_subs->len = len_inst_ch;

    return len_read_ch;
}

// " r12, r2" ->
//   return 4 (len including spaces)
//   out_register = 12
int parse_register(char* str, int* out_register) {
    int len_read_ch = 0;
    int register_num = 0;
    int head_ch = str[len_read_ch];

    // Skip spaces at the beginning of the string.
    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is 'r' or 'R', and read the next character.
    if (!is_register(head_ch)) return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    do {
        if (is_digit(head_ch)) {
            register_num = 10 * register_num + (head_ch - '0');
        } else if (head_ch == '\0') {
            return EOF;
        } else {
            return PARSE_FAILURE;
        }
        head_ch = str[++len_read_ch];
    } while (is_digit(head_ch));

    // Valid register number is from 0 to 15.
    if (register_num <= 15) {
        *out_register = register_num;
        return len_read_ch;
    } else {
        return PARSE_FAILURE;
    }
}

// " , r2" -> return 2 (len including spaces)
int skip_comma(char* str) {
    int len_read_ch = 0;
    int head_ch = str[len_read_ch];

    while (is_space(head_ch)) {
        head_ch = str[++len_read_ch];
    }

    // Check the character is 'r' or 'R', and read the next character.
    if (head_ch != ',') return PARSE_FAILURE;
    head_ch = str[++len_read_ch];

    return len_read_ch;
}

// 
// TEST
// 

static void test_parse_one_movr1r2_mov() {
	char* input = "mov r1, r2";
	char* expect = "mov";
	int expect_len_read = 3;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_movr1r2_mov_with_sp() {
	char* input = "  mov r1, r2";
	char* expect = "mov";
	int expect_len_read = 5;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_intermediate() {
	char* input = "  mov[";
	char* expect = "mov";
	int expect_len_read = 5;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

static void test_parse_one_failure() {
	char* input = "  [";
	char* expect_len_read = PARSE_FAILURE;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_one_only_sp() {
	char* input = "   ";
	int expect_len_read = EOF;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_register_r1() {
	char* input = "  r1, r2";
	int expect = 1;
	int expect_len_read = 4;
	
	int actual;
	int actual_len_read = parse_register(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
	assert_two_num_eq(expect, actual);
}

static void test_parse_register_fail() {
	char* input = "  r, r2";
	int expect_len_read = PARSE_FAILURE;
	
	int actual;
	int actual_len_read = parse_register(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_parse_register_fail_other_ch() {
	char* input = "  ar1, r2";
	int expect_len_read = PARSE_FAILURE;
	
	int actual;
	int actual_len_read = parse_register(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_skip_comma() {
	char* input = " , r2";
	int expect_len_read = 2;
	
	int actual_len_read = skip_comma(input);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void test_skip_comma_fail() {
	char* input = " a, r2";
	int expect_len_read = PARSE_FAILURE;
	
	int actual_len_read = skip_comma(input);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
}

static void unittests() {
    test_parse_one_movr1r2_mov();
    test_parse_one_movr1r2_mov_with_sp();
    test_parse_one_intermediate();
    test_parse_one_failure();
    test_parse_one_only_sp();
    test_parse_register_r1();
    test_parse_register_fail();
    test_parse_register_fail_other_ch();
    test_skip_comma();
    test_skip_comma_fail();

    printf("All unittests successfully passed.\n");
}


#if 0
int main(int argc, char* argv[]) {
    unittests();
    return 0;
}
#endif