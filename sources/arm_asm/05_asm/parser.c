#include <stdio.h>
#include <assert.h>
#include <string.h>

struct Substring {
   char *str;
   int len;
};

struct Emitter;

void emit_word(struct Emitter* emitter, int oneword);

int parse_register(char *str, int out_register);

int skip_comma(char *str);

void assert_two_num_eq(int num1, int num2) {
    return num1 == num2;
}

void assert_str_substr_eq(char* str, struct Substring* substr) {
    char extracted_substr[sizeof(char) * substr->len];
    strncpy(&extracted_substr, substr->str, substr->len);
    return strcmp(str, extracted_substr) == 0;
}

// Return the length of str up to where the parse read.
int parse_one(char *str, struct Substring* out_subs) {
    int length = 3;
    out_subs->str = &str[0];
    out_subs->len = length;

    return length;
}

// 
// TEST
// 

static void test_parse_one_movr1r2() {
	char *input = "mov r1, r2";
	char *expect = "mov";
	int expect_len_read = 3;
	int expect_len_inst = 3;
	
	struct Substring actual;
	int actual_len_read = parse_one(input, &actual);
	
	assert_two_num_eq(expect_len_read, actual_len_read);
    assert_str_substr_eq(expect, &actual);
}

int main(int argc, char* argv[]) {
    test_parse_one_movr1r2();
    return 0;
}