/***************************************************************************//**
 * \file       minmax.c
 * OVERVIEW:   Test program to exercise the matching of definitions and uses
 *              of CCs involving the carry flag. Also tests some RTL
 *              simplification code for the Sparc subx instruction (especially
 *              where arguments are all registers).
 * (C) 2001 The University of Queensland, BT group
 *============================================================================*/
/*
    $Revision$
    Compile with:
    % cc -xO4 -xinline= -o test/sparc/minmax2 test/source/minmax2.c

    The resultant code is highly compiler specific (even compiler version
    specific). The intention is to produce code like the following (for Sparc):

        10800:  9d e3 bf a0        save         %sp, -96, %sp
        10804:  84 10 3f fe        mov          -2, %g2
        10808:  88 10 3f ff        mov          -1, %g4
        1080c:  87 3e 20 1f        sra          %i0, 31, %g3
        10810:  90 a0 80 18        subcc        %g2, %i0, %o0
        10814:  86 61 00 03        subx         %g4, %g3, %g3
        10818:  86 0a 00 03        and          %o0, %g3, %g3
        1081c:  13 00 00 42        sethi        %hi(0x10800), %o1
        10820:  84 20 80 03        sub          %g2, %g3, %g2
        10824:  90 02 60 f0        add          %o1, 240, %o0
        10828:  87 38 a0 1f        sra          %g2, 31, %g3
        1082c:  84 a0 a0 03        subcc        %g2, 3, %g2
        10830:  86 60 e0 00        subx         %g3, 0, %g3
        10834:  84 08 80 03        and          %g2, %g3, %g2
        10838:  40 00 40 4f        call         printf
        1083c:  92 00 a0 03        add          %g2, 3, %o1
        10840:  81 c7 e0 08        ret
        10844:  91 e8 20 00        restore      %g0, 0, %o0

    For x86, the Sun compiler doesn't (at present) generate terribly
    interesting code, but the test is retained for completeness.

05 Apr 01 - Mike: Created
15 Jan 04 - Mike: Mods for better functional testing
*/

#include <stdio.h>


void test(int i)
{
    if (i < -2) i = -2;
    if (i > 3) i = 3;

    printf("MinMax result %d\n", i);
}


int main(int argc, char *argv[])
{
    test(-5);
    test(-2);
    test(0);
    test(argc);
    test(5);
}
