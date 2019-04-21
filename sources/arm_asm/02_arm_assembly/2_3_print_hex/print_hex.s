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
    mov r2,#32  @ Initial value of bit rotation
    b print_hex

/*
The offset to convert integer 0-15 to ascii code 0-9 or a-f:
    number (0-9) case : 0x30
    alphabet (10-15) case : 0x57 
E.g.,
    9 + 0x30 -> 0x39 : 9
    13 + 0x57 -> 0x64 : d
*/

print_hex:
    sub r2,r2,#4  @ Reset r2 to move to the next 4 bits
    lsr r3,r1,r2
    and r3,r3,#0x0F
    cmp r3,#9
    ble print_one_hex
    add r3,r3,#0x27  @ For alphabet case

print_one_hex:
    add r3,r3,#0x30
    str r3,[r0]
    cmp r2,#0
    bgt print_hex

    @Line breaks for the visibility
    mov r3,#0x0D
    str r3,[r0]
    mov r3,#0x0A
    str r3,[r0]

end:
    b end