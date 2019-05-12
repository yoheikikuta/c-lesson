#include "assembler.h"
#include "parser.h"
#include "cl_util.h"
#include <string.h>


void emit_word(struct Emitter* emitter, int oneword);

int asm_one(char* str) {
    struct Substring* substr;
    int len_read_ch = 0;
    int result_hex = 0x00000000;

    str += parse_one(str, substr);
    if(strncmp("mov", substr->str, substr->len) == 0) {
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
    return 0;
}