#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/
enum TokenType {NUMBER, SPACE};

int main() {
    int answer1 = 0;
    int answer2 = 0;

    // write something here.
    // Initialization.
    enum TokenType out_token_type;
    int out_token_value = 0;
    int next_str_int = 0;
    int answers[2];

    // Read the first string integer.
    next_str_int = cl_getc();

    // Parse one by one.
    int idx = 0;
    while (next_str_int != EOF) {
        next_str_int = parse_one(next_str_int, &out_token_type, &out_token_value);
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


int parse_one(int input_int, enum TokenType *out_token_type, int *out_token_value) {
    *out_token_type = 0;
    *out_token_value = 0;
    enum TokenType val_type;
    int str_int = 0;

    // Set the token type of the input integer.
    if (input_int == ' ') {
        val_type = SPACE;
    } else {
        val_type = NUMBER;
    }

    // Parse one unit.
    switch (val_type) {
        case SPACE:
            *out_token_value = ' ' - 0;
            *out_token_type = SPACE;
            // Move to the next non-space string.
            while ((str_int = cl_getc()) != EOF) {
                if (str_int != ' ') {break;}
            }
            break;
        case NUMBER:
            // Set the first number come from the input integer.
            *out_token_value = (input_int - '0');
            *out_token_type = NUMBER;
            // Compute output integer until reaching to the next space string.
            while ((str_int = cl_getc()) != EOF) {
                if (str_int == ' ') {break;}
                *out_token_value = (10 * *out_token_value) + (str_int - '0');
            }
            break;
    }

    // Return the next string integer.
    return str_int;
}
