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
input: r_num = 2, 2,3
return: 0xE8BD000C (ldmia r13!, {r2, r3})
 */
int asm_ldmia(int r_num, ...);
/*
input: r_dest = 2, r_1st_operand = 3, r_2nd_operand = 4
return: 0xE0832004 (add r2, r3, r4)
 */
int asm_add(int r_dest, int r_1st_operand, int r_2nd_operand);