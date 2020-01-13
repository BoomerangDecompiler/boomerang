int main(int argc, char *argv[]);
__size32 fib1(int param1);


/** address: 0x00001c4c */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    __size32 g3_2; 		// r3{7}
    int g5; 		// r5
    int local0; 		// m[g1 - 32]

    printf("Input number: ");
    scanf("%d", &local0);
    if (local0 <= 1) {
        g5 = local0;
    }
    else {
        g3_2 = fib1(local0 - 1);
        g3 = fib1(local0 - 2);
        g5 = g3_2 + g3;
    }
    printf("fibonacci(%d) = %d\n", local0, g5);
    return 0;
}

/** address: 0x00001cd8 */
__size32 fib1(int param1)
{
    int g3; 		// r3
    __size32 g3_1; 		// r3{4}
    int local5; 		// param1{6}

    local5 = param1;
    if (param1 > 1) {
        g3_1 = fib1(param1 - 1);
        g3 = fib1(param1 - 2);
        g3 = g3_1 + g3;
        local5 = g3;
    }
    param1 = local5;
    return param1;
}

