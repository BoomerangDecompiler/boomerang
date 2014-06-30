int fib(int param1, __size32 param2, __size32 param3);

// address: 0x804835c
int main(int argc, char *argv[], char *envp[]) {
    int eax; 		// r24
    int eax_1; 		// r24{39}
    int eax_2; 		// r24{41}
    int eax_3; 		// r24{72}
    int eax_4; 		// r24{106}
    int ebx; 		// r27
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    int local5; 		// eax_4{106}

    printf("Input number: ");
    scanf("%d", &eax);
    ebx = eax;
    if (eax > 1) {
        eax_3 = fib(eax - 1, ecx, eax - 1); /* Warning: also results in edx */
        ebx = eax_3;
        eax = fib(eax_3 - 1, eax_3 - 1, edx);
        printf("%d", eax + eax_3);
L2:
        eax_2 = ebx;
        local5 = eax_2;
    } else {
        eax_1 = 1;
        local5 = eax_1;
        if (eax != 1) {
            goto L2;
        }
    }
    eax_4 = local5;
    printf("fibonacci(%d) = %d\n", eax, eax_4);
    return 0;
}

// address: 0x80483e0
int fib(int param1, __size32 param2, __size32 param3) {
    int eax; 		// r24
    int eax_1; 		// r24{33}
    int ebx; 		// r27
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 local7; 		// param2{56}
    __size32 local8; 		// param3{57}

    ebx = param1;
    local7 = param2;
    local8 = param3;
    if (param1 > 1) {
        eax_1 = fib(param1 - 1, param1 - 1, param3); /* Warning: also results in ecx */
        ebx = eax_1;
        eax = fib(eax_1 - 1, ecx, eax_1 - 1);
        printf("%d", eax + eax_1);
        local7 = ecx;
        local8 = edx;
L1:
        param2 = local7;
        param3 = local8;
        eax = ebx;
    } else {
        eax = 1;
        if (param1 != 1) {
            goto L1;
        }
    }
    return eax; /* WARNING: Also returning: ecx := param2, edx := param3 */
}

