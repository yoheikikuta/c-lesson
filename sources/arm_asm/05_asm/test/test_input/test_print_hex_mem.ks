.globl _start
_start:

  ldr r13,=0x08000000

  mov r0, r15 
  bl print_hex

  mov r0, #0x68
  bl print_hex

end:
  b end

putchar:
  stmdb r13!, {r1}
  ldr r1,=0x101f1000
  str r3, [r1]
  ldmia r13!, {r1}
  mov r15, r14

print_hex:
  stmdb r13!, {r2, r3, r4, r14}
  mov r2, #0x1C
_loop:
  mov r4, #0x30

  lsr r3, r0, r2
  and r3, r3, #0x0F

  cmp r3, #0x09
  ble _convert_to_hex_ascii
  add r4, r4, #0x27

_convert_to_hex_ascii:
  add r3, r3, r4

  bl putchar
  sub r2, r2, #0x4
  cmp r2, #0x0
  bge _loop

  ldmia r13!, {r2, r3, r4, r14}
  mov r15, r14