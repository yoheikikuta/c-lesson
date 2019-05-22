#include "cl_util.h"
#include "assembler.h"
#include <stdio.h>


// USAGE: ./regression_test_single_file.out \
//   /test/test_expect/test_assembler.bin \
//   /test/test_input/test_assembler.bin
int main(int argc, char* argv[]) {
    FILE* fp_expect = get_fp(argv[1], FREADBIN);
    if (fp_expect == NULL) {
        printf("Faile to open.");
        return EOF;
    }
    FILE* fp_actual = get_fp(argv[2], FREADBIN);
    if (fp_actual == NULL) {
        printf("Faile to open.");
        return EOF;
    }

    struct Word words_expect[WORD_BUF_SIZE] = {NO_WORD_TYPE, {.number = 0}};
    struct Word words_actual[WORD_BUF_SIZE] = {NO_WORD_TYPE, {.number = 0}};
    struct Emitter emitter_expect;
    struct Emitter emitter_actual;
    emitter_expect.words = words_expect;
    emitter_expect.pos = 0;
    emitter_actual.words = words_actual;
    emitter_actual.pos = 0;

    int pos_expect = 0;
    while(fread(&emitter_expect.words[pos_expect++].u.number, 1, 4, fp_expect) != NULL) {
        emitter_expect.words[pos_expect].wtype = WORD_NUMBER;
        emitter_expect.pos = pos_expect;
    }
    int pos_actual = 0;
    while(fread(&emitter_actual.words[pos_actual++].u.number, 1, 4, fp_actual) != NULL) {
        emitter_actual.words[pos_actual].wtype = WORD_NUMBER;
        emitter_actual.pos = pos_actual;
    }

    if (pos_expect != pos_actual) {
        printf("Expect and actual binary files have different length.");
        return 1;
    }

    // Print diff of each word like "Found diff in 0x00000014: (expect) 101F1000 (actual) 10F11000".
    for (int i = 0; i < pos_expect; i++) {
        int line_num = i * 4;
        if (emitter_expect.words[i].u.number != emitter_actual.words[i].u.number) {
            printf("  Found diff in 0x%08X:", line_num);
            printf(" (expect) %08X", emitter_expect.words[i].u.number);
            printf(" (actual) %08X", emitter_actual.words[i].u.number);
            printf("\n");
        }
    }
 
    fclose(fp_expect);
    fclose(fp_actual);

    return 0;
}