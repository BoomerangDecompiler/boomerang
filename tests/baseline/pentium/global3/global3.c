int b = 7;
long long a = 0x7048860ddf79LL;

void foo1();
void foo2();

// address: 0x8048363
int main(int argc, char *argv[], char *envp[]) {
    foo1();
    printf("b = %i\n", b);
    return 0;
}

// address: 0x8048356
void foo1() {
    foo2();
    return;
}

// address: 0x8048328
void foo2() {
    b = 12;
    printf("a = %lld\n", a);
    return;
}

