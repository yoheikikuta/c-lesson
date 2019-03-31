/*
TBD
*/
void def_op();
void add_op();
void sub_op();
void mul_op();
void div_op();
void register_one_primitive(char* op_name, void (*cfunc)(void));
void register_all_primitive();