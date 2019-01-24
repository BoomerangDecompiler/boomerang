/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
/* Compile with gcc -O4 -fno-inline -o ...  or cc -xO4 -xinline= -o ...  */

#include <stdio.h>
#include <stdlib.h>


int proc1(int a, int b)
{
    return a - b;
}


int main()
{
    printf("%i\n", proc1(11, 4));
    return EXIT_SUCCESS;
}


