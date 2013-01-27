// address: 0x8048948
int main(int argc, char *argv[], char *envp[]) {
    int eax; 		// r24
    int local0; 		// m[esp - 8]
    unsigned int local1; 		// m[esp - 12]

    proc1();
    proc1();
    if (local0 == 5) {
        proc2();
    }
    if (local0 == 5) {
L19:
        proc2();
        if (local0 > 5) {
L15:
            proc2();
        } else {
L17:
            proc2();
            if (local0 > 5) {
                goto L15;
            }
        }
    } else {
        proc2();
        if (local0 >= 5) {
            goto L19;
        } else {
            proc2();
            if (local0 < 5) {
                goto L17;
            } else {
                goto L19;
            }
        }
    }
    if (local1 >= 5) {
L10:
        proc2();
        if (local1 > 5) {
L6:
            proc2();
        } else {
L8:
            proc2();
            if (local1 > 5) {
                goto L6;
            }
        }
    } else {
        proc2();
        if (local1 < 5) {
            goto L8;
        } else {
            goto L10;
        }
    }
    eax = 5 - local0;
    if (eax < 0) {
L1:
        proc2();
    } else {
        proc2();
        eax = 5 - local0;
        if (eax < 0) {
            goto L1;
        }
    }
    return 0;
}

