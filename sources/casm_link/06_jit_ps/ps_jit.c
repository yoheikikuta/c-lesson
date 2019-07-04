#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "parser.h"
#include "test_util.h"
#include "asm.h"

extern int eval(int r0, int r1, char *str);
struct Emitter {
    unsigned char* buf;
    int pos;
};
static struct Emitter emitter;
static unsigned char byte_buf[1024];

// number: 0xE59F101E -> emitter: 
//   pos       n     n+1   n+2   n+3   n+4
//   buf  0x1E, 0x10, 0x9F, 0xE5
void emit_int(struct Emitter* emitter, int number) {
    for (int i = 0; i < 4; i++) {
        emitter->buf[emitter->pos++] = (number >> (i * 8)) & 0xFF;
    }
}

/*
JIT
*/
int *binary_buf = NULL;

int* allocate_executable_buf(int size) {
    return (int*)mmap(0, size,
                 PROT_READ | PROT_WRITE | PROT_EXEC,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void ensure_jit_buf() {
    if(binary_buf == NULL) {
        binary_buf = allocate_executable_buf(1024);
    }
}

void set_binaries(struct Emitter* emitter) {
    for (int i = 0; i < emitter->pos; i = i + 4) {
        int word = 0x0;
        for (int j = 3; j >= 0; j--) {
            word += emitter->buf[j + i] << (j * 8);
        }
        binary_buf[i] = word;
    }
}

/*
Emit the following binaries into emitter:
  ldmia r13!, {r2, r3}
  add r2, r3, r2
  strmdb r13!, r2
"5 3 add" -> (stack bottom) 5 | 3 (stack bottom) -> lmdia -> r2 = 3, r3 = 5 -> r2 = 8
*/
void asm_op_add(struct Emitter* emitter) {
    int word;
    word = asm_ldmia(2, 2, 3);
    emit_int(emitter, word);
    word = asm_add(2, 3, 2);
    emit_int(emitter, word);
    word = asm_stmdb(2);
    emit_int(emitter, word);
}

/*
Emit the following binaries into emitter:
  ldmia r13!, {r2, r3}
  sub r2, r3, r2
  strmdb r13!, r2
"5 3 sub" -> (stack bottom) 5 | 3 (stack bottom) -> lmdia -> r2 = 3, r3 = 5 -> r2 = 2
*/
void asm_op_sub(struct Emitter* emitter) {
    int word;
    word = asm_ldmia(2, 2, 3);
    emit_int(emitter, word);
    word = asm_sub(2, 3, 2);
    emit_int(emitter, word);
    word = asm_stmdb(2);
    emit_int(emitter, word);
}

/*
Emit the following binaries into emitter:
  ldmia r13!, {r2, r3}
  mul r2, r3, r2
  strmdb r13!, r2
"5 3 mul" -> (stack bottom) 5 | 3 (stack bottom) -> lmdia -> r2 = 3, r3 = 5 -> r2 = 15
*/
void asm_op_mul(struct Emitter* emitter) {
    int word;
    word = asm_ldmia(2, 2, 3);
    emit_int(emitter, word);
    word = asm_mul(2, 3, 2);
    emit_int(emitter, word);
    word = asm_stmdb(2);
    emit_int(emitter, word);
}

/*
Emit the following binaries into emitter:
  ldmia r13!, {r2, r3}
  mov r4, #0 (tmp register for storing the quotient)
  mov r5, r2 (tmp register for storing the divisor)
  cmp r3, r2
  blt (to mov r2, r4)
  add r4, r4, #1
  add r2, r2, r5
  b (to cmp r3, r2)
  mov r2, r4
  strmdb r13!, r2
"7 2 div" -> (stack bottom) 7 | 2 (stack bottom) -> lmdia -> r2 = 2, r3 = 7 -> r2 = 3 (quotient)
*/
void asm_op_div(struct Emitter* emitter) {
    int word;
    word = asm_ldmia(2, 2, 3);  // ldmia r13!, {r2, r3}
    emit_int(emitter, word);
    word = asm_mov_immediate_v(4, 0);  // mov r4, #0
    emit_int(emitter, word);
    word = asm_mov_register(5, 2);  // mov r5, r2
    emit_int(emitter, word);
    word = 0xE1530002;  // cmp r3, r2
    emit_int(emitter, word);
    word = 0xBA00000B;  // blt (to mov r2, r4)
    emit_int(emitter, word);
    word = 0xE2844001;  // add r4, r4, #1
    emit_int(emitter, word);
    word = asm_add(2, 2, 5);  // add r2, r2, r5
    emit_int(emitter, word);
    word = 0xEAFFFFEB;  // b (to cmp r3, r2)
    emit_int(emitter, word);
    word = asm_mov_register(2, 4);  // mov r2, r4
    emit_int(emitter, word);
    word = asm_stmdb(2); // stmdb r13!, r2 
    emit_int(emitter, word);
}

/*
input: "3 4 add"
return: binary_buf (binaryies corresponding to the input instructions)
*/
int* jit_script(char *input) {
    ensure_jit_buf();
    emitter.buf = byte_buf;
    emitter.pos = 0;

    struct Substr remain={input, strlen(input)};
    int word;
    int op;

    while (!is_end(&remain)) {
        skip_space(&remain);
        if (is_number(remain.ptr)) {
            // Push a given number into the stack.
            int immediate_v = parse_number(remain.ptr);
            skip_token(&remain);

            word = asm_mov_immediate_v(2, immediate_v);  // mov r2, #X
            emit_int(&emitter, word);
            word = asm_stmdb(2);  // stmdb r13!, r2
            emit_int(&emitter, word);

            continue;
        } else {
            op = parse_word(&remain);
            skip_token(&remain);

            switch(op) {
                case OP_ADD:
                    asm_op_add(&emitter);
                    break;
                case OP_SUB:
                    asm_op_sub(&emitter);
                    break;
                case OP_MUL:
                    asm_op_mul(&emitter);
                    break;
                case OP_DIV:
                    asm_op_div(&emitter);
                    break;
            }

            continue;
        }
    }

    word = asm_ldmia(1, 2);  // ldmia r13!, r2
    emit_int(&emitter, word);
    word = asm_mov_register(0, 2);  // mov r0, r2
    emit_int(&emitter, word);
    word = asm_mov_register(15, 14);  // mov r15, r14
    emit_int(&emitter, word);

    set_binaries(&emitter);
    return binary_buf;
}

void test_jit_sctipr_3() {
    char* input = "3";
    int expect = 3;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

void test_jit_sctipr_3_5() {
    char* input = "3 5";
    int expect = 5;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

void test_jit_sctipr_3_5_add() {
    char* input = "3 5 add";
    int expect = 8;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

void test_jit_sctipr_10_3_sub() {
    char* input = "10 3 sub";
    int expect = 7;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

void test_jit_sctipr_4_9_mul() {
    char* input = "4 9 mul";
    int expect = 36;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

void test_jit_sctipr_3_5_div() {
    char* input = "3 5 div";
    int expect = 0;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

void test_jit_sctipr_5_2_div() {
    char* input = "5 2 div";
    int expect = 2;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

void test_jit_sctipr_18_3_div() {
    char* input = "18 3 div";
    int expect = 6;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

static void run_unit_tests() {
    test_jit_sctipr_3();
    test_jit_sctipr_3_5();
    test_jit_sctipr_3_5_add();
    test_jit_sctipr_10_3_sub();
    test_jit_sctipr_4_9_mul();
    test_jit_sctipr_3_5_div();
    test_jit_sctipr_5_2_div();
    test_jit_sctipr_18_3_div();
    printf("all test done\n");
}


int main() {
    int res;
    int (*funcvar)(int, int);

    run_unit_tests();

    res = eval(1, 5, "3 7 add r1 sub 4 mul");
    printf("res=%d\n", res);

    /*
     TODO: Make below test pass.
    */
    // funcvar = (int(*)(int, int))jit_script("3 7 add r1 sub 4 mul");

    // res = funcvar(1, 5);
    // assert_int_eq(20, res);

    // res = funcvar(1, 4);
    // assert_int_eq(24, res);

    return 0;
}

