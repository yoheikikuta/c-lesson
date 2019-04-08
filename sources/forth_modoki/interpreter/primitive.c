#include <stdio.h>
#include "dict.h"
#include "primitive.h"


void def_op() {
    // def operation: [{ELEMENT_LITERAL_NAME,"abc"}, {ELEMENT_NUMBER,123}] -> dict[{"abc", {ELEMENT_NUMBER,123}}]
    struct Element elem = {NO_ELEM_TYPE,{0}};
	struct Element elem_val = {NO_ELEM_TYPE,{0}};
	char* literal_name;
	stack_pop(&elem);
	elem_val = elem;
	stack_pop(&elem);
	literal_name = elem.u.name;

    dict_put(literal_name, &elem_val);
}

void add_op() {
    // Add operation: [{ELEMENT_NUMBER,1}, {ELEMENT_NUMBER,2}] -> [{ELEMENT_NUMBER,3}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v1+v2}};
    stack_push(&result);
}

void sub_op() {
    // Sub operation: [{ELEMENT_NUMBER,5}, {ELEMENT_NUMBER,2}] -> [{ELEMENT_NUMBER,3}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2-v1}};
    stack_push(&result);
}

void mul_op() {
    // Mul operation: [{ELEMENT_NUMBER,2}, {ELEMENT_NUMBER,4}] -> [{ELEMENT_NUMBER,8}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v1*v2}};
    stack_push(&result);
}

void div_op() {
    // Div operation: [{ELEMENT_NUMBER,5}, {ELEMENT_NUMBER,2}] -> [{ELEMENT_NUMBER,2}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2/v1}};
    stack_push(&result);
}

void mod_op() {
    // Mod operation: [{ELEMENT_NUMBER,5}, {ELEMENT_NUMBER,2}] -> [{ELEMENT_NUMBER,1}]
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2%v1}};
    stack_push(&result);
}

void eq_op() {
    // Equal operation: [{ELEMENT_NUMBER,3}, {ELEMENT_NUMBER,3}] -> [{ELEMENT_NUMBER,1}], 0 if not equal
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2==v1}};
    stack_push(&result);
}

void neq_op() {
    // Not equal operation: [{ELEMENT_NUMBER,3}, {ELEMENT_NUMBER,2}] -> [{ELEMENT_NUMBER,1}], 0 if equal
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2!=v1}};
    stack_push(&result);
}

void gt_op() {
    // Greater than operation: [{ELEMENT_NUMBER,3}, {ELEMENT_NUMBER,2}] -> [{ELEMENT_NUMBER,1}], 0 if not
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2>v1}};
    stack_push(&result);
}

void ge_op() {
    // Greater or euall operation: [{ELEMENT_NUMBER,3}, {ELEMENT_NUMBER,3}] -> [{ELEMENT_NUMBER,1}], 0 if not
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2>=v1}};
    stack_push(&result);
}

void lt_op() {
    // Lower than operation: [{ELEMENT_NUMBER,2}, {ELEMENT_NUMBER,3}] -> [{ELEMENT_NUMBER,1}], 0 if not
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2<v1}};
    stack_push(&result);
}

void le_op() {
    // Lower or equal operation: [{ELEMENT_NUMBER,3}, {ELEMENT_NUMBER,3}] -> [{ELEMENT_NUMBER,1}], 0 if not
    int v1 = stack_pop_int();
    int v2 = stack_pop_int();

    struct Element result = {ELEMENT_NUMBER, {v2<=v1}};
    stack_push(&result);
}

void pop_op() {
    // Pop operation: [{TYPE1,v1}, {TYPE2,v2}] -> [{TYPE1,v1}]
    struct Element to_be_discarded = {NO_ELEM_TYPE, {0}};
    stack_pop(&to_be_discarded);
}

void exch_op() {
    // Exchenge operation: [{TYPE1,v1}, {TYPE2,v2}] -> [{TYPE2,v2}, {TYPE1,v1}]
    struct Element elem1 = {NO_ELEM_TYPE, {0}};
    struct Element elem2 = {NO_ELEM_TYPE, {0}};

    stack_pop(&elem1);
    stack_pop(&elem2);
    stack_push(&elem1);
    stack_push(&elem2);
}

void dup_op() {
    // Duplication operation: [{TYPE1,v1}] -> [{TYPE!,v1}, {TYPE1,v1}]
    struct Element elem = {NO_ELEM_TYPE, {0}};

    stack_pop(&elem);
    stack_push(&elem);
    stack_push(&elem);
}

