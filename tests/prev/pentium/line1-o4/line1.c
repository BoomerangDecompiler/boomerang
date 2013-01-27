// address: 0x8048450
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    union { __size8 * x1; int x2; } eax_1; 		// r24{20}

    if (argc <= 1) {
        eax = 1;
    } else {
        proc1();
        eax = 1;
        if (eax_1 != 0) {
            proc2();
            if (eax != 0) {
                proc3();
                if (eax != 0) {
                    *(__size8*)eax = 0;
                }
                proc4();
            }
            proc5();
        }
    }
    return eax;
}

