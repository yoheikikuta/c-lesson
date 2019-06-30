	.text
	.syntax unified
	.eabi_attribute	67, "2.09"	@ Tag_conformance
	.eabi_attribute	6, 1	@ Tag_CPU_arch
	.eabi_attribute	8, 1	@ Tag_ARM_ISA_use
	.eabi_attribute	34, 1	@ Tag_CPU_unaligned_access
	.eabi_attribute	17, 1	@ Tag_ABI_PCS_GOT_use
	.eabi_attribute	20, 1	@ Tag_ABI_FP_denormal
	.eabi_attribute	21, 1	@ Tag_ABI_FP_exceptions
	.eabi_attribute	23, 3	@ Tag_ABI_FP_number_model
	.eabi_attribute	24, 1	@ Tag_ABI_align_needed
	.eabi_attribute	25, 1	@ Tag_ABI_align_preserved
	.eabi_attribute	38, 1	@ Tag_ABI_FP_16bit_format
	.eabi_attribute	18, 4	@ Tag_ABI_PCS_wchar_t
	.eabi_attribute	14, 0	@ Tag_ABI_PCS_R9_use
	.file	"pointer_chceck.c"
	.globl	print_str               @ -- Begin function print_str
	.p2align	2
	.type	print_str,%function
	.code	32                      @ @print_str
print_str:
	.fnstart
@ %bb.0:
	push	{r11, lr}
	mov	r11, sp
	sub	sp, sp, #8
	str	r0, [sp]
	ldr	r1, [sp]
	ldr	r0, .LCPI0_0
	bl	printf
	ldr	r0, [sp, #4]
	mov	sp, r11
	pop	{r11, lr}
	mov	pc, lr
	.p2align	2
@ %bb.1:
.LCPI0_0:
	.long	.L.str
.Lfunc_end0:
	.size	print_str, .Lfunc_end0-print_str
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	get_str_for_ptr         @ -- Begin function get_str_for_ptr
	.p2align	2
	.type	get_str_for_ptr,%function
	.code	32                      @ @get_str_for_ptr
get_str_for_ptr:
	.fnstart
@ %bb.0:
	push	{r11, lr}
	mov	r11, sp
	sub	sp, sp, #8
	str	r0, [sp, #4]
	mov	r0, #4
	bl	malloc
	str	r0, [sp]
	ldr	r0, .LCPI1_0
	str	r0, [sp]
	ldr	r0, [sp]
	ldr	r1, [sp, #4]
	str	r0, [r1]
	mov	sp, r11
	pop	{r11, lr}
	mov	pc, lr
	.p2align	2
@ %bb.1:
.LCPI1_0:
	.long	.L.str.1
.Lfunc_end1:
	.size	get_str_for_ptr, .Lfunc_end1-get_str_for_ptr
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	get_str_for_ptrptr      @ -- Begin function get_str_for_ptrptr
	.p2align	2
	.type	get_str_for_ptrptr,%function
	.code	32                      @ @get_str_for_ptrptr
get_str_for_ptrptr:
	.fnstart
@ %bb.0:
	push	{r11, lr}
	mov	r11, sp
	sub	sp, sp, #8
	str	r0, [sp, #4]
	mov	r0, #4
	bl	malloc
	str	r0, [sp]
	ldr	r0, .LCPI2_0
	str	r0, [sp]
	mov	r0, sp
	str	r0, [sp, #4]
	mov	sp, r11
	pop	{r11, lr}
	mov	pc, lr
	.p2align	2
@ %bb.1:
.LCPI2_0:
	.long	.L.str.1
.Lfunc_end2:
	.size	get_str_for_ptrptr, .Lfunc_end2-get_str_for_ptrptr
	.cantunwind
	.fnend
                                        @ -- End function
	.globl	main                    @ -- Begin function main
	.p2align	2
	.type	main,%function
	.code	32                      @ @main
main:
	.fnstart
@ %bb.0:
	push	{r11, lr}
	mov	r11, sp
	sub	sp, sp, #16
	str	r0, [r11, #-4]
	str	r1, [sp, #8]
	add	r0, sp, #4
	bl	get_str_for_ptr
	ldr	r0, [sp, #4]
	bl	print_str
	ldr	r0, [sp]
	bl	get_str_for_ptrptr
	mov	r0, #0
	mov	sp, r11
	pop	{r11, lr}
	mov	pc, lr
.Lfunc_end3:
	.size	main, .Lfunc_end3-main
	.cantunwind
	.fnend
                                        @ -- End function
	.type	.L.str,%object          @ @.str
	.section	.rodata.str1.1,"aMS",%progbits,1
.L.str:
	.asciz	"%s"
	.size	.L.str, 3

	.type	.L.str.1,%object        @ @.str.1
.L.str.1:
	.asciz	"123"
	.size	.L.str.1, 4


	.ident	"clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)"
	.section	".note.GNU-stack","",%progbits
