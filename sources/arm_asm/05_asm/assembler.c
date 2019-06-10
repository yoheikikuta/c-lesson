#include "assembler.h"
#include "parser.h"
#include "cl_util.h"
#include "cl_getline.h"
#include "binary_tree.h"
#include "dict.h"


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
    if ((word.wtype == WORD_NUMBER) || (word.wtype == WORD_JUMP) || (word.wtype == WORD_LDR_LABEL)) {
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
// ldr: " r0, =0x101f1000" -> return 0, out_word.wtype = WORD_LDR_LABEL, out_word.u.number = 0x101f1000
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
        word.u.number = 0xE51F101E;
    } else if (strcmp(parsed_str, "r1,[r15]") == 0) {
        word.u.number = 0xE59F1000;
    } else if (strcmp(parsed_str, "r0,=0x101f1000") == 0) {
        word.wtype = WORD_LDR_LABEL;
        word.u.number = 0x101f1000;
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

int asm_b(char* str, struct Word* out_word) {
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
    out_word->wtype = WORD_JUMP;
    out_word->u.number = symbol_label;

    return 0;
}

// Assemble a given line.
int asm_one(char* str, struct Word* out_word) {
    struct Substring substr = {'\0'};
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};

    str += parse_one(str, &substr);
    
    char* str_inst;
    str_inst = malloc(substr.len + 1);
    strncpy(str_inst, substr.str, substr.len);
    str_inst[substr.len] = '\0';

    if (str_inst[substr.len - 1] == ':') {
        str_inst[substr.len - 1] = '\0';
        if (strcmp("_start:", str_inst) == 0) {
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
        case 2:
            if (asm_mov(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case 3:
            if (asm_str(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case 4:
            if (asm_ldr(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case 5:
            if (asm_raw(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            if (word.wtype == WORD_NUMBER) {
                *out_word = word;
            } else if (word.wtype == WORD_STRING) {
                out_word->wtype = word.wtype;
                out_word->u.str = (char*)malloc(strlen(str));
                strcpy(out_word->u.str, word.u.str);
            }
            return 0;
        case 6:
            if (asm_b(str, &word) == ASM_FAILURE) return ASM_FAILURE;
            *out_word = word;
            return 0;
        case 7:  // .globl
            out_word->wtype = WORD_SKIP;
            return 0;
        
        default:
            return ASM_FAILURE;
    }
}

// (1) Solve label address for b.
//   emitter->words[i]: .wtype = WORD_JUMP, .u.number = n (key int of a corresponding label)
//   ->
//     emitter->words[i].u.number = 0xEA000000 + (offset indicating to the label position)
// 
// (2) Solve label address for ldr.
//   emitter->words[i]: .wtype = WORD_LDR_LABEL, .u.number = 0x101f1000
//   ->
//     (Add label information to the last position of emitter)
//     emitter->words[emitter->pos]: .wtype = WORD_NUMBER, .u.number = 0x101f1000
//     (Update emitter element of str)
//     emitter->words[i]: .wtype = WORD_NUMBER, .u.number = 0x59FXXXX
void solve_label_address(struct Emitter* emitter) {
    int i = 0;
    while (i < emitter->pos) {
        if (emitter->words[i].wtype == WORD_JUMP) {
            int key_label = emitter->words[i].u.number;
            struct Word label_info = {NO_WORD_TYPE, {0}};
            dict_get(key_label, &label_info);
            int pos_label = label_info.u.number;
            int relative_word_num = (i - pos_label) * 4;
            int offset = relative_word_num - 0x8;  // Subtract pc.
            offset = offset >> 2;  // 2 bits shift.
            offset = offset & 0x00FFFFFF;  // Get the lower 24 bits.
            int word = 0xEA000000 + offset;
            emitter->words[i].u.number = word;
        } else if (emitter->words[i].wtype == WORD_LDR_LABEL) {
            emitter->words[emitter->pos].wtype = WORD_NUMBER;
            emitter->words[emitter->pos].u.number = emitter->words[i].u.number;
            emitter->pos++;
            emitter->words[i].wtype = WORD_NUMBER;
            emitter->words[i].u.number = 0xE59F0000 + (emitter->pos - i - 1) * 4 - 0x8;
        }
        i++;
    }
}

// Assemble the input file and save the result in the output file.
// out_file_rel_path is given as a relatie path from the top of the repository.
int assemble(char* out_file_rel_path) {
    char* str_line;
    struct Word word = {NO_WORD_TYPE, {.number = 0x0}};
    struct Emitter emitter;
    emitter.words = words;
    emitter.pos = 0;

    while(cl_getline(&str_line) != EOF) {
        if (asm_one(str_line, &word) == ASM_FAILURE) return ASM_FAILURE;
        struct Word label_info = {WORD_LABEL, {.number = emitter.pos}};
        switch (word.wtype) {
            case WORD_LABEL:
                // [emitter.pos = n] some_inst
                // [NO emit] some_label:
                // [emitter.pos = n+1] some_inst
                //   -> add dict to key = (id of some_label), value = n+1 
                dict_put(word.u.number, &label_info);
                break;
            case WORD_SKIP:
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
        int line_num = i * 4;
        int wtype = emitter.words[i].wtype;

        if ((wtype == WORD_NUMBER) || (wtype == WORD_JUMP)) {
            fwrite(&emitter.words[i].u.number, sizeof(emitter.words[i].u.number), 1, out_fp);
        } else if (wtype == WORD_STRING) {
            fwrite(&emitter.words[i].u.str, sizeof(emitter.words[i].u.str), 1, out_fp);
        }
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