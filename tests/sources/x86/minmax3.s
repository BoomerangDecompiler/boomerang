	.file	"minmax2.c"
	.section	.rodata
.LC0:
	.string	"MinMax result %d\n"
	.text
.globl test
	.type	test,@function
test:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp

    /* Hand translated from the SPARC minmax2 code */
    /* %o5 -> eax  %g2 -> ebx %g4 -> ecx %g1 -> edx */
    movl    8(%ebp), %eax   /* %o0 */
    sarl    $31, %eax       /* sra   %o0, 31, %o5 */
    movl    $-2, %ebx
    subl    8(%ebp), %ebx   /* subcc %g1=-2, %o0, %g2 */
    movl    $-1, %ecx
    sbbl    %eax, %ecx      /* subx  %g4=-1, %o5, %g4 */
    andl    %ecx, %ebx      /* and   %g2, %g4, %g2 */
    movl    $-2, %edx
    subl    %ebx, %edx      /* sub   %g1=-2, %g2, %g1 */
    movl    %edx, %ebx
    sarl    $31, %ebx       /* sra   %g1, 32, %g2 */
    subl    $3, %edx        /* subcc %g1, 3, %g1 */
    sbbl    $0, %ebx        /* subx  %g2, 0, %g2 */
    andl    %ebx, %edx      /* and   %g1, %g2, %g1 */
    addl    $3, %edx        /* add   %g1, 3, %o1 */

/*	cmpl	$-2, 8(%ebp)
	jge	.L2
	movl	$-2, 8(%ebp)
L2:
	cmpl	$3, 8(%ebp)
	jle	.L3
	movl	$3, 8(%ebp)
L3: */
	subl	$8, %esp
/*	pushl	8(%ebp) */
    pushl   %edx            /* push parameter %o1 */
	pushl	$.LC0
	call	printf
	addl	$16, %esp
	leave
	ret
.Lfe1:
	.size	test,.Lfe1-test
.globl main
	.type	main,@function
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	andl	$-16, %esp
	movl	$0, %eax
	subl	%eax, %esp
	subl	$12, %esp
	pushl	$-5
	call	test
	addl	$16, %esp
	subl	$12, %esp
	pushl	$-2
	call	test
	addl	$16, %esp
	subl	$12, %esp
	pushl	$0
	call	test
	addl	$16, %esp
	subl	$12, %esp
	pushl	8(%ebp)
	call	test
	addl	$16, %esp
	subl	$12, %esp
	pushl	$5
	call	test
	addl	$16, %esp
	movl	$0, %eax
	leave
	ret
.Lfe2:
	.size	main,.Lfe2-main
	.ident	"GCC: (GNU) 3.2.2 20030222 (Red Hat Linux 3.2.2-5)"
