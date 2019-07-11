/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


struct pair {
    int a;
    int b;
};

struct pair twofib(int n)
{
    struct pair r;
    if (n == 0) {
        r.a = 0;
        r.b = 1;
    }
    else {
        r = twofib(n-1);
        int a = r.a;
        r.a = r.b;
        r.b += a;
    }
    return r;
}


int main()
{
    int n;
    printf("Enter number: ");
    scanf("%d", &n);

    printf("Fibonacci of %d is %d\n", n, twofib(n).a);
    return EXIT_SUCCESS;
}
