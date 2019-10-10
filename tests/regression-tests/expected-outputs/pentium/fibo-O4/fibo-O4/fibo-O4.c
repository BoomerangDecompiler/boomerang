int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x080487cc */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    union { int; void *; } eax_1; 		// r24{6}
    union { int; void *; } local0; 		// m[esp - 8]

    printf("Input number: ");
    scanf("%d", &local0);
    if (local0 <= 1) {
        eax = local0;
    }
    else {
        eax_1 = fib(local0 - 1);
        eax = fib(local0 - 2);
        eax += eax_1;
    }
    printf("fibonacci(%d) = %d\n", local0, eax);
    return 0;
}

/** address: 0x08048798 */
__size32 fib(int param1)
{
    int eax; 		// r24
    __size32 eax_1; 		// r24{5}

    if (param1 <= 1) {
        eax = param1;
    }
    else {
        eax = fib(param1 - 1);
        eax_1 = fib(param1 - 2);
        eax = eax_1 + eax;
    }
    return eax;
}

