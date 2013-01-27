// address: 0x8048328
int main(int argc, char *argv[], char *envp[]) {
    if (argc > (unsigned int)0xee6b27ff) {
        proc1();
    }
    if (argc <= (unsigned int)0xefffffff) {
        proc1();
    }
    if (argc > 1) {
        proc1();
    }
    if (0 - argc < -2) {
        proc1();
    }
    return 0;
}

