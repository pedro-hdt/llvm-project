	.text
	.file	"test.c"
	.globl	test                    # -- Begin function test
	.p2align	1
	.type	test,@function
test:                                   # @test
# %bb.0:
	addi	sp, sp, -16
	sw	ra, 12(sp)
	sw	s0, 8(sp)
	addi	s0, sp, 16
	addi	a0, zero, 5
	lw	s0, 8(sp)
	lw	ra, 12(sp)
	addi	sp, sp, 16
	ret
.Lfunc_end0:
	.size	test, .Lfunc_end0-test
                                        # -- End function
	.globl	main                    # -- Begin function main
	.p2align	1
	.type	main,@function
main:                                   # @main
# %bb.0:
	addi	sp, sp, -32
	sw	ra, 28(sp)
	sw	s0, 24(sp)
	sw	s1, 20(sp)
	addi	s0, sp, 32
	sw	zero, -16(s0)
	sw	a0, -20(s0)
	sw	a1, -24(s0)
	addi	a0, zero, 7
	sw	a0, -28(s0)
	lw	a0, -28(s0)
	lw	a1, -20(s0)
	mul	a0, a0, a1
	sw	a0, -32(s0)
	lw	a0, -32(s0)
	addi	a0, a0, 1
	sw	a0, -32(s0)
	lw	s1, -32(s0)
	call	test
	add	a0, s1, a0
	lw	s1, 20(sp)
	lw	s0, 24(sp)
	lw	ra, 28(sp)
	addi	sp, sp, 32
	ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
                                        # -- End function
	.ident	"clang version 10.0.0-4ubuntu1 "
	.section	".note.GNU-stack","",@progbits
