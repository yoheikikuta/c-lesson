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
    char* str = input.u.name;

    return str;
}

void def_op() {
    // def operation: [{LITERAL_NAME,"abc"}, {NUMBER,123}] -> dict[{"abc", {NUMBER,123}}]
    int val = stack_pop_int();
    struct Data elem = {NUMBER, {val}};
    char* literal_name = stack_pop_str();

    dict_put(literal_name, &elem);
}

void add_op() {
    // Add operation: [{NUMBER,1}, {NUMBER,2}] -> [{NUMBER,3}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Data result = {NUMBER, {v1+v2}};
    stack_push(&result);
}

void sub_op() {
    // Sub operation: [{NUMBER,5}, {NUMBER,2}] -> [{NUMBER,3}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Data result = {NUMBER, {v2-v1}};
    stack_push(&result);
}

void mul_op() {
    // Mul operation: [{NUMBER,2}, {NUMBER,4}] -> [{NUMBER,8}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Data result = {NUMBER, {v1*v2}};
    stack_push(&result);
}

void div_op() {
    // Div operation: [{NUMBER,5}, {NUMBER,2}] -> [{NUMBER,2}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Data result = {NUMBER, {v2/v1}};
    stack_push(&result);
}

void register_one_primitive(char* op_name, void (*cfunc)(void)) {
    struct Element opelem = {ELEMENT_C_FUNC, {.cfunc = cfunc}};
    dict_put(op_name, &opelem);
}

void register_all_primitive() {
    register_one_primitive("def", def_op);
    register_one_primitive("add", add_op);
    register_one_primitive("sub", sub_op);
    register_one_primitive("mul", mul_op);
    register_one_primitive("div", div_op);
}

void compile_exec_array(int ch, struct Token* token, struct Element* out_opelem) {
    struct Element arr[MAX_NAME_OP_MUMBERS];

    do {
        ch = parse_one(ch, token);
        switch (token->ltype) {
            case NUMBER:
                arr[0].etype = ELEMENT_NUMBER;
                arr[0].u.exec_array = token->u.number;
                break;
              
            default:
                printf("Unknown type %d\n", token->ltype);
                break;
        }
    } while (ch != '}');

    struct ElementArray *elem_arr = (struct ElementArray*)malloc(sizeof(struct ElementArray)+sizeof(struct Element));
    elem_arr->len = 1;
    memcpy(elem_arr->elements, arr, sizeof(struct Element));

    out_opelem->etype = ELEMENT_EXECUTABLE_ARRAY;
    out_opelem->u.exec_array = elem_arr;
}



