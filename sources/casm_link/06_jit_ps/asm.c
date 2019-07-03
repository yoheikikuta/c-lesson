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

/*
input: r_dest = 2, r_1st_operand = 3, r_2nd_operand = 4
return: 0xE0832004 (add r2, r3, r4)
 */
int asm_add(int r_dest, int r_1st_operand, int r_2nd_operand) {
    int word = 0xE0800000;
    word += r_dest << 12;
    word += r_1st_operand << 16;
    word += r_2nd_operand;

    return word;
}

/*
input: r_dest = 2, r_1st_operand = 3, r_2nd_operand = 4
return: 0xE0432004 (sub r2, r3, r4)
 */
int asm_sub(int r_dest, int r_1st_operand, int r_2nd_operand) {
    int word = 0xE0400000;
    word += r_dest << 12;
    word += r_1st_operand << 16;
    word += r_2nd_operand;

    return word;
}

/*
input: r_dest = 2, r_1st_operand = 3, r_2nd_operand = 4
return: 0xE0020493 (mul r2, r3, r4)
 */
int asm_mul(int r_dest, int r_1st_operand, int r_2nd_operand) {
    int word = 0xE0000090;
    word += r_dest << 16;
    word += r_1st_operand;
    word += r_2nd_operand << 8;

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

void test_asm_add() {
    int input_r_dest = 2;
    int input_r_1st_operand = 3;
    int input_r_2nd_operand = 4;
    int expect = 0xE0832004;

    int word = asm_add(input_r_dest, input_r_1st_operand, input_r_2nd_operand);

    assert_int_eq(expect, word);
}

void test_asm_sub() {
    int input_r_dest = 2;
    int input_r_1st_operand = 3;
    int input_r_2nd_operand = 4;
    int expect = 0xE0432004;

    int word = asm_sub(input_r_dest, input_r_1st_operand, input_r_2nd_operand);

    assert_int_eq(expect, word);
}

void test_asm_mul() {
    int input_r_dest = 2;
    int input_r_1st_operand = 3;
    int input_r_2nd_operand = 4;
    int expect = 0xE0020493;

    int word = asm_mul(input_r_dest, input_r_1st_operand, input_r_2nd_operand);

    assert_int_eq(expect, word);
}

static void run_unit_tests() {
    test_asm_mov_immediate_v();
    test_asm_mov_register();
    test_asm_stmdb();
    test_asm_ldmia();
    test_asm_add();
    test_asm_sub();
    test_asm_mul();
    printf("all test done\n");
}


#if 0
int main() {
    run_unit_tests();
    return 0;
}
#endif
