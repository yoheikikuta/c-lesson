#include <stdio.h>
// #define INVESTIGATE 1
#undef INVESTIGATE

void print_address(int address) {
    printf("address: %x\n", address);
}

void print_msg(char *str) {
    printf("We get (%s)\n", str);
}

void func3 () {
    char *target;
#ifdef INVESTIGATE
    print_address(*( (int*)(*((int*)(*((int*)((int)&target + 4))))) ) - 8);
    print_msg((char*)(*((int*)( *( (int*)(*((int*)(*((int*)((int)&target + 4))))) ) - 8 ) )));
#endif
    print_msg((char*)(*((int*)( *( (int*)(*((int*)(*((int*)((int)&target + 4))))) ) - 8 ) )));
    printf("We are in func3\n");
}

void func2() {
    char *msg = "func2 message.";
#ifdef INVESTIGATE
    print_address(*( (int*)(*((int*)((int)&msg + 4))) ) - 8);
    print_msg((char*)(*((int*)( *( (int*)(*((int*)((int)&msg + 4))) ) - 8 ) )));
#endif
    printf("We are in func2, %s\n", msg);
    func3();
}

void func1() {
    char *msg = "func1 msg";
#ifdef INVESTIGATE
    print_address(*( (int*)((int)&msg + 4) ) - 8);
    print_msg((char*)(*((int*)( *( (int*)((int)&msg + 4) ) - 8 ))));
#endif
    printf("We are in func1, %s\n", msg);
    func2();
}


int main() {
    char *main_msg = "We are in main.";
#ifdef INVESTIGATE
    print_address((int)&main_msg);
#endif
    printf("We are in main, %s\n", main_msg);
    func1();
    return 0;
}

