#pragma once
#include "parser.h"
#include "dict.h"
#include "unsolved_label_address_list.h"
#include "assembler.h"
#include <stdio.h>

enum FileOpenMode { FREAD, FREADBIN, FWRITE };

/*
Assert if num1 != num2.
Assert if str1 != str2.
Assert if str != substr.
Return 1 if Word1 == Word2, 0 else.
Return 1 if KevValue1 == KevValue2, 0 else.
Return 1 if LinkedList1 == LinkedList2, 0 else.
Assert if Word1 != Word2.
Assert if KeyValue1 != KeyValue2.
Assert if LinkedList1 != LinkedList2.
Return the file pointer of a given relative file path.
*/
void assert_two_num_eq(int num1, int num2);
void assert_two_str_eq(char* str1, char* str2);
void assert_str_substr_eq(char* str, struct Substring* substr);
int two_word_eq(struct Word* w1, struct Word* w2);
int two_keyvalue_eq(struct KeyValue* kv1, struct KeyValue* kv2);
int two_linkedlist_eq(struct LinkedList* list1, struct LinkedList* list2);
void assert_two_word_eq(struct Word* w1, struct Word* w2);
void assert_two_keyvalue_eq(struct KeyValue* kv1, struct KeyValue* kv2);
void assert_two_linkedlist_eq(struct LinkedList* list1, struct LinkedList* list2);
FILE* get_fp(char* rel_path, enum FileOpenMode mode);