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


// dict_array[Null, Node(next = NULL), Node1(next = Node2), ...] -> dict_array[Null, Null, Null, ...]
// free each Node to release memory.
static void reset_dict() {
    Node_t* cur_nd;
    Node_t* nd_to_be_free;

    for (int i=0; i < TABLE_SIZE; i++) {
        cur_nd = dict_array[i];
        if (cur_nd != NULL) {
            while (cur_nd != NULL) {
                nd_to_be_free = cur_nd;
                cur_nd = cur_nd->next;
                free(nd_to_be_free);
            }
            dict_array[i] = NULL;
        }
    }

    return;
}

static int hash(char* str) {
    unsigned int val = 0;

    while(*str) {
        val += *str++;
    }

    return (int)(val % TABLE_SIZE);
}

static Node_t* create_new_node(char* key, Data_t* elem) {
    Node_t* cur_nd;
    cur_nd = malloc(sizeof(Node_t));
    cur_nd->next = NULL;
    cur_nd->key = key;
    cur_nd->value = *elem;

    return cur_nd;
}

// Add head/tail node or update the value of the node according to input key.
static void update_or_insert_list(Node_t** head_nd_ptr, char* key, Data_t* elem) {
    Node_t** cur_nd_ptr = head_nd_ptr;

    // head node != NULL and the key is duplicated -> update the value of the key
    while (*cur_nd_ptr != NULL) {
        if (streq((*cur_nd_ptr)->key, key)) {
            // (key of the current node) == (input key) case
            (*cur_nd_ptr)->value = *elem;
            return;
        } else {
            cur_nd_ptr = &((*cur_nd_ptr)->next);
        }
    }

    // head node == NULL -> add a head node 
    // head node != NULL and key is new -> add a tail node
    *cur_nd_ptr = create_new_node(key, elem);

    return;
}

void dict_put(char* key, Data_t* elem) {
    int idx = hash(key);
    Node_t** head_nd_ptr = &(dict_array[idx]);

    update_or_insert_list(head_nd_ptr, key, elem);

    return;
}

int dict_get(char* key, Data_t* out_elem) {
    int idx = hash(key);
    Node_t* head_nd = dict_array[idx];
    Node_t* cur_nd = head_nd;

    while (cur_nd != NULL) {
        if (streq(cur_nd->key, key)) {
            *out_elem = cur_nd->value;
            return 1;
        }
        cur_nd = cur_nd->next;
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
    int expect = 1; // 97 (a) modulo 16 = 1

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
    printf("All unittests are successfully passed.\n");

    return 0;
}
#endif
