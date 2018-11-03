#include "stack.h"
#include "cl_util.h"
#include <assert.h>

#define DICT_SIZE 1024

static int dict_pos = 0;

static void reset_dict() {
    dict_pos = 0;
}

struct KeyValue {
    char *key;
    struct Data value;
};

static struct KeyValue dict_array[DICT_SIZE];

void dict_put(char* key, struct Data *elem){
    struct KeyValue put_key_value = {key, *elem};
    dict_array[dict_pos++] = put_key_value;
};

int dict_get(char* key, struct Data *out_elem){
    for(int i = 0; i < dict_pos; i++) {
        if (dict_array[i].key == key) {
            *out_elem = dict_array[i].value;
            return 1;
        }
    }
};

void dict_print_all(){;};

static int two_keyvalue_eq(struct KeyValue *kv1, struct KeyValue *kv2) {
    // Return 1 if two KeyValues are identical.
    if (kv1->key == kv2->key) {
        return two_data_eq(&kv1->value, &kv2->value);
    } else {
        return 0;
    }
}

static void assert_two_keyvalue_eq(struct KeyValue *kv1, struct KeyValue *kv2) {
    assert(two_keyvalue_eq(kv1, kv2));
}

static void test_one_put() {
    char *input_key = "key";
    struct Data input_data = {NUMBER, {123}};
    struct KeyValue expect = {"key", {NUMBER, {123}}};

    dict_put(input_key, &input_data);
    struct KeyValue *actual = &dict_array[0];

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_one_put_one_get() {
    char *input_key = "key";
    struct Data input_data = {NUMBER, {123}};
    struct Data expect_data = {NUMBER, {123}};
    int expect = 1;

    dict_put(input_key, &input_data);
    struct Data actual_data = {UNKNOWN, {0}};
    int actual = dict_get("key", &actual_data);

    assert(expect == actual);
    assert_two_data_eq(&expect_data, &actual_data);
    reset_dict();
}

static void test_two_put() {
    char *input_key_1 = "key1";
    struct Data input_data_1 = {NUMBER, {123}};
    char *input_key_2 = "key2";
    struct Data input_data_2 = {LITERAL_NAME, .u.name = "abc"};
    struct KeyValue expect = {"key2", {LITERAL_NAME, .u.name = "abc"}};

    dict_put(input_key_1, &input_data_1);
    dict_put(input_key_2, &input_data_2);
    struct KeyValue *actual = &dict_array[1];

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_two_put_two_get() {
    char *input_key_1 = "key1";
    struct Data input_data_1 = {NUMBER, {123}};
    char *input_key_2 = "key2";
    struct Data input_data_2 = {LITERAL_NAME, .u.name = "abc"};
    struct Data expect_data_1 = {NUMBER, {123}};
    struct Data expect_data_2 = {LITERAL_NAME, .u.name = "abc"};
    int expect_1 = 1;
    int expect_2 = 1;

    dict_put(input_key_1, &input_data_1);
    dict_put(input_key_2, &input_data_2);
    struct Data actual_data_1 = {UNKNOWN, {0}};
    struct Data actual_data_2 = {UNKNOWN, {0}};
    int actual_1 = dict_get("key1", &actual_data_1);
    int actual_2 = dict_get("key2", &actual_data_2);

    assert(expect_1 == actual_1);
    assert_two_data_eq(&expect_data_1, &actual_data_1);
    assert(expect_2 == actual_2);
    assert_two_data_eq(&expect_data_2, &actual_data_2);

    reset_dict();
}


int main () {
    test_one_put();
    test_one_put_one_get();
    test_two_put();
    test_two_put_two_get();

    return 0;
}
