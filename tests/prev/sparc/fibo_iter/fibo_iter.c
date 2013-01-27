int fib();

// address: 0x10704
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[o6 - 20]
    int o0; 		// r8

    printf("Input number: ");
    scanf("%d", &local0);
    o0 = fib();
    printf("fibonacci(%d) = %d\n", local0, o0);
    return 0;
}

// address: 0x106c4
int fib() {
    int o0; 		// r8
    int o1; 		// r9
    int o1_1; 		// r9{28}
    int o2; 		// r10
    int o2_1; 		// r10{29}
    int o3; 		// r11

    if (o0 <= 1) {
    } else {
        o2 = 1;
        o3 = 1;
        if (o0 > 2) {
            o1 = o0 - 2;
            do {
                o1_1 = o1;
                o2_1 = o2;
                o2 = o2_1 + o3;
                o1 = o1_1 - 1;
                o3 = o2_1;
            } while (ADDFLAGS(o1_1, -1, o1_1 - 1));
        }
        o0 = o2;
    }
    return o0;
}

