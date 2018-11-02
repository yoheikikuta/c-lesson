#include <string.h>
#include "cl_util.h"
#include "stack.h"

int streq(char *s1, char *s2) {
    return strcmp(s1, s2) == 0;
}

int two_data_eq(struct Data *d1, struct Data *d2) {
    if (d1->dtype == d2->dtype) {
        if (d1->dtype == NUMBER) {
            return d1->u.number == d2->u.number;
        } else if (d1->dtype == LITERAL_NAME) {
            return streq(d1->u.name, d2->u.name);
        }
    } else {
        return 0;
    }
}

void assert_two_data_eq(struct Data *d1, struct Data *d2) {
    assert(two_data_eq(d1, d2));
}