void eval() {
    int ch = EOF;
    struct Token token = {
        UNKNOWN,
        {0}
    };
    struct Element opelem = {
        ELEMENT_C_FUNC,
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
                dict_get(token.u.name, &opelem);

                if (opelem.etype == ELEMENT_C_FUNC) {
                    opelem.u.cfunc();
                } else if (dict_get(token.u.name, &token)) {
                    // use binded var name: dict[{"abc", {NUMBER,123}}] -> [{NUMBER, 123}]
                    stack_push(&token);
                }
                break;
            case LITERAL_NAME:
                stack_push(&token);
                break;
            case OPEN_CURLY:
                compile_exec_array(ch, &token, &opelem);
                stack_push(&opelem);
                // Need one more parse_one to move ch ahead
                ch = parse_one(ch, &token);
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
    char* input = "123";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_num_two() {
    char* input = "123 456";
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

static void test_eval_executable_array_num_one() {
    char* input = "{12}";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem = {ELEMENT_NUMBER, {12}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert(expect_exec.etype == actual.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem, &actual.u.exec_array->elements[0]);
}

static void test_eval_executable_array_num_two() {
    char* input = "{1 2}";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem1 = {ELEMENT_NUMBER, {1}};
    struct Element expect_exec_opelem2 = {ELEMENT_NUMBER, {2}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert(expect_exec.etype == actual.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem1, &actual.u.exec_array->elements[0]);
    assert_two_exec_opelem_eq(&expect_exec_opelem2, &actual.u.exec_array->elements[1]);
}

static void test_eval_executable_array_num_two_sep() {
    char* input = "{1} {2}";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem1 = {ELEMENT_NUMBER, {1}};
    struct Element expect_exec_opelem2 = {ELEMENT_NUMBER, {2}};

    cl_getc_set_src(input);

    eval();

    struct Element actual1 = {NO_ELEM_TYPE, {0}};
    struct Element actual2 = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual1);
    stack_pop(&actual2);

    assert(expect_exec.etype == actual1.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem1, &actual1.u.exec_array->elements[0]);
    assert(expect_exec.etype == actual2.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem2, &actual2.u.exec_array->elements[0]);
}

static void test_eval_executable_array_num_three_nest() {
    char* input = "{1 {2} 3}";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem1 = {ELEMENT_NUMBER, {1}};
    struct Element expect_exec_opelem2 = {ELEMENT_NUMBER, {2}};
    struct Element expect_exec_opelem3 = {ELEMENT_NUMBER, {3}};

    cl_getc_set_src(input);

    eval();

    struct Element actual1 = {NO_ELEM_TYPE, {0}};
    struct Element actual2 = {NO_ELEM_TYPE, {0}};
    struct Element actual3 = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual1);
    stack_pop(&actual2);
    stack_pop(&actual3);

    assert(expect_exec.etype == actual1.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem1, &actual1.u.exec_array->elements[0]);
    assert(expect_exec.etype == actual2.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem2, &actual2.u.exec_array->elements[1].u.exec_array->elements[0]);
    assert(expect_exec.etype == actual3.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem3, &actual3.u.exec_array->elements[2]);
}

static void test_eval_executable_array_literal_name() {
    char* input = "{/abc}";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem = {ELEMENT_LITERAL_NAME, {.name="abc"}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert(expect_exec.etype == actual.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem, &actual.u.exec_array->elements[0]);
}

static void test_eval_executable_array_executable_name() {
    char* input = "{abc}";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem = {ELEMENT_EXECUTABLE_NAME, {.name="abc"}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert(expect_exec.etype == actual.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem, &actual.u.exec_array->elements[0]);
}

static void test_eval_num_add() {
    char* input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_num_sub() {
    char* input = "5 3 sub";
    int expect = 2;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_num_mul() {
    char* input = "2 4 mul";
    int expect = 8;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_num_div() {
    char* input = "5 2 div";
    int expect = 2;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_num_add_complicated() {
    char* input = "12 34 add 5 6 add add";
    int expect = 57;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_def_store() {
    char* input = "/abc 123 def";
    struct Data expect = {NUMBER, {123}};

    cl_getc_set_src(input);

    eval();

    struct Data actual = {UNKNOWN, {0}};
    dict_get("abc", &actual);

    assert_two_data_eq(&expect, &actual);
}

static void test_eval_def_pop() {
    char* input = "/abc 123 def abc";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_def_add() {
    char* input = "/abc 123 def 321 abc add";
    int expect = 444;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}

static void test_eval_literal_name() {
    char* input = "/abc";
    char* expect = "abc";

    cl_getc_set_src(input);

    eval();

    char* actual;
    actual = stack_pop_str();

    assert(streq(expect, actual));
}

static void test_eval_unknown() {
    char* input = "123 ? 456";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);
}


int main() {
    register_all_primitive();

    test_eval_num_one();
    test_eval_num_two();
    test_eval_executable_array_num_one();
    // test_eval_executable_array_num_two();
    // test_eval_executable_array_num_two_sep();
    // test_eval_executable_array_num_three_nest();
    // test_eval_executable_array_literal_name();
    // test_eval_executable_array_executable_name();
    test_eval_num_add();
    test_eval_num_sub();
    test_eval_num_mul();
    test_eval_num_div();
    test_eval_num_add_complicated();
    test_eval_def_store();
    test_eval_def_pop();
    test_eval_def_add();
    test_eval_literal_name();
    test_eval_unknown();

    return 0;
}
