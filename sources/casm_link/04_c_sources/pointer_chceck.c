int print_str(char* v) {
    printf("%s", v);
}

void get_str_for_ptr(char** out_v) {
    char* v = malloc(sizeof(4));
    v = "123";
    *out_v = v;
}

void get_str_for_ptrptr(char** out_v) {
    char* v = malloc(sizeof(4));
    v = "123";
    out_v = &v;
}

int main(int argc, char* argv[]) {
    char* ptr;
    char** ptrptr;

    get_str_for_ptr(&ptr);
    print_str(ptr);

    get_str_for_ptrptr(ptrptr);
    // print_str(*ptrptr);  // Error occurs (as expected).
}