# This program is to test code with interleaved integer and floating point
# defining and using of flags

# Use test/source/Makefile.interleavedcc

	.file	"interleavedcc.s"
.section	".text"
	.align 4
	.global interleaved
	.type	 interleaved,#function
	.proc	04
interleaved:

#        15d3c:  81 a8 0a 21        fcmps        %f0, %f1
#        15d40:  01 00 00 00        nop          
#        15d44:  d0 07 bf ec        ld           [%fp - 20], %o0
#        15d48:  80 a2 20 00        cmp          %o0, 0
#        15d4c:  03 80 00 0d        fbne         0x15d80
#        15d50:  01 00 00 00        nop          
#        15d54:  16 80 00 05        bge          0x15d68
#        15d58:  90 10 20 02        mov          2, %o0

# First get the arguments to the appropriate registers. %o0 is already correct
    st      %o1, [%sp+72]
    ld      [%sp+72], %f0
    st      %o2, [%sp+76]
    ld      [%sp+76], %f1

    fcmps   %f0, %f1
    nop          
#   ld      [%fp - 20], %o0
    cmp     %o0, 0
    fbne    fpdiff
    nop        
    bge     out
    mov     2, %o0          ! Quadrant 2
    mov     3, %o0          ! Quadrant 3
    ba,a    out

fpdiff:
    bge     intge2
    mov     1, %o0          ! Quadrant 1
    ba,a    out

intge2:
    mov     0, %o0          ! Quadrant 0

out:
    jmp     %o7+8
    nop

.LLfe1:
	.size	 interleaved,.LLfe1-interleaved
