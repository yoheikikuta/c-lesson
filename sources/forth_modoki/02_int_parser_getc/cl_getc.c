#include "clesson.h"
#include <string.h>

//static const char* const input = "123 456";
static char* input = "123 456";
static int pos = 0;


int cl_getc() {
    if(strlen(input) == pos)
        return EOF;
    return input[pos++];
}

void cl_getc_set_src(char* str){
    input = str;
    // Set pos = 0 for multiple times inputs.
    pos = 0;
}
