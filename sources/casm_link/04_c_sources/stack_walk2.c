#include <stdio.h>
// #define INVESTIGATE 1
#undef INVESTIGATE

void print_address(int address) {
    printf("address: %x\n", address);
}

void print_msg(char *str) {
    printf("We get (%s)\n", str);
}

int get_target_address_in_N_nested_call(int var_address, int N) {
    int target_address = *((int*)(var_address + 4));
    for (int i = 1; i < N; i++) {
        target_address = *((int*)target_address);
    }
    return target_address - 8;
}

void func3 () {
    char *target;
#ifdef INVESTIGATE
    print_address(*( (int*)(*((int*)(*((int*)((int)&target + 4))))) ) - 8);
    print_msg((char*)(*((int*)( *( (int*)(*((int*)(*((int*)((int)&target + 4))))) ) - 8 ) )));
    // print_address(get_target_address_in_N_nested_call((int)&target, 3));
    // print_msg((char*)(*((int*)( get_target_address_in_N_nested_call((int)&msg, 3) ))));
#endif
    print_msg((char*)(*((int*)( get_target_address_in_N_nested_call((int)&target, 3) ))));
    printf("We are in func3\n");
}

void func2() {
    char *msg = "func2 message.";
#ifdef INVESTIGATE
    print_address(*( (int*)(*((int*)((int)&msg + 4))) ) - 8);
    print_msg((char*)(*((int*)( *( (int*)(*((int*)((int)&msg + 4))) ) - 8 ) )));
    // print_address(get_target_address_in_N_nested_call((int)&msg, 2));
    // print_msg((char*)(*((int*)( get_target_address_in_N_nested_call((int)&msg, 2) ))));
#endif
    printf("We are in func2, %s\n", msg);
    func3();
}

void func1() {
    char *msg = "func1 msg";
#ifdef INVESTIGATE
    print_address(*( (int*)((int)&msg + 4) ) - 8);
    print_msg((char*)(*((int*)( *( (int*)((int)&msg + 4) ) - 8 ))));
    // print_address(get_target_address_in_N_nested_call((int)&msg, 1));
    // print_msg((char*)(*((int*)( get_target_address_in_N_nested_call((int)&msg, 1) ))));
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

