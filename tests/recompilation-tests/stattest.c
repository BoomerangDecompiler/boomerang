/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


int main(int argc, char* argv[])
{
    struct stat st;
    const int res = stat("test/source/stattest.c", &st);

    printf("Stat returns %d; size of file is %d\n", res, st.st_size);

    return res;
}

