/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *chomp(char *s, int size, FILE *f)
{
    char *res = fgets(s, size, f);
    if (res) {
        char *p = strchr(res, '\n');
        if (p) {
            *p = 0;
        }
    }

    return res;
}

int main(int argc, char **argv)
{
    FILE *f;
    char line[1024];

    if (argc < 2) {
        return 1;
    }

    f = fopen(argv[1], "r");
    if (f == NULL) {
        return EXIT_FAILURE;
    }

    if (chomp(line, sizeof(line), f)) {
        printf("%s\n", line);
    }

    fclose(f);
    return EXIT_SUCCESS;
}
