/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * Tests cases where the only thing defining a parameter is the return location
 * from an earlier call (tricky for Sparc)
 * Tests "don't bother to pop the last call's argument" for Pentium.
 * Compile with -fno-inline or equivalent
 */

#include <stdio.h>
#include <stdlib.h>


int add5(int arg1)
{
    return arg1 + 5;
}


int add10(int arg2)
{
    return 10 + arg2;
}


int add15(int arg3)
{
    return 5  + arg3 + 10;
}


void printarg(int arg4)
{
    printf("Fifty five is %d\n", arg4);
}


int main()
{
    printarg( add5( add10( add15( 25 ))));
    return EXIT_SUCCESS;
}
