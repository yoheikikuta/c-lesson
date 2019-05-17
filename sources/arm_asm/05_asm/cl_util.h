#pragma once
#include "parser.h"
#include <stdio.h>

/*
Assert if num1 != num2.
Assert if str1 != str2.
Assert if str != substr.
Return the file pointer of a given relative file path.
*/
void assert_two_num_eq(int num1, int num2);
void assert_two_str_eq(char* str1, char* str2);
void assert_str_substr_eq(char* str, struct Substring* substr);
FILE* get_fp(char* rel_path);