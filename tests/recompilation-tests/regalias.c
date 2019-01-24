/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


int main()
{
    int r;
    __asm__("movl $0x87654321, %%eax;"
            "movb $0x12, %%al;"
            "movb $0x34, %%ah"
        : "=a" (r));

    printf("%08X\n", r);
    return EXIT_SUCCESS;
}
