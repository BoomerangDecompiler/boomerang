__size32 proc1(__size32 param1);
void proc2(__size32 param1);
void proc3();
void proc4();

// address: 0x401150
int main(int argc, char *argv[], char *envp[]) {
    if (argc > 7) {
L2:
        proc1(0x40a152);
        break;
    }
    switch(argc) {
    case 0:
        goto L2;
    case 1:
        goto L2;
    case 2:
        proc1(0x40a128);
        break;
    case 3:
        proc1(0x40a12e);
        break;
    case 4:
        proc1(0x40a136);
        break;
    case 5:
        proc1(0x40a13d);
        break;
    case 6:
        proc1(0x40a144);
        break;
    case 7:
        proc1(0x40a14a);
        break;
    }
    return 0;
}

// address: 0x4038e4
__size32 proc1(__size32 param1) {
    proc2(0);
    return param1;
}

// address: 0x403ad0
void proc2(__size32 param1) {
    if (param1 != 0) {
L26:
        if (flags) {
L0:
            *(__size32*)(esp - 4) = ecx;
            proc4();
        }
        if ( !flags) {
            if ( !flags) {
                goto L3;
            }
            *(__size32*)(ebp - 20) = edx;
            *(__size32*)(ebp - 16) = edx;
            *(__size8*)(ebp - 9) = 0;
            *(__size32*)(ebp - 8) = edx;
            *(__size32*)(ebp - 4) = edx;
            *(__size32*)(ebp - 28) = ecx;
            do {
                if (flags || flags) {
                    goto L5;
                }
            } while (flags);
            switch(ecx) {
            case 0:
            case 1:
                if ( !flags) {
                }
L5:
                *(__size32*)(ebp - 20)++;
                if ( !flags) {
                    goto L0;
                }
                *(__size32*)(esp - 4) = edx;
                *(__size32*)(esp - 4) = eax;
                proc3();
            case 2:
                *(__size32*)(ebp + 28) += 4;
                *(__size32*)(ebp - 48) = ecx;
                if ( !flags) {
                    if ( !flags) {
                        *(__size32*)(ebp - 4) = eax;
L13:
                    }
                    *(__size32*)(ebp - 4) = edx;
                    goto L13;
                }
                if ( !flags) {
                    *(__size32*)(ebp - 8) = edx;
                }
                goto L5;
            }
            if (flags) {
                goto L5;
            }
            *(__size8*)(ebp - 9) = bl;
        }
L3:
        if ( !(flags || flags)) {
            *(__size32*)(esp - 4) = edx;
            *(__size32*)(esp - 4) = ebx;
            proc3();
        }
        *(__size32*)(esp - 4) = eax;
        *(__size32*)(esp - 4) = ebx;
        proc3();
    }
    goto L26;
}

// address: 0x100403a68
void proc3() {
}

// address: 0x100403a20
void proc4() {
}

