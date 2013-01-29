int fib(int param1);

// address: 80487cc
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{39}
    int local0; 		// m[esp - 8]
    int local4; 		// m[esp - 20]
    int local5; 		// m[esp - 24]

    printf("Input number: ");
    scanf("%d", &local0);
    if (local0 <= 1) {
        eax = local0;
    } else {
        local4 = local0 - 1;
        eax_1 = fib(local4);
        local5 = local0 - 2;
        eax = fib(local5);
        eax += eax_1;
    }
    printf("fibonacci(%d) = %d\n", local0, eax);
    return 0;
}

// address: 8048798
int fib(int param1) {
    int eax; 		// r24
    __size32 eax_1; 		// r24{18}

    if (param1 <= 1) {
        eax = param1;
    } else {
        eax_1 = fib(param1 - 1);
        eax = fib(param1 - 2);
        eax += eax_1;
    }
    return eax;
}

