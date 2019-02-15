int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x000106fc */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 20]
    int o2; 		// r10

    printf("Input number: ");
    scanf("%d", &local0);
    fib(local0);
    printf("fibonacci(%d) = %d\n", local0, o2);
    return 0;
}

/** address: 0x000106ac */
__size32 fib(int param1)
{
    int g0; 		// r0
    int o2; 		// r10
    __size32 o2_1; 		// r10{4}

    g0 = param1 - 1;
    if (param1 <= 1) {
        o2 = param1;
    }
    else {
        o2_1 = fib(param1 - 1);
        g0 = fib(param1 - 2); /* Warning: also results in o2 */
        o2 += o2_1;
    }
    return g0; /* WARNING: Also returning: o2 := o2 */
}

