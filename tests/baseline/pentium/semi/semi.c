// address: 0x8048328
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    __size32 local0; 		// m[esp - 28]
    unsigned int local1; 		// m[esp - 32]

    eax = 0;
    if (argc <= 2) {
        do {
            if (argc == 11) {
                goto L1;
            }
        } while (argc <= 11);
L1:
    } else {
        do {
            if (argc <= 2) {
                if (argc <= 3) {
L8:
                    printf("9");
L6:
                    local0 = 0x804849c;
                    printf("5");
                } else {
                    if (argc <= 4) {
                        goto L4;
                    } else {
                        goto L6;
                    }
                }
            } else {
                goto L8;
            }
L4:
        } while (argc <= 5);
    }
    return 7;
}

