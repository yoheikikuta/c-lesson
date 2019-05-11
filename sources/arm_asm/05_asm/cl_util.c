#include "cl_util.h"
#include "cl_getline.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>


void assert_two_num_eq(int num1, int num2) {
    return assert(num1 == num2);
}

void assert_str_substr_eq(char* str, struct Substring* substr) {
    char extracted_substr[BUF_SIZE] = {'\0'};
    strncpy(&extracted_substr, substr->str, substr->len);

    return assert(strcmp(str, extracted_substr) == 0);
}