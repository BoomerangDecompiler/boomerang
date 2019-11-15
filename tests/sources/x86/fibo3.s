# This version is compiled with gcc version 4.0.0, no optimisation
# It has been changed to assign to ecx and edx, to test removal of unused returns
	.file	"fibo.c"
	.text
.globl fib
	.type	fib, @function
fib:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$4, %esp
	cmpl	$1, 8(%ebp)
	jle	.L2
	movl	8(%ebp), %eax
	decl	%eax
	subl	$12, %esp
	pushl	%eax
	call	fib
	addl	$16, %esp
	push	%edx			# Align stack, and ...
	push	%edx			# ... make a use of %edx (but is dead code)
	push	%eax			# Save intermediate result
	movl	8(%ebp), %eax
	subl	$2, %eax
	pushl	%eax
	call	fib
	pop		%edx			# Remove parameter, assign to edx
	pop		%ecx			# Get intermediate result, assign to ecx
	addl	$8, %esp
	addl	%eax, %ecx		# Add the two intermediate results
	movl	%ecx, -8(%ebp)
	jmp	.L4
.L2:
	movl	8(%ebp), %eax
	movl	%eax, -8(%ebp)
.L4:
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
	pushl	$.LC0
	call	printf
	addl	$16, %esp
	subl	$8, %esp
	leal	-8(%ebp), %eax
	pushl	%eax
	pushl	$.LC1
	call	scanf
	addl	$16, %esp
	movl	-8(%ebp), %eax
	pushl	%eax
	call	fib
	addl	$4, %esp
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
