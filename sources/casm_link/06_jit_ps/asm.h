/*
input: r_dest = 2, immediate_v = 3
return: 0xE3A02003 (mov r2, #3)
 */
int asm_mov_immediate_v(int r_dest, int immediate_v);
/*
input: r_dest = 2, r_operand = 3
return: 0xE1A02003 (mov r2, r3)
 */
int asm_mov_register(int r_dest, int r_operand);
/*
input: r = 2
return: 0xE92D0004 (stmdb r13!, r2)
 */
int asm_stmdb(int r);
/*
input: r = 2
return: 0xE8BD0004 (ldmia r13!, r2)
 */
int asm_ldmia(int r);