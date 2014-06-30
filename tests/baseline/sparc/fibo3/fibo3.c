int fib(int param1, int param2);

// address: 0x1071c
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[o6 - 20]
    int o0; 		// r8
    int o4; 		// r12

    printf("Input number: ");
    scanf("%d", &local0);
    o0 = fib(local0, o4);
    printf("fibonacci(%d) = %d\n", local0, o0);
    return 0;
}

// address: 0x106ac
int fib(int param1, int param2) {
    int g1; 		// r1
    int i4; 		// r28
    int local19; 		// m[o6 - 20]
    int o0; 		// r8
    int o0_1; 		// r8{38}
    int o4; 		// r12

    i4 = param2;
    if (param1 <= 1) {
        g1 = param1;
        local19 = param1;
    } else {
        o0_1 = fib(param1 - 1, param2); /* Warning: also results in o4 */
        o0 = fib(param1 - 2, o4); /* Warning: also results in g1 */
        i4 = o0_1;
        local19 = o0_1 + o0;
    }
    return local19; /* WARNING: Also returning: g1 := g1, o4 := i4 */
}

