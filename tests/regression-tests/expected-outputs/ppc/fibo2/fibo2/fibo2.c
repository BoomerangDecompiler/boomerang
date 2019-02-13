int main(int argc, char *argv[]);
__size32 fib1();
__size32 fib2(int param1);


/** address: 0x10000504 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    g3 = fib1();
    printf("fibonacci(%d) = %d\n", local0, g3);
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
    int g3_1; 		// r3{5}
    __size32 g9; 		// r9
    int local0; 		// m[g1 - 20]

    if (param1 <= 1) {
        local0 = param1;
    }
    else {
        g3_1 = fib1();
        g3 = fib1(); /* Warning: also results in g9 */
        local0 = g3_1 + g3;
    }
    return local0; /* WARNING: Also returning: g9 := g9 */
}

