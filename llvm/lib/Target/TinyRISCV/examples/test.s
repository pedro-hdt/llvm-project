	.text
	.file	"test.c"
	.globl	main                    # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:
	addi	x2, x2, -32
	sw	x1, 28(x2)
	sw	x8, 24(x2)
	addi	x8, x2, 32
	sw	x0, -12(x8)
	addi	x10, x0, 7
	sw	x10, -16(x8)
	addi	x10, x0, 10
	sw	x10, -20(x8)
	lw	x10, -16(x8)
	lw	x11, -20(x8)
	call	__mulsi3
	lw	x8, 24(x2)
	lw	x1, 28(x2)
	addi	x2, x2, 32
	jalr	x0, 0(x1)
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.ident	"clang version 10.0.0-4ubuntu1 "
	.section	".note.GNU-stack","",@progbits
