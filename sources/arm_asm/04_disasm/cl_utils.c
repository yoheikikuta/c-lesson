#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

static char buf[100*1024];

static int to_buffer = 0;
static int pos = 0;


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
    if(word == 0xE3A01068) {
      cl_printf("mov r1, #0x68\n");
      return 1;
    }
    return 0;
}

//
// TEST
//

static void test_print_asm_mov_0x68() {
    int input = 0xE3A01068;
    int expect = 1;

    int actual = 0;
    actual = print_asm(input);

    assert(expect == actual);
}

static void unit_tests() {
    test_print_asm_mov_0x68();
    printf("All unittests successfully passed.\n");
}


int main(int argc, char* argv[]) {
    unit_tests();
    return 0;
}