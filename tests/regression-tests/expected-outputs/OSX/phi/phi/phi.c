int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x00001cf0 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    __size32 g31; 		// r31
    int local0; 		// m[g1 - 32]

    printf(/* machine specific */ (int) LR + 724);
    scanf(/* machine specific */ (int) LR + 720);
    g3 = fib(local0); /* Warning: also results in g31 */
    *(__size32*)(g30 + 68) = g3;
    printf(g31 + 740);
    return 0;
}

/** address: 0x00001c34 */
__size32 fib(int param1)
{
    int g0; 		// r0
    union { int; void *; } g1; 		// r1
    int g3; 		// r3
    union { int; void *; } g30; 		// r30
    __size32 g31; 		// r31
    int g9; 		// r9

    g30 = g1 - 96;
    g31 = /* machine specific */ (int) LR;
    if (param1 <= 1) {
        if (param1 == 1) {
        }
    }
    else {
        g3 = fib(param1 - 1); /* Warning: also results in g30 */
        *(__size32*)(g30 + 120) = g3;
        g3 = fib(g9 - 1); /* Warning: also results in g30, g31 */
        *(__size32*)(g30 + 64) = g3;
        printf(g31 + 908);
        g0 = *(g30 + 120);
        *(int*)(g30 + 68) = g0;
    }
    g3 = *(g30 + 68);
    return g3; /* WARNING: Also returning: g30 := g30, g31 := g31 */
}

