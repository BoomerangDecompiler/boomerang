int b = 7;

void foo1(long long param1);
void foo2(long long param1);

// address: 0x100004a0
int main(int argc, char *argv[], char *envp[]) {
    foo1(argv);
    printf("b = %i\n", b);
    return 0;
}

// address: 0x10000470
void foo1(long long param1) {
    foo2(param1);
    return;
}

// address: 0x10000418
void foo2(long long param1) {
    b = 12;
    printf("a = %lld\n", param1);
    return;
}

