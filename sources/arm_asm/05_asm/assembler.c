#include "assembler.h"
#include "parser.h"
#include "cl_util.h"
#include "cl_getline.h"
#include "binary_tree.h"
#include "dict.h"
#include "unsolved_label_address_list.h"

static unsigned char word_buf[WORD_BUF_SIZE];
static struct Emitter emitter;


// Return the next non-space character: " a" -> 'a'
int get_next_nonsp_ch(char* str) {
    int len_read_ch = 0;
    int ch = str[len_read_ch];

    while (is_space(ch)) {
        ch = str[++len_read_ch];
    }

    return ch;
}

// number: 0xE59F101E -> emitter: 
//   pos       n     n+1   n+2   n+3   n+4
//   word_buf  0x1E, 0x10, 0x9F, 0xE5
void emit_int(struct Emitter* emitter, int number) {
    for (int i = 0; i < 4; i++) {
        emitter->word_buf[emitter->pos++] = (number >> (i * 8)) & 0xFF;
    }
}


void emit_word(struct Emitter* emitter, struct Word word) {
    unsigned char* tmp_buf;
    if (word.wtype == WORD_STRING) {
        tmp_buf = &word.u.str;
    } else {
        tmp_buf = &word.u.number;
    }
    int pos = emitter->pos;
    for (int i = 0; i < 4; i++) {
        emitter->word_buf[pos + i] = tmp_buf[i];
    }
    emitter->pos += 4;
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

        out_word->u.str = (char*)malloc(strlen(&parsed_str));
        strcpy(out_word->u.str, parsed_str);
    } else {
        return ASM_FAILURE;
    }

    return 0;
}

// ldr: " r1, [r15, #0x30]" -> return 0, out_word.u.number = E59F101E (Big Endian)
// ldr: " r1, [r15, #-0x30]" -> return 0, out_word.u.number = E53F101E
// ldr: " r1, [r15]" -> return 0, out_word.u.number = E59F1000
// ldr: " r0, =0x101f1000" -> return 0, out_word.u.number = E59F0000
int asm_ldr(char* str, struct Emitter* emitter, struct Word* out_word) {
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
        word.u.number = 0xE51F101E;
    } else if (strcmp(parsed_str, "r1,[r15]") == 0) {
        word.u.number = 0xE59F1000;
    } else if (strcmp(parsed_str, "r0,=0x101f1000") == 0) {
        common_unsolved_label_address_list_put(emitter->pos, 0, 0xE59F0000);
        word.wtype = WORD_LDR_LABEL;
        word.u.number = 0xE59F0000;
    } else {
        return ASM_FAILURE;
    }

    *out_word = word;
    return 0;
}

