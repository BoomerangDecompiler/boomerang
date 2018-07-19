int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x100004bc */
int main(int argc, char *argv[])
{
    __size32 g1; 		// r1
    int g29; 		// r29
    int g3; 		// r3
    int g4; 		// r4
    int g5; 		// r5
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    g5 = scanf("%d", &local0); /* Warning: also results in %g6, %g7, %g8, %g10, %g11, %g12 */
    if (local0 > 1) {
        g3 = fib(local0 - 1); /* Warning: also results in g5, %g6, %g7, %g8, %g10, %g11, %g12, %CR1 */
        g1 = fib(g3 - 1); /* Warning: also results in g3, g29 */
        printf("%d", g29 + g3);
        g4 = *(g1 + 8);
        printf("fibonacci(%d) = %d\n", g4, g29);
    }
    else {
        g5 = 1;
        if (local0 != 1) {
            g5 = local0;
        }
        printf("fibonacci(%d) = %d\n", local0, g5);
    }
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1)
{
    __size32 CR1; 		// r65
    __size1 CR1_1; 		// r65
    __size32 g1; 		// r1
    __size32 g10; 		// r10
    __size32 g11; 		// r11
    __size32 g12; 		// r12
    __size32 g29; 		// r29
    int g3; 		// r3
    int g3_2; 		// r3{0}
    __size32 g5; 		// r5
    __size32 g6; 		// r6
    __size32 g7; 		// r7
    __size32 g8; 		// r8
    int local3; 		// g3{0}
    __size32 local4; 		// CR1{0}

    g1 = g1 - 32;
    local4 = CR1;
    local4 = CR1;
    if (param1 > 1) {
        g3 = fib(param1 - 1);
        g1 = fib(g3 - 1); /* Warning: also results in g3, g29, CR1 */
        %CR1 = %CR1 & ~0x4;
        g5 = printf("%d", g29 + g3); /* Warning: also results in g6, g7, g8, g10, g11, g12 */
        local4 = CR1_1;
        g3 = g29;
        g1 += 32;
        local3 = g3;
    }
    else {
        g3_2 = 1;
        local3 = g3_2;
        if (param1 != 1) {
            g3 = param1;
            local3 = g3;
        }
    }
    g3 = local3;
    CR1 = local4;
    g29 = *(g1 + 20);
    return g3; /* WARNING: Also returning: g5 := g5, g6 := g6, g7 := g7, g8 := g8, g10 := g10, g11 := g11, g12 := g12, g29 := g29, CR1 := CR1 */
}

