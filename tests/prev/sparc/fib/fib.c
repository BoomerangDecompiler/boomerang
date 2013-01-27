int fib(int param1);

// address: 0x1069c
int main(int argc, char *argv[], char *envp[]) {
    int o0; 		// r8

    o0 = fib(10);
    printf("%i\n", o0);
    return 0;
}

// address: 0x106c0
int fib(int param1) {
    int i0; 		// r24
    int o0; 		// r8
    int o0_1; 		// r8{33}

    i0 = param1;
    if (param1 > 1) {
        o0_1 = fib(param1 - 1);
        o0 = fib(param1 - 2);
        i0 = o0_1 + o0;
    }
    return i0;
}

