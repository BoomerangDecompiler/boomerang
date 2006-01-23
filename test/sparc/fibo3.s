# This version is compiled with gcc version 4.0.0, no optimisation
# Can't use %o4 and %o5 to save the intermediate result, since the save and restore instructions effectively save and
# restore them via the %i4 and %i5 registers (since they are not overwritten; a bit like callee save registers).
# So %i4 is used to save the intermediate result now (more like the pentium version of fibo3).
# %i5 is saved and restored along only one path (like %edx in the pentium version).
# Note that %g2 and up seem to be clobbered by the system, but they can still be used to test data flow.

	.file	"fibo.c"
	.section	".text"
	.align 4
	.global fib
	.type	fib, #function
	.proc	04
fib:
	!#PROLOGUE# 0
	save	%sp, -120, %sp
	!#PROLOGUE# 1
	st	%i0, [%fp+68]
	ld	[%fp+68], %g1
	cmp	%g1, 1
	ble	.LL2
	nop
	ld	[%fp+68], %g1
	add	%g1, -1, %g1
	mov	%g1, %o0
	st		%i5, [%fp+72]	! Save %i5 for no good reason
	call	fib, 0
	 nop
	mov	%o0, %i4			! Save intermediate result
	ld	[%fp+68], %g1
	add	%g1, -2, %o0
	call	fib, 0
	 nop
	ld		[%fp+72], %i5	! Restore %i5
	add	%i4, %o0, %l0		! Use intermediate result
	st	%l0, [%fp-20]
	b	.LL1
	 nop
.LL2:
	ld	[%fp+68], %g1
	st	%g1, [%fp-20]
.LL1:
	ld	[%fp-20], %i0
	ret
	restore
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
	add	%fp, -20, %o1
	sethi	%hi(.LLC1), %g1
	or	%g1, %lo(.LLC1), %o0
	call	scanf, 0
	nop
	ld	[%fp-20], %o0
	call	fib, 0
	 nop
	st	%o0, [%fp-24]
	sethi	%hi(.LLC2), %g1
	or	%g1, %lo(.LLC2), %o0
	ld	[%fp-20], %o1
	ld	[%fp-24], %o2
	call	printf, 0
	 nop
	mov	0, %i0
	ret
	restore
	.size	main, .-main
	.ident	"GCC: (GNU) 3.4.3 (csl-sol210-3_4-branch+sol_rpath)"
