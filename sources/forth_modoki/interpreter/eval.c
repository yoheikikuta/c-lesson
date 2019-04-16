#include <assert.h>
#include "clesson.h"
#include "cl_util.h"
#include "dict.h"
#include "parser.h"
#include "stack.h"
#include "primitive.h"
#include "continuation.h"


void compile_exec_array(int ch, struct Token* token, struct Element* out_opelem) {
    // Compile the inside of an executable array
    // {1 2 {3 4 add} mul} ->
    //   out_opelem->u.exec_array->elements [(NUM,1), (NUM,2), out_opelem_nested, (NAME,mul)]
    //   out_opelem_nested->u.exec_array->elements [(NUM,3), (NUM,4), (NAME,add)]
    struct Element arr[MAX_NAME_OP_MUMBERS];
    struct Element arr_for_nested = {NO_ELEM_TYPE, {0}};
    struct Element compile_dict_elem = {NO_ELEM_TYPE, {0}};
    struct Emitter emitter;
    int elem_num = 0;

    do {
        ch = parse_one(ch, token);
        switch (token->ltype) {
            case NUMBER:
                arr[elem_num].etype = ELEMENT_NUMBER;
                arr[elem_num].u.number = token->u.number;
                elem_num++;
                break;
            case SPACE:
                break;
            case EXECUTABLE_NAME:
                if (compile_dict_get(token->u.name, &compile_dict_elem)) {
                    // Compile if, ifelse, while, repeat operations
                    emitter.pos = elem_num;
                    emitter.elems = arr;
                    compile_dict_elem.u.compile_func(&emitter);
                    elem_num = emitter.pos;
                } else {
                    arr[elem_num].etype = ELEMENT_EXECUTABLE_NAME;
                    arr[elem_num].u.name = token->u.name;
                    elem_num++;
                }
                break;
            case LITERAL_NAME:
                arr[elem_num].etype = ELEMENT_LITERAL_NAME;
                arr[elem_num].u.name = token->u.name;
                elem_num++;
                break;
            case OPEN_CURLY: {
                compile_exec_array(ch, token, &arr_for_nested);
                arr[elem_num].etype = arr_for_nested.etype;
                arr[elem_num].u.exec_array = arr_for_nested.u.exec_array;
                elem_num++;
                ch = parse_one(ch, token);
                break;
            }
             
            default:
                printf("Unknown type %d\n", token->ltype);
                break;
        }
    } while (ch != '}');

    struct ElementArray *elem_arr = (struct ElementArray*)malloc(sizeof(struct ElementArray)+sizeof(struct Element)*elem_num);
    elem_arr->len = elem_num;
    memcpy(elem_arr->elements, arr, sizeof(struct Element)*elem_num);

    out_opelem->etype = ELEMENT_EXECUTABLE_ARRAY;
    out_opelem->u.exec_array = elem_arr;
}

