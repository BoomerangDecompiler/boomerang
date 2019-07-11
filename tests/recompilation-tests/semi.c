/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * This test program has a case where a semidominator (node L/6) is not the
 * same as its dominator (semi[6] = 4, idom[6] = 2)
 */

#include <stdio.h>


int main(int argc, char *argv[])
{
    int x = 1;                      /* A */
    if (argc > 2) {
        do {
            x = 2;                  /* B */
            if (argc > 2) goto nine;
            x = 3;                  /* D */
            if (argc > 3) {
                x = 4;              /* F */
                if (argc > 4) {
                    five:
                    x = 5;          /* I */
                    printf("5");
                } else
                    x = 8;          /* K */
            }
            else {
                nine:
                x = 9;              /* G */
                printf("9");
                x = 10;             /* J */
                goto five;
            }
            x = 6;                  /* L */
        } while (argc < 6);
    } else {
        do {
            x = 11;                 /* C */
            if (argc == 11) goto thirteen;
            x = 12;
        } while (argc < 12);
        thirteen:
        x = 13;                     /* H */
    }
    x = 7;                          /* M */
    return x;
}
