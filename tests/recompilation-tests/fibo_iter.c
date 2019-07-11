/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * Compile with
 *   gcc -O4 -fno-inline -o fibo_iter test/source/fibo_iter.c
 */

#include <stdio.h>
#include <stdlib.h>


int fib (int x)
{
    int n, fibn, fibn_1, save;
    if (x <= 1) {
        return x;
    }

    n = 2;
    fibn = 1;       /* fib(2) = 1 */
    fibn_1 = 1;     /* fib(1) = 1 */

    while (n < x) {
        save = fibn;
        fibn = fibn + fibn_1;   /* fib(n+1) = fib(n) + fib(n-1) */
        fibn_1 = save;          /* fib(n-1) = old fib(n) */
        n++;
    }

    return fibn;
}


int main()
{
    int number;

    printf("Input number: ");
    scanf("%d", &number);

    printf("fibonacci(%d) = %d\n", number, fib(number));
    return EXIT_SUCCESS;
}
