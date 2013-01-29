__size32 fib1();
int fib2(int param1);

// address: 10000504
int main(int argc, char *argv[], char *envp[]) {
    int g3; 		// r3
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    g3 = fib1();
    printf("fibonacci(%d) = %d\n", local0, g3);
    return 0;
}

// address: 10000440
__size32 fib1() {
    int g3; 		// r3

    g3 = fib2(g3);
    return g3;
}

// address: 10000480
int fib2(int param1) {
    int g29; 		// r29
    int g3; 		// r3
    int local5; 		// m[g1 - 20]

    if (param1 <= 1) {
        local5 = param1;
    } else {
        fib1();
        g3 = fib1();
        local5 = g29 + g3;
    }
    return local5;
}

