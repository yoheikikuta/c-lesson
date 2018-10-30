#include "clesson.h"
#include "cl_util.h"
#include "parser.h"
#include "stack.h"
#include <assert.h>


void eval() {
    int ch = EOF;
    struct Token token = {
        UNKNOWN,
        {0}
    };

    do {
        ch = parse_one(ch, &token);
        if (token.ltype != UNKNOWN) {
            switch (token.ltype) {
                case NUMBER:
                    stack_push(&token);
                    break;
                case SPACE:
                    break;
                case EXECUTABLE_NAME:
                    if (streq(token.u.name, "add")) {
                        // Add operation: [{NUMBER,1}, {NUMBER,2}] -> [{NUMBER,3}]
                        struct Data dummy = {UNKNOWN, {0}};
                        stack_pop(&dummy);
                        int v1 = dummy.u.number;
                        stack_pop(&dummy);
                        int v2 = dummy.u.number;

                        struct Data result = {NUMBER, {v1+v2}};
                        stack_push(&result);
                    }
                    break;

                default:
                    printf("Unknown type %d\n", token.ltype);
                    break;
            }
        }
    } while (ch != EOF);
}

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    struct Data dummy = {UNKNOWN, {0}};
    stack_pop(&dummy);
    actual = dummy.u.number;

    assert(expect == actual);

}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    cl_getc_set_src(input);

    eval();

    int actual1 = 0;
    int actual2 = 0;
    struct Data dummy = {UNKNOWN, {0}};
    stack_pop(&dummy);
    actual1 = dummy.u.number;
    stack_pop(&dummy);
    actual2 = dummy.u.number;

    assert(expect1 == actual1);
    assert(expect2 == actual2);
}

static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    struct Data dummy = {UNKNOWN, {0}};
    stack_pop(&dummy);
    actual = dummy.u.number;

    assert(expect == actual);
}


int main() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();

    return 0;
}
