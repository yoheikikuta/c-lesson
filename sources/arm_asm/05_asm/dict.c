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

void dict_put(int key, struct Word *elem) {
    int index = 0;
    if (find_key_index(key, &index)) {
        dict_array[index].value = *elem;
    } else {
        struct KeyValue key_value_elem = {key, *elem};
        dict_array[dict_pos++] = key_value_elem;
    }
};

int dict_get(int key, struct Word *out_elem) {
    int index = 0;
    if (find_key_index(key, &index)) {
        *out_elem = dict_array[index].value;
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
    struct Word input_word = {WORD_NUMBER, .u.number = 123};
    struct KeyValue expect = {2, {WORD_NUMBER, .u.number = 123}};

    dict_put(input_key, &input_word);
    struct KeyValue* actual = &dict_array[0];

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_one_put_one_get() {
    int input_key = 2;
    struct Word input_word = {WORD_STRING, .u.str = "test"};
    struct Word expect_word = {WORD_STRING, .u.str = "test"};
    int expect = 1;

    dict_put(input_key, &input_word);
    struct Word actual_word = {NO_WORD_TYPE, {0}};
    int actual = dict_get(2, &actual_word);

    assert(expect == actual);
    assert_two_word_eq(&expect_word, &actual_word);
    reset_dict();
}

static void test_two_put() {
    int input_key_1 = 1;
    struct Word input_word_1 = {WORD_STRING, .u.str = "test"};
    char *input_key_2 = 2;
    struct Word input_word_2 = {WORD_NUMBER, .u.number = 5};
    struct KeyValue expect_1 = {1, {WORD_STRING, .u.str = "test"}};
    struct KeyValue expect_2 = {2, {WORD_NUMBER, .u.number = 5}};

    dict_put(input_key_1, &input_word_1);
    dict_put(input_key_2, &input_word_2);
    struct KeyValue* actual_1 = &dict_array[0];
    struct KeyValue* actual_2 = &dict_array[1];

    assert_two_keyvalue_eq(&expect_1, actual_1);
    assert_two_keyvalue_eq(&expect_2, actual_2);
    reset_dict();
}

static void test_two_put_two_get() {
    int input_key_1 = 1;
    struct Word input_word_1 = {WORD_STRING, .u.str = "test"};
    int input_key_2 = 2;
    struct Word input_word_2 = {WORD_NUMBER, .u.number = 5};
    struct Word expect_word_1 = {WORD_STRING, .u.str = "test"};
    struct Word expect_word_2 = {WORD_NUMBER, .u.number = 5};
    int expect_1 = 1;
    int expect_2 = 1;

    dict_put(input_key_1, &input_word_1);
    dict_put(input_key_2, &input_word_2);
    struct Word actual_word_1 = {NO_WORD_TYPE, {0}};
    struct Word actual_word_2 = {NO_WORD_TYPE, {0}};
    int actual_1 = dict_get(1, &actual_word_1);
    int actual_2 = dict_get(2, &actual_word_2);

    assert(expect_1 == actual_1);
    assert_two_word_eq(&expect_word_1, &actual_word_1);
    assert(expect_2 == actual_2);
    assert_two_word_eq(&expect_word_2, &actual_word_2);

    reset_dict();
}

static void test_rewrite_dict() {
    int input_key = 2;
    struct Word input_word_1 = {WORD_NUMBER, .u.number = 123};
    struct Word input_word_2 = {WORD_STRING, .u.str = "abc"};
    struct KeyValue expect = {2, {WORD_STRING, .u.str = "abc"}};

    dict_put(input_key, &input_word_1);
    dict_put(input_key, &input_word_2);
    struct KeyValue* actual = &dict_array[0];

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_get_nil_key() {
    char *input_key = 1;
    struct Word input_word = {WORD_NUMBER, {123}};
    int expect = 0;

    dict_put(input_key, &input_word);
    struct Word actual_word = {NO_WORD_TYPE, {0}};
    int actual = dict_get(100, &actual_word);

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