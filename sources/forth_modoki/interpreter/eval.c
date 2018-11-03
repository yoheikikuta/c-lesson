#include <assert.h>
#include "clesson.h"
#include "cl_util.h"
#include "dict.h"
#include "parser.h"
#include "stack.h"


int stack_pop_int(){
    // Return the value of popped NUMBER data from the stack.
    struct Data input = {UNKNOWN, {0}};
    stack_pop(&input);
    int num = input.u.number;

    return num;
}

char* stack_pop_str(){
    // Return the value of popped LITERAL_NAME data from the stack.
    struct Data input = {UNKNOWN, {0}};
    stack_pop(&input);
    char *str = input.u.name;

    return str;
}

void eval() {
    int ch = EOF;
    struct Token token = {
        UNKNOWN,
        {0}
    };

    do {
        ch = parse_one(ch, &token);
        switch (token.ltype) {
            case NUMBER:
                stack_push(&token);
                break;
            case SPACE:
                break;
            case EXECUTABLE_NAME:
                if (streq(token.u.name, "add")) {
                    // Add operation: [{NUMBER,1}, {NUMBER,2}] -> [{NUMBER,3}]
                    int v1 = stack_pop_int();
                    int v2 = stack_pop_int();

                    struct Data result = {NUMBER, {v1+v2}};
                    stack_push(&result);
                } else if (streq(token.u.name, "def")) {
                    // def operation: [{LITERAL_NAME,"abc"}, {NUMBER,123}] -> dict[{"abc", {NUMBER,123}}]
                    int val = stack_pop_int();
                    struct Data elem = {NUMBER, {val}};
                    char *literal_name = stack_pop_str();
                    dict_put(literal_name, &elem);
                } else if (dict_get(token.u.name, &token)) {
                    // use binded var name: dict[{"abc", {NUMBER,123}}] -> [{NUMBER, 123}]
                    stack_push(&token);
                }
                break;
            case LITERAL_NAME:
                stack_push(&token);
                break;
            case UNKNOWN:
                printf("Terminate eval on the way due to the UNKNOWN Type\n");
                break;

            default:
                printf("Unknown type %d\n", token.ltype);
                break;
        }
    } while (ch != EOF);
}

//
// TEST
//

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

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
    actual1 = stack_pop_int();
    actual2 = stack_pop_int();

    assert(expect1 == actual1);
    assert(expect2 == actual2);
}

static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_num_add_complicated() {
    char *input = "12 34 add 5 6 add add";
    int expect = 57;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_def_store() {
    char *input = "/abc 123 def";
    struct Data expect = {NUMBER, {123}};

    cl_getc_set_src(input);

    eval();

    struct Data actual = {UNKNOWN, {0}};
    dict_get("abc", &actual);

    assert_two_data_eq(&expect, &actual);
}

static void test_eval_def_add() {
    char *input = "/abc 123 def 321 abc add";
    int expect = 444;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_literal_name() {
    char *input = "/abc";
    char *expect = "abc";

    cl_getc_set_src(input);

    eval();

    char *actual;
    actual = stack_pop_str();

    assert(streq(expect, actual));
}

static void test_eval_unknown() {
    char *input = "123 ? 456";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}


int main() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();
    test_eval_num_add_complicated();
    test_eval_def_store();
    test_eval_def_add();
    test_eval_literal_name();
    test_eval_unknown();

    return 0;
}
