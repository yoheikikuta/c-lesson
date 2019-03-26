#include <string.h>
#include "cl_util.h"
#include "stack.h"

int streq(char* s1, char* s2) {
    return strcmp(s1, s2) == 0;
}

int two_data_eq(Data_t* d1, Data_t* d2) {
    if (d1->dtype == d2->dtype) {
        if (d1->dtype == NUMBER) {
            return d1->u.number == d2->u.number;
        } else if (d1->dtype == LITERAL_NAME) {
            return streq(d1->u.name, d2->u.name);
        } else {
            fprintf(stderr, "Never reached here.\n");
            return 0;
        }
    } else {
        return 0;
    }
}

int two_exec_opelem_eq(Element_t* e1, Element_t* e2) {
    if (e1->etype == e2->etype) {
        if (e1->etype == ELEMENT_NUMBER) {
            return e1->u.number == e2->u.number;
        } else if (e1->etype == ELEMENT_LITERAL_NAME || e1->etype == ELEMENT_EXECUTABLE_NAME) {
            return streq(e1->u.name, e2->u.name);
        } else {
            fprintf(stderr, "Never reached here.\n");
            return 0;
        }
    } else {
        return 0;
    }
}

void assert_two_data_eq(Data_t* d1, Data_t* d2) {
    assert(two_data_eq(d1, d2));
}

void assert_two_exec_opelem_eq(Element_t* e1, Element_t* e2) {
    assert(two_exec_opelem_eq(e1, e2));
}