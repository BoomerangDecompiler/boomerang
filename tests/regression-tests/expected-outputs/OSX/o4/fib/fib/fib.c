int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x00001d20 */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    g3 = fib(10);
    printf("%i\n", g3);
    return 0;
}

/** address: 0x00001d68 */
__size32 fib(int param1)
{
    int g3; 		// r3
    __size32 g3_1; 		// r3{4}
    int local5; 		// param1{6}

    local5 = param1;
    if (param1 > 1) {
        g3_1 = fib(param1 - 1);
        g3 = fib(param1 - 2);
        g3 = g3_1 + g3;
        local5 = g3;
    }
    param1 = local5;
    return param1;
}

