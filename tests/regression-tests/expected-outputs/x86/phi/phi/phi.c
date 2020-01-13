int main(int argc, char *argv[]);
__size32 fib(int param1, __size32 param2);


/** address: 0x0804835c */
int main(int argc, char *argv[])
{
    int eax_1; 		// r24{8}
    int eax_10; 		// r24{10}
    int eax_11; 		// r24{3}
    int eax_4; 		// r24{2}
    int eax_5; 		// r24{13}
    int eax_8; 		// r24{15}
    int ebx; 		// r27
    int edx; 		// r26
    int local5; 		// eax_10{10}

    printf("Input number: ");
    scanf("%d", &eax_11);
    ebx = eax_11;
    if (eax_11 > 1) {
        eax_5 = fib(eax_11 - 1, eax_11 - 1); /* Warning: also results in edx */
        ebx = eax_5;
        eax_8 = fib(eax_5 - 1, edx);
        printf("%d", eax_8 + eax_5);
bb0x8048396:
        eax_4 = ebx;
        local5 = eax_4;
    }
    else {
        eax_1 = 1;
        local5 = eax_1;
        if (eax_11 != 1) {
            goto bb0x8048396;
        }
    }
    eax_10 = local5;
    printf("fibonacci(%d) = %d\n", eax_11, eax_10);
    return 0;
}

/** address: 0x080483e0 */
__size32 fib(int param1, __size32 param2)
{
    int eax; 		// r24
    int eax_1; 		// r24{11}
    int eax_4; 		// r24{12}
    __size32 edx; 		// r26
    __size32 local8; 		// param2{1}

    local8 = param2;
    if (param1 > 1) {
        eax_1 = fib(param1 - 1, param2);
        eax_4 = fib(eax_1 - 1, eax_1 - 1);
        edx = printf("%d", eax_4 + eax_1);
        local8 = edx;
bb0x80483f7:
        param2 = local8;
        eax = param1;
    }
    else {
        eax = 1;
        if (param1 != 1) {
            goto bb0x80483f7;
        }
    }
    return eax; /* WARNING: Also returning: edx := param2 */
}

