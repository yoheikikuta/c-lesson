.globl _start
_start:
    ldr r0,=0x101f1000
    ldr r1,=message
end:
    b end

message:
    .raw  "hello, world\n"