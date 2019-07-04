// Retrun 1 and print disassembled result if disassemble is succeeded, 0 else.
int try_print_asm(int word);
// 0x101F1000 -> print 00 10 1F 10\n (little endian) 
void print_asm_hex_dump(int word);