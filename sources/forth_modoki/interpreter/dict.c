#include <assert.h>
#include <stdlib.h>
#include "cl_util.h"
#include "dict.h"
#include "stack.h"


typedef struct KeyValue {
    char* key;
    Data_t value;
} KeyValue_t;

typedef struct Node {
    char* key;
    Data_t value;
    struct Node* next;
} Node_t;


#define TABLE_SIZE 16
static Node_t* dict_array[TABLE_SIZE];


static void reset_dict() {
    for (int i=0; i < TABLE_SIZE; i++) {
        if (dict_array[i] == NULL) {
            continue;
        } else {
            free(dict_array[i]);
            dict_array[i] = NULL;
        }
    }
}

static int hash(char* str) {
    unsigned int val = 0;
    while(*str) {
        val += *str++;
    }
    return (int)(val % TABLE_SIZE);
}

static int* create_new_node(char* key, Data_t* elem) {
    Node_t* head;
    head = malloc(sizeof(Node_t));
    head->next = NULL;
    head->key = key;
    head->value = *elem;

    return head;
}

static void update_or_insert_list(Node_t** headPtr, char* key, Data_t* elem) {
    // If there is the same key in some node, overwrite value in the node.
    // If there isn't, add a new node in the end of list.
    while (*headPtr != NULL) {
        if (streq((*headPtr)->key, key)) {
            (*headPtr)->value = *elem;
            return;
        }
        headPtr = &((*headPtr)->next);
    }

    Node_t* head;
    head = create_new_node(key, elem);
    *headPtr = head;
}

void dict_put(char* key, Data_t* elem) {
    int idx = hash(key);
    Node_t* head = dict_array[idx];
    Node_t** headPtr = &head;
    if (head == NULL) {
       head = create_new_node(key, elem);
       dict_array[idx] = head;
       return;
    }
    update_or_insert_list(headPtr, key, elem);
}

