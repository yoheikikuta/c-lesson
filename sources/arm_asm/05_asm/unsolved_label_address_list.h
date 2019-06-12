#pragma once

struct LinkedList {
	int emitter_pos;
	int label_id;
	int word;
	struct LinkedList* next;
};

/*
Initialize unsolved_label_address_list.
Put add_elem into unsolved_label_address_list.
Create an elem from the input and put the elem into unsolved_label_address_list.
Return 1 and get a list of unsolved_label_address_list, 0 if no list.
*/
void linkedlist_init();
void linkedlist_put(struct LinkedList* add_elem);
void common_unsolved_label_address_list_put(int emitter_pos, int symbol_label, int word);
int linkedlist_get(struct LinkedList* out_list);