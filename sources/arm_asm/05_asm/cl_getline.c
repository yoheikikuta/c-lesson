#include "cl_getline.h"
#include "cl_util.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


// Get one line of the file pointed as fp:
//   "Text\n" -> return 4, out_buf = ['T','e','x','t','\0']
//   "  \n" -> return 2, out_buf = [' ',' ','\0']
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
    *out_buf = buf;
    
    return strlen(buf);
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
    FILE* fp = get_fp("/sources/arm_asm/05_asm/test_input/test.txt");
    cl_getline_set_file(fp);

	test_getline_first_line();
    test_getline_second_line();
    test_getline_last_line();

    printf("All unittests successfully passed.\n");
}


#if 0
int main(){
    unittests();
	return 0;
}
#endif