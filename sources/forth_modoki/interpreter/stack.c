#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cl_util.h"
#include "stack.h"


static struct Element stack[STACK_SIZE];
static int stack_num = 0;

void reset_stack() {
    stack_num = 0;
}

int stack_push(struct Element* push_elem) {
    if (stack_num == STACK_SIZE) {
        return 0;
    } else {
        stack[stack_num++] = *push_elem;

        return 1;
    }
}

int stack_pop(struct Element* out_elem) {
    if (stack_num == 0) {
        return 0;
    } else {
        *out_elem = stack[--stack_num];

        return 1;
    }
}

int stack_pop_int(){
    // Return the value of popped NUMBER data from the stack.
    struct Element input = {NO_ELEM_TYPE, {0}};
    stack_pop(&input);
    int num = input.u.number;

    return num;
}

char* stack_pop_str(){
    // Return the value of popped LITERAL_NAME data from the stack.
    struct Element input = {NO_ELEM_TYPE, {0}};
    stack_pop(&input);
    char* str = input.u.name;

    return str;
}

int stack_copy(struct Element* out_elem, int idx) {
    if (stack_num == 0) {
        return 0;
    } else {
        // PostScript's index starts with 0 from the stack top.
        *out_elem = stack[stack_num - idx - 1];

        return 1;
    }
}

void stack_print_all() {
	struct Element elem = {UNKNOWN,{0}};
	printf("----- STACK  TOP -----\n");
	while(stack_pop(&elem)) {
		switch(elem.etype) {
			case ELEMENT_NUMBER:
				printf("Number: %d\n", elem.u.number);
				break;
			case ELEMENT_LITERAL_NAME:
				printf("Literal name: %s\n", elem.u.name);
				break;
			default:
				printf("Unknown type: %d\n", elem.etype);
				break;
		}
	}
	printf("-----STACK BOTTOM-----\n");
}

//
// TEST
//

static void test_pop_from_empty() {
    struct Element input = {NO_ELEM_TYPE, {0}};
    int expect = 0;
    int actual = stack_pop(&input);

    assert(expect == actual);
    reset_stack();
}

static void test_push_to_full() {
    struct Element input = {NO_ELEM_TYPE, {0}};
    int expect = 0;
    while (stack_num < STACK_SIZE) {
        stack_push(&input);
    }
    int actual = stack_push(&input);

    assert(expect == actual);
    reset_stack();
}

static void test_one_push() {
    struct Element input = {ELEMENT_NUMBER, {123}};
    struct Element expect_data = {ELEMENT_NUMBER, {123}};
    int expect = 1;

    int actual = stack_push(&input);
    struct Element* actual_data = &stack[0];

    assert(expect == actual);
    assert_two_data_eq(&expect_data, actual_data);
    reset_stack();
}

static void test_one_push_one_pop() {
    struct Element input = {ELEMENT_NUMBER, {123}};
    struct Element expect_data_pop = {ELEMENT_NUMBER, {123}};
    int expect = 1;

    stack_push(&input);
    struct Element actual_data_pop;
    int actual = stack_pop(&actual_data_pop);

    assert(expect == actual);
    assert_two_data_eq(&expect_data_pop, &actual_data_pop);
    reset_stack();
}

static void test_two_push() {
    struct Element input_1 = {ELEMENT_NUMBER, {123}};
    struct Element input_2 = {ELEMENT_LITERAL_NAME, .u.name = "add"};
    struct Element expect_data_1 = {ELEMENT_NUMBER, {123}};
    struct Element expect_data_2 = {ELEMENT_LITERAL_NAME, .u.name = "add"};
    int expect_1 = 1;
    int expect_2 = 1;

    int actual_1 = stack_push(&input_1);
    int actual_2 = stack_push(&input_2);
    struct Element* actual_data_1 = &stack[0];
    struct Element* actual_data_2 = &stack[1];

    assert(expect_1 == actual_1);
    assert(expect_2 == actual_2);
    assert_two_data_eq(&expect_data_1, actual_data_1);
    assert_two_data_eq(&expect_data_2, actual_data_2);

    reset_stack();
}

static void test_two_push_two_pop() {
    struct Element input_1 = {ELEMENT_NUMBER, {123}};
    struct Element input_2 = {ELEMENT_LITERAL_NAME, .u.name = "add"};
    struct Element expect_data_pop_1 = {ELEMENT_NUMBER, {123}};
    struct Element expect_data_pop_2 = {ELEMENT_LITERAL_NAME, .u.name = "add"};
    int expect_1 = 1;
    int expect_2 = 1;

    stack_push(&input_1);
    stack_push(&input_2);

    struct Element actual_data_pop_1, actual_data_pop_2;
    int actual_2 = stack_pop(&actual_data_pop_2);
    int actual_1 = stack_pop(&actual_data_pop_1);

    assert(expect_1 == actual_1);
    assert(expect_2 == actual_2);
    assert_two_data_eq(&expect_data_pop_1, &actual_data_pop_1);
    assert_two_data_eq(&expect_data_pop_2, &actual_data_pop_2);

    reset_stack();
}

static void unit_tests() {
    test_pop_from_empty();
    test_push_to_full();
    test_one_push();
    test_one_push_one_pop();
    test_two_push();
    test_two_push_two_pop();

    printf("All unit tests are successfully passed.\n");
}


#if 0
int main() {
    unit_tests();

    return 0;
}
#endif
