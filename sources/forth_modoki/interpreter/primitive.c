#include <stdio.h>
#include "dict.h"


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

