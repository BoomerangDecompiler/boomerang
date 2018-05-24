int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x100004a8 */
int main(int argc, char *argv[])
{
    int g31; 		// r31
    int local0; 		// m[g1 - 24]

    printf(0x10000904);
    scanf(0x10000914);
    if (local0 > 1) {
        g31 = fib(local0 - 1);
        fib(g31 - 2);
        printf(0x10000918);
    }
    else {
        printf(0x10000918);
    }
    return 0;
}

/** address: 0x10000434 */
__size32 fib(int param1)
{
    __size32 g0; 		// r0
    __size32 g1; 		// r1
    __size32 g31; 		// r31

    g0 = /* machine specific */ (int) LR;
    g1 = g1 - 32;
    if (param1 > 1) {
        g31 = fib(param1 - 1);
        g0 = fib(g31 - 2); /* Warning: also results in g1 */
        g1 += 32;
    }
    g31 = *(g1 + 28);
    return g0; /* WARNING: Also returning: g31 := g31 */
}

