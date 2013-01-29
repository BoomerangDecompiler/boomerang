void b(unsigned int param1);
void c(unsigned int param1);
void d(int param1);
void f(int param1);
void h(int param1);
void j(int param1);
void l(int param1);
void e();
void g(int param1);
void i();
void k(int param1);

// address: 804837c
int main(int argc, char *argv[], char *envp[]) {
    proc1();
    b(argc * 3);
    return 0;
}

// address: 80483c7
void b(unsigned int param1) {
    proc1();
    c(param1 - 1);
    return;
}

// address: 80483f2
void c(unsigned int param1) {
    proc1();
    if (param1 <= 6) {
        switch(param1) {
        case 0:
        case 1:
        case 2:
            d(2);
            break;
        case 3:
            f(3);
            break;
        case 4:
            h(4);
            break;
        case 5:
            j(5);
            break;
        case 6:
            l(6);
            break;
        }
    }
    return;
}

// address: 804846a
void d(int param1) {
    proc1();
    if (param1 > 1) {
        e();
    }
    return;
}

// address: 80484c7
void f(int param1) {
    int local0; 		// m[esp - 28]

    proc1();
    if (param1 > 1) {
        local0 = param1 - 1;
        g(local0);
    }
    return;
}

// address: 8048529
void h(int param1) {
    proc1();
    if (param1 > 0) {
        i();
    }
    return;
}

// address: 8048575
void j(int param1) {
    proc1();
    if (param1 > 1) {
        k(param1);
    }
    return;
}

// address: 80485d5
void l(int param1) {
    proc1();
    if (param1 > 1) {
        proc5();
    }
    return;
}

// address: 804849b
void e() {
    proc1();
    proc2();
    return;
}

// address: 80484f8
void g(int param1) {
    proc1();
    if (param1 > 1) {
        proc3();
    }
    return;
}

// address: 804855a
void i() {
    proc1();
    return;
}

// address: 80485a4
void k(int param1) {
    proc1();
    if (param1 > 1) {
        proc4();
    }
    return;
}

