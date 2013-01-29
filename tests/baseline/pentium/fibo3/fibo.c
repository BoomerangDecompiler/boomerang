// address: 80483f6
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    int local0; 		// m[esp - 12]

    printf("Input number: ");
    scanf("%d", &local0);
    eax = fib(local0);
    printf("fibonacci(%d) = %d\n", local0, eax);
    return 0;
}

// address: 80483b0
int fib(int param1) {
    int eax; 		// r24
    int eax_1; 		// r24{24}
    int local2; 		// m[esp - 12]

    if (param1 <= 1) {
        local2 = param1;
    } else {
        eax_1 = fib(param1 - 1);
        eax = fib(param1 - 2);
        local2 = eax_1 + eax;
    }
    return local2;
}

