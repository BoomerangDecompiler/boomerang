/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * Compile with
 *   gcc -O4 -o fromssa2 -fno-unroll-loops test/source/fromssa2.c
 */

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    int a, x;
    a = 0;

    do {
        a = a+1;
        x = a;
        printf("%d ", a);
    } while (a < 10);

    printf("a is %d, x is %d\n", a, x);
    return EXIT_SUCCESS;
}

