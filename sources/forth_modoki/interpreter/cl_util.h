#pragma once
#include <assert.h>
#include <stdio.h>
#include "stack.h"

/*
return 1 if two strings are identical.
return 1 if two Elements are identical.
assert if two Elemenst are not identical.
  The above two functions are alomost the same as the following two function.
  We also keep the above two functions though, in principle, they can be omitted.
return 1 if two Elements are identical.
assert if two Elements are not identical.
*/
int streq(char* s1, char* s2);
int two_data_eq(struct Element* d1, struct Element* d2);
void assert_two_data_eq(struct Element* d1, struct Element* d2);
int two_exec_opelem_eq(struct Element* d1, struct Element* d2);
void assert_two_exec_opelem_eq(struct Element* d1, struct Element* d2);
