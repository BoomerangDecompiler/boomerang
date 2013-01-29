int b = 7;

void foo1(double param1);
void foo2(double param1);

// address: 10000498
int main(int argc, char *argv[], char *envp[]) {
    foo1(argv);
    printf("b = %i\n", b);
    return 0;
}

// address: 10000468
void foo1(double param1) {
    foo2(param1);
    return;
}

// address: 10000418
void foo2(double param1) {
    b = 12;
    printf("a = %f\n", param1);
    return;
}

