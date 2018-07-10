int main(int argc, char *argv[]);
__size32 fib1(int param1);

/** address: 0x080483a0 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    int local0; 		// m[esp - 8]

    printf("Input number: ");
    %edx = scanf("%d", &local0);
    eax = fib1(local0);
    printf("fibonacci(%d) = %d\n", local0, eax);
    return 0;
}

/** address: 0x0804835c */
__size32 fib1(int param1)
{
    int eax; 		// r24
    __size32 eax_1; 		// r24{0}
    __size32 edx; 		// r26

    if (param1 > 1) {
        eax_1 = fib1(param1 - 1);
        eax = fib1(param1 - 2); /* Warning: also results in edx */
        eax += eax_1;
    }
    else {
        eax = param1;
    }
    return eax; /* WARNING: Also returning: edx := edx */
}

