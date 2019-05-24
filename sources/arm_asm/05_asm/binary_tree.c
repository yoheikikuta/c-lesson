#include <stdlib.h>
#include "cl_util.h"

#define NO_EXISTING_NODE -1
#define MNEMONIC_ID_INIT 1
#define LABEL_ID_INIT 100000

struct Node {
    char* name;
    int value;
    struct Node* left;
    struct Node* right;
};

struct Node mnemonic_root = {"m-root", MNEMONIC_ID_INIT, NULL, NULL};
struct Node label_root = {"l-root", LABEL_ID_INIT, NULL, NULL};

int mnemonic_id = MNEMONIC_ID_INIT;
int label_id = LABEL_ID_INIT;

enum Side {LEFT, RIGHT};


void reset_trees() {
    mnemonic_root.right = NULL;
    mnemonic_root.left = NULL;
    mnemonic_id = MNEMONIC_ID_INIT;

    label_root.right = NULL;
    label_root.left = NULL;
    label_id = LABEL_ID_INIT;
}

int return_no_existing_node(char* str, int value, struct Node* node, enum Side side) {
    return NO_EXISTING_NODE;
}

int add_new_node(char* str, int value, struct Node* node, enum Side side) {
    char* str_copied;
    int len_str = strlen(str) + 1;
    str_copied = malloc(len_str);
    strcpy(str_copied, str);

	struct Node* node_new = malloc(sizeof(struct Node));
	node_new->name = str_copied;
	node_new->value = value;
	node_new->left = NULL;
	node_new->right = NULL;

    if (side == LEFT) {
        node->left = node_new;
    } else if (side == RIGHT) {
        node->right = node_new;
    }

	return node_new->value;
}

int find_or_add_node(char* str, int value, struct Node* node, int* func) {
    int (*pfunc)(char* str, int value, struct Node* node, enum Side side) = func;
	int str_compared = strncmp(str, node->name, strlen(node->name));
    
	if(str_compared == 0) {
		return node->value;
	} else if(str_compared > 0){
		if(node->right != NULL) {
			return find_or_add_node(str, value, node->right, func);
		} else {
            return pfunc(str, value, node, RIGHT);
		}
	} else if(str_compared < 0){
		if(node->left != NULL) {
			return find_or_add_node(str, value, node->left, func);
		} else {
            return pfunc(str, value, node, LEFT);
		}
	}
}

int to_mnemonic_symbol(char* str) {
	int value = 0;

	value = find_or_add_node(str, mnemonic_id, &mnemonic_root, &return_no_existing_node);
	if(value == NO_EXISTING_NODE) {
		mnemonic_id++;
		value = find_or_add_node(str, mnemonic_id, &mnemonic_root, &add_new_node);
		return value;
	} else {
		return value;
	}
}

// 
// TEST
// 

static void test_find_or_add_node_no_node() {
	char* input = "mov";
    int input_value = 0;
    struct Node node = mnemonic_root;
    int expect = NO_EXISTING_NODE;
	
    int actual = find_or_add_node(input, input_value, &node, &return_no_existing_node);
	
	assert_two_num_eq(expect, actual);
}

static void test_find_or_add_node_single_node() {
	char* input = "m-root";
    int input_value = 0;
    struct Node node = mnemonic_root;
    int expect = 1;
	
    int actual = find_or_add_node(input, input_value, &node, &return_no_existing_node);
	
	assert_two_num_eq(expect, actual);
}

static void test_find_or_add_node_no_second_node() {
	char* input = "str";
    int input_value = 0;
    struct Node node = mnemonic_root;
    int expect = NO_EXISTING_NODE;
	
    int actual = find_or_add_node(input, input_value, &node, &return_no_existing_node);
	
	assert_two_num_eq(expect, actual);
}

static void test_find_or_add_node_second_node() {
	char* input = "str";
    int input_value = 0;
    struct Node node = mnemonic_root;
    struct Node node_second = {"str", 2, NULL, NULL};
    node.right = &node_second;
    int expect = 2;
	
    int actual = find_or_add_node(input, input_value, &node, &return_no_existing_node);
	
	assert_two_num_eq(expect, actual);
}

static void test_find_or_add_node_new_node() {
	char* input_str = "str";
    int input_value = 2;
    struct Node node = mnemonic_root;
    int expect = 2;
	
    int actual = find_or_add_node(input_str, input_value, &node, &add_new_node);
	
	assert_two_num_eq(expect, actual);
}

static void test_find_or_add_node_two_nodes() {
	char* input_str_1 = "str";
    int input_value_1 = 2;
	char* input_str_2 = "ldr";
    int input_value_2 = 3;
    struct Node node = mnemonic_root;
    int expect = 3;
	
    find_or_add_node(input_str_1, input_value_1, &node, &add_new_node);
    int actual = find_or_add_node(input_str_2, input_value_2, &node, &add_new_node);
	
	assert_two_num_eq(expect, actual);
}

static void test_to_mnemonic_symbol_root() {
	char* input_str = "m-root";
    int expect = 1;
	
    int actual = to_mnemonic_symbol(input_str);
	
	assert_two_num_eq(expect, actual);
    reset_trees();
}

static void test_to_mnemonic_symbol_add() {
	char* input_str = "str";
    int expect = 2;
	
    int actual = to_mnemonic_symbol(input_str);
	
	assert_two_num_eq(expect, actual);
    reset_trees();
}

static void test_to_mnemonic_symbol_add_and_find() {
	char* input_str = "str";
    int expect = 2;

	to_mnemonic_symbol(input_str);
    int actual = to_mnemonic_symbol(input_str);
	
	assert_two_num_eq(expect, actual);
    reset_trees();
}

static void test_to_mnemonic_symbol_find_intermediate() {
	char* input_str_1 = "str";
	char* input_str_2 = "ldr";
    int expect = 2;

	to_mnemonic_symbol(input_str_1);
	to_mnemonic_symbol(input_str_2);
    int actual = to_mnemonic_symbol(input_str_1);
	
	assert_two_num_eq(expect, actual);
    reset_trees();
}


int main(int argc, char* argv[]) {
    test_find_or_add_node_no_node();
    test_find_or_add_node_single_node();
    test_find_or_add_node_no_second_node();
    test_find_or_add_node_second_node();
    test_find_or_add_node_new_node();
    test_find_or_add_node_two_nodes();
    test_to_mnemonic_symbol_root();
    test_to_mnemonic_symbol_add();
    test_to_mnemonic_symbol_add_and_find();
    test_to_mnemonic_symbol_find_intermediate();
    return 0;
}