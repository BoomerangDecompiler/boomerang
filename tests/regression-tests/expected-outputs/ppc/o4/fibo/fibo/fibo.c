int main(int argc, char *argv[]);
__size32 fib(int param1, __size32 param2);


/** address: 0x100004b4 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    __size32 g3_2; 		// r3{8}
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    if (local0 > 1) {
        g3_2 = fib(local0 - 1, 0x10000530);
        g3 = fib(local0 - 2, 0x1000053c);
        printf("fibonacci(%d) = %d\n", local0, g3_2 + g3);
    }
    else {
        printf("fibonacci(%d) = %d\n", local0, local0);
    }
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1, __size32 param2)
{
    int g3; 		// r3
    __size32 g3_1; 		// r3{9}
    __size32 g4; 		// r4

    g4 = param2;
    if (param1 > 1) {
        g3_1 = fib(param1 - 1, 0x10000488);
        g3 = fib(param1 - 2, 0x10000494); /* Warning: also results in g4 */
        g3 = g3_1 + g3;
    }
    else {
        g3 = param1;
    }
    return g3; /* WARNING: Also returning: g4 := g4 */
}

