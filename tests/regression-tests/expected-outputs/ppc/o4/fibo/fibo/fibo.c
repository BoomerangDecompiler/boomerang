int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x100004b4 */
int main(int argc, char *argv[])
{
    __size32 g1; 		// r1
    __size32 g29; 		// r29
    int g3; 		// r3
    int g31; 		// r31
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    if (local0 > 1) {
        g31 = fib(local0 - 1);
        g1 = fib(g31 - 2); /* Warning: also results in g3, g29 */
        g31 = *(g1 + 8);
        printf("fibonacci(%d) = %d\n", g31, g29 + g3);
    }
    else {
        printf("fibonacci(%d) = %d\n", local0, local0);
    }
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1)
{
    __size32 g1; 		// r1
    int g29; 		// r29
    int g29_1; 		// r29{0}
    __size32 g31; 		// r31
    __size32 g4; 		// r4

    g1 -= 32;
    g4 = /* machine specific */ (int) LR;
    g29 = param1;
    if (param1 > 1) {
        g31 = fib(param1 - 1);
        g1 = fib(g31 - 2); /* Warning: also results in g4 */
        g29 = *(g1 + 20);
        g1 += 32;
    }
    g29_1 = g29;
    g31 = *(g1 + 28);
    g29 = *(g1 + 20);
    return g29_1; /* WARNING: Also returning: g4 := g4, g29 := g29, g31 := g31 */
}