void index_op() {
    // Index operation: [{TYPE1,v1}, {TYPE2,v2}, {TYPE3,v3}, {ELEMENT_NUMBER,1}]
    // -> [{TYPE1,v1}, {TYPE2,v2}, {TYPE3,v3}, {TYPE2,v2}]
    struct Element elem_idx = {NO_ELEM_TYPE, {0}};
    struct Element elem = {NO_ELEM_TYPE, {0}};

    stack_pop(&elem_idx);
    stack_copy(&elem, elem_idx.u.number);
    stack_push(&elem);
}

void roll_op() {
    // Roll operation: [{TYPE1,v1}, {TYPE2,v2}, {TYPE3,v3}, {ELEMENT_NUMBER,2}, {ELEMENT_NUMBER,1}]
    // -> [{TYPE1,v1}, {TYPE3,v3}, {TYPE2,v2}]
    struct Element elem_roll_num = {NO_ELEM_TYPE, {0}};
    struct Element elem_roll_times = {NO_ELEM_TYPE, {0}};

    stack_pop(&elem_roll_times);
    stack_pop(&elem_roll_num);
    struct Element elem_arr[elem_roll_num.u.number];
    for (int i = 0; i < elem_roll_num.u.number; i++) {
        struct Element elem = {NO_ELEM_TYPE, {0}};
        stack_pop(&elem);
        elem_arr[i] = elem;
    }
    for (int i = 0; i < elem_roll_num.u.number; i++) {
        int stack_idx = (elem_roll_num.u.number - 1 - i + elem_roll_times.u.number) % elem_roll_num.u.number;
        stack_push(&elem_arr[stack_idx]);
    }
}

void exec_op() {
    // Execution operation: execute executable arrays on the stack top
    struct Element opelem = {NO_ELEM_TYPE, {0}};

    stack_pop(&opelem);
    eval_exec_array(opelem.u.exec_array);
}

void if_op() {
    // If operation: [{ELEMENT_NUMBER, 1}, {ELEMENT_EXECUTABLE_ARRAY, exec_array}] 
    // -> execute exec_array. Do nothing if ELEMENT_NUMBER is 0
    struct Element opelem = {NO_ELEM_TYPE, {0}};
    struct Element boolean_flg = {NO_ELEM_TYPE, {0}};

    stack_pop(&opelem);
    stack_pop(&boolean_flg);
    if (boolean_flg.u.number == 1) {
        eval_exec_array(opelem.u.exec_array);
    }
}

void ifelse_op() {
    // Ifelse operation: [{ELEMENT_NUMBER, 1}, {ELEMENT_EXECUTABLE_ARRAY1, exec_array1}, {ELEMENT_EXECUTABLE_ARRAY2, exec_array2}] 
    // -> execute exec_array1. Execute exec_array2 if ELEMENT_NUMBER is 0
    struct Element opelem_true = {NO_ELEM_TYPE, {0}};
    struct Element opelem_false = {NO_ELEM_TYPE, {0}};
    struct Element boolean_flg = {NO_ELEM_TYPE, {0}};

    stack_pop(&opelem_false);
    stack_pop(&opelem_true);
    stack_pop(&boolean_flg);
    if (boolean_flg.u.number == 1) {
        eval_exec_array(opelem_true.u.exec_array);
    } else if (boolean_flg.u.number == 0) {
        eval_exec_array(opelem_false.u.exec_array);
    }
}

void while_op() {
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
}

void repeat_op() {
    // Repeat operation: 3 { exec_array } repeat -> execute exec_array 3 times
    struct Element opelem_num = {NO_ELEM_TYPE, {0}};
    struct Element opelem_body = {NO_ELEM_TYPE, {0}};

    stack_pop(&opelem_body);
    stack_pop(&opelem_num);
    int i = opelem_num.u.number;
    for (i; i > 0; i--) {
        eval_exec_array(opelem_body.u.exec_array);
    }
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
    register_one_primitive("mod", mod_op);
    register_one_primitive("eq", eq_op);
    register_one_primitive("neq", neq_op);
    register_one_primitive("gt", gt_op);
    register_one_primitive("ge", ge_op);
    register_one_primitive("lt", lt_op);
    register_one_primitive("le", le_op);
    register_one_primitive("pop", pop_op);
    register_one_primitive("exch", exch_op);
    register_one_primitive("dup", dup_op);
    register_one_primitive("index", index_op);
    register_one_primitive("roll", roll_op);
    register_one_primitive("exec", exec_op);
    register_one_primitive("if", if_op);
    register_one_primitive("ifelse", ifelse_op);
    register_one_primitive("while", while_op);
    register_one_primitive("repeat", repeat_op);
}

