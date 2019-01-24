/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * This version of fibo passes parameters and the return value via a
 * dummy function fib1 (in RISC machines, there will be no code to pass
 * parameters or return the value. Call graph:
 *  main
 *   |
 *   f1<--+
 *   |    |
 *   f2---+
 *
 * Compile with:
 *   gcc -O4 -fno-inline -o test/sparc/fibo2 test/source/fibo2.c
 */

#include <stdio.h>
#include <stdlib.h>


int fib2(int x);

int fib1(int x)
{
    return fib2(x);
}


int fib2 (int x)
{
    if (x > 1) {
        return fib1(x - 1) + fib1(x - 2);
    }
    else {
        return x;
    }
}


int main()
{
    int number;

    printf ("Input number: ");
    scanf ("%d", &number);

    printf("fibonacci(%d) = %d\n", number, fib1(number));
}
