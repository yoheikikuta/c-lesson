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
    } else if (('0' <= head_char) && (head_char <= '9')) {
        // Set the first number come from the head character.
        *out_token_value = (head_char - '0');
        *out_token_type = NUMBER;

        // Compute output integer until reaching to the next space string.
        // Since while loop continues to the next space character,
        // out_token_value has to be computed after if condition.
        while ((cur_char = cl_getc()) != EOF) {
            if ((cur_char <= '0') || ('9' <= cur_char)) {
                return cur_char;
            }
            *out_token_value = (10 * *out_token_value) + (cur_char - '0');
        }
    }

    return cur_char;
}

