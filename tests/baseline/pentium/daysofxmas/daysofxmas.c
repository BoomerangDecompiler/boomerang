// address: 8048350
int main(int argc, char *??[], char *envp[]) {
    union { unsigned int x216; char x217; char x213; char x209; char x205; char x201; char x197; char x193; char x189; char x185; char x181; char x177; char x173; char x169; char x165; char x161; char x157; char x153; char x149; char x145; char x141; char x128; char x126; char x122; char x120; char x116; char x114; char x110; char x108; char x104; char x102; char x98; char x96; char x92; char x90; char x86; char x84; char x80; char x78; char x74; char x72; char x68; char x66; char x62; char x60; char x56; char x54; char x50; char x48; char x44; char x42; char x38; char x36; char x32; char x30; char x26; char x24; char x20; char x18; char x10; char x8; } dl; 		// r10
    int eax; 		// r24
    char * *eax_1; 		// r24
    int ebx; 		// r27
    int local14; 		// m[esp + 8]
    int local6; 		// m[esp - 36]
    int local7; 		// m[esp - 40]
    int local8; 		// m[esp - 44]

    if (argc <= 1) {
        if (argc < 0) {
            if (argc < -72) {
L2:
                eax = main(local8, local7, local6);
            } else {
                if (argc >= -50) {
                    if (*envp == 47) {
                    }
                    goto L2;
                } else {
                    eax = (int) *envp;
                    if (local14 == eax) {
                        eax = (int) *(envp + 31);
                        putchar(eax);
                    } else {
                        goto L2;
                    }
                }
            }
            ebx = eax;
        } else {
            if (argc <= 0) {
                dl = *envp;
                ebx = 0;
                if (dl == 47) {
L12:
                    ebx = 1;
                } else {
                    eax_1 = main(-61, (int) dl, 0x8048540);
                    eax = main(0, eax_1, envp + 1);
                    if (eax != 0) {
                        goto L12;
                    }
                }
            } else {
L17:
                goto L2;
            }
        }
    } else {
        if (argc <= 2) {
            eax = main(-86, 0, envp + 1);
            eax = main(-87, 1 - local14, eax + envp);
            main(-79, -13, eax + envp);
        }
        if (argc < local14) {
            main(argc + 1, local14, envp);
        }
        eax = main(-94, argc - 27, envp);
        if (eax != 0 && argc == 2) {
            if (local14 <= 12) {
                goto L17;
            } else {
                ebx = 9;
            }
        } else {
            ebx = 16;
        }
    }
    return ebx;
}

