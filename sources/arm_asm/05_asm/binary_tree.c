#include "binary_tree.h"
#include "cl_util.h"
#include <stdlib.h>

struct Node mnemonic_root = {"m-root", MNEMONIC_ID_INIT, NULL, NULL};
struct Node label_root = {"l-root", LABEL_ID_INIT, NULL, NULL};

int mnemonic_id = MNEMONIC_ID_INIT;
int label_id = LABEL_ID_INIT;


// Reset binary trees.
void reset_trees() {
    mnemonic_root.right = NULL;
    mnemonic_root.left = NULL;
    mnemonic_id = MNEMONIC_ID_INIT;

    label_root.right = NULL;
    label_root.left = NULL;
    label_id = LABEL_ID_INIT;
}

int return_no_existing_node() {
    return NO_EXISTING_NODE;
}

// Add a new node into the tree: ("mov", 2, out_node, LEFT) -> 
//   return 2,
//   out_node = {"root", 1, {"mov", 2, NULL, NULL}, NULL}
int add_new_node(char* str, int value, struct Node* out_node, enum Side side) {
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
        out_node->left = node_new;
    } else if (side == RIGHT) {
        out_node->right = node_new;
    }

	return node_new->value;
}

// Find or Add a node in the tree:
//   (find) func = return_no_existing_node -> return node's value (if exists) or NO_EXISTING_NODE.
//   (add) func = add_new_node -> retrun value and add a new node with str & value.
int find_or_add_node(char* str, int value, struct Node* node, NOTFOUND_CALLBACK_T on_not_found_callback) {
	int str_compared = strcmp(str, node->name);
    
	if(str_compared == 0) {
		return node->value;
	} else if(str_compared > 0){
		if(node->right != NULL) {
			return find_or_add_node(str, value, node->right, on_not_found_callback);
		} else {
            return on_not_found_callback(str, value, node, RIGHT);
		}
	} else if(str_compared < 0){
		if(node->left != NULL) {
			return find_or_add_node(str, value, node->left, on_not_found_callback);
		} else {
            return on_not_found_callback(str, value, node, LEFT);
		}
	}
}

int find_node(char* str, int value, struct Node* node) {
    return find_or_add_node(str, value, node, &return_no_existing_node);
}

int add_node(char* str, int value, struct Node* node) {
    return find_or_add_node(str, value, node, &add_new_node);
}

// Convert a mnemonic to a symbol: "mov" ->
//   (if mov exists in the tree) return the corresponding mnemonic id.
//   (if not exists) return the corresponding new mnemonic id and add the node.
int to_mnemonic_symbol(char* str) {
	int value = 0;

	value = find_node(str, mnemonic_id, &mnemonic_root);
	if(value == NO_EXISTING_NODE) {
		value = add_node(str, ++mnemonic_id, &mnemonic_root);
		return value;
	} else {
		return value;
	}
}

// Convert a label to a symbol: "label:" ->
//   (if "label" exists in the tree) return the corresponding label id.
//   (if not exists) return the corresponding new label id and add the node.
int to_label_symbol(char* str) {
	int value = 0;

	value = find_node(str, label_id, &label_root);
	if(value == NO_EXISTING_NODE) {
		value = add_node(str, ++label_id, &label_root);
		return value;
	} else {
		return value;
	}
}

void set_mnemonics() {
    // Need to match the order with that of macro definitions in binary_tree.h.
    to_mnemonic_symbol("mov");
    to_mnemonic_symbol("str");
    to_mnemonic_symbol("ldr");
    to_mnemonic_symbol(".raw");
    to_mnemonic_symbol("b");
    to_mnemonic_symbol(".globl");
    to_mnemonic_symbol("ldrb");
    to_mnemonic_symbol("add");
    to_mnemonic_symbol("cmp");
    to_mnemonic_symbol("bne");
    to_mnemonic_symbol("sub");
    to_mnemonic_symbol("lsr");
    to_mnemonic_symbol("and");
    to_mnemonic_symbol("ble");
    to_mnemonic_symbol("bgt");
    to_mnemonic_symbol("bl");
    to_mnemonic_symbol("stmdb");
    to_mnemonic_symbol("ldmia");
    to_mnemonic_symbol("bge");
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

static void unittests() {
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

    printf("All unittests successfully passed.\n");
}

#if 0
int main(int argc, char* argv[]) {
    unittests();
    return 0;
}
#endif