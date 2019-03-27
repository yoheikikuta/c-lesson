#pragma once
#include <assert.h>
#include <stdio.h>
#include "stack.h"

/*
return 1 if two strings are identical.
return 1 if two Datas are identical.
assert if two Datas are not identical.
*/
int streq(char* s1, char* s2);
int two_data_eq(struct Element* d1, struct Element* d2);
void assert_two_data_eq(struct Element* d1, struct Element* d2);
