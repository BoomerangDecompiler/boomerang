/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


int a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };


int main()
{
    int sum = 0;
    int i;

    for (i=0; i < 10; i++) {
        sum += a[i];
    }

    printf("Sum is %d\n", sum);
    return EXIT_SUCCESS;
}
