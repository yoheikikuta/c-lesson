#include "asm.h"
#include "parser.h"
#include "stdio.h"
#include <stdarg.h>


/*
input: r_dest = 2, immediate_v = 3
return: 0xE3A02003 (mov r2, #3)
 */
int asm_mov_immediate_v(int r_dest, int immediate_v) {
    int word = 0xE3A00000;
    word += r_dest << 12;
    word += immediate_v;
    
    return word;
}

/*
input: r_dest = 2, r_operand = 3
return: 0xE1A02003 (mov r2, r3)
 */
int asm_mov_register(int r_dest, int r_operand) {
    int word = 0xE1A00000;
    word += r_dest << 12;
    word += r_operand;
    
    return word;
}

/*
input: r = 2
return: 0xE92D0004 (stmdb r13!, r2)
 */
int asm_stmdb(int r) {
    int word = 0xE92D0000;
    word += 0x1 << r; 

    return word;
}

/*
input: r_num = 2, 2,3
return: 0xE8BD000C (ldmia r13!, {r2, r3})
 */
int asm_ldmia(int r_num, ...) {
    va_list ap;
    va_start(ap, r_num);

    int word = 0xE8BD0000;
    for (int i = 0; i < r_num; i++) {
        word += 0x1 << va_arg(ap, int);
    }

    return word;
}

// 
// TEST
// 

void test_asm_mov_immediate_v() {
    int input_r_dest = 10;
    int input_immediate_v = 5;
    int expect = 0xE3A0A005;

    int word = asm_mov_immediate_v(input_r_dest, input_immediate_v);

    assert_int_eq(expect, word);
}

void test_asm_mov_register() {
    int input_r_dest = 15;
    int input_r_operand = 14;
    int expect = 0xE1A0F00E;

    int word = asm_mov_register(input_r_dest, input_r_operand);

    assert_int_eq(expect, word);
}

void test_asm_stmdb() {
    int input_r = 2;
    int expect = 0xE92D0004;

    int word = asm_stmdb(input_r);

    assert_int_eq(expect, word);
}

void test_asm_ldmia() {
    int input_r_num = 1;
    int input_r = 2;
    int expect = 0xE8BD0004;

    int word = asm_ldmia(input_r_num, input_r);

    assert_int_eq(expect, word);
}

static void run_unit_tests() {
    test_asm_mov_immediate_v();
    test_asm_mov_register();
    test_asm_stmdb();
    test_asm_ldmia();
    printf("all test done\n");
}


#if 0
int main() {
    run_unit_tests();
    return 0;
}
#endif
