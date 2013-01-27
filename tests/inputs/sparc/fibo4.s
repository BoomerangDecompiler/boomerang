# This version is compiled with gcc version 4.0.0, no optimisation
# It has been changed to use esi as the parameter to fib, while preserving esi with a push and pop.
# This is the SPARC version of a test program that fails dcc and REC
# The parameter is passed in %i4 in main, which is saved and restored in fib
# For added interest, this version returns the result in %o2
	.file	"fibo.c"
	.section	".text"
	.align 4
	.global fib
	.type	fib, #function
	.proc	04
fib:
	add		%sp,-96,%sp			! Make some space
	st		%i4, [%sp+80]		! Save %i4
	st		%o7, [%sp+84]		! Save return address
	cmp		%i4, 1				! i4 is preserved, yet is the parameter
	ble		.LL2
	nop
	call	fib, 0
	add		%i4, -1, %i4		! Arg for first call
	st		%o2, [%sp+88]		! Save intermediate result
	ld		[%sp+80], %i4		! Original param
	call	fib, 0
	add		%i4, -2, %i4		! Arg for second call
	ld		[%sp+88],%g1		! Intermediate result
	b		.LL1
	add		%o2, %g1, %o2		! Sum to %o2
.LL2:
	mov		%i4, %o2			! Copy param to result
.LL1:
	ld		[%sp+84],%o7		! Restore return address
	ld		[%sp+80], %i4		! Restore %i4
	retl
	add		%sp,96,%sp			! Restore the stack
	.size	fib, .-fib
	.section	".rodata"
	.align 8
.LLC0:
	.asciz	"Input number: "
	.align 8
.LLC1:
	.asciz	"%d"
	.align 8
.LLC2:
	.asciz	"fibonacci(%d) = %d\n"
	.section	".text"
	.align 4
	.global main
	.type	main, #function
	.proc	04
main:
	!#PROLOGUE# 0
	save	%sp, -120, %sp
	!#PROLOGUE# 1
	sethi	%hi(.LLC0), %g1
	or	%g1, %lo(.LLC0), %o0
	call	printf, 0
	 nop
	add	%fp, -20, %o5
	sethi	%hi(.LLC1), %g1
	or	%g1, %lo(.LLC1), %o0
	mov	%o5, %o1
	call	scanf, 0
	 nop

	call	fib, 0
	ld	[%fp-20], %i4

	st	%o2, [%fp-24]
	sethi	%hi(.LLC2), %g1
	or	%g1, %lo(.LLC2), %o0
	ld	[%fp-20], %o1
	ld	[%fp-24], %o2
	call	printf, 0
	 nop
	mov	0, %g1
	mov	%g1, %i0
	ret
	restore
	.size	main, .-main
	.ident	"GCC: (GNU) 3.4.3 (csl-sol210-3_4-branch+sol_rpath)"
