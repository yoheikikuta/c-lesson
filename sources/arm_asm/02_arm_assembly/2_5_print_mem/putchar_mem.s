.globl _start
_start:

  ldr r13,=0x08000000
  ldr r0,=msg1
  bl print
  ldr r0,=msg2
  bl print
end:
  b end

/*
  putchar:
    arg r3: 1 byte char.
    used internal register: 
      r1: UART.
*/
putchar:
  stmdb r13!, {r1}
  ldr r1,=0x101f1000
  str r3, [r1]
  ldmia r13!, {r1}
  mov r15, r14

/*
  print:
    arg r0: Address of target string, must end by \0.
    used internal register: 
      r3: 1 byte char.
*/
print:
  stmdb r13!, {r3, r14}
  ldrb r3, [r0]
_loop:
  bl putchar

  add r0, r0, #1
  ldrb r3, [r0]
  cmp r3, #0
  bne _loop
  ldmia r13!, {r3, r14}
  mov r15, r14

msg1: .asciz "First text.\n"
msg2: .asciz "Second text!\n"