/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


int a = 5;
int b = 7;


void foo2()
{
    b = 12;
    printf("a = %i\n", a);
}


void foo1()
{
    foo2();
}


int main()
{
    foo1();
    printf("b = %i\n", b);
    return EXIT_SUCCESS;
}
