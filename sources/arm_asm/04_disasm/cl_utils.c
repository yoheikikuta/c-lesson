#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static char buf[100*1024];

static int to_buffer = 0;
static int pos = 0;


int streq(char* s1, char* s2) {
    return strcmp(s1, s2) == 0;
}

void assert_two_str_eq(char* s1, char* s2) {
    assert(streq(s1, s2));
}

void cl_clear_output() {
    pos = 0;
    buf[0] = '\0';
}

char *cl_get_result(int num) {
    int null_count=0;
    int i=0;
    while (null_count != num) {
        if (buf[i] == '\0') {
            null_count++;
        }
        i++;
    }
    return &buf[i];
}

void cl_enable_buffer_mode() {
    to_buffer = 1;
}

void cl_disable_buffer_mode() {
    to_buffer = 0;
}

void cl_printf(char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);

    if(to_buffer) {
        pos += vsprintf(&buf[pos], fmt, arg_ptr);
        pos++;
    } else {
        vprintf(fmt, arg_ptr);
    }
    va_end(arg_ptr);
}

int print_asm(int word) {
    if(0xE3A00000 == (word & 0xE3A00000)) {
        // MOV: mov rX, 0xXX
        // Breakdown of 32 bits: [16 bits] [dest register 4 bits] [4 bits] [immediate value 8 bits]
        int register_v = (word & 0x0000F000) >> 4*3;
        int immdediate_v = (word & 0x000000FF);
        cl_printf("mov r%i, #0x%x\n", register_v, immdediate_v);
        return 1;
    } else if(0xEA000000 == (word & 0xEA000000)) {
        // BRANCH: b [r15, #0xXX]
        // Breakdown of 32 bits: [4 bits] 101 [1 bit] [offset 24 bits]
        int offset_v;
        char* offset_s;
        int is_negative = (word << 8) < 0;
        if(is_negative) {
            offset_v = (word | 0xFF000000);  // 1111 1111 [offset 24 bits]
            offset_s = "#-0x";
        } else {
            offset_v = (word & 0x00FFFFFF);  // 0000 0000 [offset 24 bits]
            offset_s = "#0x";
        }
        offset_v = abs(offset_v << 2);  // ARM specifications
        cl_printf("b [r15, %s%x]\n", offset_s, offset_v);
        return 1;
    } else if(word == 0xE59F0038) {
        // LDR: ldr r0, [r15, 0x40]
        cl_printf("ldr r0, [r15, #0x40]\n");
        return 1;
    } else if(0xE5800000 == (word & 0xE5800000)) {
        // STR: str rX, [r0]
        // Breakdown of 32 bits: [4 bits] 01 [10 bits] [dest register 4 bit] [12 bits]
        int register_v = (word & 0x0000F000) >> 4*3;
        cl_printf("str r%i, [r0]\n", register_v);
        return 1;
    }
    
    return 0;
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
    unit_tests();
    return 0;
}