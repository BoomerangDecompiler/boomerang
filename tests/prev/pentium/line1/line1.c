// address: 0x80484a3
int main(int argc, char *argv[], char *envp[]) {
    int eax; 		// r24
    __size32 local0; 		// m[esp - 0x420]

    if (argc > 1) {
        proc1();
        if (eax != 0) {
            proc2();
            if (eax != 0) {
                proc3();
            }
            proc4();
        } else {
            local0 = 1;
        }
    } else {
        local0 = 1;
    }
    return local0;
}

