int fib(int param1);

// address: 0x100004e0
int main(int argc, char *argv[], char *envp[]) {
    int g3; 		// r3
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    g3 = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, g3);
    return 0;
}

// address: 0x10000440
int fib(int param1) {
    int local0; 		// m[g1 - 20]
    int local1; 		// m[g1 - 36]
    int local2; 		// m[g1 - 32]
    int local3; 		// m[g1 - 28]
    int local4; 		// m[g1 - 32]{54}

    if (param1 > 1) {
        local1 = 2;
        local2 = 1;
        local3 = 1;
        local4 = local2;
        while (local1 < param1) {
            local2 = local4 + local3;
            local3 = local4;
            local1++;
            local4 = local2;
        }
        local0 = local4;
    } else {
        local0 = param1;
    }
    return local0;
}

