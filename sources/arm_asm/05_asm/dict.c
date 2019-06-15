#include "dict.h"
#include "assembler.h"
#include "cl_util.h"
#include <assert.h>

struct KeyValue dict_array[DICT_SIZE];
static int dict_pos = 0;


static void reset_dict() {
    dict_pos = 0;
}

static int find_key_index(int key, int* out_index) {
    *out_index = 0;
    for (int i=0; i < dict_pos; i++) {
        if (dict_array[i].key == key) {
            *out_index = i;
            return 1;
        }
    }
    return 0;
}

void dict_put(int key, int value) {
    int index = 0;
    if (find_key_index(key, &index)) {
        dict_array[index].value = value;
    } else {
        struct KeyValue key_value_elem = {key, value};
        dict_array[dict_pos++] = key_value_elem;
    }
};

int dict_get(int key, int* value) {
    int index = 0;
    if (find_key_index(key, &index)) {
        *value = dict_array[index].value;
        return 1;
    } else {
        return 0;
    }
};

// 
// TEST
// 

static void test_one_put() {
    int input_key = 2;
    int input_value = 123;
    struct KeyValue expect = {2, 123};

    dict_put(input_key, input_value);
    struct KeyValue* actual = &dict_array[0];

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_one_put_one_get() {
    int input_key = 2;
    int input_value = 123;
    int expect_value = 123;
    int expect = 1;

    dict_put(input_key, input_value);
    int actual_value = 0;
    int actual = dict_get(2, &actual_value);

    assert(expect == actual);
    assert(expect_value == actual_value);
    reset_dict();
}

static void test_two_put() {
    int input_key_1 = 1;
    int input_value_1 = 123;
    int input_key_2 = 2;
    int input_value_2 = 456;
    struct KeyValue expect_1 = {1, 123};
    struct KeyValue expect_2 = {2, 456};

    dict_put(input_key_1, input_value_1);
    dict_put(input_key_2, input_value_2);
    struct KeyValue* actual_1 = &dict_array[0];
    struct KeyValue* actual_2 = &dict_array[1];

    assert_two_keyvalue_eq(&expect_1, actual_1);
    assert_two_keyvalue_eq(&expect_2, actual_2);
    reset_dict();
}

static void test_two_put_two_get() {
    int input_key_1 = 1;
    int input_value_1 = 123;
    int input_key_2 = 2;
    int input_value_2 = 456;
    int expect_value_1 = 123;
    int expect_value_2 = 456;
    int expect_1 = 1;
    int expect_2 = 1;

    dict_put(input_key_1, input_value_1);
    dict_put(input_key_2, input_value_2);
    int actual_value_1 = 0;
    int actual_value_2 = 0;
    int actual_1 = dict_get(1, &actual_value_1);
    int actual_2 = dict_get(2, &actual_value_2);

    assert(expect_1 == actual_1);
    assert(expect_value_1 == actual_value_1);
    assert(expect_2 == actual_2);
    assert(expect_value_2 == actual_value_2);

    reset_dict();
}

static void test_rewrite_dict() {
    int input_key = 2;
    int input_value_1 = 123;
    int input_value_2 = 456;
    struct KeyValue expect = {2, 456};

    dict_put(input_key, input_value_1);
    dict_put(input_key, input_value_2);
    struct KeyValue* actual = &dict_array[0];

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_get_nil_key() {
    int *input_key = 1;
    int input_value = 123;
    int expect = 0;

    dict_put(input_key, input_value);
    int actual_value = 0;
    int actual = dict_get(100, &actual_value);

    assert(expect == actual);
    reset_dict();
}

static void unittests() {
    test_one_put();
    test_one_put_one_get();
    test_two_put();
    test_two_put_two_get();
    test_rewrite_dict();
    test_get_nil_key();

    printf("All unittests successfully passed.\n");
}


#if 0
int main() {
    unittests();
    return 0;
}
#endif