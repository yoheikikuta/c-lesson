#pragma once

struct LinkedList {
	int emitter_pos;
	int label_id;
	int word;
	struct LinkedList* next;
};

/*
unsolved_label_address_list = NULL.
Create a node from the input and put it into unsolved_label_address_list.
*out_node = unsolved_label_address_list.
*/
void linkedlist_init();
void create_and_add_node(int emitter_pos, int symbol_label, int word);
void get_first_node(struct LinkedList** out_node);