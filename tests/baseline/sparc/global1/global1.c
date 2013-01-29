int b = 7;
int a = 5;

void foo1();
void foo2();

// address: 10750
int main(int argc, char *argv[], char *envp[]) {
    foo1();
    printf("b = %i\n", b);
    return 0;
}

// address: 10738
void foo1() {
    foo2();
    return;
}

// address: 106fc
void foo2() {
    b = 12;
    printf("a = %i\n", a);
    return;
}

