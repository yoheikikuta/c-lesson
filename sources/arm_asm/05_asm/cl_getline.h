#pragma once
#include <stdio.h>

#define BUF_SIZE 256

static char buf[BUF_SIZE] = {'\0'};
static FILE* fp = NULL;

/*
Return the length of the line and pack strings.
Set input file pointer.
*/
int cl_getline(char** out_buf);
void cl_getline_set_file(FILE* input_fp);