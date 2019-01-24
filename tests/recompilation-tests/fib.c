/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


int fib(int x)
{
    if (x <= 1) {
        return x;
    }
    else {
        return fib(x - 1) + fib(x - 2);
    }
}


int main()
{
    printf("%i\n", fib(10));
    return EXIT_SUCCESS;
}
