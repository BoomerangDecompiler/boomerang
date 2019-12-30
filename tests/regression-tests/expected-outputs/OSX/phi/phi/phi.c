int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x00001cf0 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int local0; 		// m[g1 - 32]

    printf("Input number: ");
    scanf("%d", &local0);
    g3 = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, g3);
    return 0;
}

/** address: 0x00001c34 */
__size32 fib(int param1)
{
    int g3_2; 		// r3{5}
    int g3_5; 		// r3{4}
    int local0; 		// m[g1 - 28]

    if (param1 <= 1) {
        if (param1 != 1) {
            local0 = param1;
        }
        else {
            local0 = 1;
        }
    }
    else {
        g3_5 = fib(param1 - 1);
        g3_2 = fib(g3_5 - 1);
        printf("%d", g3_5 + g3_2);
        local0 = g3_5;
    }
    return local0;
}

