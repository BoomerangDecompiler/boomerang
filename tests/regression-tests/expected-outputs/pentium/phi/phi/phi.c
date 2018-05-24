int main(int argc, char *argv[]);
__size32 fib(int param1, __size32 param2, __size32 param3);

/** address: 0x0804835c */
int main(int argc, char *argv[])
{
    int eax_1; 		// r24{0}
    int eax_10; 		// r24{0}
    int eax_11; 		// r24{0}
    int eax_12; 		// r24{0}
    int eax_4; 		// r24{0}
    int eax_7; 		// r24{0}
    int ebx; 		// r27
    int ecx; 		// r25
    int edx; 		// r26
    int local5; 		// eax_11{0}

    printf("Input number: ");
    ecx = scanf("%d", &eax_12);
    ebx = eax_12;
    if (eax_12 > 1) {
        eax_1 = fib(eax_12 - 1, ecx, eax_12 - 1); /* Warning: also results in edx */
        ebx = eax_1;
        eax_4 = fib(eax_1 - 1, eax_1 - 1, edx);
        printf("%d", eax_4 + eax_1);
bb0x8048396:
        eax_10 = ebx;
        local5 = eax_10;
    }
    else {
        eax_7 = 1;
        local5 = eax_7;
        if (eax_12 != 1) {
            goto bb0x8048396;
        }
    }
    eax_11 = local5;
    printf("fibonacci(%d) = %d\n", eax_12, eax_11);
    return 0;
}

/** address: 0x080483e0 */
__size32 fib(int param1, __size32 param2, __size32 param3)
{
    int eax; 		// r24
    int eax_1; 		// r24{0}
    int ebx; 		// r27
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 local7; 		// param2{0}
    __size32 local8; 		// param3{0}

    ebx = param1;
    local7 = param2;
    local8 = param3;
    if (param1 > 1) {
        eax_1 = fib(param1 - 1, param1 - 1, param3); /* Warning: also results in ecx */
        ebx = eax_1;
        eax = fib(eax_1 - 1, ecx, eax_1 - 1);
        ecx = printf("%d", eax + eax_1); /* Warning: also results in edx */
        local7 = ecx;
        local8 = edx;
bb0x80483f7:
        param2 = local7;
        param3 = local8;
        eax = ebx;
    }
    else {
        eax = 1;
        if (param1 != 1) {
            goto bb0x80483f7;
        }
    }
    return eax; /* WARNING: Also returning: ecx := param2, edx := param3 */
}

