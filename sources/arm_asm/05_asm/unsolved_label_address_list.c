#include "cl_util.h"
#include "unsolved_label_address_list.h"
#include "assembler.h"
#include <stdio.h>
#include <assert.h>

static struct LinkedList* unsolved_label_address_list;
int cur_list_pos = 0;

// Initialize unsolved_label_address_list.
void unsolved_label_address_list_init() {
    unsolved_label_address_list = malloc(sizeof(struct LinkedList));
    unsolved_label_address_list->emitter_pos = 0;
    unsolved_label_address_list->label_id = 0;
    unsolved_label_address_list->word = 0;
    unsolved_label_address_list->next = NULL;
    cur_list_pos = 0;
}

// Put element into unsolved_label_address_list.
void linkedlist_put(struct LinkedList* add_elem) {
    struct LinkedList* list = unsolved_label_address_list;
    while (list->next != NULL) {list = list->next;}

    if (list->word == 0) {
        // Initial case.
        list->emitter_pos = add_elem->emitter_pos;
        list->label_id = add_elem->label_id;
        list->word = add_elem->word;
    } else {
        struct LinkedList* last_elem;
        last_elem = malloc(sizeof(struct LinkedList));
        last_elem = add_elem;
        last_elem->next = NULL;
        list->next = last_elem;
    }
}

// Create an elem from the input and put the elem into unsolved_label_address_list.
void common_unsolved_label_address_list_put(int emitter_pos, int symbol_label, int word) {
    struct LinkedList* list;
    list = malloc(sizeof(struct LinkedList));

    list->emitter_pos = emitter_pos;
    list->label_id = symbol_label;
    list->word = word;

    linkedlist_put(list);
}

// unsolved_label_address_list = list0->list1->list2->list3, ...
// cur_list_pos = 2
//   -> Return 1, out_list = list2
int linkedlist_get(struct LinkedList* out_list) {
    struct LinkedList* list = unsolved_label_address_list;
    int pos = 0;

    // Retrun 0 if no elem in unsolved_label_address_list.
    if (list->word == 0) {return 0;}

    while (pos < cur_list_pos) {
        if(list->next == NULL) return 0;
        list = list->next;
        pos++;
    }
    cur_list_pos++;
    
    *out_list = *list;
    return 1;
}

// 
// TEST
// 

static void test_linkedlist_put_get_one() {
    linkedlist_init();
    struct LinkedList expect = {.emitter_pos = 1, .label_id = 10, .word = 0xEA000000, .next = NULL};

    struct LinkedList actual;
    linkedlist_put(&expect);
    linkedlist_get(&actual);

    assert_two_linkedlist_eq(&expect, &actual);
}

static void test_linkedlist_put_get_two() {
    linkedlist_init();
    struct LinkedList expect1 = {.emitter_pos = 1, .label_id = 10, .word = 0xEA000000, .next = NULL};
    struct LinkedList expect2 = {.emitter_pos = 5, .label_id = 15, .word = 0xEB000000, .next = NULL};

    struct LinkedList actual1;
    struct LinkedList actual2;
    linkedlist_put(&expect1);
    linkedlist_put(&expect2);
    linkedlist_get(&actual1);
    linkedlist_get(&actual2);

    assert_two_linkedlist_eq(&expect1, &actual1);
    assert_two_linkedlist_eq(&expect2, &actual2);
}

static void test_linkedlist_put_two_get_three() {
    linkedlist_init();
    int expect = 0;

    struct LinkedList input = {.emitter_pos = 1, .label_id = 10, .word = 0xEA000000, .next = NULL};
    struct LinkedList actual_list;
    linkedlist_put(&input);
    linkedlist_put(&input);
    linkedlist_get(&actual_list);
    linkedlist_get(&actual_list);
    int actual = linkedlist_get(&actual_list);
    
    assert(expect == actual);
}

static void unittests() {
    test_linkedlist_put_get_one();
    test_linkedlist_put_get_two();
    test_linkedlist_put_two_get_three();

    printf("All unittests successfully passed.\n");
}


#if 0
int main() {
	unittests();
}
#endif