#include <stdio.h>
#include <sys/mman.h>

int sum_till(int a) {
    int sum=0;
    for(int i = 0; i < a; i++) {
        sum+=i;
    }
    return sum;
}

int sum_till_inline(int a) {
    int res;
    /*
    Use r1 as loop counter.
    Use r2 to store temp result.
    */
    asm("mov r1, #0");
    asm("mov r2, #0");
    asm("loop:");
    asm("cmp r0, r1");
    asm("beq end");
    asm("add r2, r2, r1"); // sum+=i
    asm("add r1, r1, #1"); // i++    
    asm("b loop");
    asm("end:");
    asm("mov %0, r2" :"=r"(res));
    return res;
}


/*
JIT
*/
int *binary_buf;

int* allocate_executable_buf(int size) {
    return (int*)mmap(0, size,
                 PROT_READ | PROT_WRITE | PROT_EXEC,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}


void jit_sum_till() {
    /*
    input:
      r0: input integer
    Use r2 to store tmp result.
    Use r3 as loop counter.
    */
    binary_buf[0] = 0xe3a02000;  //  0: mov r2, #0
    binary_buf[1] = 0xe3a03000;  //  4: mov r3, #0
    binary_buf[2] = 0xea000001;  //  8: b (Go to 0x14)
    binary_buf[3] = 0xe0822003;  //  c: add r2, r2, r3
    binary_buf[4] = 0xe2833001;  // 10: add r3, r3, #1
    binary_buf[5] = 0xe1530000;  // 14: cmp r3, r0
    binary_buf[6] = 0xbafffffb;  // 18: blt (Go to 0xc)
    binary_buf[7] = 0xe1a00002;  // 1c: mov r0, r2
    binary_buf[8] = 0xe1a0f00e;  // 20: mov r15, r14
}


/*
test code
*/


void assert_true(int boolflag) {
    if(!boolflag) {
        printf("assert fail\n");
    }
}


int main() {
    int res;
    int (*funcvar)(int);

    res = sum_till(10);
    assert_true(res == 45);

    res = sum_till_inline(10);
    assert_true(res == 45);

    /* TODO: Pass below code.
    */
    binary_buf = allocate_executable_buf(1024);

    jit_sum_till();

    funcvar = (int(*)(int))binary_buf;
    res = funcvar(10);
    assert_true(res == 45);

    res = funcvar(11);
    assert_true(res == 55);

    return 0;
}

