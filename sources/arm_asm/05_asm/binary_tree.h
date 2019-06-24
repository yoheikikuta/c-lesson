#pragma once

#define NO_EXISTING_NODE -1
#define MNEMONIC_ID_INIT 1
#define LABEL_ID_INIT 100000
// mnemonic ids.
#define MOV 2
#define STR 3
#define LDR 4
#define RAW 5
#define B 6
#define GLOBAL 7
#define LDRB 8
#define ADD 9
#define CMP 10
#define BNE 11
#define SUB 12
#define LSR 13
#define AND 14
#define BLE 15
#define BGT 16
#define BL 17
#define STMDB 18
#define LDMIA 19
#define BGE 20

struct Node {
    char* name;
    int value;
    struct Node* left;
    struct Node* right;
};

enum Side {LEFT, RIGHT};

typedef int (*NOTFOUND_CALLBACK_T)(char* str, int value, struct Node* node, enum Side side);

/*
Return symbol int corresponding to a mnemonic.
Return symbol int corresponding to a label.
Set mnemonics symbols.
*/
int to_mnemonic_symbol(char* str);
int to_label_symbol(char* str);
void set_mnemonics();