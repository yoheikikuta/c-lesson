#pragma once

/*
return 1 if two strings are identical.
assert if two strings are not identical.
reset the buffer.
return the char pointer of a string stored in the buffer
Enable the buffer mode
Disable the buffer mode
Store a string in the buffer if in the buffer mode, stdout else
return pointer of str of registers, such as "r2, r8, r12"
*/
int streq(char* s1, char* s2);
void assert_two_str_eq(char* s1, char* s2);
void cl_clear_output();
char *cl_get_result(int num);
void cl_enable_buffer_mode();
void cl_disable_buffer_mode();
void cl_printf(char *fmt, ...);
char *get_registers_str_from_lower_16bits(int word);