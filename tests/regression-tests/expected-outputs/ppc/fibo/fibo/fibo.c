int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x100004c4 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    g3 = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, g3);
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1)
{
    int g3; 		// r3
    int g3_1; 		// r3{4}
    int local5; 		// m[g1 - 20]

    if (param1 <= 1) {
        local5 = param1;
    }
    else {
        g3_1 = fib(param1 - 1);
        g3 = fib(param1 - 2);
        local5 = g3_1 + g3;
    }
    return local5;
}

