/*
 * Program with a control-flow join that does not need a phi-function
 * (in block 4)
 * From Appel (Modern Compiler Implementation in Java) 2nd Edition p401
 * (Figure 19.2)
 */

#include <stdio.h>

void main(int argc) {
    int x=0;
    int a, b, c;
    printf("Figure 19.2\n");        // Burn block number 0

    x = 1;
    b = argc;
    a = 0;
    printf("1");

    x = 2;
    if (b < 4) {

        x = 3;
        a = b;
    }

    // In this block, there is no need for a phi-function for b
    x = 4;
    c = a+b;
    printf("C is %d\n", c);
}
