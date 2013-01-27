	.file	"restoredparam.s"
	.section	.rodata
.LC0:
	.string	"Result is %d\n"

	.text
.globl main
	.type	main,@function
main:
	mov		4(%esp),%ebx		# argc to %ebx (callee save)
	call	twice
	push	%eax				# push result
	pushl	$.LC0				# push format string
	call	printf
	add		$8, %esp			# remove 2 words from stack
	xorl	%eax, %eax
	ret

.Lfe1:
	.size	main,.Lfe1-main

# parameter is passed in register %ebx
twice:
	push	%ebx				# save ebx
	mov		%ebx, %eax			# copy parameter to return retisger, %eax
	addl	%eax, %eax			# double eax
	mov		$55, %ebx			# usually preserved registers are overwritten
	pop		%ebx				# restore ebx
	ret

