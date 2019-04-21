/*
 arm-none-eabi-as print_hex.s -o print_hex.o
 arm-none-eabi-ld print_hex.o -Ttext 0x00010000 -o print_hex.elf
 arm-none-eabi-objcopy print_hex.elf -O binary print_hex.bin
 qemu-system-arm -M versatilepb -m 128M -nographic -kernel print_hex.bin -serial mon:stdio
*/
.globl _start
_start:
    ldr r0,=0x101f1000
    ldr r1,=0xdeadbeaf
    mov r2,#28  @Initial value of bit rotation
    b print_hex
print_hex:
    lsr r3,r1,r2
    and r3,r3,#0x0F
    add r3,r3,#0x37 @e.g., 13 + 0x37 -> 0x44 : D
    str r3,[r0]
    lsr r3,r1,#24
    and r3,r3,#0x0F
    add r3,r3,#0x37
    str r3,[r0]
    lsr r3,r1,#20
    and r3,r3,#0x0F
    add r3,r3,#0x37
    str r3,[r0]
    lsr r3,r1,#16
    and r3,r3,#0x0F
    add r3,r3,#0x37
    str r3,[r0]
    lsr r3,r1,#12
    and r3,r3,#0x0F
    add r3,r3,#0x37
    str r3,[r0]
    lsr r3,r1,#8
    and r3,r3,#0x0F
    add r3,r3,#0x37
    str r3,[r0]
    lsr r3,r1,#4
    and r3,r3,#0x0F
    add r3,r3,#0x37
    str r3,[r0]
    lsr r3,r1,#0
    and r3,r3,#0x0F
    add r3,r3,#0x37
    str r3,[r0]
    mov r3,#0x0D
    str r3,[r0]
    mov r3,#0x0A
    str r3,[r0]
end:
    b end
