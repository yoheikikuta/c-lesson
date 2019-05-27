#pragma once

#define NO_EXISTING_NODE -1
#define MNEMONIC_ID_INIT 1
#define LABEL_ID_INIT 100000

struct Node {
    char* name;
    int value;
    struct Node* left;
    struct Node* right;
};

enum Side {LEFT, RIGHT};

/*
Return symbol int corresponding to a mnemonic.
*/
int to_mnemonic_symbol(char* str);