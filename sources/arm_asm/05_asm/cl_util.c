#include "cl_util.h"
#include "cl_getline.h"
#include <stdio.h>
#include <assert.h>

void assert_two_num_eq(int num1, int num2) {
    return assert(num1 == num2);
}

void assert_two_str_eq(char* str1, char* str2) {
    return assert(strcmp(str1, str2) == 0);
}

void assert_str_substr_eq(char* str, struct Substring* substr) {
    char extracted_substr[BUF_SIZE] = {'\0'};
    strncpy(&extracted_substr, substr->str, substr->len);

    return assert(strcmp(str, extracted_substr) == 0);
}

int two_word_eq(struct Word* w1, struct Word* w2) {
    if (w1->wtype == w2->wtype) {
        if (w1->wtype == WORD_NUMBER) {
            return w1->u.number == w2->u.number;
        } else {
            return strcmp(w1->u.str, w2->u.str) == 0;
        }
    } else {
        return 0;
    }
}

int two_keyvalue_eq(struct KeyValue* kv1, struct KeyValue* kv2) {
    // Return 1 if two KeyValues are identical.
    if (kv1->key == kv2->key) {
        return two_word_eq(&kv1->value, &kv2->value);
    } else {
        return 0;
    }
}

void assert_two_word_eq(struct Word* w1, struct Word* w2) {
    assert(two_word_eq(w1, w2));
}

void assert_two_keyvalue_eq(struct KeyValue* kv1, struct KeyValue* kv2) {
    assert(two_keyvalue_eq(kv1, kv2));
}

// Return the file pointer of a given relative file path
// from the top directory of the repository.
FILE* get_fp(char* rel_path, enum FileOpenMode mode) {
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    char* fname = strcat(cwd, rel_path);
    FILE* fp = NULL;
    if (mode == FREAD) {
        if((fp = fopen(fname, "r")) == NULL) {
            printf("ERROR: cannot read the given file.\n");
            return EOF;
        }
    } else if (mode == FREADBIN) {
        if((fp = fopen(fname, "rb")) == NULL) {
            printf("ERROR: cannot read the given file.\n");
            return EOF;
        }
    } else if (mode == FWRITE) {
        fp = fopen(fname, "wb+");
    }
    return fp;
}