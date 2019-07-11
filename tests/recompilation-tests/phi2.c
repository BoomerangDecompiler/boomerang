/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * Tests transforming out of SSA form.
 * Compile with:
 *   gcc -o phi2 /path/to/phi2.c
 * (i.e. no optimisation)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int proc1 (int x, const char *s)
{
    int z = 0;
    if (x > 2) {
        x = strlen(s);
        z = strlen(s);
        printf("%d", x + z);
    }
    else {
        x = strlen(s);
    }

    printf("%d, %d", x, z);
    return x;
}


int main(int argc, char *argv[])
{
    printf("%d\n", proc1(argc, argv[1]));
    return EXIT_SUCCESS;
}
