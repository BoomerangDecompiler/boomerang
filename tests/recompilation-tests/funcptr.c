/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


typedef void (*FPTR)();


void hello()
{
    printf("Hello, ");
}


void world()
{
    printf("world!\n");
}


int main()
{
    FPTR p;

    p = hello;
    (*p)();

    p = world;
    (*p)();

    return EXIT_SUCCESS;
}
