mov r1, r2
mov r15,r10
mov r1, #0xFF
ldr r1, [r15, #0x30]
str r12, [r1]
.raw 0x101f1000