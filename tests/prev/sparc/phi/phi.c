int fib(int param1);

// address: 0x10748
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[o6 - 20]
    int o0; 		// r8

    printf("Input number: ");
    scanf("%d", &local0);
    o0 = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, o0);
    return 0;
}

// address: 0x106c4
int fib(int param1) {
    int local17; 		// m[o6 - 20]
    int o0; 		// r8
    int o0_1; 		// r8{36}

    if (param1 <= 1) {
        if (param1 != 1) {
            local17 = param1;
        } else {
            local17 = 1;
        }
    } else {
        o0_1 = fib(param1 - 1);
        o0 = fib(param1 - 2);
        local17 = o0_1 + o0;
    }
    return local17;
}

