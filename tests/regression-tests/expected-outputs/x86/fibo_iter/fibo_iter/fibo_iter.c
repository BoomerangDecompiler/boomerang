int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x0804838c */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    int local0; 		// m[esp - 8]

    printf("Input number: ");
    scanf("%d", &local0);
    eax = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, eax);
    return 0;
}

/** address: 0x0804835c */
__size32 fib(int param1)
{
    int eax; 		// r24
    int ebx; 		// r27
    int ecx; 		// r25
    int ecx_1; 		// r25{9}
    int edx; 		// r26
    int edx_1; 		// r26{10}

    eax = param1;
    if (param1 > 1) {
        ecx = 1;
        ebx = 1;
        if (param1 > 2) {
            edx = param1 - 2;
            do {
                edx_1 = edx;
                ecx_1 = ecx;
                ecx = ecx_1 + ebx;
                edx = edx_1 - 1;
                ebx = ecx_1;
            } while (edx_1 != 1);
        }
        eax = ecx;
    }
    return eax;
}

