__size32 fib1();
__size32 fib2(int param1);

// address: 0x1cf0
int main(int argc, char *argv[], char *envp[]) {
    int g3; 		// r3

    printf(/* machine specific */ (int) LR + 720);
    scanf(/* machine specific */ (int) LR + 736);
    g3 = fib1();
    *(__size32*)(g30 + 68) = g3;
    printf(/* machine specific */ (int) LR + 740);
    return 0;
}

// address: 0x1c3c
__size32 fib1() {
    int g3; 		// r3
    __size32 g30; 		// r30

    g3 = fib2(g3); /* Warning: also results in g30 */
    return g30; /* WARNING: Also returning: g30 := g30, g3 := g3, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30, g30 := g30 */
}

// address: 0x1c78
__size32 fib2(int param1) {
    __size32 g1; 		// r1
    int g29; 		// r29
    int g3; 		// r3
    __size32 g30; 		// r30
    __size32 g30_1; 		// r30{69}
    __size32 g30_2; 		// r30{86}
    __size32 g30_3; 		// r30{168}
    __size32 local7; 		// g30_3{168}

    g30_1 = g1 - 96;
    local7 = g30_1;
    if (param1 <= 1) {
    } else {
        fib1();
        g3 = fib1(); /* Warning: also results in g30_2 */
        local7 = g30_2;
        *(__size32*)(g30_2 + 64) = g29 + g3;
    }
    g30_3 = local7;
    g3 = *(g30_3 + 64);
    return g30; /* WARNING: Also returning: g3 := g3, g30 := g30, g30 := g30_3, g30 := g30, g30 := g30 */
}

