/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
/* Generate a set instruction (Pentium) */

#include <stdio.h>

int main(int argc, char *argv[])
{
    const char c = (argc > 1) ? 1 : 0;
    printf("Result is %d\n", c);
    return c;
}
