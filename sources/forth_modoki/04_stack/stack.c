#include <stdio.h>

enum DataType {
    NUMBER;
    LITERAL_NAME;
};

struct Data {
    enum DataType dtype;
    union {
        int number;
        char *name;
    } u
};

int stack_push(struct Data *push_elem) {;}
int stack_pop(struct Data *pop_elem) {;}
