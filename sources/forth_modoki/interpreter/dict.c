#include "stack.h"
#include <assert.h>

#define DICT_SIZE 1024

static int dict_pos = 0;

struct KeyValue {
    char *key;
    struct Data value;
};

static struct KeyValue dict_array[DICT_SIZE];

void dict_put(char* key, struct Data *elem);
int dict_get(char* key, struct Data *out_elem);
void dict_print_all();

int main () {
    return 0;
}
