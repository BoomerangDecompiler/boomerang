int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x100004bc */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int g31; 		// r31
    int g5; 		// r5
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    if (local0 > 1) {
        g3 = fib(local0 - 1);
        fib(g3 - 1);
        printf(g31 + 0x92c);
        printf("fibonacci(%d) = %d\n", local0, g3);
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
    int g3; 		// r3
    int g3_2; 		// r3{7}
    int g3_4; 		// r3{4}
    int local3; 		// g3{9}

    if (param1 > 1) {
        g3_4 = fib(param1 - 1);
        g3 = fib(g3_4 - 1);
        printf("%d", g3_4 + g3);
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
    return g3;
}

