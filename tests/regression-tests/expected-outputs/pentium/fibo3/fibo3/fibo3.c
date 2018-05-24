int main(int argc, char *argv[]);
__size32 fib(int param1, int param2, int param3);

/** address: 0x080483f6 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26
    int local0; 		// m[esp - 12]

    printf("Input number: ");
    ecx = scanf("%d", &local0); /* Warning: also results in edx */
    eax = fib(local0, ecx, edx);
    printf("fibonacci(%d) = %d\n", local0, eax);
    return 0;
}

/** address: 0x080483b0 */
__size32 fib(int param1, int param2, int param3)
{
    int eax_1; 		// r24{0}
    int eax_4; 		// r24{0}
    int ecx; 		// r25
    int edx; 		// r26
    int local2; 		// m[esp - 12]
    int local8; 		// param2{0}
    int local9; 		// param3{0}

    local8 = param2;
    local9 = param3;
    if (param1 <= 1) {
        local2 = param1;
    }
    else {
        eax_1 = fib(param1 - 1, param2, param3); /* Warning: also results in ecx, edx */
        eax_4 = fib(param1 - 2, ecx, edx);
        edx = param1 - 2;
        ecx = eax_1 + eax_4;
        local2 = eax_1 + eax_4;
        local8 = ecx;
        local9 = edx;
    }
    param2 = local8;
    param3 = local9;
    return local2; /* WARNING: Also returning: ecx := param2, edx := param3 */
}

