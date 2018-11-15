#include <assert.h>
#include <stdlib.h>
#include "cl_util.h"
#include "dict.h"
#include "stack.h"


typedef struct KeyValue {
    char *key;
    struct Data value;
} KeyValue;

typedef struct Node {
    char *key;
    struct Data value;
    struct Node *next;
} Node;


#define TABLE_SIZE 16
static struct Node *dict_array[TABLE_SIZE];


static void reset_dict() {
    struct Node *head;
    for (int i=0; i < TABLE_SIZE; i++) {
        if (head == NULL) {
            continue;
        } else {
            dict_array[i] = NULL;
            // This is a tentative implementation.
            // Need to free previously allocated memory?
        }
    }
}

static int hash(char *str) {
    unsigned int val = 0;
    while(*str) {
        val += *str++;
    }
    return (int)(val % TABLE_SIZE);
}

void update_or_insert_list(struct Node *head, char *key, struct Data *elem) {
    head->key = key;
    head->value = *elem;
}

void dict_put(char* key, struct Data *elem) {
    int idx = hash(key);
    struct Node *head = dict_array[idx];
    if (head == NULL) {
       head = malloc(sizeof(Node));
       head->next = NULL;
       head->key = key;
       head->value = *elem;
       dict_array[idx] = head;
       return;
    }
    // WRITE ME
    update_or_insert_list(head, key, elem);
}

int dict_get(char* key, struct Data *out_elem) {
    int idx = hash(key);
    struct Node *head = dict_array[idx];
    if (head == NULL) {
        return 0;
    } else {
        *out_elem = dict_array[idx]->value;
        return 1;
    }
}

// void dict_print_all() {
//     printf("All keys in the dict: ");
//     for (int i=0; i < dict_pos; i++) {
//         printf("%s ", dict_array[i]->key);
//     }
//     printf("\n");
// };

//
// TEST
//

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

static void test_hash() {
    char *input_key = "a";
    int expect = 1; // 97 (a) module 16 = 1

    int actual = hash(input_key);

    assert(expect == actual);
}

static void test_hash_longkey() {
    char *input_key = "aaaaaaaaaaaa";
    int expect = 12; // 97*12 modulo 16 = 140

    int actual = hash(input_key);

    assert(expect == actual);
}

static void test_one_put() {
    char *input_key = "key";
    struct Data input_data = {NUMBER, {123}};
    struct KeyValue expect = {"key", {NUMBER, {123}}};

    dict_put(input_key, &input_data);
    int idx = hash(input_key);
    struct Node *actual_node = &dict_array[idx];
    struct KeyValue *actual = {actual_node->key, actual_node->value};

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
    int idx_2 = hash(input_key_2);
    struct Node *actual_node = &dict_array[idx_2];
    struct KeyValue *actual = {actual_node->key, actual_node->value};

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

static void test_rewrite_dict() {
    char *input_key = "key";
    struct Data input_data_1 = {NUMBER, {123}};
    struct Data input_data_2 = {LITERAL_NAME, .u.name = "abc"};
    struct KeyValue expect = {"key", {LITERAL_NAME, .u.name = "abc"}};

    dict_put(input_key, &input_data_1);
    dict_put(input_key, &input_data_2);
    int idx = hash(input_key);
    struct Node *actual_node = &dict_array[idx];
    struct KeyValue *actual = {actual_node->key, actual_node->value};

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_get_nil_key() {
    char *input_key = "key";
    struct Data input_data = {NUMBER, {123}};
    int expect = 0;

    dict_put(input_key, &input_data);
    struct Data actual_data = {UNKNOWN, {0}};
    int actual = dict_get("nil_key", &actual_data);

    assert(expect == actual);
    reset_dict();
}


#if 1
int main () {
    test_hash();
    test_hash_longkey();
    test_one_put();
    test_one_put_one_get();
    test_two_put();
    test_two_put_two_get();
    test_rewrite_dict();
    test_get_nil_key();

    return 0;
}
#endif
