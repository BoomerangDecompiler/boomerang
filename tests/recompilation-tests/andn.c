/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * Mainly for sparc, ppc and other processors with andn style operators (NAND)
 *
 * Note: it's somewhat pot luck whether the andn etc instructions get used.
 * This worked in with gcc 3.1 for SPARC:
 * Compile with gcc -O1
 */


#include <stdio.h>
#include <stdlib.h>


int main()
{
    const int a = 0x55AA55AA;
    const int b = 0x12345678;
    const int c = 0x98765432;

    printf("a andn b is %d\n",  a & ~b);
    printf("b orn  a is %d\n",  b | ~a);
    printf("a xorn c is %d\n",  a ^ ~c);

    return EXIT_SUCCESS;
}

