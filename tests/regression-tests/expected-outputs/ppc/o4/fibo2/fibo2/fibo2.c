int main(int argc, char *argv[]);
__size32 fib2(int param1);

/** address: 0x100004b8 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 8]

    printf(0x100008b8);
    scanf(0x100008c8);
    fib2(local0);
    printf(0x100008cc);
    return 0;
}

/** address: 0x10000440 */
__size32 fib2(int param1)
{
    __size32 g1; 		// r1
    __size32 g31; 		// r31
    __size32 g4; 		// r4

    g1 = g1 - 32;
    g4 = /* machine specific */ (int) LR;
    if (param1 > 1) {
        g31 = fib2(param1 - 1);
        g1 = fib2(g31 - 2); /* Warning: also results in g4 */
        g1 += 32;
    }
    g31 = *(g1 + 28);
    return g4; /* WARNING: Also returning: g31 := g31 */
}