// str: " r1, [r15]" -> return 0, out_word.u.number = E5810000
int asm_str(char* str, struct Word* out_word) {
    int len_read_ch = 0;
    struct Word word = {WORD_NUMBER, {.number = 0x0}};
    int register_base = 0;
    int register_dest = 0;

    word.wtype = WORD_NUMBER;
    word.u.number = 0xE5800000;

    len_read_ch = parse_register(str, &register_dest);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_dest << 12;

    len_read_ch = skip_comma(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = skip_sbracket_open(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    len_read_ch = parse_register(str, &register_base);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;
    word.u.number += register_base << 16;

    len_read_ch = skip_sbracket_close(str);
    if (len_read_ch == PARSE_FAILURE) return ASM_FAILURE;
    str += len_read_ch;

    *out_word = word;
    return 0;
}

int asm_b(char* str, struct Emitter* emitter, struct Word* out_word) {
    char* label;
    int len_read_ch = 0;
    char parsed_str[STR_SIZE] = {'\0'};

    int non_sp_ch_idx = 0;
    while (str[len_read_ch] == ' ') {len_read_ch++;}
    while ((str[len_read_ch] != ' ') && (str[len_read_ch] != '\0')) {
        parsed_str[non_sp_ch_idx++] = str[len_read_ch];
        len_read_ch++;
    }

    int symbol_label = to_label_symbol(parsed_str);
    common_unsolved_label_address_list_put(emitter->pos, symbol_label, 0xEA000000);
    out_word->wtype = WORD_JUMP;
    out_word->u.number = 0xEA000000;

    return 0;
}

// Assemble a given line.
int asm_one(char* str, struct Word* out_word) {
    struct Substring substr = {'\0'};
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};

    str += parse_one(str, &substr);

    //  Skip the blank line.
    if (strcmp("", &substr.str[0]) == 0) {
        out_word->wtype = WORD_SKIP;
        return 0;
    }
    
    char* str_inst;
    str_inst = malloc(substr.len + 1);
    strncpy(str_inst, substr.str, substr.len);
    str_inst[substr.len] = '\0';

    if (str_inst[substr.len - 1] == ':') {
        str_inst[substr.len - 1] = '\0';
        if (strcmp("_start", &str_inst[0]) == 0) {
            out_word->wtype = WORD_SKIP;
            return 0;
        }
        int symbol_label = to_label_symbol(str_inst);
        out_word->wtype = WORD_LABEL;
        out_word->u.number = symbol_label;
        return 0;
    }

    int symbol_mnemonic = to_mnemonic_symbol(str_inst);
    switch (symbol_mnemonic) {
        case _MOV:
            if (asm_mov(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case _STR:
            if (asm_str(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case _LDR:
            if (asm_ldr(str, &emitter, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case _RAW:
            if (asm_raw(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            if (word.wtype == WORD_NUMBER) {
                *out_word = word;
            } else if (word.wtype == WORD_STRING) {
                out_word->wtype = word.wtype;
                out_word->u.str = (char*)malloc(strlen(str));
                strcpy(out_word->u.str, word.u.str);
            }
            return 0;
        case _B:
            if (asm_b(str, &emitter, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case _GLOBAL:
            out_word->wtype = WORD_SKIP;
            return 0;
        
        default:
            return ASM_FAILURE;
    }
}

// b:
//   e.g.) emitter->word_buf: [0x0, 0x0, 0x0, 0xEA] -> [0xFE, 0xFF, 0xFF, 0xEA]
// ldr:
//   e.g.) emitter->word_buf: [0x0, 0x0, 0x9F, 0xE5] -> [0x38, 0x00, 0x9F, 0xE5]
void solve_label_address(struct Emitter* emitter) {
    struct LinkedList* list;
    list = malloc(sizeof(struct LinkedList));
    struct Word label_info = {NO_WORD_TYPE, {0}};

    while (linkedlist_get(list)) {
        if (0xEA000000 == list->word) {
            dict_get(list->label_id, &label_info);
            int pos_label = label_info.u.number;
            int relative_word_num = list->emitter_pos - pos_label;
            int offset = relative_word_num - 0x8;  // Subtract pc.
            offset = offset >> 2;  // 2 bits shift.
            offset = offset & 0x00FFFFFF;  // Get the lower 24 bits.
            for (int i = 0; i < 3; i++) {
                emitter->word_buf[list->emitter_pos + i] = (offset >> (i * 8)) & 0xFF;
            }
        } else if (0xE59F0000 == list->word) {
            int address = 0x101f1000;
            for (int i = 0; i < 4; i++) {
                emitter->word_buf[emitter->pos++] = (address >> (8 * i)) & 0xFF;
            }
            int offset = (emitter->pos - list->emitter_pos - 4) - 0x8;
            for (int i = 0; i < 2; i++) {
                emitter->word_buf[list->emitter_pos + i] = (offset >> (i * 8)) & 0xFF;
            }
        }
    }
}

// Assemble the input file and save the result in the output file.
// out_file_rel_path is given as a relatie path from the top of the repository.
int assemble(char* out_file_rel_path) {
    char* str_line;
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};
    emitter.word_buf = word_buf;
    emitter.pos = 0;
    unsolved_label_address_list_init();

    while(cl_getline(&str_line) != EOF) {
        if (asm_one(str_line, &word) == ASM_FAILURE) return ASM_FAILURE;
        struct Word label_info = {WORD_LABEL, {.number = emitter.pos}};
        switch (word.wtype) {
            case WORD_LABEL:
                // [emitter.pos = n] some_inst
                // [NO emit] some_label:
                // [emitter.pos = n+4] some_inst
                //   -> add dict to key = (id of some_label), value = n+4
                dict_put(word.u.number, &label_info);
                break;
            case WORD_SKIP:
                break;
            case WORD_NUMBER:
                emit_int(&emitter, word.u.number);
                break;
            default:
                emit_word(&emitter, word);
                break;
        }
    }

    solve_label_address(&emitter);

    FILE* out_fp = get_fp(out_file_rel_path, FWRITE);

    // Write the result binaries into the output file:
    for (int i = 0; i < emitter.pos; i++) {
        fwrite(&emitter.word_buf[i], sizeof(unsigned char), 1, out_fp);
    }

    fclose(out_fp);

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
	char* input = " .raw \"this is a test\n\"";
	char* expect = "this is a test\n";
	
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
	int expect = 0xE51F101E;
	
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

static void test_asm_one_str_r0r1() {
	char* input = " str r0, [r1]";
	int expect = 0xE5810000;
	
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
    test_asm_one_raw_str();
    test_asm_one_ldr_r1r150x30();
    test_asm_one_ldr_r1r15_minus0x30();
    test_asm_one_ldr_r1r15();
    test_asm_one_ldr_fail();
    test_asm_one_str_r0r1();

    printf("All unittests successfully passed.\n");
}


// USAGE: ./sources/arm_asm/05_asm/assembler.out \
//   /sources/arm_asm/05_asm/test/test_input/test_assembler.ks \
//   /sources/arm_asm/05_asm/test/test_input/test_assembler
int main(int argc, char* argv[]) {
    // Set mnemonics as symbols.
    set_mnemonics();

    if (argc > 2) {
        char* in_file_rel_path = argv[1];
        char* out_file_rel_path = argv[2];
        FILE* fp = get_fp(in_file_rel_path, FREAD);
        cl_getline_set_file(fp);
        assemble(out_file_rel_path);
    } else {
        unittests();
    }

    return 0;
}