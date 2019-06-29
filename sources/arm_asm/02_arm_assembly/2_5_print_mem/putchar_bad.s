.globl _start
_start:

  ldr r0,=msg1
  bl print
  ldr r0,=msg2
  bl print
end:
  b end

/*
  putchar:
    arg r0: target character which is printed.
    used internal register: r1.
*/
putchar:
  ldr r1,=0x101f1000
  str r0, [r1]
  mov r15, r14


/*
  print:
    arg r0: Address of target string, must end by \0.
    used internal register: r0, r1, r2, r4.
*/
print:
  mov r2, r0
  mov r4, r14
  ldrb r0, [r0]
_loop:
  bl putchar
  add r2, r2, #1
  ldrb r0, [r2]
  cmp r0, #0
  bne _loop
  mov r15, r4

msg1: .asciz "First text.\n"
msg2: .asciz "Second text!\n"