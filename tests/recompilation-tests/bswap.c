/* Compile with gcc -O0 */
#include <stdio.h>

int bswap(int x) {
    __asm__("movl 8(%ebp), %eax");
    __asm__("bswap %eax");
}

int main() {
    int r = bswap(0x12345678);
    printf("Output is %x\n", r);
    return 0;
}

