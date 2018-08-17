int main(int argc, char *argv[]);
__size32 fib2(int param1);

/** address: 0x100004b8 */
int main(int argc, char *argv[])
{
    __size32 g1; 		// r1
    int g3; 		// r3
    int g4; 		// r4
    int local0; 		// m[g1 - 8]

    printf("Input number: ");
    scanf("%d", &local0);
    g1 = fib2(local0); /* Warning: also results in g3 */
    g4 = *(g1 + 8);
    printf("fibonacci(%d) = %d\n", g4, g3);
    return 0;
}

/** address: 0x10000440 */
__size32 fib2(int param1)
{
    __size32 g1; 		// r1
    int g29; 		// r29
    __size32 g31; 		// r31
    __size32 g4; 		// r4

    g1 -= 32;
    g4 = /* machine specific */ (int) LR;
    g29 = param1;
    if (param1 > 1) {
        g31 = fib2(param1 - 1);
        g1 = fib2(g31 - 2); /* Warning: also results in g4 */
        g29 = *(g1 + 20);
        g1 += 32;
    }
    g31 = *(g1 + 28);
    return g29; /* WARNING: Also returning: g4 := g4, g31 := g31 */
}

