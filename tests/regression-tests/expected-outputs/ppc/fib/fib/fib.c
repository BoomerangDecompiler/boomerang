int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    g3 = fib(10);
    printf("%i\n", g3);
    return 0;
}

/** address: 0x10000470 */
__size32 fib(int param1)
{
    int g3_1; 		// r3{7}
    int g3_4; 		// r3{8}
    int local0; 		// m[g1 - 20]

    if (param1 > 1) {
        g3_1 = fib(param1 - 1);
        g3_4 = fib(param1 - 2);
        local0 = g3_1 + g3_4;
    }
    else {
        local0 = param1;
    }
    return local0;
}

