#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static char buf[100*1024];
char registers_str[69] = {'\0'};

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

// int expressing [16 bits] [register list 16 bits] ->
//   one register: "r3"
//   mutiple registers: "r2, r8, r12"
char* get_registers_str_from_lower_16bits(int word) {
    int is_first_register = 1;
    for (int i = 0; registers_str[i] != '\0'; i++) {
        registers_str[i] = '\0';
    }

    for (int i = 0; i < 16; i++) {
        if (1 == ((word >> i) & 1)) {
            char register_num_part[3] = "";  // Need 2+1 bytes
            char register_str[6] = "";
            snprintf(register_num_part, 3, "%i", i);

            if (is_first_register) {
                register_str[0] = 'r';
                strcat(register_str, register_num_part);
                strcat(registers_str, register_str);
                is_first_register = 0;
            } else {
                register_str[0] = ',';
                register_str[1] = ' ';
                register_str[2] = 'r';
                strcat(register_str, register_num_part);
                strcat(registers_str, register_str);
            }
        }
    }
    return &registers_str[0];
}