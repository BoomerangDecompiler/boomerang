/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


char gca[5] = {2, 4, 6, 8, 10};


void mid(char ca[])
{
    printf("Middle element is %d\n", ca[2]);
}


void fst(char ca[])
{
    printf("First element is %d\n", ca[10]);
}


int main()
{
    int i, sum = 0;
    mid(gca);
    fst(gca - 10);      /* Pass the address of the "-10th" element */

    char* p = gca;
    for (i=0; i < 5; i++) {
        sum += *p++;
    }

    printf("Sum is %d\n", sum);
    return EXIT_SUCCESS;
}
