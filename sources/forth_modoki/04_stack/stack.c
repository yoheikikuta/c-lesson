#include <stdio.h>
#include <assert.h>
#include <string.h>

#define STACK_SIZE 1024


enum DataType {
    NUMBER,
    LITERAL_NAME,
    UNKNOWN,
};

struct Data {
    enum DataType dtype;
    union {
        int number;
        char *name;
    } u;
};

static struct Data stack[STACK_SIZE];
static int stack_num = 0;

static void reset_stack() {
    stack_num = 0;
}

int stack_push(struct Data *push_elem) {
    stack[0] = *push_elem;
    stack_num++;

    return 1;
}

int stack_pop(struct Data *pop_elem) {
    if (stack_num == 0) {
        return 0;
    } else {
        *pop_elem = stack[0];
        stack_num--;

        return 1;
    }
}


static int streq(char *s1, char *s2) {
    // Return 1 if two strings are identical.
    return strcmp(s1, s2) == 0;
}

static int two_data_eq(struct Data *d1, struct Data *d2) {
    // Return 1 if two Data are identical.
    if (d1->dtype == d2->dtype) {
        if (d1->dtype == NUMBER) {
            return d1->u.number == d2->u.number;
        } else if (d1->dtype == LITERAL_NAME) {
            return streq(d1->u.name, d2->u.name);
        }
    } else {
        return 0;
    }
}

static void assert_two_data_eq(struct Data *d1, struct Data *d2) {
    assert(two_data_eq(d1, d2));
}

static void test_pop_from_empty() {
    struct Data input = {UNKNOWN, {0}};
    int expect = 0;
    int actual = stack_pop(&input);

    assert(expect == actual);
    reset_stack();
}

static void test_one_push() {
    struct Data input = {NUMBER, {123}};
    struct Data expect_data = {NUMBER, {123}};
    int expect = 1;

    int actual = stack_push(&input);
    struct Data *actual_data = &stack[0];

    assert(expect == actual);
    assert_two_data_eq(&expect_data, actual_data);
    reset_stack();
}

static void test_one_push_one_pop() {
    struct Data input = {NUMBER, {123}};
    struct Data expect_data_pop = {NUMBER, {123}};
    int expect = 1;

    stack_push(&input);
    struct Data actual_data_pop;
    int actual = stack_pop(&actual_data_pop);

    assert(expect == actual);
    assert_two_data_eq(&expect_data_pop, &actual_data_pop);
    reset_stack();
}

static void test_two_push() {
    struct Data input_1 = {NUMBER, {123}};
    struct Data input_2 = {LITERAL_NAME, .u.name = "add"};
    struct Data expect_data_1 = {NUMBER, {123}};
    struct Data expect_data_2 = {LITERAL_NAME, .u.name = "add"};
    int expect_1 = 1;
    int expect_2 = 1;

    int actual_1 = stack_push(&input_1);
    int actual_2 = stack_push(&input_2);
    struct Data *actual_data_1 = &stack[0];
    struct Data *actual_data_2 = &stack[1];

    assert(expect_1 == actual_1);
    assert(expect_2 == actual_2);
    assert_two_data_eq(&expect_data_1, actual_data_1);
    assert_two_data_eq(&expect_data_2, actual_data_2);

    reset_stack();
}

static void test_two_push_two_pop() {
    struct Data input_1 = {NUMBER, {123}};
    struct Data input_2 = {LITERAL_NAME, .u.name = "add"};
    struct Data expect_data_pop_1 = {NUMBER, {123}};
    struct Data expect_data_pop_2 = {LITERAL_NAME, .u.name = "add"};
    int expect_1 = 1;
    int expect_2 = 1;

    stack_push(&input_1);
    stack_push(&input_2);

    struct Data actual_data_pop_1, actual_data_pop_2;
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
    test_one_push();
    test_one_push_one_pop();
    test_two_push();
    test_two_push_two_pop();
}


int main() {
    unit_tests();

    return 1;
}
