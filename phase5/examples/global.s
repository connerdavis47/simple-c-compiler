	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 10, 14	sdk_version 10, 14
	.globl	_foo                    ## -- Begin function foo
	.p2align	4, 0x90
_foo:                                   ## @foo
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq	_z@GOTPCREL(%rip), %rax
	movq	_y@GOTPCREL(%rip), %rcx
	movq	_x@GOTPCREL(%rip), %rdx
	movl	$1, (%rdx)
	movl	$2, (%rcx)
	movl	$3, (%rax)
	movl	-4(%rbp), %eax
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
	.comm	_x,4,2                  ## @x
	.comm	_y,4,2                  ## @y
	.comm	_z,4,2                  ## @z

.subsections_via_symbols
