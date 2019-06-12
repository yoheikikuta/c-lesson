#pragma once

struct LinkedList {
	int emitter_pos;
	int label_id;
	int word;
	struct LinkedList* next;
};

void linkedlist_init();
void linkedlist_put(struct LinkedList* add_elem);
int linkedlist_get(struct LinkedList* out_list);