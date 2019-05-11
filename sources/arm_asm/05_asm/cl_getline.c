#include "cl_getline.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


// Get one line of the file pointed as fp:
//   "Text\n" -> return 4, out_buf = ['T','e','x','t','\0']
int cl_getline(char** out_buf) {
    if(fp != NULL){
        if(fgets(buf, BUF_SIZE, fp) == NULL) {
            return EOF;
        }
    }

    int len = strlen(buf);
    if (buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }

    int len_without_line_end = strlen(buf);
    *out_buf = buf;
    return len_without_line_end;
}

void cl_getline_set_file(FILE* input_fp){
    fp = input_fp;
}

// 
// TEST
// 

static void test_getline_first_line() {
    int expect = 11;

    char* str_line;
    int actual = cl_getline(&str_line);

    assert(expect == actual);
}

static void test_getline_second_line() {
    int expect = 12;

    char* str_line;
    int actual = cl_getline(&str_line);

    assert(expect == actual);
}

static void test_getline_last_line() {
    int expect = EOF;

    char* str_line;
    int actual = cl_getline(&str_line);

    assert(expect == actual);
}

// Read the input test text file and execute all unittests.
static void unittests() {
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    char* fname = strcat(cwd, "/sources/arm_asm/05_asm/test_input/test.txt");
    FILE* fp = NULL;
    if((fp = fopen(fname, "r")) == NULL) {
        printf("ERROR: cannot read the given file.\n");
        return EOF;
    }
    cl_getline_set_file(fp);

	test_getline_first_line();
    test_getline_second_line();
    test_getline_last_line();

    printf("All unittests successfully passed.\n");
}


int main(){
    unittests();
	return 0;
}