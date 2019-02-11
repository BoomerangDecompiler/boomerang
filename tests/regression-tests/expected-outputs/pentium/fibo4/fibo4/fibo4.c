int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x080483df */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    int local0; 		// m[esp - 12]

    printf("Input number: ");
    scanf("%d", &local0);
    eax = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, eax);
    return 0;
}

/** address: 0x080483b0 */
__size32 fib(int param1)
{
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{4}
    int local4; 		// m[esp - 12]

    if (param1 <= 1) {
        local4 = param1;
    }
    else {
        eax_1 = fib(param1 - 1);
        eax = fib(param1 - 2);
        local4 = eax_1 + eax;
    }
    return local4;
}

