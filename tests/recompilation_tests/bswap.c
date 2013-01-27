// Compile with gcc -O0
#include <stdio.h>

int bswap(int x) {
    asm("mov 8(%ebp), %eax");
    asm("bswap %eax");
}

int main() {
    int r = bswap(0x12345678);
    printf("Output is %x\n", r);
    return 0;
}

