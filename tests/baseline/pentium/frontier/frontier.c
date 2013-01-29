// address: 80482f4
int main(int argc, int argv, int envp) {
    int local1; 		// m[esp + 4]{38}
    int local2; 		// local1{110}

    local2 = argc;
    if (argc == 5) {
        do {
            local1 = local2;
            local1 = local1 - 1;
            if (local1 > 1) {
                local1 = local1 - 1;
                if (local1 > 2) {
L4:
                    goto L0;
                }
                goto L1;
            }
            if (local1 == 12) {
L2:
                goto L0;
            }
L1:
            local2 = local1;
        } while (local1 > 0);
    } else {
        if (argc > 5) {
            if (argc == 9) {
                if (argc != 10) {
                    goto L2;
                } else {
                    goto L2;
                }
                goto L2;
            }
        } else {
            if (argc == 2) {
                do {
                } while (argc > 0);
                goto L4;
            }
        }
    }
L0:
    return 13;
}

