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


int find_node(char* str, struct Node* node) {
	int str_compared = strncmp(str, node->name, strlen(node->name));

	if(str_compared == 0) {
		return node->value;
	} else if(str_compared > 0) {
		if(node->right != NULL) {
			return find_node(str, node->right);
		} else {
			return NO_EXISTING_NODE;
		}
	} else if(str_compared < 0){
		if(node->left != NULL) {
			return find_node(str, node->left);
		} else {
			return NO_EXISTING_NODE;
		}
	}
}

int add_node(char* str, int value, struct Node* node) {
	int str_compared = strncmp(str, node->name, strlen(node->name));
    
	if(str_compared == 0) {
		return node->value;
	} else if(str_compared > 0){
		if(node->right != NULL) {
			return add_node(str, node->right, value);
		} else {
			char str_copied[strlen(str) + 1];
            strcpy(&str_copied, str);
			
			struct Node* node_right = malloc(sizeof(struct Node));
			node_right->name = &str_copied;
			node_right->value = value;
			node_right->left = NULL;
			node_right->right = NULL;
			node->right = node_right;

			return node_right->value;
		}
	} else if(str_compared < 0){
		if(node->left != NULL) {
			return add_node(str, node->left, value);
		} else {
			char str_copied[strlen(str) + 1];
            strcpy(&str_copied, str);

			struct Node *node_left = malloc(sizeof(struct Node));
			node_left->name = &str_copied;
			node_left->value = value;
			node_left->left = NULL;
			node_left->right = NULL;
			node->left = node_left;

			return node_left->value;
		}
	}
}

int to_mnemonic_symbol(char* str) {
	int value = 0;

	value = find_node(str, &mnemonic_root);
	if(value == NO_EXISTING_NODE) {
		mnemonic_id++;
		value = add_node(str, &mnemonic_root, mnemonic_id);
		return value;
	} else {
		return value;
	}
}

// 
// TEST
// 

static void test_find_node_no_node() {
	char* input = "mov";
    struct Node node = mnemonic_root;
    int expect = NO_EXISTING_NODE;
	
    int actual = find_node(input, &node);
	
	assert_two_num_eq(expect, actual);
}

static void test_find_node_single_node() {
	char* input = "m-root";
    struct Node node = mnemonic_root;
    int expect = 1;
	
    int actual = find_node(input, &node);
	
	assert_two_num_eq(expect, actual);
}

static void test_find_node_no_second_node() {
	char* input = "str";
    struct Node node = mnemonic_root;
    int expect = NO_EXISTING_NODE;
	
    int actual = find_node(input, &node);
	
	assert_two_num_eq(expect, actual);
}

static void test_find_node_second_node() {
	char* input = "str";
    struct Node node = mnemonic_root;
    struct Node node_second = {"str", 2, NULL, NULL};
    node.right = &node_second;
    int expect = 2;
	
    int actual = find_node(input, &node);
	
	assert_two_num_eq(expect, actual);
}

static void test_add_node_new_node() {
	char* input_str = "str";
    int input_value = 2;
    struct Node node = mnemonic_root;
    int expect = 2;
	
    int actual = add_node(input_str, input_value, &node);
	
	assert_two_num_eq(expect, actual);
}


int main(int argc, char* argv[]) {
    test_find_node_no_node();
    test_find_node_single_node();
    test_find_node_no_second_node();
    test_find_node_second_node();
    test_add_node_new_node();
    return 0;
}