#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "cl_utils.h"

#define INSTRUCTION_BYTE_SIZE 4


int print_asm(int word) {
    if (0xE3A00000 == (word & 0xE3A00000)) {
        // MOV: mov rX, 0xXX
        // Breakdown of 32 bits: [16 bits] [dest register 4 bits] [4 bits] [immediate value 8 bits]
        int register_v = (word & 0x0000F000) >> 4*3;
        int immdediate_v = (word & 0x000000FF);
        cl_printf("mov r%i, #0x%x\n", register_v, immdediate_v);
        return 1;
    } else if (0xEA000000 == (word & 0xEA000000)) {
        // BRANCH: b [r15, #0xXX]
        // Breakdown of 32 bits: [4 bits] 101 [1 bit] [offset 24 bits]
        int offset_v;
        char* offset_s;
        int is_negative = (word << 8) < 0;
        if (is_negative) {
            offset_v = (word | 0xFF000000);  // 1111 1111 [offset 24 bits]
            offset_s = "#-0x";
        } else {
            offset_v = (word & 0x00FFFFFF);  // 0000 0000 [offset 24 bits]
            offset_s = "#0x";
        }
        offset_v = abs(offset_v << 2);  // ARM specifications
        cl_printf("b [r15, %s%x]\n", offset_s, offset_v);
        return 1;
    } else if (word == 0xE59F0038) {
        // LDR: ldr r0, [r15, 0x40]
        cl_printf("ldr r0, [r15, #0x40]\n");
        return 1;
    } else if (0xE5800000 == (word & 0xE5800000)) {
        // STR: str rX, [r0]
        // Breakdown of 32 bits: [4 bits] 01 [10 bits] [dest register 4 bit] [12 bits]
        int register_v = (word & 0x0000F000) >> 4*3;
        cl_printf("str r%i, [r0]\n", register_v);
        return 1;
    }
    
    return 0;
}

void file_disassemble(FILE* fp) {
    // Print disassembled binary file contents:
    //   0x00010000  ldr r0, [r15, #0x40]
    //   0x00010004  mov r1, #0x68
    //   ...
    int one_inst_bytes[INSTRUCTION_BYTE_SIZE];
    int pos_byte = 0;
    int word;
    int c;
    int address = 0x00010000;

    while ( (c = fgetc(fp)) != EOF) {
        // Read bytes into one_inst_bytes up to INSTRUCTION_BYTE_SIZE
        one_inst_bytes[pos_byte++] = c;
        if (pos_byte < INSTRUCTION_BYTE_SIZE) {
            continue;
        }
        pos_byte = 0;

        // Create one word (little endian)
        word = 0x00000000;
        for (int i = INSTRUCTION_BYTE_SIZE; i >= 0; --i) {
            word = word + (one_inst_bytes[i] << i*8);
        }

        // Print one line:
        //   known instructions: [address] [mnemonic]
        //   unknown instructions: [address] [hex dump]
        cl_printf("0x%08X  ", address);
        if (!print_asm(word)) {
            for (int i = 0; i < INSTRUCTION_BYTE_SIZE; i++) {
                cl_printf("%02X ", one_inst_bytes[i]);
            }
            cl_printf("\n");
        }
        address = address + 0x04;
    }
}

//
// TEST
//

static void test_print_asm_mov_0x68() {
    int input = 0xE3A01068;
    char* expect = "mov r1, #0x68\n";

    char* actual;
    print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_mov_0x65() {
    int input = 0xE3A01065;
    char* expect = "mov r1, #0x65\n";

    char* actual;
    print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_mov_r10_0x10() {
    int input = 0xE3A0A010;
    char* expect = "mov r10, #0x10\n";

    char* actual;
    print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_b_negative() {
    int input = 0xEAFFFFFE;
    char* expect = "b [r15, #-0x8]\n";

    char* actual;
    print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_b_positive() {
    int input = 0xEA000004;
    char* expect = "b [r15, #0x10]\n";

    char* actual;
    print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_str() {
    int input = 0xE5801000;
    char* expect = "str r1, [r0]\n";

    char* actual;
    print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_str_dest_r2() {
    int input = 0xE5802000;
    char* expect = "str r2, [r0]\n";

    char* actual;
    print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_ldr() {
    int input = 0xE59F0038;
    char* expect = "ldr r0, [r15, #0x40]\n";

    char* actual;
    print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_not_instruction() {
    int input = 0x64646464;
    int expect = 0;

    int actual = 0;
    actual = print_asm(input);

    assert(expect == actual);
}

static void unit_tests() {
    cl_enable_buffer_mode();

    test_print_asm_mov_0x68();
    test_print_asm_mov_0x65();
    test_print_asm_mov_r10_0x10();
    test_print_asm_b_negative();
    test_print_asm_b_positive();
    test_print_asm_str();
    test_print_asm_str_dest_r2();
    test_print_asm_ldr();
    test_print_asm_not_instruction();
    printf("All unittests successfully passed.\n");

    cl_disable_buffer_mode();
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        FILE *fp = NULL;
        char *fname = argv[1];
        if ((fp = fopen(fname, "r")) == NULL) {
			printf("ERROR: cannot read the given file.\n");
		}

        file_disassemble(fp);

        fclose(fp);
    } else {
        unit_tests();
    }
    return 0;
}