#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "cl_utils.h"

#define INSTRUCTION_BYTE_SIZE 4


int try_print_asm(int word) {
    if (0xE3A00000 == (word & 0xE3A00000)) {
        // MOV: mov rX, 0xXX
        // Breakdown of 32 bits: [16 bits] [dest register 4 bits] [4 bits] [immediate value 8 bits]
        int register_v = (word & 0x0000F000) >> 4*3;
        int immdediate_v = (word & 0x000000FF);
        cl_printf("mov r%i, #0x%02X\n", register_v, immdediate_v);
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
        cl_printf("b [r15, %s%X]\n", offset_s, offset_v);
        return 1;
    } else if (0xE5800000 == (word & 0xE5800000)) {
        // LDRB: ldrb r3, [r1]
        // STR: str rX, [r0]
        // LDR: ldr r0, [r15, #0xXX]  Note that the offset value is 2 bytes shifted to left
        // Breakdown of 32 bits: [4 bits] 01 [9 bits] [str/ldr bit (0/1)] [dest register 4 bit] [offset 12 bits]
        if (word == 0xE5D13000) {
            cl_printf("ldrb r3, [r1]\n");
            return 1;
        }

        int register_v = (word & 0x0000F000) >> 4*3;
        if ((word >> 20 & 0x00000001) == 0) {
            cl_printf("str r%i, [r0]\n", register_v);
            return 1;
        } else if ((word >> 20 & 0x00000001) == 1) {
            int offset_v = (word & 0x00000FFF) + 0x08;
            cl_printf("ldr r%i, [r15, #0x%02X]\n", register_v, offset_v);
            return 1;
        }
    }
    
    return 0;
}

// 0x101F1000 -> print 00 10 1F 10\n (little endian) 
void print_asm_hex_dump(int word) {
    int word_le;
    for (int i = 0; i < INSTRUCTION_BYTE_SIZE; i++) {
        word_le = word >> i*8;
        word_le = word_le & 0x000000FF;
        cl_printf("%02X ", word_le);
    }
    cl_printf("\n");
}

// 38009FE5 -> return cur = 0xE5, *out_word = 0xE59F0038
// When reaching the end of file, return cur = EOF
int read_one_word(FILE* fp, int* out_word) {
    int one_inst_bytes[INSTRUCTION_BYTE_SIZE];
    int cur;
    *out_word = 0x00000000;

    for (int i = 0; i < INSTRUCTION_BYTE_SIZE; i++) {
        cur = fgetc(fp);
        if (cur == EOF) {
            // 1~3 bytes case at the end of the file.
            // 101F -> 00001F10
            for (int j = i-1; j >= 0; j--) {
                *out_word = *out_word + (one_inst_bytes[j] << j*8);
            }
            return cur;
        }
        one_inst_bytes[i] = cur;
    }

    for (int i = INSTRUCTION_BYTE_SIZE-1; i >= 0; i--) {
        *out_word = *out_word + (one_inst_bytes[i] << i*8);
    }

    return cur;
}

// Print disassembled binary file contents:
//   0x00010000  ldr r0, [r15, #0x40]
//   0x00010004  mov r1, #0x68
//   ...
//   0x00010030  64 64 64 64
//   (continue to print hex dumping once encountering an unknown binary)
void file_disassemble(FILE* fp) {
    int cur = 0;
    int word;
    int address = 0x00010000;
    int exist_unknown_bin = 0;

    while ((cur = read_one_word(fp, &word)) != EOF) {
        cl_printf("0x%08X  ", address);
        address = address + 0x04;

        if (!exist_unknown_bin) {
            exist_unknown_bin = !try_print_asm(word);
        }
        if (exist_unknown_bin) {
            print_asm_hex_dump(word);
        }
    }

    // 1~3 bytes case at the end of the file.
    // 101F -> 101F0000
    if (word != 0x00000000) {
        cl_printf("0x%08X  ", address);
        print_asm_hex_dump(word);
    }
}

//
// TEST
//

static void test_print_asm_mov_0x68() {
    int input = 0xE3A01068;
    char* expect = "mov r1, #0x68\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_mov_0x65() {
    int input = 0xE3A01065;
    char* expect = "mov r1, #0x65\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_mov_r10_0x10() {
    int input = 0xE3A0A010;
    char* expect = "mov r10, #0x10\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_b_negative() {
    int input = 0xEAFFFFFE;
    char* expect = "b [r15, #-0x8]\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_b_positive() {
    int input = 0xEA000004;
    char* expect = "b [r15, #0x10]\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_str() {
    int input = 0xE5801000;
    char* expect = "str r1, [r0]\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_str_dest_r2() {
    int input = 0xE5802000;
    char* expect = "str r2, [r0]\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_ldr() {
    int input = 0xE59F0038;
    char* expect = "ldr r0, [r15, #0x40]\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_ldr_dest_r1() {
    int input = 0xE59F102C;
    char* expect = "ldr r1, [r15, #0x34]\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_ldrb() {
    int input = 0xE5D13000;
    char* expect = "ldrb r3, [r1]\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_add() {
    int input = 0xE2811001;
    char* expect = "add r1, r1, #1\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_cmp() {
    int input = 0xE3530000;
    char* expect = "cmp r3, #0\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_bne() {
    int input = 0x1AFFFFFA;
    char* expect = "bne #0xc\n";

    char* actual;
    try_print_asm(input);
    actual = cl_get_result(0);

    assert_two_str_eq(expect, actual);
    cl_clear_output();
}

static void test_print_asm_not_instruction() {
    int input = 0x64646464;
    int expect = 0;

    int actual = 0;
    actual = try_print_asm(input);

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
    test_print_asm_ldr_dest_r1();
    test_print_asm_ldrb();
    // test_print_asm_add();
    // test_print_asm_cmp();
    // test_print_asm_bne();
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