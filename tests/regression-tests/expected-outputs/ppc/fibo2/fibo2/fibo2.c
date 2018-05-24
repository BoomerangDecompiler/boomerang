int main(int argc, char *argv[]);
__size32 fib1();
__size32 fib2(int param1);

/** address: 0x10000504 */
int main(int argc, char *argv[])
{
    printf(0x10000920);
    scanf(0x10000930);
    fib1();
    printf(0x10000934);
    return 0;
}

/** address: 0x10000440 */
__size32 fib1()
{
    int g3; 		// r3
    __size32 g9; 		// r9

    g3 = fib2(g3); /* Warning: also results in g9 */
    return g3; /* WARNING: Also returning: g9 := g9 */
}

/** address: 0x10000480 */
__size32 fib2(int param1)
{
    int g3; 		// r3
    int g3_1; 		// r3{0}
    __size32 g9; 		// r9
    int local5; 		// m[g1 - 20]

    if (param1 <= 1) {
        local5 = param1;
    }
    else {
        g3_1 = fib1();
        g3 = fib1(); /* Warning: also results in g9 */
        local5 = g3_1 + g3;
    }
    return local5; /* WARNING: Also returning: g9 := g9 */
}

