#include <stdio.h>
#include <assert.h>

static const char* const input = "123 456  1203";


int main() {
    int answer1 = 0;
    int answer2 = 0;
    int answer3 = 0;

    // write something here.
    int start = 0;
    int end = 0;
    int finish_flg = 0;

    int answers[3];
    int idx = 0;

    while (finish_flg != 1) {
        for (int i = start; input[i] != '\0'; i++) {
            if (input[i] != ' ') {
                start = i;
                for (int j = start; input[j] != '\0'; j++) {
                    if (input[j] == ' ') {
                        end = j - 1; // one character before the space.
                        break;
                    }
                    else {
                        end = j;
                        finish_flg = 1;
                    }
                }

                // store splited strings into arrays.
                int length = end - start;
                char substr[length];
                for (int k = 0; k <= length; k++){
                    substr[k] = input[start+k];
                }
                answers[idx] = atoi(substr);
                idx += 1;

                // proceed with loop index by strings length
                i += end - start;
            }
        }
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