void eval_exec_array(struct ElementArray *opelems) {
    // Evaluate a compiled executable array (VM-like)
    // E.g., opelems->elements [1, 2, {add}, exec] -> stack [3]
    struct Element opelem = {NO_ELEM_TYPE, {0}};
    struct Element flg = {NO_ELEM_TYPE, {0}};
    struct Continuation cont = {0 ,{0}, 0};
    int i = 0;
    cont.ctype = CONT_CONTINUATION;
    cont.u.exec_array = opelems;
    cont.pc = 0;

    co_stack_push(&cont);

    while (co_stack_pop(&cont)) {
        // Discard all local variables up to reaching the next CONT_CONTINUATION
        if (cont.ctype == CONT_ELEMENT) {continue;};

        for (i = cont.pc; i < cont.u.exec_array->len; i++) {
            if (cont.u.exec_array->elements[i].etype == ELEMENT_EXEC_PRIMITIVE) {
                if (cont.u.exec_array->elements[i].u.number == OP_EXEC) {
                    // Primitive exec operation:
                    // ... {1 2 add} exec ... -> co_stack [(... {1 2 add} exec ...; pc=i+1), ({1 2 add}; pc=0)] 
                    cont.pc = ++i;
                    co_stack_push(&cont);
                    stack_pop(&opelem);
                    cont.pc = 0;
                    cont.u.exec_array = opelem.u.exec_array;
                    co_stack_push(&cont);
                    break;
                } else if (cont.u.exec_array->elements[i].u.number == OP_JMP) {
                    // Primitive jump operation:
                    // (positive case) 3 jmp 1 2 3 -> 3
                    // (negative case) 1 2 3 -4 jmp -> 1
                    stack_pop(&opelem);
                    if (opelem.u.number >= 0) {
                        i += opelem.u.number - 1;
                    } else {
                        i += opelem.u.number - 1;
                    }
                    continue;
                } else if (cont.u.exec_array->elements[i].u.number == OP_JMP_NOT_IF) {
                    // Primitive jump_not_if operation:
                    // (positive case) 0 3 jmp_not_if 1 2 3 -> 3
                    // (negative case) 1 2 3 0 -5 jmp -> 1
                    stack_pop(&opelem);
                    stack_pop(&flg);
                    if (!flg.u.number) {
                        if (opelem.u.number >= 0) {
                            i += opelem.u.number - 1;
                        } else {
                            i += opelem.u.number - 1;
                        }
                    }
                    continue;
                } else if (cont.u.exec_array->elements[i].u.number == OP_STORE) {
                    // Primitive store operation:
                    // 1 store -> co_stack [1]
                    stack_pop(&opelem);
                    struct Continuation local_var;
                    local_var.ctype = CONT_ELEMENT;
                    local_var.u.elem = opelem;
                    co_stack_push(&local_var);
                    continue;
                } else if (cont.u.exec_array->elements[i].u.number == OP_LOAD) {
                    // Primitive load operation:
                    // co_stack [0, 1, 2, 3], 1 load -> stack [2]
                    stack_pop(&opelem);
                    struct Element load_var;
                    co_load_variable(opelem.u.number, &load_var);
                    stack_push(&load_var);
                    continue;
                } else if (cont.u.exec_array->elements[i].u.number == OP_LPOP) {
                    // Primitive lpop operation:
                    // co_stack [0, 1] -> co_stack [0]
                    struct Continuation local_var;
                    co_stack_pop(&local_var);
                    continue;
                }
            } else if (cont.u.exec_array->elements[i].etype == ELEMENT_EXECUTABLE_NAME) {
                dict_get(cont.u.exec_array->elements[i].u.name, &opelem);

                if (opelem.etype == ELEMENT_EXECUTABLE_ARRAY) {
                    // Nested ELEMENT_EXECUTABLE_ARRAYs:
                    // ... {1 {3 4} 2} ... -> co_stack [(... {1 {3 4} 2} ...; pc=i+1), ({3 4}, pc=0)] 
                    cont.pc = ++i;
                    co_stack_push(&cont);
                    cont.pc = 0;
                    cont.u.exec_array = opelem.u.exec_array;
                    co_stack_push(&cont);
                    break;
                } else if (opelem.etype == ELEMENT_C_FUNC) {
                    // add, sub, ...
                    opelem.u.cfunc();
                } else {
                    // use binded var name: dict[{"abc", 123}] -> stack [123]
                    stack_push(&opelem);
                }
            } else {
                switch (cont.u.exec_array->elements[i].etype) {
                    case ELEMENT_NUMBER:
                        stack_push(&cont.u.exec_array->elements[i]);
                        break;
                    case ELEMENT_LITERAL_NAME:
                        stack_push(&cont.u.exec_array->elements[i]);
                        break;
                    case ELEMENT_EXECUTABLE_ARRAY:
                        stack_push(&cont.u.exec_array->elements[i]);
                        break;
                    default:
                        printf("Unknown type %d\n", cont.u.exec_array->elements[i].etype);
                        break;
                }
            }
        }
    }
}

