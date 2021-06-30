	.text
	.file	"test.c"
	.globl	test                    # -- Begin function test
	.p2align	2
	.type	test,@function
test:                                   # @test
# %bb.0:
	addi	x2, x2, -16
	sw	x1, 12(x2)
	sw	x8, 8(x2)
	addi	x8, x2, 16
	addi	x10, x0, 5
	lw	x8, 8(x2)
	lw	x1, 12(x2)
	addi	x2, x2, 16
	jalr	x0, 0(x1)
.Lfunc_end0:
	.size	test, .Lfunc_end0-test
                                        # -- End function
	.globl	main                    # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:
	addi	x2, x2, -32
	sw	x1, 28(x2)
	sw	x8, 24(x2)
	sw	x9, 20(x2)
	addi	x8, x2, 32
	sw	x0, -16(x8)
	sw	x10, -20(x8)
	sw	x11, -24(x8)
	addi	x10, x0, 7
	sw	x10, -28(x8)
	lw	x10, -28(x8)
	lw	x11, -20(x8)
	mul	x10, x10, x11
	sw	x10, -32(x8)
	lw	x10, -32(x8)
	addi	x10, x10, 1
	sw	x10, -32(x8)
	lw	x10, -20(x8)
	addi	x11, x0, 3
	blt	x10, x11, .LBB1_2
	jal	x0, .LBB1_1
.LBB1_1:
	lw	x9, -32(x8)
	call	test
	add	x10, x9, x10
	sw	x10, -16(x8)
	jal	x0, .LBB1_3
.LBB1_2:
	addi	x10, x0, 8
	sw	x10, -16(x8)
	jal	x0, .LBB1_3
.LBB1_3:
	lw	x10, -16(x8)
	lw	x9, 20(x2)
	lw	x8, 24(x2)
	lw	x1, 28(x2)
	addi	x2, x2, 32
	jalr	x0, 0(x1)
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
                                        # -- End function
	.ident	"clang version 10.0.0-4ubuntu1 "
	.section	".note.GNU-stack","",@progbits
