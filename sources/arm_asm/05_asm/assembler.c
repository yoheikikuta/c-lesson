#include "assembler.h"
#include "parser.h"
#include "cl_util.h"
#include "cl_getline.h"


void emit_word(struct Emitter* emitter, int oneword) {
    int pos = emitter->pos;
    emitter->word_buf[pos] = oneword;
    emitter->pos += 1;
}

// Assemble a given line:
//   " mov r1, r2" -> return E1A01002 (Bit Endian)
int asm_one(char* str) {
    struct Substring substr = {'\0'};
    int len_read_ch = 0;
    int result_hex = 0x00000000;

    str += parse_one(str, &substr);
    if(strncmp("mov", substr.str, substr.len) == 0) {
        result_hex += 0xE1A00000;
        int register_dest = 0;
        int register_op2 = 0;
        str += parse_register(str, &register_dest);
        result_hex += register_dest << 12;
        str += skip_comma(str);
        str += parse_register(str, &register_op2);
        result_hex += register_op2;
        
        return result_hex;
    }

    return ASM_FAILURE;
}

int assemble() {
    char* str_line;
    struct Emitter emitter;
    for (int i = 0; i < WORD_BUF_SIZE; i++) {
        emitter.word_buf[i] = 0;
    }
    emitter.pos = 0;

    while(cl_getline(&str_line) != EOF) {
        emit_word(&emitter, asm_one(str_line));
    }

    // At this stage, print dumped hex numbers for debugging.
    // 00000000 0210A0E1 (Little Endian)
    // 00000004 0AF0A0E1
    // ...
    for (int i = 0; i < emitter.pos; i++) {
        int line_num = i * 4;
        int word = emitter.word_buf[i];
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
	
	int actual = asm_one(input);
	
	assert_two_num_eq(expect, actual);
}


int main(int argc, char* argv[]) {
    test_asm_one_movr1r2();

    FILE* fp = get_fp("/sources/arm_asm/05_asm/test_input/test_assembler.s");
    cl_getline_set_file(fp);
    assemble();

    return 0;
}