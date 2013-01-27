// address: 0x80483e4
int main(int argc, char *argv[], char *envp[]) {
    float local0; 		// m[esp - 12]
    double st; 		// r32

    proc1();
    proc2();
    st = local0;
    if (0x40a00000 == st) {
        proc3();
    }
    st = local0;
    if (0x40a00000 != st) {
        proc3();
    }
    st = local0;
    if (5. > st) {
        proc3();
    }
    st = local0;
    if (st >= 5.) {
        proc3();
    }
    st = local0;
    if (5. >= st) {
        proc3();
    }
    st = local0;
    if (st > 5.) {
        proc3();
    }
    return 0;
}

