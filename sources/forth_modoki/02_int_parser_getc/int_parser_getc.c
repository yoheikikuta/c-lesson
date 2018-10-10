#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/
enum TokenType {NUMBER, SPACE};


static int is_digit(int c) {
    if (('0' <= c) && (c <= '9')) {
        return 1;
    } else {
        return 0;
    }
}


int parse_one(int head_char, enum TokenType *out_token_type, int *out_token_value) {
    *out_token_type = 0;
    *out_token_value = 0;
    int cur_char = 0;

    // Read the first character and reset head_char for the initial case.
    if (head_char == '\0') {
        head_char = cl_getc();
    }

    // Parse one unit for each head_char type.
    if (head_char == ' ') {
        *out_token_value = ' ';
        *out_token_type = SPACE;

        // Move to the next non-space chracter.
        while ((cur_char = cl_getc()) != EOF) {
            if (cur_char != ' ') {
                return cur_char;
            }
        }
    } else if (is_digit(head_char)) {
        // Set the first number come from the head character.
        *out_token_value = (head_char - '0');
        *out_token_type = NUMBER;

        // Compute output integer until reaching to the next space string.
        // Since while loop continues to the next space character,
        // out_token_value has to be computed after if condition.
        while ((cur_char = cl_getc()) != EOF) {
            if (!is_digit(cur_char)) {
                return cur_char;
            }
            *out_token_value = (10 * *out_token_value) + (cur_char - '0');
        }
    }

    return cur_char;
}


static void test_parse_one_123() {
    char *input = "123";
    int expected = 123;

    cl_getc_set_src(input);
    int head_char = '\0';
    enum TokenType out_token_type;
    int actual = 0;

    parse_one(head_char, &out_token_type, &actual);

    assert(expected == actual);
}


static void test_parse_one_123_456() {
    char *input = "123  456";
    enum TokenType expected_t1 = NUMBER;
    int expected_v1 = 123;
    int expected_h1 = ' ';
    enum TokenType expected_t2 = SPACE;
    int expected_v2 = ' ';
    int expected_h2 = '4';
    enum TokenType expected_t3 = NUMBER;
    int expected_v3 = 456;
    int expected_h3 = EOF;

    cl_getc_set_src(input);
    int head_char = '\0';
    enum TokenType actual_t;
    int actual_v = 0;

    head_char = parse_one(head_char, &actual_t, &actual_v);
    assert(expected_t1 == actual_t);
    assert(expected_v1 == actual_v);
    assert(expected_h1 == head_char);

    head_char = parse_one(head_char, &actual_t, &actual_v);
    assert(expected_t2 == actual_t);
    assert(expected_v2 == actual_v);
    assert(expected_h2 == head_char);

    head_char = parse_one(head_char, &actual_t, &actual_v);
    assert(expected_t3 == actual_t);
    assert(expected_v3 == actual_v);
    assert(expected_h3 == head_char);
}


int main() {
    int answer1 = 0;
    int answer2 = 0;

    // Unit test.
    test_parse_one_123();
    test_parse_one_123_456();

    // Reset the input string.
    cl_getc_set_src("123 456");

    // write something here.
    // Initialization.
    enum TokenType out_token_type;
    int out_token_value = 0;
    int head_char = '\0';
    int answers[2];

    // Parse one by one.
    int idx = 0;
    while (head_char != EOF) {
        head_char = parse_one(head_char, &out_token_type, &out_token_value);
        if (out_token_type == NUMBER) {
            answers[idx] = out_token_value;
            idx++;
        }
    }

    // Store the results.
    answer1 = answers[0];
    answer2 = answers[1];

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);

    return 1;

}

