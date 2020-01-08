int main(int argc, char *argv[]);
__size32 fib2(int param1);


/** address: 0x100004b8 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int local0; 		// m[g1 - 8]

    printf("Input number: ");
    scanf("%d", &local0);
    g3 = fib2(local0);
    printf("fibonacci(%d) = %d\n", local0, g3);
    return 0;
}

/** address: 0x10000440 */
__size32 fib2(int param1)
{
    int g3; 		// r3
    __size32 g3_1; 		// r3{3}

    if (param1 > 1) {
        g3_1 = fib2(param1 - 1);
        g3 = fib2(param1 - 2);
        g3 = g3_1 + g3;
    }
    else {
        g3 = param1;
    }
    return g3;
}

