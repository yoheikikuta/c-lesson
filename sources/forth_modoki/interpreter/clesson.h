#pragma once
#include <stdio.h>

/*
return one character and move cursor.
set a string pointer as input and pos as 0,
set a file pointer as input_fp.
*/
int cl_getc();
void cl_getc_set_src(char* str);
void cl_getc_set_file(FILE* input_fp);