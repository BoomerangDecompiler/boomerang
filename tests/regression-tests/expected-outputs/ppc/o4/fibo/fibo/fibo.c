int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x100004b4 */
int main(int argc, char *argv[])
{
    int g31; 		// r31
    int local0; 		// m[g1 - 24]

    printf(0x10000914);
    scanf(0x10000924);
    if (local0 > 1) {
        g31 = fib(local0 - 1);
        fib(g31 - 2);
        printf(0x10000928);
    }
    else {
        printf(0x10000928);
    }
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1)
{
    __size32 g1; 		// r1
    __size32 g31; 		// r31
    __size32 g4; 		// r4

    g1 = g1 - 32;
    g4 = /* machine specific */ (int) LR;
    if (param1 > 1) {
        g31 = fib(param1 - 1);
        g1 = fib(g31 - 2); /* Warning: also results in g4 */
        g1 += 32;
    }
    g31 = *(g1 + 28);
    return g4; /* WARNING: Also returning: g31 := g31 */
}

