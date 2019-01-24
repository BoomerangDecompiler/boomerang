/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int fib(int x)
{
    int z;
    if (x > 1) {
        x = fib(x-1);
        z = fib(x-1);
        printf("%d", x+z);
        return x;
    }
    else {
        /* Force a definition of eax */
        if (x == 1) {
            return strlen("x");
        }
        else {
            return x;
        }
    }
}


int main()
{
    int number;

    printf("Input number: ");
    scanf("%d", &number);

    printf("fibonacci(%d) = %d\n", number, fib(number));
    return EXIT_SUCCESS;
}
