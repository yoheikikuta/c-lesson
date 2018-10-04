#include <stdio.h>
#include <assert.h>


static const char* const input = "123 456  1203";


int main() {
    int answer1 = 0;
    int answer2 = 0;
    int answer3 = 0;

    // write something here.
    char *rest = input;
    int parsed_len = 0;
    int answers[3] = {0,0,0};
    int rest_len = 0;

    rest_len = cl_strlen(rest);

    // Eliminate the head space(s).
    skip_space(rest, rest_len, &parsed_len);
    rest = rest + parsed_len;
    rest_len = cl_strlen(rest);

    int lc = 0;
    while (rest_len > 0) {
        // Parse one int.
        answers[lc] = parse_one_int(rest, rest_len, &parsed_len);

        // Move the pointer by parsed_len.
        rest = rest + parsed_len;

        // Eliminate space(s) up to the next character.
        skip_space(rest, rest_len, &parsed_len);
        rest = rest + parsed_len;

        // Compute rest_len of the rest string.
        rest_len = cl_strlen(rest);

        lc++;
    }

    answer1 = answers[0];
    answer2 = answers[1];
    answer3 = answers[2];

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);
    assert(answer3 == 1203);

    return 1;
}

int cl_strlen(char *s) {
    int len = 0;
    while (*s++) {
        len++;
    }

    return len;
}

int parse_one_int(char *s, int len, int *out_parsed_len) {
    int parsed_int = 0;
    *out_parsed_len = 0;

    for (int i = 0; i < len; i++) {
        if (s[i] == ' ') {
            break;
        }
        parsed_int = 10*parsed_int + (s[i] - '0');
        *out_parsed_len += 1;
    }

    return parsed_int;
}

void skip_space(char *s, int len, int *out_parsed_len) {
    *out_parsed_len = 0;

    for (int i = 0; i < len; i++) {
        if (s[i] != ' ') {
            break;
        }
        *out_parsed_len += 1;
    }

}
