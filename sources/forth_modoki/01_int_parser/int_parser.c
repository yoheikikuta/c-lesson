#include <stdio.h>
#include <assert.h>

static const char* const input = "123 456  1203";


int main() {
    int answer1 = 0;
    int answer2 = 0;
    int answer3 = 0;

    // write something here.
    int parsed_len = 0;

    // 1st int.
    int len = 0;
    len = getcharlen(input);

    answer1 = cl_atoi(input, len, &parsed_len);

    // 2nd int.
    int len1 = 0;
    char *rest1 = input + parsed_len;
    len1 = getcharlen(rest1);

    for (int i = 0; i < len1; i++) {
        rest1 += 1;
        len1 -= 1;
        if (rest1[i] != ' ') {
            break;
        }
    }

    answer2 = cl_atoi(rest1, len1, &parsed_len);

    // 3rd int.
    int len2 = 0;
    char *rest2 = rest1 + parsed_len;
    len2 = getcharlen(rest2);

    for (int i = 0; i < len2; i++) {
        rest2 += 1;
        len2 -= 1;
        if (rest2[i] != ' ') {
            break;
        }
    }

    answer3 = cl_atoi(rest2, len2, &parsed_len);

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);
    assert(answer3 == 1203);

    return 1;
}

int cl_atoi(char *s, int len, int *out_parsed_len) {
    int result = 0;
    *out_parsed_len = 0;
    for (int i = 0; i < len; i++) {
        if (s[i] == ' ') {
            break;
        }
        result = 10*result + (s[i] - '0');
        *out_parsed_len += 1;
    }
    return result;
}

int getcharlen(char *s) {
    int len = 0;
    for (int i=0; s[i] != '\0'; i++){
        len += 1;
    }
    return len;
}
