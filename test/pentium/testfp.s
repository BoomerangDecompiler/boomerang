	.file	"hello.c"
	.section	.rodata
.LC0:
	.string	"Hello, world\n"
result:  .string "Result is %f\n"
	.align 8
three:	.long	3
five:	.long	5
	.section	.data
res1:	.long 0
res2:	.long 0

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

    filds three
    filds five
    fsub %st(1), %st
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf
    fstp %st

    filds three
    filds five
    fsub %st, %st(1)
    fstp %st	# Want st(1)
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

    filds three
    filds five
    fsubr %st(1), %st
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf
    fstp %st

    #filds three
    #filds five
    #fsubp %st(1), %st	# This one makes no sense: result to %st, then pop it; doesn't exist!
    #fstpl  res1
    #push res2
    #push res1
    #push  $result
    #call printf

    #filds three
    #filds five
    #fsubrp %st(1), %st		# Also makes no sense, and does not exist!
    #fstpl  res1
    #push res2
    #push res1
    #push  $result
    #call printf

    filds three
    filds five
    fsubr %st, %st(1)	# Want DC E9
    fstp %st	# Want st(1)
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

    filds three
    filds five
    fsubp %st, %st(1)
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

    filds three
    filds five
    fsubrp %st, %st(1)
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf


    filds three
    filds five
    fdiv %st(1), %st
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf
    fstp %st

    filds three
    filds five
    fdiv %st, %st(1)
    fstp %st
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

    filds three
    filds five
    fdivr %st(1), %st
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf
    fstp %st

    #filds three
    #filds five
    #fdivp %st(1), %st		# Makes no sense: result to %st, then pop it!
    #fstpl  res1
    #push res2
    #push res1
    #push  $result
    #call printf

    #filds three
    #filds five
    #fdivrp %st(1), %st		# Also makes no sense!
    #fstpl  res1
    #push res2
    #push res1
    #push  $result
    #call printf

    filds three
    filds five
    fdivr %st, %st(1)
    fstp %st	# Want st(1)
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

    filds three
    filds five
    fdivp %st, %st(1)
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

    filds three
    filds five
    fdivrp %st, %st(1)
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

###

    filds three
    fisub five
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

    filds three
    fisubr five
    fstpl  res1
    push res2
    push res1
    push  $result
    call printf

    rdtsc

	addl	$16, %esp
	leave
	ret
.Lfe1:
	.size	main,.Lfe1-main
	.ident	"GCC: (GNU) 3.2.2 20030222 (Red Hat Linux 3.2.2-5)"
