union { __size32[] x9; void () x10; void (int) x13; void (int) x26; } global18;

__size32 b(int param1);
unsigned int c(unsigned int param1);
int d(int param1);
int f(int param1);
int h(int param1);
int j(int param1);
int l(int param1);
__size32 e(int param1);
int g(int param1);
__size32 i(int param1);
void k(int param1);

// address: 107ac
int main(int argc, char *argv[], char *envp[]) {
    printf("a(%d)\n", argc);
    b(argc + argc + argc);
    return 0;
}

// address: 107f4
__size32 b(int param1) {
    __size32 g1; 		// r1

    printf("b(%d)\n", param1);
    g1 = c(param1 - 1);
    return g1;
}

// address: 10830
unsigned int c(unsigned int param1) {
    unsigned int g1; 		// r1

    printf("c(%d)\n", param1);
    g1 = param1;
    if (param1 <= 6) {
        g1 = global18[param1];
        switch(param1) {
        case 0:
        case 1:
        case 2:
            g1 = d(2);
            break;
        case 3:
            g1 = f(3);
            break;
        case 4:
            g1 = h(4);
            break;
        case 5:
            g1 = j(5);
            break;
        case 6:
            g1 = l(6);
            break;
        }
    }
    return g1;
}

// address: 108e0
int d(int param1) {
    int g1; 		// r1

    printf("d(%d)\n", param1);
    g1 = param1;
    if (param1 > 1) {
        g1 = e(param1 - 1);
    }
    return g1;
}

// address: 10968
int f(int param1) {
    int g1; 		// r1

    printf("f(%d)\n", param1);
    g1 = param1;
    if (param1 > 1) {
        g1 = g(param1 - 1);
    }
    return g1;
}

// address: 10a00
int h(int param1) {
    int g1; 		// r1

    printf("h(%d)\n", param1);
    g1 = param1;
    if (param1 > 0) {
        g1 = i(param1 - 1);
    }
    return g1;
}

// address: 10a74
int j(int param1) {
    printf("j(%d)\n", param1);
    if (param1 > 1) {
        k(param1);
    }
    return param1;
}

// address: 10b04
int l(int param1) {
    int g1; 		// r1

    printf("l(%d)\n", param1);
    g1 = param1;
    if (param1 > 1) {
        g1 = b(param1 + 2);
    }
    return g1;
}

// address: 1092c
__size32 e(int param1) {
    __size32 g1; 		// r1

    printf("e(%d)\n", param1);
    g1 = c(param1 >> 1);
    return g1;
}

// address: 109b4
int g(int param1) {
    int g1; 		// r1

    printf("g(%d)\n", param1);
    g1 = param1;
    if (param1 > 1) {
        g1 = f(param1 - 1);
    }
    return g1;
}

// address: 10a4c
__size32 i(int param1) {
    printf("i(%d)\n", param1);
    return 0x10c00;
}

// address: 10ab8
void k(int param1) {
    printf("k(%d)\n", param1);
    if (param1 > 1) {
        e(param1 - 1);
    }
    return;
}

