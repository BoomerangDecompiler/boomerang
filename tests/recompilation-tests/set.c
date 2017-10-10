/* Generate a set instruction (Pentium) */

#include <stdio.h>

int main(int argc) {
    char c = (argc > 1) ? 1 : 0;
    printf("Result is %d\n", c);
    return c;
}
