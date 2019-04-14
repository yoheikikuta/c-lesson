#include <assert.h>
#include <stdlib.h>
#include "cl_util.h"
#include "dict.h"
#include "stack.h"


struct KeyValue {
    char* key;
    struct Element value;
};

struct Node {
    char* key;
    struct Element value;
    struct Node* next;
};


#define TABLE_SIZE 16
static struct Node* dict_array[TABLE_SIZE];
struct Element* compile_dict_array[TABLE_SIZE];


// dict_array[Null, Node(next = NULL), Node1(next = Node2), ...] -> dict_array[Null, Null, Null, ...]
// free each Node to release memory.
static void reset_dict() {
    struct Node* cur_nd;
    struct Node* nd_to_be_free;

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

static struct Node* create_new_node(char* key, struct Element* elem) {
    struct Node* cur_nd;
    cur_nd = malloc(sizeof(struct Node));
    cur_nd->next = NULL;
    cur_nd->key = key;
    cur_nd->value = *elem;

    return cur_nd;
}

// Add head/tail node or update the value of the node according to input key.
static void update_or_insert_list(struct Node** head_nd_ptr, char* key, struct Element* elem) {
    struct Node** cur_nd_ptr = head_nd_ptr;

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

static void dict_put_commmon(struct Node** table, char* key, struct Element* elem) {
    int idx = hash(key);
    struct Node** head_nd_ptr = &(table[idx]);

    update_or_insert_list(head_nd_ptr, key, elem);

    return;
}

void dict_put(char* key, struct Element* elem) {
    return dict_put_commmon(dict_array, key, elem);
}

void compile_dict_put(char* key, struct Element* elem) {
    return dict_put_commmon(compile_dict_array, key, elem);
}

static int dict_get_common(struct Node** table, char *key, struct Element *out_elem) {
    int idx = hash(key);
    struct Node* head_nd = table[idx];
    struct Node* cur_nd = head_nd;

    while (cur_nd != NULL) {
        if (streq(cur_nd->key, key)) {
            *out_elem = cur_nd->value;
            return 1;
        }
        cur_nd = cur_nd->next;
    }

    return 0;
}

int dict_get(char *key, struct Element *out_elem) {
    return dict_get_common(dict_array, key, out_elem);
}

int compile_dict_get(char *key, struct Element *out_elem) {
    return dict_get_common(compile_dict_array, key, out_elem);
}

void dict_print_all() {
    printf("All keys : values in the dict are\n");
    printf("---------------\n");
    for (int i=0; i < TABLE_SIZE; i++) {
        struct Node* cur = dict_array[i];
        while (cur != NULL) {
            if (cur->value.etype == NUMBER) {
                int num = cur->value.u.number;
                printf("%s : %i\n", cur->key, num);
            } else if (cur->value.etype == LITERAL_NAME) {
                char* name = cur->value.u.name;
                printf("%s : %s\n", cur->key, name);
            }
            cur = cur->next;
        }
    }
    printf("---------------\n");
};

//
// TEST
//

static int two_keyvalue_eq(struct KeyValue* kv1, struct KeyValue* kv2) {
    // Return 1 if two KeyValues are identical.
    if (kv1->key == kv2->key) {
        return two_data_eq(&kv1->value, &kv2->value);
    } else {
        return 0;
    }
}

static void assert_two_keyvalue_eq(struct KeyValue* kv1, struct KeyValue* kv2) {
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
    struct Element input_data = {ELEMENT_NUMBER, {123}};
    struct KeyValue expect = {"key", {ELEMENT_NUMBER, {123}}};

    dict_put(input_key, &input_data);
    int idx = hash(input_key);
    struct Node* actual_node = &dict_array[idx];
    struct KeyValue* actual = {actual_node->key, actual_node->value};

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_one_put_one_get() {
    char* input_key = "key";
    struct Element input_data = {ELEMENT_NUMBER, {123}};
    struct Element expect_data = {ELEMENT_NUMBER, {123}};
    int expect = 1;

    dict_put(input_key, &input_data);
    struct Element actual_data = {NO_ELEM_TYPE, {0}};
    int actual = dict_get("key", &actual_data);

    assert(expect == actual);
    assert_two_data_eq(&expect_data, &actual_data);
    reset_dict();
}

static void test_two_put() {
    char* input_key_1 = "key1";
    struct Element input_data_1 = {ELEMENT_NUMBER, {123}};
    char* input_key_2 = "key2";
    struct Element input_data_2 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct KeyValue expect = {"key2", {ELEMENT_LITERAL_NAME, .u.name = "abc"}};

    dict_put(input_key_1, &input_data_1);
    dict_put(input_key_2, &input_data_2);
    int idx_2 = hash(input_key_2);
    struct Node* actual_node = &dict_array[idx_2];
    struct KeyValue* actual = {actual_node->key, actual_node->value};

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_two_put_two_get() {
    char* input_key_1 = "key1";
    struct Element input_data_1 = {ELEMENT_NUMBER, {123}};
    char* input_key_2 = "key2";
    struct Element input_data_2 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element expect_data_1 = {ELEMENT_NUMBER, {123}};
    struct Element expect_data_2 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    int expect_1 = 1;
    int expect_2 = 1;

    dict_put(input_key_1, &input_data_1);
    dict_put(input_key_2, &input_data_2);
    struct Element actual_data_1 = {NO_ELEM_TYPE, {0}};
    struct Element actual_data_2 = {NO_ELEM_TYPE, {0}};
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
    struct Element input_data_1 = {ELEMENT_NUMBER, {123}};
    struct Element input_data_2 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct KeyValue expect = {"key", {ELEMENT_LITERAL_NAME, .u.name = "abc"}};

    dict_put(input_key, &input_data_1);
    dict_put(input_key, &input_data_2);
    int idx = hash(input_key);
    struct Node* actual_node = &dict_array[idx];
    struct KeyValue* actual = {actual_node->key, actual_node->value};

    assert_two_keyvalue_eq(&expect, actual);
    reset_dict();
}

static void test_get_nil_key() {
    char* input_key = "key";
    struct Element input_data = {ELEMENT_NUMBER, {123}};
    int expect = 0;

    dict_put(input_key, &input_data);
    struct Element actual_data = {NO_ELEM_TYPE, {0}};
    int actual = dict_get("nil_key", &actual_data);

    assert(expect == actual);
    reset_dict();
}

static void test_put_colliding_key() {
    // This test depends on the dictionary algorithm (hash table).
    char* input_key_1 = "key";
    char* input_key_2 = "yek";
    struct Element input_data_1 = {ELEMENT_NUMBER, {123}};
    struct Element input_data_2 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element expect_data_1 = {ELEMENT_NUMBER, {123}};
    struct Element expect_data_2 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};

    dict_put(input_key_1, &input_data_1);
    dict_put(input_key_2, &input_data_2);
    int idx_1 = hash(input_key_1);
    int idx_2 = hash(input_key_2);
    struct Element actual_data_1 = {NO_ELEM_TYPE, {0}};
    struct Element actual_data_2 = {NO_ELEM_TYPE, {0}};
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
