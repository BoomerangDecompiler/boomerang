int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x100004e0 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    g3 = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, g3);
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1)
{
    int local0; 		// m[g1 - 20]
    int local1; 		// m[g1 - 36]
    int local2; 		// m[g1 - 32]
    int local3; 		// m[g1 - 28]
    int local4; 		// m[g1 - 32]{10}
    int local5; 		// m[g1 - 32]{14}
    int local9; 		// local4{10}

    if (param1 > 1) {
        local1 = 2;
        local2 = 1;
        local9 = local2;
        local3 = 1;
        local4 = local9;
        while (local1 < param1) {
            local5 = local4 + local3;
            local9 = local5;
            local3 = local4;
            local1++;
            local4 = local9;
        }
        local0 = local4;
    }
    else {
        local0 = param1;
    }
    return local0;
}

