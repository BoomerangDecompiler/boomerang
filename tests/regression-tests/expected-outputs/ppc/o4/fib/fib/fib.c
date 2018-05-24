int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x10000470 */
int main(int argc, char *argv[])
{
    fib(9);
    fib(8);
    printf(0x10000864);
    return 0;
}

/** address: 0x10000418 */
__size32 fib(int param1)
{
    int g29; 		// r29
    int g3; 		// r3
    int g3_1; 		// r3{0}
    __size32 g4; 		// r4

    g4 = /* machine specific */ (int) LR;
    g29 = param1;
    if (param1 > 1) {
        g3_1 = fib(param1 - 1);
        g3 = fib(param1 - 2); /* Warning: also results in g4 */
        g29 = g3_1 + g3;
    }
    return g29; /* WARNING: Also returning: g4 := g4 */
}

