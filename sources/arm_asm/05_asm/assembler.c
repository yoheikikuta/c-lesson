#include "assembler.h"
#include "parser.h"
#include "cl_util.h"
#include "cl_getline.h"


static struct Word words[WORD_BUF_SIZE] = {NO_WORD_TYPE, {.number = 0}};

// Return the next non-space character: " a" -> 'a'
int get_next_nonsp_ch(char* str) {
    int len_read_ch = 0;
    int ch = str[len_read_ch];

    while (is_space(ch)) {
        ch = str[++len_read_ch];
    }

    return ch;
}

void emit_word(struct Emitter* emitter, struct Word word) {
    int pos = emitter->pos;
    emitter->words[pos].u.number = word.u.number;
    emitter->pos += 1;
}

// Return 1 if the input str is equal to substr.str.
int is_str_equal_to_substr(char* str, struct Substring substr) {
    return strncmp(str, substr.str, substr.len) == 0;
}

// mov: " r1, r2" -> return 0, out_result_hex = E1A01002 (Big Endian)
int asm_mov(char* str, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {WORD_NUMBER, {.number = 0x0}};
    int register_dest = 0;

    len_read_ch = parse_register(str, &register_dest);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    int next_ch = get_next_nonsp_ch(str);

    if ((next_ch == 'r') || (next_ch == 'R')) {
        // 0xE1A0X00X
        word.u.number = 0xE1A00000;
        word.u.number += register_dest << 12;
        int register_op2 = 0;            
        len_read_ch = parse_register(str, &register_op2);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;
        word.u.number += register_op2;
    } else if (next_ch == '#') {
        // 0xE3A0XXXX
        word.u.number = 0xE3A00000;
        word.u.number += register_dest << 12;
        int immediate_v = 0;
        len_read_ch = parse_immediate_value(str, &immediate_v);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;
        word.u.number += immediate_v;
    } else {
        return ASM_FAILURE;
    }

    *out_word = word;
    return 0;
}

// .raw: " 0x12345678" -> return 0, out_result_hex = 0x12345678
int asm_raw(char* str, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};

    int number = 0x0;
    word.wtype = WORD_NUMBER;
    len_read_ch = parse_int(str, &number);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number = number;

    *out_word = word;
    return 0;
}

// Assemble a given line.
int asm_one(char* str, struct Word* out_word) {
    struct Substring substr = {'\0'};
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};

    str += parse_one(str, &substr);
    if (is_str_equal_to_substr("mov", substr)) {
        if (asm_mov(str, &word) == ASM_FAILURE) return ASM_FAILURE; 
        *out_word = word;
        return 0;
    } else if (is_str_equal_to_substr(".raw", substr)) {
        if (asm_raw(str, &word) == ASM_FAILURE) return ASM_FAILURE;
        *out_word = word;
        return 0;
    } else {
        return ASM_FAILURE;
    }
}

// At this stage, assemble input file and print dumped hex numbers.
// To be implemented to make the output file later.
int assemble() {
    char* str_line;
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};
    struct Emitter emitter;
    emitter.words = words;
    emitter.pos = 0;

    while(cl_getline(&str_line) != EOF) {
        if (asm_one(str_line, &word) == ASM_FAILURE) return ASM_FAILURE;
        emit_word(&emitter, word);
    }

    // At this stage, print dumped hex numbers for debugging.
    // 00000000 0210A0E1 (Little Endian)
    // 00000004 0AF0A0E1
    // ...
    for (int i = 0; i < emitter.pos; i++) {
        int line_num = i * 4;
        int word = emitter.words[i].u.number;
        printf("%08X %02X%02X%02X%02X\n", line_num, word & 0xFF, (word >> 8) & 0xFF, (word >> 16) & 0xFF, (word >> 24) & 0xFF);
    }

    return 0;
}

// 
// TEST
// 

static void test_asm_one_movr1r2() {
	char* input = "mov r1, r2";
	int expect = 0xE1A01002;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_movr10xFF() {
	char* input = "mov r1,#0xFF";
	int expect = 0xE3A010FF;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_movr150x00008000() {
	char* input = "mov r15, #0x00008000";
	int expect = 0xE3A0F902;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_movr15m0xFF000000() {
	char* input = "mov r15, #0xFF000000";
	int expect = 0xE3A0F4FF;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_mov_fail() {
	char* input = "mov a15, #0xFF";
	int expect = ASM_FAILURE;
	
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};
	int actual = asm_one(input, &word);
	
	assert_two_num_eq(expect, actual);
}

static void test_asm_one_raw_int() {
	char* input = " .raw 0x12345678";
	int expect = 0x12345678;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void unittests() {
    test_asm_one_movr1r2();
    test_asm_one_movr10xFF();
    test_asm_one_movr150x00008000();
    test_asm_one_movr15m0xFF000000();
    test_asm_one_mov_fail();
    test_asm_one_raw_int();

    printf("All unittests successfully passed.\n");
}


int main(int argc, char* argv[]) {
    unittests();

    FILE* fp = get_fp("/sources/arm_asm/05_asm/test_input/test_assembler.ks");
    cl_getline_set_file(fp);
    assemble();

    return 0;
}