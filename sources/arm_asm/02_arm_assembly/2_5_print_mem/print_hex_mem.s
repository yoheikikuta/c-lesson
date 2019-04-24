.globl _start
_start:

  ldr r13,=0x08000000

  mov r0, r15 
  bl print_hex

  mov r0, #0x68
  bl print_hex

end:
  b end

/*
  putchar : Print one character.
    args :
      r3: 1 byte char.
    used internal registers : 
      r1: UART.
*/
putchar:
  stmdb r13!, {r1}
  ldr r1,=0x101f1000
  str r3, [r1]
  ldmia r13!, {r1}
  mov r15, r14

/*
  print_hex : Input 0xabcd1234 -> Print abcd1234.
    args : 
      r0 : Address of target string, must end by \0.
    used internal registers :
      r2 : Bit rotation number used in lsr (0-28).
      r3 : Each 4 bits of r0 to be printed.
      r4 : Offset to convert integers (0-15) to ascii characters (0-9 or a-f).
*/

print_hex:
  stmdb r13!, {r2, r3, r4, r14}
  mov r2, #28
_loop:
  mov r4, #0x30  @ Set initial offset for number cases

  lsr r3, r0, r2
  and r3, r3, #0x0F  @ r3: 0100 1101 -> 0000 1101

  cmp r3, #9
  ble _convert_to_hex_ascii
  add r4, r4, #0x27  @ Add offset for alphabet cases

  /*
    _convert_to_hex_ascii : 
      e.g.,
        9 + 0x30 = 0x39 ('9')
        13 + 0x30 + 0x27 = 0x64 ('d')
  */
_convert_to_hex_ascii:
  add r3, r3, r4

  bl putchar
  sub r2, r2, #4  @ Reset r2 to move to the next 4 bits
  cmp r2, #0
  bge _loop

  @ Add linebreak for the visibility.
  mov r3, #0x0a
  putchar

  @ Go to the next instruction.
  ldmia r13!, {r2, r3, r4, r14}
  mov r15, r14