int dict_get(char* key, Data_t* out_elem) {
    int idx = hash(key);
    Node_t* head = dict_array[idx];
    Node_t** headPtr = &head;

    while (*headPtr != NULL) {
        if (streq((*headPtr)->key, key)) {
            *out_elem = (*headPtr)->value;
            return 1;
        }
        headPtr = &((*headPtr)->next);
    }

    return 0;
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

static int two_keyvalue_eq(KeyValue_t* kv1, KeyValue_t* kv2) {
    // Return 1 if two KeyValues are identical.
    if (kv1->key == kv2->key) {
        return two_data_eq(&kv1->value, &kv2->value);
    } else {
        return 0;
    }
}

static void assert_two_keyvalue_eq(KeyValue_t* kv1, KeyValue_t* kv2) {
    assert(two_keyvalue_eq(kv1, kv2));
}

static void test_hash() {
    char* input_key = "a";
    int expect = 1; // 97 (a) module 16 = 1

    int actual = hash(input_key);

    assert(expect == actual);
}

static void test_hash_longkey() {
    char* input_key = "aaaaaaaaaaaa";
    int expect = 12; // 97*12 modulo 16 = 140

    int actual = hash(input_key);

    assert(expect == actual);
}

static void test_one_put() {
    char* input_key = "key";
    Data_t input_data = {NUMBER, {123}};
    KeyValue_t expect = {"key", {NUMBER, {123}}};

    dict_put(input_key, &input_data);
    int idx = hash(input_key);
    Node_t* actual_node = &dict_array[idx];
    KeyValue_t* actual = {actual_node->key, actual_node->value};

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_one_put_one_get() {
    char* input_key = "key";
    Data_t input_data = {NUMBER, {123}};
    Data_t expect_data = {NUMBER, {123}};
    int expect = 1;

    dict_put(input_key, &input_data);
    Data_t actual_data = {UNKNOWN, {0}};
    int actual = dict_get("key", &actual_data);

    assert(expect == actual);
    assert_two_data_eq(&expect_data, &actual_data);
    reset_dict();
}

static void test_two_put() {
    char* input_key_1 = "key1";
    Data_t input_data_1 = {NUMBER, {123}};
    char* input_key_2 = "key2";
    Data_t input_data_2 = {LITERAL_NAME, .u.name = "abc"};
    KeyValue_t expect = {"key2", {LITERAL_NAME, .u.name = "abc"}};

    dict_put(input_key_1, &input_data_1);
    dict_put(input_key_2, &input_data_2);
    int idx_2 = hash(input_key_2);
    Node_t* actual_node = &dict_array[idx_2];
    KeyValue_t* actual = {actual_node->key, actual_node->value};

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_two_put_two_get() {
    char* input_key_1 = "key1";
    Data_t input_data_1 = {NUMBER, {123}};
    char* input_key_2 = "key2";
    Data_t input_data_2 = {LITERAL_NAME, .u.name = "abc"};
    Data_t expect_data_1 = {NUMBER, {123}};
    Data_t expect_data_2 = {LITERAL_NAME, .u.name = "abc"};
    int expect_1 = 1;
    int expect_2 = 1;

    dict_put(input_key_1, &input_data_1);
    dict_put(input_key_2, &input_data_2);
    Data_t actual_data_1 = {UNKNOWN, {0}};
    Data_t actual_data_2 = {UNKNOWN, {0}};
    int actual_1 = dict_get("key1", &actual_data_1);
    int actual_2 = dict_get("key2", &actual_data_2);

    assert(expect_1 == actual_1);
    assert_two_data_eq(&expect_data_1, &actual_data_1);
    assert(expect_2 == actual_2);
    assert_two_data_eq(&expect_data_2, &actual_data_2);

    reset_dict();
}

static void test_rewrite_dict() {
    char* input_key = "key";
    Data_t input_data_1 = {NUMBER, {123}};
    Data_t input_data_2 = {LITERAL_NAME, .u.name = "abc"};
    KeyValue_t expect = {"key", {LITERAL_NAME, .u.name = "abc"}};

    dict_put(input_key, &input_data_1);
    dict_put(input_key, &input_data_2);
    int idx = hash(input_key);
    Node_t* actual_node = &dict_array[idx];
    KeyValue_t* actual = {actual_node->key, actual_node->value};

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_get_nil_key() {
    char* input_key = "key";
    Data_t input_data = {NUMBER, {123}};
    int expect = 0;

    dict_put(input_key, &input_data);
    Data_t actual_data = {UNKNOWN, {0}};
    int actual = dict_get("nil_key", &actual_data);

    assert(expect == actual);
    reset_dict();
}

static void test_put_colliding_key() {
    char* input_key_1 = "key";
    char* input_key_2 = "yek";
    Data_t input_data_1 = {NUMBER, {123}};
    Data_t input_data_2 = {LITERAL_NAME, .u.name = "abc"};
    Data_t expect_data_1 = {NUMBER, {123}};
    Data_t expect_data_2 = {LITERAL_NAME, .u.name = "abc"};

    dict_put(input_key_1, &input_data_1);
    dict_put(input_key_2, &input_data_2);
    int idx_1 = hash(input_key_1);
    int idx_2 = hash(input_key_2);
    Data_t actual_data_1 = {UNKNOWN, {0}};
    Data_t actual_data_2 = {UNKNOWN, {0}};
    dict_get("key", &actual_data_1);
    dict_get("yek", &actual_data_2);

    assert_two_data_eq(&expect_data_1, &actual_data_1);
    assert_two_data_eq(&expect_data_2, &actual_data_2);

    reset_dict();
}


#if 0
int main () {
    test_hash();
    test_hash_longkey();
    test_one_put();
    test_one_put_one_get();
    test_two_put();
    test_two_put_two_get();
    test_rewrite_dict();
    test_get_nil_key();
    test_put_colliding_key();

    return 0;
}
#endif
