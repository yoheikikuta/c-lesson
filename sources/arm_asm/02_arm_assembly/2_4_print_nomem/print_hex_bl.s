.globl _start

/*
r0  : Input of print_hex 
r1  : UART address
r2  : Bit rotation number used in lsr (0-32)
r3  : Each 4 bits of r0 to be printed
r14 : Link register used in bl
r15 : Program counter
*/

_start:

  ldr r1,=0x101f1000

  mov r2, #32
  mov r0, r15
  bl print_hex

  mov r2, #32
  mov r0, #0x68
  bl print_hex

end:
  b end


print_hex:
  sub r2, r2, #4  @ Reset r2 to move to the next 4 bits
  lsr r3, r0, r2
  and r3, r3, #0x0F
  cmp r3, #9
  ble print_one_hex
  add r3, r3, #0x27  @ For alphabet case

print_one_hex:
  add r3, r3, #0x30
  str r3, [r1]
  cmp r2, #0
  bgt print_hex

  @ Line breaks for the visibility
  mov r3, #0x0D
  str r3, [r1]
  mov r3, #0x0A
  str r3, [r1]

  @ Go to the next address
  mov r15, r14