#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "parser.h"
#include "test_util.h"

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

int* jit_script(char *input) {
    ensure_jit_buf();
    emitter.buf = byte_buf;
    emitter.pos = 0;
    /*
    Function only returns 3.
    */
    struct Substr remain={input, strlen(input)};
    int word;

    while (!is_end(&remain)) {
        skip_space(&remain);
        if (is_number(remain.ptr)) {
            word = 0xE3A00000 + parse_number(remain.ptr);  // mov r0, #X
            emit_int(&emitter, word);
            skip_token(&remain);
            continue;
        } else {
            continue;
        }
    }
    word = 0xe1a0f00e;  // mov r15, r14
    emit_int(&emitter, word);

    set_binaries(&emitter);
    return binary_buf;
}

void test_jit_sctipr_only_3() {
    char* input = "3";
    int expect = 3;

    int (*funcvar)(int, int);
    funcvar = (int(*)(int, int))jit_script(input);
    int res = funcvar(0, 0);

    assert_int_eq(expect, res);
}

static void run_unit_tests() {
    test_jit_sctipr_only_3();
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