void eval() {
    // Evaluate an input string (Interpreter-like)
    // "1 2 add" -> stack [3]
    int ch = EOF;
    struct Token token = {UNKNOWN, {0}};
    struct Element opelem = {NO_ELEM_TYPE, {0}};
    struct Element opelem_if_true = {NO_ELEM_TYPE, {0}};
    struct Element opelem_if_false = {NO_ELEM_TYPE, {0}};
    struct Element flg = {NO_ELEM_TYPE, {0}};

    do {
        ch = parse_one(ch, &token);
        switch (token.ltype) {
            case NUMBER:
                opelem.etype = ELEMENT_NUMBER;
                opelem.u.number = token.u.number;
                stack_push(&opelem);
                break;
            case SPACE:
                break;
            case EXECUTABLE_NAME:
                dict_get(token.u.name, &opelem);
                
                if (streq(token.u.name, "exec")) {
                    // Exec operation: {op1} exec -> evaluate {op1}
                    stack_pop(&opelem);
                    eval_exec_array(opelem.u.exec_array);
                } else if (streq(token.u.name, "if")) {
                    // If operation: flg {op1} -> {op1} if flg is 1, do nothing if 0
                    stack_pop(&opelem_if_true);
                    stack_pop(&flg);
                    if (flg.u.number == 1) {
                        eval_exec_array(opelem_if_true.u.exec_array);
                    }
                } else if (streq(token.u.name, "ifelse")) {
                    // Ifelse operation: flg {op1} {op2} -> {op1} if flg is 1, {op2} if 0
                    stack_pop(&opelem_if_false);
                    stack_pop(&opelem_if_true);
                    stack_pop(&flg);
                    if (flg.u.number == 1) {
                        eval_exec_array(opelem_if_true.u.exec_array);
                    } else if (flg.u.number == 0) {
                        eval_exec_array(opelem_if_false.u.exec_array);
                    }
                } else if (streq(token.u.name, "while")) {
                    // While operation: { exec_array1 } { exec_array2 } while
                    // -> execute exec_array1, then repeat exec_array1 until the stack top is true (==1), then execute exec_array2
                    struct Element opelem_cond = {NO_ELEM_TYPE, {0}};
                    struct Element opelem_body = {NO_ELEM_TYPE, {0}};
                    struct Element boolean_flg = {NO_ELEM_TYPE, {0}};

                    stack_pop(&opelem_body);
                    stack_pop(&opelem_cond);
                    do {
                        eval_exec_array(opelem_cond.u.exec_array);
                        stack_pop(&boolean_flg);
                        if (boolean_flg.u.number) {
                            eval_exec_array(opelem_body.u.exec_array);
                        }
                    } while (boolean_flg.u.number);
                } else if (streq(token.u.name, "repeat")) {
                    // Repeat operation: 3 { exec_array } repeat -> execute exec_array 3 times
                    struct Element opelem_num = {NO_ELEM_TYPE, {0}};
                    struct Element opelem_body = {NO_ELEM_TYPE, {0}};

                    stack_pop(&opelem_body);
                    stack_pop(&opelem_num);
                    int i = opelem_num.u.number;
                    for (i; i > 0; i--) {
                        eval_exec_array(opelem_body.u.exec_array);
                    }
                } else if (opelem.etype == ELEMENT_C_FUNC) {
                    // add, sub, ...
                    opelem.u.cfunc();
                } else if (opelem.etype == ELEMENT_EXECUTABLE_ARRAY) {
                    // Evaluate a compiled executable array
                    eval_exec_array(opelem.u.exec_array);
                } else {
                    // Use binded var name: dict[{"abc", {ELEMENT_NUMBER,123}}] -> [{ELEMENT_NUMBER, 123}]
                    stack_push(&opelem);
                }
                break;            
            case LITERAL_NAME:
                opelem.etype = ELEMENT_LITERAL_NAME;
                opelem.u.name = token.u.name;
                stack_push(&opelem);
                break;
            case OPEN_CURLY:
                compile_exec_array(ch, &token, &opelem);
                stack_push(&opelem);
                // E.g., {1 2}3 case ch = 1 token = 2
                // Call one more parse_one, then get ch = 3 token = 1
                ch = parse_one(ch, &token);
                break;
            case UNKNOWN:
                printf("Terminate eval on the way due to the UNKNOWN Type\n");
                break;
            case END_OF_FILE:
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

    reset_stack();
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

    reset_stack();
}

static void test_eval_executable_array_num_one() {
    char* input = "{ 12 }";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem = {ELEMENT_NUMBER, {12}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert(expect_exec.etype == actual.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem, &actual.u.exec_array->elements[0]);

    reset_stack();
}

static void test_eval_executable_array_num_two() {
    char* input = "{ 1 2 }";
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

    reset_stack();
}

static void test_eval_executable_array_num_two_sep() {
    char* input = "{ 1 } { 2 }";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem1 = {ELEMENT_NUMBER, {1}};
    struct Element expect_exec_opelem2 = {ELEMENT_NUMBER, {2}};

    cl_getc_set_src(input);

    eval();

    struct Element actual1 = {NO_ELEM_TYPE, {0}};
    struct Element actual2 = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual2);
    stack_pop(&actual1);

    assert(expect_exec.etype == actual1.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem1, &actual1.u.exec_array->elements[0]);
    assert(expect_exec.etype == actual2.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem2, &actual2.u.exec_array->elements[0]);

    reset_stack();
}

static void test_eval_executable_array_num_three_nest() {
    char* input = "{ 1 { 2 } 3 }";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem1 = {ELEMENT_NUMBER, {1}};
    struct Element expect_exec_opelem2 = {ELEMENT_NUMBER, {2}};
    struct Element expect_exec_opelem3 = {ELEMENT_NUMBER, {3}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert(expect_exec.etype == actual.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem1, &actual.u.exec_array->elements[0]);
    assert_two_exec_opelem_eq(&expect_exec_opelem2, &actual.u.exec_array->elements[1].u.exec_array->elements[0]);
    assert_two_exec_opelem_eq(&expect_exec_opelem3, &actual.u.exec_array->elements[2]);

    reset_stack();
}

static void test_eval_executable_array_literal_name() {
    char* input = "{ /abc }";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem = {ELEMENT_LITERAL_NAME, {.name="abc"}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert(expect_exec.etype == actual.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem, &actual.u.exec_array->elements[0]);

    reset_stack();
}

static void test_eval_executable_array_executable_name() {
    char* input = "{ abc }";
    struct Element expect_exec = {ELEMENT_EXECUTABLE_ARRAY, {0}};
    struct Element expect_exec_opelem = {ELEMENT_EXECUTABLE_NAME, {.name="abc"}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert(expect_exec.etype == actual.etype);
    assert_two_exec_opelem_eq(&expect_exec_opelem, &actual.u.exec_array->elements[0]);

    reset_stack();
}

static void test_eval_num_add() {
    char* input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_sub() {
    char* input = "5 3 sub";
    int expect = 2;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_mul() {
    char* input = "2 4 mul";
    int expect = 8;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_div() {
    char* input = "5 2 div";
    int expect = 2;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_mod() {
    char* input = "5 2 mod";
    int expect = 1;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_eq() {
    char* input = "2 2 eq";
    int expect = 1;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_neq() {
    char* input = "3 2 neq";
    int expect = 1;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_gt() {
    char* input = "3 2 gt";
    int expect = 1;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_ge() {
    char* input = "3 3 ge";
    int expect = 1;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}
static void test_eval_num_lt() {
    char* input = "2 3 lt";
    int expect = 1;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_le() {
    char* input = "3 3 le";
    int expect = 1;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_pop() {
    char* input = "1 2 pop";
    int expect = 1;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_exch() {
    char* input = "1 2 3 exch";
    int expect1 = 2;
    int expect2 = 3;

    cl_getc_set_src(input);

    eval();

    int actual1 = 0;
    int actual2 = 0;
    actual1 = stack_pop_int();
    actual2 = stack_pop_int();

    assert(expect1 == actual1);
    assert(expect2 == actual2);

    reset_stack();
}

static void test_eval_num_dup() {
    char* input = "1 2 dup";
    int expect1 = 2;
    int expect2 = 2;

    cl_getc_set_src(input);

    eval();

    int actual1 = 0;
    int actual2 = 0;
    actual1 = stack_pop_int();
    actual2 = stack_pop_int();

    assert(expect1 == actual1);
    assert(expect2 == actual2);

    reset_stack();
}

static void test_eval_num_index() {
    char* input = "10 20 30 40 1 index";
    // Expected output: [10,20,30,40,30]
    int expect = 30;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_roll() {
    char* input = "10 20 30 40 3 2 roll";
    // Expected output: [10,30,40,20]
    int expect1 = 20;
    int expect2 = 40;
    int expect3 = 30;

    cl_getc_set_src(input);

    eval();

    int actual1 = 0;
    int actual2 = 0;
    int actual3 = 0;
    actual1 = stack_pop_int();
    actual2 = stack_pop_int();
    actual3 = stack_pop_int();

    assert(expect1 == actual1);
    assert(expect2 == actual2);
    assert(expect3 == actual3);

    reset_stack();
}

static void test_eval_exec() {
    char* input = "{ 2 3 add } exec";
    int expect = 5;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_exec_continue() {
    char* input = "{ 2 3 add } exec 5 add";
    int expect = 10;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_exec_nested() {
    char* input = "{ 3 { 123 } repeat } exec";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual1 = 0;
    int actual2 = 0;
    int actual3 = 0;
    actual1 = stack_pop_int();
    actual2 = stack_pop_int();
    actual3 = stack_pop_int();

    assert(expect == actual1);
    assert(expect == actual2);
    assert(expect == actual3);

    reset_stack();
}

static void test_eval_if() {
    char* input = "1 { 2 3 add } if";
    int expect = 5;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_if_nested() {
    char* input = "1 { 2 3 add 0 { 1 add} if } if";
    int expect = 5;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_ifelse() {
    char* input = "0 { 1 } { 2 3 add } ifelse";
    int expect = 5;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_ifelse_continue() {
    char* input = "0 { 1 } { 2 3 add } ifelse 5 add";
    int expect = 10;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_ifelse_nested() {
    char* input = "0 { 1 } { 1 { 2 3 add } { } ifelse } ifelse";
    int expect = 5;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_while() {
    char* input = "1 { 2 add dup 4 lt } { 10 add } while";
    // Expected output: [1] -> [3 3] -> (3 < 4) -> [5 5] -> (5 not less than 4) -> [15]
    int expect = 15;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_while_continue() {
    char* input = "1 { 2 add dup 4 lt } { 10 add } while 5 add";
    int expect = 20;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_while_nested() {
    char* input = "1 { dup 5 lt } { { dup 6 lt } { 2 add } while } while";
    // Expected: 1 > 3 > 5 > 7
    int expect = 7;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_repeat() {
    char* input = "1 3 { 2 add } repeat";
    int expect = 7;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_repeat_continue() {
    char* input = "1 3 { 2 add } repeat 3 add";
    int expect = 10;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_repeat_nested() {
    char* input = "1 3 { 2 { 2 add } repeat } repeat";
    int expect = 13;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_num_add_complicated() {
    char* input = "12 34 add 5 6 add add";
    int expect = 57;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_def_store() {
    char* input = "/abc 123 def";
    struct Element expect = {ELEMENT_NUMBER, {123}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    dict_get("abc", &actual);

    assert_two_data_eq(&expect, &actual);

    reset_stack();
}

static void test_eval_def_pop() {
    char* input = "/abc 123 def abc";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_def_add() {
    char* input = "/abc 123 def 321 abc add";
    int expect = 444;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_literal_name() {
    char* input = "/abc";
    char* expect = "abc";

    cl_getc_set_src(input);

    eval();

    char* actual;
    actual = stack_pop_str();

    assert(streq(expect, actual));

    reset_stack();
}

static void test_eval_unknown() {
    char* input = "123 ? 456";
    int expect = 123;

    cl_getc_set_src(input);

    eval();

    int actual = 0;
    actual = stack_pop_int();

    assert(expect == actual);

    reset_stack();
}

static void test_eval_executable_array_literal_name_bind() {
    char* input = "/abc { 1 2 add } def abc";
    struct Element expect = {ELEMENT_NUMBER, {3}};

    cl_getc_set_src(input);

    eval(); 

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert_two_exec_opelem_eq(&expect, &actual);

    reset_stack();
}

static void test_eval_executable_array_literal_name_bind_nest() {
    char* input = "/ZZ { 6 } def /YY { 4 ZZ 5 } def /XX { 1 2 YY 3 } def XX";
    // Expected output: [1 2 4 6 5 3]
    struct Element expect1 = {ELEMENT_NUMBER, {1}};
    struct Element expect2 = {ELEMENT_NUMBER, {2}};
    struct Element expect3 = {ELEMENT_NUMBER, {4}};
    struct Element expect4 = {ELEMENT_NUMBER, {6}};
    struct Element expect5 = {ELEMENT_NUMBER, {5}};
    struct Element expect6 = {ELEMENT_NUMBER, {3}};

    cl_getc_set_src(input);

    eval(); 

    struct Element actual1 = {NO_ELEM_TYPE, {0}};
    struct Element actual2 = {NO_ELEM_TYPE, {0}};
    struct Element actual3 = {NO_ELEM_TYPE, {0}};
    struct Element actual4 = {NO_ELEM_TYPE, {0}};
    struct Element actual5 = {NO_ELEM_TYPE, {0}};
    struct Element actual6 = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual6);
    stack_pop(&actual5);
    stack_pop(&actual4);
    stack_pop(&actual3);
    stack_pop(&actual2);
    stack_pop(&actual1);

    assert_two_exec_opelem_eq(&expect1, &actual1);
    assert_two_exec_opelem_eq(&expect2, &actual2);
    assert_two_exec_opelem_eq(&expect3, &actual3);
    assert_two_exec_opelem_eq(&expect4, &actual4);
    assert_two_exec_opelem_eq(&expect5, &actual5);
    assert_two_exec_opelem_eq(&expect6, &actual6);

    reset_stack();
}

static void test_eval_executable_array_literal_name_bind_nest_inner() {
    char* input = "/abc { 1 2 add 1 { 1 2 add } if add } def abc";
    struct Element expect = {ELEMENT_NUMBER, {6}};

    cl_getc_set_src(input);

    eval();

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert_two_exec_opelem_eq(&expect, &actual);

    reset_stack();
}


static void test_eval_line_break() {
    char* input = "1\n2 \nadd\n";
    struct Element expect = {ELEMENT_NUMBER, {3}};

    cl_getc_set_src(input);

    eval(); 

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert_two_exec_opelem_eq(&expect, &actual);

    reset_stack();
}

static void test_eval_comment() {
    char* input = "1 2 %comment\nadd %also comment";
    struct Element expect = {ELEMENT_NUMBER, {3}};

    cl_getc_set_src(input);

    eval(); 

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert_two_exec_opelem_eq(&expect, &actual);

    reset_stack();
}

static void test_eval_exec_array_lazy_eval() {
    char* input = "/a { { 345 } ifelse } def 1 { 123 } a";
    struct Element expect = {ELEMENT_NUMBER, {123}};

    cl_getc_set_src(input);

    eval(); 

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert_two_exec_opelem_eq(&expect, &actual);

    reset_stack();
}

static void test_eval_exec_array_nested_exec() {
    char* input = "/f { { 1 3 add } exec 3 } def f";
    struct Element expect1 = {ELEMENT_NUMBER, {3}};
    struct Element expect2 = {ELEMENT_NUMBER, {4}};

    cl_getc_set_src(input);

    eval(); 

    struct Element actual1 = {NO_ELEM_TYPE, {0}};
    struct Element actual2 = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual1);
    stack_pop(&actual2);

    assert_two_exec_opelem_eq(&expect1, &actual1);

    reset_stack();
}

static void test_eval_factorial() {
    char* input = "/factorial { dup { dup 1 gt } { 1 sub exch 1 index mul exch } while pop } def 10 factorial";
    struct Element expect = {ELEMENT_NUMBER, {3628800}};

    cl_getc_set_src(input);

    eval(); 

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert_two_exec_opelem_eq(&expect, &actual);

    reset_stack();
}

static void test_eval_factorial_without_space_in_curly_brace() {
    char* input = "/factorial {dup {dup 1 gt} {1 sub exch 1 index mul exch} while pop} def 10 factorial";
    struct Element expect = {ELEMENT_NUMBER, {3628800}};

    cl_getc_set_src(input);

    eval(); 

    struct Element actual = {NO_ELEM_TYPE, {0}};
    stack_pop(&actual);

    assert_two_exec_opelem_eq(&expect, &actual);

    reset_stack();
}

static void unit_tests() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_executable_array_num_one();
    test_eval_executable_array_num_two();
    test_eval_executable_array_num_two_sep();
    test_eval_executable_array_num_three_nest();
    test_eval_executable_array_literal_name();
    test_eval_executable_array_executable_name();
    test_eval_num_add();
    test_eval_num_sub();
    test_eval_num_mul();
    test_eval_num_div();
    test_eval_num_mod();
    test_eval_num_eq();
    test_eval_num_neq();
    test_eval_num_gt();
    test_eval_num_ge();
    test_eval_num_lt();
    test_eval_num_le();
    test_eval_num_pop();
    test_eval_num_exch();
    test_eval_exec_continue();
    test_eval_exec_nested();
    test_eval_num_dup();
    test_eval_num_index();
    test_eval_num_roll();
    test_eval_exec();
    test_eval_if();
    test_eval_if_nested();
    test_eval_ifelse();
    test_eval_ifelse_continue();
    test_eval_ifelse_nested();
    test_eval_while();
    test_eval_while_continue();
    test_eval_while_nested();
    test_eval_repeat();
    test_eval_repeat_continue();
    test_eval_repeat_nested();
    test_eval_num_add_complicated();
    test_eval_def_store();
    test_eval_def_pop();
    test_eval_def_add();
    test_eval_literal_name();
    test_eval_unknown();
    test_eval_executable_array_literal_name_bind();
    test_eval_executable_array_literal_name_bind_nest();
    test_eval_executable_array_literal_name_bind_nest_inner();
    test_eval_line_break();
    test_eval_comment();
    test_eval_exec_array_lazy_eval();
    test_eval_exec_array_nested_exec();
    test_eval_factorial();
    test_eval_factorial_without_space_in_curly_brace();

    printf("All unittests successfully passed.\n");
}


int main(int argc, char* argv[]) {
    register_all_primitive();
    if (argc > 1) {
        FILE *fp = NULL;
        char *fname = argv[1];
        if((fp = fopen(fname, "r")) == NULL) {
			printf("ERROR: cannot read the given file.\n");
		}
        cl_getc_set_file(fp);
        eval();
        fclose(fp);
        stack_print_all();
    } else {
        unit_tests();
    }
    return 0;
}
