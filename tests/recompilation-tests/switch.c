/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/* Switch jump table recognition test */

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    switch(argc)
    {
    case 2:  printf("Two!\n");   break;
    case 3:  printf("Three!\n"); break;
    case 4:  printf("Four!\n");  break;
    case 5:  printf("Five!\n");  break;
    case 6:  printf("Six!\n");   break;
    case 7:  printf("Seven!\n"); break;
    default: printf("Other!\n"); break;
    }

    return EXIT_SUCCESS;
}
