#include <stdio.h>

struct Substring {
   char *str;
   int len;
};

struct Emitter;

void emit_word(struct Emitter* emitter, int oneword);

int parse_register(char *str, int out_register);

int skip_comma(char *str);

// Return the length of str up to where the parse read.
int parse_one(char *str, struct Substring* out_subs);

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
	assert_two_str_eq(expect, actual.str);
	assert_two_num_eq(expect_len_inst, actual.len);
}

int main(int argc, char* argv[]) {
    // test_parse_one_movr1r2();
    return 0;
}