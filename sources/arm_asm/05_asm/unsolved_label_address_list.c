#include "cl_util.h"
#include "unsolved_label_address_list.h"
#include "assembler.h"
#include <stdio.h>
#include <assert.h>

static struct LinkedList* unsolved_label_address_list;


void unsolved_label_address_list_init() {
    unsolved_label_address_list = NULL;
}

// No node case:
//   unsolved_label_address_list = node
// Existing some node case:
//   Add node at the tail of unsolved_label_address_list.
void put_node(struct LinkedList* node) {

    if (unsolved_label_address_list == NULL) {
        // Adding the first node case.
        struct LinkedList* first_node;
        first_node = malloc(sizeof(struct LinkedList));
        first_node = node;
        first_node->next = NULL;
        unsolved_label_address_list = first_node;
    } else {
        // Adding the last node case.
        struct LinkedList* list = unsolved_label_address_list;
        while (list->next != NULL) {list = list->next;}

        struct LinkedList* last_node;
        last_node = malloc(sizeof(struct LinkedList));
        last_node = node;
        last_node->next = NULL;
        list->next = last_node;
    }
}

void create_and_add_node(int emitter_pos, int symbol_label, int word) {
    struct LinkedList* node;
    node = malloc(sizeof(struct LinkedList));

    node->emitter_pos = emitter_pos;
    node->label_id = symbol_label;
    node->word = word;
    node->next = NULL;

    put_node(node);
}

void get_first_node(struct LinkedList** out_node) {
    *out_node = unsolved_label_address_list;
}

// 
// TEST
// 

static void test_put_node_get_one() {
    unsolved_label_address_list_init();
    struct LinkedList expect = {.emitter_pos = 1, .label_id = 10, .word = 0xEA000000, .next = NULL};

    struct LinkedList* actual;
    put_node(&expect);
    get_first_node(&actual);

    assert_two_linkedlist_eq(&expect, actual);
}

static void test_put_node_get_two() {
    unsolved_label_address_list_init();
    struct LinkedList expect1 = {.emitter_pos = 1, .label_id = 10, .word = 0xEA000000, .next = NULL};
    struct LinkedList expect2 = {.emitter_pos = 5, .label_id = 15, .word = 0xEB000000, .next = NULL};

    struct LinkedList* actual;
    put_node(&expect1);
    put_node(&expect2);
    get_first_node(&actual);

    assert_two_linkedlist_eq(&expect1, actual);
    assert_two_linkedlist_eq(&expect2, actual->next);
}

static void test_get_first_node_no_node() {
    unsolved_label_address_list_init();
    int expect = NULL;

    struct LinkedList* actual;
    get_first_node(&actual);
    
    assert(expect == actual);
}

static void test_put_node_get_three() {
    unsolved_label_address_list_init();
    struct LinkedList expect1 = {.emitter_pos = 1, .label_id = 10, .word = 0xEA000000, .next = NULL};
    struct LinkedList expect2 = {.emitter_pos = 5, .label_id = 15, .word = 0xEB000000, .next = NULL};
    struct LinkedList expect3 = {.emitter_pos = 10, .label_id = 20, .word = 0xEC000000, .next = NULL};

    struct LinkedList* actual;
    put_node(&expect1);
    put_node(&expect2);
    put_node(&expect3);
    get_first_node(&actual);

    assert_two_linkedlist_eq(&expect1, actual);
    assert_two_linkedlist_eq(&expect2, actual->next);
    assert_two_linkedlist_eq(&expect3, actual->next->next);
}

static void unittests() {
    test_put_node_get_one();
    test_put_node_get_two();
    test_get_first_node_no_node();
    test_put_node_get_three();

    printf("All unittests successfully passed.\n");
}


#if 0
int main() {
	unittests();
}
#endif