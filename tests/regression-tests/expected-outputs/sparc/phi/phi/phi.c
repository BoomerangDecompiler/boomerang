int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x00010748 */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 20]
    int o0; 		// r8

    printf("Input number: ");
    scanf("%d", &local0);
    o0 = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, o0);
    return 0;
}

/** address: 0x000106c4 */
__size32 fib(int param1)
{
    int local1; 		// m[o6 - 20]
    int o0; 		// r8
    int o0_1; 		// r8{4}

    if (param1 <= 1) {
        if (param1 != 1) {
            local1 = param1;
        }
        else {
            local1 = 1;
        }
    }
    else {
        o0_1 = fib(param1 - 1);
        o0 = fib(param1 - 2);
        local1 = o0_1 + o0;
    }
    return local1;
}

