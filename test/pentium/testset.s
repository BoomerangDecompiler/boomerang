	.file	"hello.c"
	.section	.rodata
.LC0:
	.string	"Hello, set\n"
.LC_c:
    .string "argc <u 3: %d\n";
.LC_n:
    .string "(argc - 4) >= 0: %d\n";

	.text
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
	pushl	$.LC0
	call	printf

# Tests

    mov     8(%ebp), %eax    # argc
    mov     $0, %edx
    cmpl    $3, %eax
    setc    %dl
    push    %edx
    push    $.LC_c
    call    printf

    mov     8(%ebp), %eax    # argc
    mov     $0, %ecx
    subl    $4, %eax
    setns   %cl
    push    %ecx
    push    $.LC_n
    call    printf

    xorl    %eax, %eax
	addl	$16, %esp
	leave
	ret
.Lfe1:
	.size	main,.Lfe1-main
	.ident	"GCC: (GNU) 3.2.2 20030222 (Red Hat Linux 3.2.2-5)"
