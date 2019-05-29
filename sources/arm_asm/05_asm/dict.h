#pragma once
#include "assembler.h"

#define DICT_SIZE 256

struct KeyValue {
    int key;
    struct Word value;
};