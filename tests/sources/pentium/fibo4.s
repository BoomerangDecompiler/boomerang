# This version is compiled with gcc version 4.0.0, no optimisation
# It has been changed to use esi as the parameter to fib, while preserving esi with a push and pop.
# This is the i386 version of a test program that fails dcc and REC
	.file	"fibo.c"
	.text
.globl fib
	.type	fib, @function
fib:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%esi			# esi is the parameter, but also a preserved
	cmpl	$1, %esi
	jle	.L2
	decl	%esi
	call	fib
	pop		%esi
	pushl	%esi
	subl	$2, %esi
	pushl	%eax			# Save intermediate value
	call	fib
	pop		%ecx			# Get intermediate result, assign to ecx
	addl	%eax, %ecx		# Add the two intermediate results
	movl	%ecx, -8(%ebp)
	jmp	.L4
.L2:
	movl	%esi, -8(%ebp)
.L4:
	pop		%esi			# Restore esi
	movl	-8(%ebp), %eax
	movl	-4(%ebp), %ebx
	leave
	ret
	.size	fib, .-fib
	.section	.rodata
.LC0:
	.string	"Input number: "
.LC1:
	.string	"%d"
.LC2:
	.string	"fibonacci(%d) = %d\n"
	.text
.globl main
	.type	main, @function
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	andl	$-16, %esp
	movl	$0, %eax
	addl	$15, %eax
	addl	$15, %eax
	shrl	$4, %eax
	sall	$4, %eax
	subl	%eax, %esp
	subl	$12, %esp
	pushl	$.LC0		# Input number:
	call	printf
	addl	$16, %esp
	subl	$8, %esp
	leal	-8(%ebp), %eax
	pushl	%eax
	pushl	$.LC1		# %d
	call	scanf
	addl	$16, %esp
	movl	-8(%ebp), %esi
	call	fib
	movl	%eax, -4(%ebp)
	movl	-8(%ebp), %eax
	subl	$4, %esp
	pushl	-4(%ebp)
	pushl	%eax
	pushl	$.LC2
	call	printf
	addl	$16, %esp
	movl	$0, %eax
	leave
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 4.0.0 20050519 (Red Hat 4.0.0-8)"
	.section	.note.GNU-stack,"",@progbits
