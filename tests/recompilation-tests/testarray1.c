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


int main()
{
    int i, sum = 0;
    for (i=0; i < 5; i++) {
        sum += gca[i];
    }

    printf("Sum is %d\n", sum);
    return EXIT_SUCCESS;
}
