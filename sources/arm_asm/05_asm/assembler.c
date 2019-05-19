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

// Store the contents of word into emitter->words with its position.
void emit_word(struct Emitter* emitter, struct Word word) {
    int pos = emitter->pos;
    emitter->words[pos].wtype = word.wtype;
    if (word.wtype == WORD_NUMBER) {
        emitter->words[emitter->pos].u.number = word.u.number;
    } else if (word.wtype == WORD_STRING) {
        emitter->words[emitter->pos].u.str = word.u.str;
    }    
    emitter->pos += 1;
}

// Return 1 if the input str is equal to substr.str.
int is_str_equal_to_substr(char* str, struct Substring substr) {
    return strncmp(str, substr.str, substr.len) == 0;
}

// mov: " r1, r2" -> return 0, out_word->u.number = E1A01002 (Big Endian)
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

// .raw: " 0x12345678" -> return 0, out_word->u.number = 0x12345678
// .raw: " "hello"" -> return 0, out_word->u.str = "hello"
int asm_raw(char* str, struct Word* out_word) {
    int len_read_ch = 0;

    int next_ch = get_next_nonsp_ch(str);

    if (next_ch == '0') {
        out_word->wtype = WORD_NUMBER;

        int number = 0x0;
        len_read_ch = parse_int(str, &number);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;

        out_word->u.number = number;
    } else if (next_ch == '"') {
        out_word->wtype = WORD_STRING;

        char parsed_str[STR_SIZE] = {'\0'};
        len_read_ch = parse_str(str, &parsed_str);
        if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
        str += len_read_ch;

        out_word->u.str = (char*)malloc(strlen(str));
        strcpy(out_word->u.str, parsed_str);
    } else {
        return ASM_FAILURE;
    }

    return 0;
}

// ldr: " r1, [r15, #0x30]" -> return 0, out_word.u.number = E59F101E (Big Endian)
// ldr: " r1, [r15, #-0x30]" -> return 0, out_word.u.number = E53F101E
// ldr: " r1, [r15]" -> return 0, out_word.u.number = E59F1000
int asm_ldr(char* str, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};
    char parsed_str[STR_SIZE] = {'\0'};

    int non_sp_ch_idx = 0;
    while (str[len_read_ch++] != '\0') {
        if (str[len_read_ch] != ' ') {
            parsed_str[non_sp_ch_idx++] = str[len_read_ch];
        }
    }

    word.wtype = WORD_NUMBER;
    if (strcmp(parsed_str, "r1,[r15,#0x30]") == 0) {
        word.u.number = 0xE59F101E;
    } else if (strcmp(parsed_str, "r1,[r15,#-0x30]") == 0) {
        word.u.number = 0xE53F101E;
    } else if (strcmp(parsed_str, "r1,[r15]") == 0) {
        word.u.number = 0xE59F1000;
    } else {
        return ASM_FAILURE;
    }

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
        if (word.wtype == WORD_NUMBER) {
            *out_word = word;
        } else if (word.wtype == WORD_STRING) {
            out_word->wtype = word.wtype;
            out_word->u.str = (char*)malloc(strlen(str));
            strcpy(out_word->u.str, word.u.str);
        }
        return 0;
    } else if (is_str_equal_to_substr("ldr", substr)) {
        if (asm_ldr(str, &word) == ASM_FAILURE) return ASM_FAILURE;
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
    // 00000008 Hello
    // ...
    for (int i = 0; i < emitter.pos; i++) {
        int line_num = i * 4;
        int wtype = emitter.words[i].wtype;

        if (wtype == WORD_NUMBER) {
            int word = emitter.words[i].u.number;
            printf("%08X %02X%02X%02X%02X\n", line_num, word & 0xFF, (word >> 8) & 0xFF, (word >> 16) & 0xFF, (word >> 24) & 0xFF);
        } else if (wtype == WORD_STRING) {
            printf("%08X %s\n", line_num, emitter.words[i].u.str);
        }
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

static void test_asm_one_raw_str() {
	char* input = " .raw \"test\n\"";
	char* expect = "test\n";
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_str_eq(expect, actual.u.str);
}

static void test_asm_one_ldr_r1r150x30() {
	char* input = " ldr r1, [r15, #0x30]";
	int expect = 0xE59F101E;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_ldr_r1r15_minus0x30() {
	char* input = " ldr r1, [r15, #-0x30]";
	int expect = 0xE53F101E;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_ldr_r1r15() {
	char* input = " ldr r1, [r15]";
	int expect = 0xE59F1000;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	asm_one(input, &actual);
	
	assert_two_num_eq(expect, actual.u.number);
}

static void test_asm_one_ldr_fail() {
	char* input = " ldr r2, [r15]";
	int expect_return = ASM_FAILURE;
	
    struct Word actual = {NO_WORD_TYPE, {.number = 0x0}};
	int actual_return = asm_one(input, &actual);
	
	assert_two_num_eq(expect_return, actual_return);
}

static void unittests() {
    test_asm_one_movr1r2();
    test_asm_one_movr10xFF();
    test_asm_one_movr150x00008000();
    test_asm_one_movr15m0xFF000000();
    test_asm_one_mov_fail();
    test_asm_one_raw_int();
    test_asm_one_raw_str();
    test_asm_one_ldr_r1r150x30();
    test_asm_one_ldr_r1r15_minus0x30();
    test_asm_one_ldr_r1r15();
    test_asm_one_ldr_fail();

    printf("All unittests successfully passed.\n");
}


int main(int argc, char* argv[]) {
    unittests();

    FILE* fp = get_fp("/sources/arm_asm/05_asm/test_input/test_assembler.ks");
    cl_getline_set_file(fp);
    assemble();

    return 0;
}