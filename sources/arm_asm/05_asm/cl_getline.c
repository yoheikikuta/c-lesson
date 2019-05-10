#include <stdio.h>
#include <string.h>
#include <assert.h>

#define BUF_SIZE 256
static char buf[BUF_SIZE];
static FILE* fp = NULL;


// Get one line of the file pointed as fp:
//   -> return its length, and pack the string into out_buf
int cl_getline(char **out_buf) {
    if(fp != NULL){
        if(fgets(buf, BUF_SIZE, fp) == NULL) {
            return EOF;
        }
    }
    int len = strlen(buf);
    *out_buf = buf;
    return len;
}

void cl_getline_set_file(FILE* input_fp){
    fp = input_fp;
}

// 
// TEST
// 

static void test_getline_first_line() {
    int expect = 12;

    char* str_line;
    int actual = cl_getline(&str_line);

    assert(expect == actual);
}

static void test_getline_second_line() {
    int expect = 13;

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

int main(){
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    char *fname = strcat(cwd, "/sources/arm_asm/05_asm/test_input/test.txt");
    FILE *fp = NULL;
    if((fp = fopen(fname, "r")) == NULL) {
        printf("ERROR: cannot read the given file.\n");
    }
    cl_getline_set_file(fp);

	test_getline_first_line();
    test_getline_second_line();
    test_getline_last_line();
	return 0;
}