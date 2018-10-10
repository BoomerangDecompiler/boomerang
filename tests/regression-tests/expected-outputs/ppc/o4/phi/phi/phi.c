int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x100004bc */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int g3_2; 		// r3{12}
    int g5; 		// r5
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    if (local0 > 1) {
        g3_2 = fib(local0 - 1);
        g3 = fib(g3_2 - 1);
        printf("%d", g3_2 + g3);
        printf("fibonacci(%d) = %d\n", local0, g3_2);
    }
    else {
        g5 = 1;
        if (local0 != 1) {
            g5 = local0;
        }
        printf("fibonacci(%d) = %d\n", local0, g5);
    }
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1)
{
    __size32 CR1; 		// r65
    __size1 CR1_1; 		// r65
    int g3; 		// r3
    int g3_2; 		// r3{11}
    int g3_4; 		// r3{25}
    int local3; 		// g3{14}
    __size32 local4; 		// CR1{23}

    local4 = CR1;
    if (param1 > 1) {
        g3_4 = fib(param1 - 1, CR1); /* Warning: also results in CR1 */
        g3 = fib(g3_4 - 1, CR1); /* Warning: also results in CR1 */
        %CR1 = %CR1 & ~0x4;
        printf("%d", g3_4 + g3);
        local4 = CR1_1;
        g3 = g3_4;
    }
    else {
        g3_2 = 1;
        local3 = g3_2;
        if (param1 != 1) {
            g3 = param1;
            local3 = g3;
        }
        g3 = local3;
    }
    CR1 = local4;
    return g3; /* WARNING: Also returning: CR1 := CR1 */
}

