/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


int main()
{
    int a=5; int b;
    unsigned u=5; unsigned v;

    scanf("%d", &b);
    scanf("%d", &v);

    if (a == b) printf("Equal\n");
    if (a != b) printf("Not Equal\n");
    if (a >  b) printf("Greater\n");
    if (a <= b) printf("Less or Equal\n");
    if (a >= b) printf("Greater or Equal\n");
    if (a <  b) printf("Less\n");
    if (u >  v) printf("Greater Unsigned\n");
    if (u <= v) printf("Less or Equal Unsigned\n");
    if (u >= v) printf("Carry Clear\n");
    if (u <  v) printf("Carry Set\n");
    if ((a - b) >= 0) printf("Minus\n");
    if ((a - b) < 0) printf("Plus\n");

    return EXIT_SUCCESS;
}
