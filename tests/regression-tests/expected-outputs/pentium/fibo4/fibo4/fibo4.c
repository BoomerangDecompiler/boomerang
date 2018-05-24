int main(int argc, char *argv[]);
__size32 fib(__size32 param1, int param2);

/** address: 0x080483df */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    int ecx; 		// r25
    int local0; 		// m[esp - 12]

    printf("Input number: ");
    ecx = scanf("%d", &local0);
    eax = fib(ecx, local0);
    printf("fibonacci(%d) = %d\n", local0, eax);
    return 0;
}

/** address: 0x080483b0 */
__size32 fib(__size32 param1, int param2)
{
    __size32 eax_1; 		// r24{0}
    __size32 eax_4; 		// r24{0}
    __size32 ecx; 		// r25
    int local2; 		// m[esp - 12]
    __size32 local5; 		// param1{0}

    local5 = param1;
    if (param2 <= 1) {
        local2 = param2;
    }
    else {
        eax_1 = fib(param1, param2 - 1); /* Warning: also results in ecx */
        eax_4 = fib(ecx, param2 - 2);
        ecx = eax_1 + eax_4;
        local2 = eax_1 + eax_4;
        local5 = ecx;
    }
    param1 = local5;
    return local2; /* WARNING: Also returning: ecx := param1 */
}

