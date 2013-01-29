double a = 5.2;
int b = 7;

void foo1();
void foo2();

// address: 8048363
int main(int argc, char *argv[], char *envp[]) {
    foo1();
    printf("b = %i\n", b);
    return 0;
}

// address: 8048356
void foo1() {
    foo2();
    return;
}

// address: 8048328
void foo2() {
    b = 12;
    printf("a = %f\n", a);
    return;
}

