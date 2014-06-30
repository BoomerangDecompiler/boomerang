int b = 7;
int a = 5;

void foo1();
void foo2();

// address: 0x804835d
int main(int argc, char *argv[], char *envp[]) {
    foo1();
    printf("b = %i\n", b);
    return 0;
}

// address: 0x8048350
void foo1() {
    foo2();
    return;
}

// address: 0x8048328
void foo2() {
    b = 12;
    printf("a = %i\n", a);
    return;
}

