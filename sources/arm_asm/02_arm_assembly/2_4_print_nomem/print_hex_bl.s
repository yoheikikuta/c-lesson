.globl _start

/*
r0  : Input of print_hex 
r1  : UART address
r2  : Bit rotation number used in lsr (0-32)
r3  : Each 4 bits of r0 to be printed
r4  : Offset to convert integers (0-15) to ascii characters (0-9 or a-f)
r14 : Link register used in bl
r15 : Program counter
*/

_start:

  ldr r1,=0x101f1000
  mov r2, #32

  mov r0, r15
  bl print_hex

  mov r0, #0x68
  bl print_hex

end:
  b end


print_hex:
  mov r4, #0x30  @ Set initial offset for number cases

  sub r2, r2, #4  @ Reset r2 to move to the next 4 bits
  lsr r3, r0, r2
  and r3, r3, #0x0F  @ r3: 0100 1101 -> 0000 1101

  cmp r3, #9
  ble convert_to_hex_ascii
  add r4, r4, #0x27  @ Add offset for alphabet cases

convert_to_hex_ascii:
  add r3, r3, r4
  b print_one_hex

print_one_hex:
  str r3, [r1]
  cmp r2, #0
  bgt print_hex

  @ Line breaks for the visibility
  mov r3, #0x0D
  str r3, [r1]
  mov r3, #0x0A
  str r3, [r1]

  @ Reset r2 and go to the next address
  mov r2, #32
  mov r15, r14