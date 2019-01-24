/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


int bswap(int x)
{
    __asm__("bswapl %0;" : "=r" (x) : "r" (x));
    return x;
}


int main()
{
    printf("Output is %x\n", bswap(0x12345678U));
    return EXIT_SUCCESS;
}
