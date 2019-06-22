.globl _start
_start:
    ldr r0,=0x101f1000
    ldr r1,=0xdeadbeaf
    mov r2,#0x20
    b print_hex

print_hex:
    sub r2,r2,#0x4
    lsr r3,r1,r2
    and r3,r3,#0x0F
    cmp r3,#0x9
    ble print_one_hex
    add r3,r3,#0x27

print_one_hex:
    add r3,r3,#0x30
    str r3,[r0]
    cmp r2,#0x0
    bgt print_hex

    mov r3,#0x0D
    str r3,[r0]
    mov r3,#0x0A
    str r3,[r0]

end:
    b end