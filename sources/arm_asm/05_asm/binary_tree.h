#pragma once

#define NO_EXISTING_NODE -1
#define MNEMONIC_ID_INIT 1
#define LABEL_ID_INIT 100000
// mnemonic ids.
#define _MOV 2
#define _STR 3
#define _LDR 4
#define _RAW 5
#define _B 6
#define _GLOBAL 7
#define _LDRB 8

struct Node {
    char* name;
    int value;
    struct Node* left;
    struct Node* right;
};

enum Side {LEFT, RIGHT};

/*
Return symbol int corresponding to a mnemonic.
Return symbol int corresponding to a label.
Set mnemonics symbols.
*/
int to_mnemonic_symbol(char* str);
int to_label_symbol(char* str);
void set_mnemonics();