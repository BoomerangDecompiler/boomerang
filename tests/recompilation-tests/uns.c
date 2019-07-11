/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    if (argc >= 4000000000U) {
        printf("Population exceeds %u\n", 4000000000U);
    }

    if (argc < 0xF0000000) {
        printf("The mask is %x\n", 0xF0000000);
    }

    unsigned int u = (unsigned int)argc;
    if (u >= 2) {
        printf("Arguments supplied\n");
    }

    if (-argc < -2) {
        printf("Three or more arguments\n");
    }

    return EXIT_SUCCESS;
}
