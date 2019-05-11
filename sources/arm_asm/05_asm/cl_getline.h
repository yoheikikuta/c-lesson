#pragma once
#include <stdio.h>

#define BUF_SIZE 256

static char buf[BUF_SIZE] = {'\0'};
static FILE* fp = NULL;

/*
return the length of the line and pack strings.
set input file pointer.
*/
int cl_getline(char** out_buf);
void cl_getline_set_file(FILE* input_fp);