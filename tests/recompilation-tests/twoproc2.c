/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


int proc1(int a, int b)
{
    return a + b;
}


int main()
{
    printf("%i\n", proc1(3, 4));
    printf("%i\n", proc1(5, 6));
    return EXIT_SUCCESS;
}
