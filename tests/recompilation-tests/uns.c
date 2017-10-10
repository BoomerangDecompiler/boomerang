#include <stdio.h>

int main(int argc) {
    if (argc >= 4000000000U)
        printf("Population exceeds %u\n", 4000000000U);
    if (argc < 0xF0000000)
        printf("The mask is %x\n", 0xF0000000);
    unsigned u = (unsigned) argc;
    if (u >= 2)
        printf("Arguments supplied\n");
    if (-argc < -2)
        printf("Three or more arguments\n");
    return 0;
}
