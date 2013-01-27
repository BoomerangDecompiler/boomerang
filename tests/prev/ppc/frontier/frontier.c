// address: 0x100003f0
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[g1 - 40]

    if (argc == 5) {
        local0 = argc - 1;
        if (argc <= 1) {
            if (argc - 1 != 12) {
L2:
                if (local0 > 0) {
                }
            } else {
L3:
            }
        } else {
            local0 = argc - 2;
            if (argc - 1 <= 2) {
                goto L2;
            } else {
L5:
            }
        }
    } else {
        if (argc > 5) {
            if (argc == 9) {
                if (argc != 10) {
                    goto L3;
                } else {
                    goto L3;
                }
                goto L3;
            }
        } else {
            if (argc == 2) {
                if (argc > 0) {
                }
                goto L5;
            }
        }
    }
    return 13;
}

