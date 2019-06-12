#include "cl_util.h"
#include "assembler.h"
#include <stdio.h>


// USAGE: ./regression_test_single_file.out \
//   /test/test_expect/test_assembler.bin \
//   /test/test_input/test_assembler.bin
int main(int argc, char* argv[]) {
    FILE* fp_expect = get_fp(argv[1], FREADBIN);
    if (fp_expect == NULL) {
        printf("Failed to open.");
        return EOF;
    }
    FILE* fp_actual = get_fp(argv[2], FREADBIN);
    if (fp_actual == NULL) {
        printf("Failed to open.");
        return EOF;
    }

    struct Word expect_word_buf[WORD_BUF_SIZE] = {NO_WORD_TYPE, {.number = 0}};
    struct Word actual_word_buf[WORD_BUF_SIZE] = {NO_WORD_TYPE, {.number = 0}};
    struct Emitter expect_emitter;
    struct Emitter actual_emitter;
    expect_emitter.word_buf = expect_word_buf;
    expect_emitter.pos = 0;
    actual_emitter.word_buf = expect_word_buf;
    actual_emitter.pos = 0;

    int pos_expect = 0;
    while(fread(&expect_emitter.word_buf[pos_expect], 1, 1, fp_expect) != NULL) {
        expect_emitter.pos = pos_expect;
        pos_expect++;
    }
    int pos_actual = 0;
    while(fread(&actual_emitter.word_buf[pos_actual], 1, 1, fp_actual) != NULL) {
        actual_emitter.pos = pos_actual;
        pos_actual++;
    }

    if (pos_expect != pos_actual) {
        printf("Expect and actual binary files have different length.");
        return 1;
    }

    // Print diff of each word like "Found diff in 0x00000014: (expect) 101F1000 (actual) 10F11000".
    int expect_word = 0x0;
    int actual_word = 0x0;
    for (int i = 0; i < pos_expect; i++) {
        if (i % 4 == 0) {
            expect_word = 0x0;
            actual_word = 0x0;
            expect_word += expect_emitter.word_buf[i];
            actual_word += actual_emitter.word_buf[i];
        } else if (i % 4 == 1) {
            expect_word += expect_emitter.word_buf[i] << 8;
            actual_word += actual_emitter.word_buf[i] << 8;
        } else if (i % 4 == 2) {
            expect_word += expect_emitter.word_buf[i] << 16;
            actual_word += actual_emitter.word_buf[i] << 16;
        } else if (i % 4 == 3) {
            expect_word += expect_emitter.word_buf[i] << 24;
            actual_word += actual_emitter.word_buf[i] << 24;
            if (expect_word != actual_word) {
                printf("  Found diff in 0x%08X:", ((i - 3) / 4) * 4);
                printf(" (expect) %08X", expect_word);
                printf(" (actual) %08X", actual_word);
                printf("\n");
            }
        }
    }
 
    fclose(fp_expect);
    fclose(fp_actual);

    return 0;
}