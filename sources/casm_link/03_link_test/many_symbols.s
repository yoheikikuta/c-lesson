	.arch armv5t
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"many_symbols.c"
	.text
	.comm	g_in_hello_uninit,4,4
	.global	g_in_hello
	.data
	.align	2
	.type	g_in_hello, %object
	.size	g_in_hello, 4
g_in_hello:
	.word	1
	.align	2
	.type	g_static_in_hello, %object
	.size	g_static_in_hello, 4
g_static_in_hello:
	.word	1
	.local	g_static_uninit
	.comm	g_static_uninit,4,4
	.global	g_text
	.section	.rodata
	.align	2
.LC0:
	.ascii	"abc\000"
	.data
	.align	2
	.type	g_text, %object
	.size	g_text, 4
g_text:
	.word	.LC0
	.comm	g_text_uninit,4,4
	.global	g_text_arr
	.align	2
	.type	g_text_arr, %object
	.size	g_text_arr, 4
g_text_arr:
	.ascii	"def\000"
	.comm	g_large_buf,4194304,4
	.text
	.align	2
	.global	print_something
	.syntax unified
	.arm
	.fpu softvfp
	.type	print_something, %function
print_something:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	str	r0, [fp, #-8]
	ldr	r0, [fp, #-8]
	bl	func_in_main
	ldr	r3, .L3
	ldr	r3, [r3]
	mov	r0, r3
	bl	func_in_main
	ldr	r3, .L3+4
	ldr	r3, [r3]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, pc}
.L4:
	.align	2
.L3:
	.word	g_text_uninit
	.word	g_in_main
	.size	print_something, .-print_something
	.ident	"GCC: (Ubuntu/Linaro 7.4.0-1ubuntu1~18.04.1) 7.4.0"
	.section	.note.GNU-stack,"",%progbits
