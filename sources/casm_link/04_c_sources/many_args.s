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
	.file	"many_args.c"
	.globl	many_sum                @ -- Begin function many_sum
	.p2align	2
	.type	many_sum,%function
	.code	32                      @ @many_sum
many_sum:
	.fnstart
@ %bb.0:
	sub	sp, sp, #16
	ldr	r12, [sp, #48]
	ldr	r12, [sp, #44]
	ldr	r12, [sp, #40]
	ldr	r12, [sp, #36]
	ldr	r12, [sp, #32]
	ldr	r12, [sp, #28]
	ldr	r12, [sp, #24]
	ldr	r12, [sp, #20]
	ldr	r12, [sp, #16]
	str	r0, [sp, #12]
	str	r1, [sp, #8]
	str	r2, [sp, #4]
	str	r3, [sp]
	ldr	r0, [sp, #12]
	ldr	r1, [sp, #8]
	add	r0, r0, r1
	ldr	r1, [sp, #4]
	add	r0, r0, r1
	ldr	r1, [sp]
	add	r0, r0, r1
	ldr	r1, [sp, #16]
	add	r0, r0, r1
	ldr	r1, [sp, #20]
	add	r0, r0, r1
	ldr	r1, [sp, #24]
	add	r0, r0, r1
	ldr	r1, [sp, #28]
	add	r0, r0, r1
	ldr	r1, [sp, #32]
	add	r0, r0, r1
	ldr	r1, [sp, #36]
	add	r0, r0, r1
	ldr	r1, [sp, #40]
	add	r0, r0, r1
	ldr	r1, [sp, #44]
	add	r0, r0, r1
	ldr	r1, [sp, #48]
	add	r0, r0, r1
	lsl	r0, r0, #1
	add	sp, sp, #16
	mov	pc, lr
.Lfunc_end0:
	.size	many_sum, .Lfunc_end0-many_sum
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
	sub	sp, sp, #48
	mov	r0, #0
	str	r0, [r11, #-4]
	mov	r0, #1
	str	r0, [sp, #32]
	mov	r0, #2
	str	r0, [sp, #28]
	mov	r0, #3
	str	r0, [sp, #24]
	mov	r0, #4
	str	r0, [sp, #20]
	mov	r0, #5
	str	r0, [sp, #16]
	mov	r0, #6
	str	r0, [sp, #12]
	mov	r0, #7
	str	r0, [sp, #8]
	mov	r0, #8
	str	r0, [sp, #4]
	mov	r0, #9
	str	r0, [sp]
	mov	r0, #13
	mov	r1, #12
	mov	r2, #11
	mov	r3, #10
	bl	many_sum
	str	r0, [r11, #-8]
	ldr	r1, [r11, #-8]
	ldr	r0, .LCPI1_0
	bl	printf
	mov	r0, #0
	mov	sp, r11
	pop	{r11, lr}
	mov	pc, lr
	.p2align	2
@ %bb.1:
.LCPI1_0:
	.long	.L.str
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cantunwind
	.fnend
                                        @ -- End function
	.type	.L.str,%object          @ @.str
	.section	.rodata.str1.1,"aMS",%progbits,1
.L.str:
	.asciz	"result is %d\n"
	.size	.L.str, 14


	.ident	"clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)"
	.section	".note.GNU-stack","",%progbits
