# SPARC assembly source code for the elfhash test
# assemble with gcc -o output elfhashtest.s

	.file	"elfhashtest.c"
	.section	".rodata"
	.align 8
.LLC0:
	.asciz	"main"
	.align 8
.LLC1:
	.asciz	"elf_hash(\"main\") is %x\n"
	.section	".text"
	.align 4
	.global main
	.type	main,#function
	.proc	04
main:
	!#PROLOGUE# 0
	save	%sp, -112, %sp
	!#PROLOGUE# 1
	sethi	%hi(.LLC0), %o0
	or	%o0, %lo(.LLC0), %o0
	call	elf_hash, 0
	 nop
	mov	%o0, %o1
	sethi	%hi(.LLC1), %o0
	or	%o0, %lo(.LLC1), %o0
	call	printf, 0
	 nop
	mov	0, %o0
	mov	%o0, %i0
	nop
	ret
	restore
.LLfe1:
	.size	main,.LLfe1-main

elf_hash:
	ldsb  [ %o0 ], %o3
	mov  %o0, %g1
	sethi  %hi(0xf0000000), %o5
	cmp  %o3, 0
	be  L57c
	clr  %o4
	sll  %o4, 4, %o4
L54C:
	add  %o4, %o3, %o3
	inc  %g1
	andcc  %o3, %o5, %o4
	be,a   L56C
	andn  %o3, %o4, %o4
	srl  %o4, 0x18, %o2
	xor  %o3, %o2, %o3
	andn  %o3, %o4, %o4
L56C:
	ldsb  [ %g1 ], %o3
	cmp  %o3, 0
	bne,a   L54C
	sll  %o4, 4, %o4
L57c:
	retl 
	mov  %o4, %o0


	.ident	"GCC: (GNU) 3.1"
