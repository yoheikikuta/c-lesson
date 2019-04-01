/*
TBD
*/
void def_op();
void add_op();
void sub_op();
void mul_op();
void div_op();
void eq_op();
void neq_op();
void gt_op();
void ge_op();
void lt_op();
void le_op();
void pop_op();
void exch_op();
void dup_op();
void index_op();
void roll_op();
void register_one_primitive(char* op_name, void (*cfunc)(void));
void register_all_primitive();