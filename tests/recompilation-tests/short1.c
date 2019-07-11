/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdlib.h>
#include <stdio.h>


int test(int a, int b, int c)
{
    if (a < b || b < c) {
        return 1;
    }

    return 0;
}


int main()
{
    printf("Result for 4, 5, 6: %d\n", test(4, 5, 6));
    printf("Result for 6, 5, 4: %d\n", test(6, 5, 4));
    printf("Result for 4, 6, 5: %d\n", test(4, 6, 5));
    printf("Result for 6, 4, 5: %d\n", test(6, 4, 5));
    return EXIT_SUCCESS;
}
