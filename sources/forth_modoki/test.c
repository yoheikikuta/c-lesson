#include <stdio.h>
#include <stdlib.h>

enum ElementType {
    NO_ELEM_TYPE,
    ELEMENT_C_FUNC
};

struct Element {
   enum ElementType etype;
   union {
     int dummmy;
     struct {
       struct Element *byte_codes;
       int len;
     } exec_array;
   } u;
};

struct ElementArray {
   int len;
   struct Element elements[0];
};

int main(int argc, char* argv[]){
    struct ElementArray *arrs = (
        struct ElementArray*)malloc(sizeof(struct ElementArray)+sizeof(struct Element)*2
        +sizeof(struct ElementArray)+sizeof(struct Element)*3);
    arrs[0].len = 2;
    arrs[0].elements[0].etype = ELEMENT_C_FUNC;

    printf("%i\n", arrs[0].len);
    printf("%i\n", arrs[0].elements[0].etype);

    // でも[1]はアクセス出来ないから以下のコードは駄目
    arrs[2].len = 3;
    printf("%i\n", arrs[2].len);

    printf("%i\n", "abc"=="bac");
    // printf("%i\n", 1==2);
    exit(0);
}