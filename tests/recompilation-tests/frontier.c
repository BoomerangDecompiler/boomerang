/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/* Test program for dominance frontier code
    Based on Figure 19.1 (p405) of "Modern Compiler Implementation in Java",
    Andrew W. Appel, 2nd Edition, Cambridge University Press, 2002.
    ISBN 0 521 82060 X
*/

#include <stdio.h>


int main(int argc, char  *argv[])
{
    int x = 1;
    switch (argc) {
        case 2:
            x = 2;
            do {
                x = 3;
            } while (argc > 0);
            four:
            x = 4;
            break;
        case 5:
            do {
                x = 5;
                if (argc-- > 1) {
                    x = 6;
                    if (argc-- > 2)
                        goto four;
                } else {
                    x = 7;
                    if (argc == 12)
                        goto twelve;
                }
                x = 8;
            } while (argc > 0);
            break;
        case 9:
            x = 9;
            if (argc == 10)
                x = 10;
            else
                x = 11;
            twelve:
            x = 12;
            break;
    }

    x = 13;
    return x;
}
