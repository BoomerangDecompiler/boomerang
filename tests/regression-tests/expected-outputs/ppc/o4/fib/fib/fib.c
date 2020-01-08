int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x10000470 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    __size32 g3_2; 		// r3{1}

    g3_2 = fib(9);
    g3 = fib(8);
    printf("%i\n", g3_2 + g3);
    return 0;
}

/** address: 0x10000418 */
__size32 fib(int param1)
{
    int g29; 		// r29
    int g3; 		// r3
    int g3_1; 		// r3{1}

    g29 = param1;
    if (param1 > 1) {
        g3_1 = fib(param1 - 1);
        g3 = fib(param1 - 2);
        g29 = g3_1 + g3;
    }
    return g29;
}

